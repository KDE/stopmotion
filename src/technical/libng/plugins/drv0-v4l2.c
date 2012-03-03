/*
 * interface to the v4l2 driver
 *
 *   (c) 1998-2002 Gerd Knorr <kraxel@bytesex.org>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>

#include <asm/types.h>		/* XXX glibc */
#include "../videodev2.h"

#include "../grab-ng.h"

#include "../struct-dump.h"
#include "../struct-v4l2.h"

/* ---------------------------------------------------------------------- */

/* open+close */
static void*   v4l2_open(char *device);
static int     v4l2_close(void *handle);

/* attributes */
static char*   v4l2_devname(void *handle);
static int     v4l2_flags(void *handle);
static struct ng_attribute* v4l2_attrs(void *handle);
static int     v4l2_read_attr(struct ng_attribute*);
static void    v4l2_write_attr(struct ng_attribute*, int val);

/* overlay */
static int   v4l2_setupfb(void *handle, struct ng_video_fmt *fmt, void *base);
static int   v4l2_overlay(void *handle, struct ng_video_fmt *fmt, int x, int y,
			  struct OVERLAY_CLIP *oc, int count, int aspect);

/* capture video */
static int v4l2_setformat(void *handle, struct ng_video_fmt *fmt);
static int v4l2_startvideo(void *handle, int fps, unsigned int buffers);
static void v4l2_stopvideo(void *handle);
static struct ng_video_buf* v4l2_nextframe(void *handle);
static struct ng_video_buf* v4l2_getimage(void *handle);

/* tuner */
static unsigned long v4l2_getfreq(void *handle);
static void v4l2_setfreq(void *handle, unsigned long freq);
static int v4l2_tuned(void *handle);

/* ---------------------------------------------------------------------- */

#define WANTED_BUFFERS 32

#define MAX_INPUT   16
#define MAX_NORM    16
#define MAX_FORMAT  32
#define MAX_CTRL    32

struct v4l2_handle {
    int                         fd;

    /* device descriptions */
    int                         ninputs,nstds,nfmts;
    struct v4l2_capability	cap;
    struct v4l2_streamparm	streamparm;
    struct v4l2_input		inp[MAX_INPUT];
    struct v4l2_standard      	std[MAX_NORM];
    struct v4l2_fmtdesc		fmt[MAX_FORMAT];
    struct v4l2_queryctrl	ctl[MAX_CTRL*2];

    /* attributes */
    int                         nattr;
    struct ng_attribute         *attr;

    /* capture */
    int                            fps,first;
    long long                      start;
    struct v4l2_format             fmt_v4l2;
    struct ng_video_fmt            fmt_me;
    struct v4l2_requestbuffers     reqbufs;
    struct v4l2_buffer             buf_v4l2[WANTED_BUFFERS];
    struct ng_video_buf            buf_me[WANTED_BUFFERS];
    unsigned int                   queue,waiton;

    /* overlay */
    struct v4l2_framebuffer        ov_fb;
    struct v4l2_format             ov_win;
    struct v4l2_clip               ov_clips[256];
#if 0
    enum v4l2_field                ov_fields;
#endif
    int                            ov_error;
    int                            ov_enabled;
    int                            ov_on;
};

/* ---------------------------------------------------------------------- */

struct ng_vid_driver v4l2_driver = {
    name:          "v4l2",
    open:          v4l2_open,
    close:         v4l2_close,

    get_devname:   v4l2_devname,
    capabilities:  v4l2_flags,
    list_attrs:    v4l2_attrs,

    setupfb:       v4l2_setupfb,
    overlay:       v4l2_overlay,

    setformat:     v4l2_setformat,
    startvideo:    v4l2_startvideo,
    stopvideo:     v4l2_stopvideo,
    nextframe:     v4l2_nextframe,
    getimage:      v4l2_getimage,
    
    getfreq:       v4l2_getfreq,
    setfreq:       v4l2_setfreq,
    is_tuned:      v4l2_tuned,
};

static __u32 xawtv_pixelformat[VIDEO_FMT_COUNT] = {
    [ VIDEO_RGB08 ]    = V4L2_PIX_FMT_HI240,
    [ VIDEO_GRAY ]     = V4L2_PIX_FMT_GREY,
    [ VIDEO_RGB15_LE ] = V4L2_PIX_FMT_RGB555,
    [ VIDEO_RGB16_LE ] = V4L2_PIX_FMT_RGB565,
    [ VIDEO_RGB15_BE ] = V4L2_PIX_FMT_RGB555X,
    [ VIDEO_RGB16_BE ] = V4L2_PIX_FMT_RGB565X,
    [ VIDEO_BGR24 ]    = V4L2_PIX_FMT_BGR24,
    [ VIDEO_BGR32 ]    = V4L2_PIX_FMT_BGR32,
    [ VIDEO_RGB24 ]    = V4L2_PIX_FMT_RGB24,
    [ VIDEO_YUYV ]     = V4L2_PIX_FMT_YUYV,
    [ VIDEO_UYVY ]     = V4L2_PIX_FMT_UYVY,
    [ VIDEO_YUV422P ]  = V4L2_PIX_FMT_YUV422P,
    [ VIDEO_YUV420P ]  = V4L2_PIX_FMT_YUV420,
};

static struct STRTAB stereo[] = {
    {  V4L2_TUNER_MODE_MONO,   "mono"    },
    {  V4L2_TUNER_MODE_STEREO, "stereo"  },
    {  V4L2_TUNER_MODE_LANG1,  "lang1"   },
    {  V4L2_TUNER_MODE_LANG2,  "lang2"   },
    { -1, NULL },
};

/* ---------------------------------------------------------------------- */
/* debug output                                                           */

#define PREFIX "ioctl: "

static int
xioctl(int fd, int cmd, void *arg, int mayfail)
{
    int rc;

    rc = ioctl(fd,cmd,arg);
    if (0 == rc && ng_debug < 2)
	return rc;
    if (mayfail && errno == mayfail && ng_debug < 2)
	return rc;
    print_ioctl(stderr,ioctls_v4l2,PREFIX,cmd,arg);
    fprintf(stderr,": %s\n",(rc == 0) ? "ok" : strerror(errno));
    return rc;
}

static void
print_bufinfo(struct v4l2_buffer *buf)
{
    static char *type[] = {
	[V4L2_BUF_TYPE_VIDEO_CAPTURE] = "video-cap",
	[V4L2_BUF_TYPE_VIDEO_OVERLAY] = "video-over",
	[V4L2_BUF_TYPE_VIDEO_OUTPUT]  = "video-out",
	[V4L2_BUF_TYPE_VBI_CAPTURE]   = "vbi-cap",
	[V4L2_BUF_TYPE_VBI_OUTPUT]    = "vbi-out",
    };

    fprintf(stderr,"v4l2: buf %d: %s 0x%x+%d, used %d\n",
	    buf->index,
	    buf->type < sizeof(type)/sizeof(char*)
	    ? type[buf->type] : "unknown",
	    buf->m.offset,buf->length,buf->bytesused);
}

/* ---------------------------------------------------------------------- */
/* helpers                                                                */

static void
get_device_capabilities(struct v4l2_handle *h)
{
    int i;
    
    for (h->ninputs = 0; h->ninputs < MAX_INPUT; h->ninputs++) {
	h->inp[h->ninputs].index = h->ninputs;
	if (-1 == xioctl(h->fd, VIDIOC_ENUMINPUT, &h->inp[h->ninputs], EINVAL))
	    break;
    }
    for (h->nstds = 0; h->nstds < MAX_NORM; h->nstds++) {
	h->std[h->nstds].index = h->nstds;
	if (-1 == xioctl(h->fd, VIDIOC_ENUMSTD, &h->std[h->nstds], EINVAL))
	    break;
    }
    for (h->nfmts = 0; h->nfmts < MAX_FORMAT; h->nfmts++) {
	h->fmt[h->nfmts].index = h->nfmts;
	h->fmt[h->nfmts].type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(h->fd, VIDIOC_ENUM_FMT, &h->fmt[h->nfmts], EINVAL))
	    break;
    }

    h->streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(h->fd,VIDIOC_G_PARM,&h->streamparm);

    /* controls */
    for (i = 0; i < MAX_CTRL; i++) {
	h->ctl[i].id = V4L2_CID_BASE+i;
	if (-1 == xioctl(h->fd, VIDIOC_QUERYCTRL, &h->ctl[i], EINVAL) ||
	    (h->ctl[i].flags & V4L2_CTRL_FLAG_DISABLED))
	    h->ctl[i].id = -1;
    }
    for (i = 0; i < MAX_CTRL; i++) {
	h->ctl[i+MAX_CTRL].id = V4L2_CID_PRIVATE_BASE+i;
	if (-1 == xioctl(h->fd, VIDIOC_QUERYCTRL, &h->ctl[i+MAX_CTRL], EINVAL) ||
	    (h->ctl[i+MAX_CTRL].flags & V4L2_CTRL_FLAG_DISABLED))
	    h->ctl[i+MAX_CTRL].id = -1;
    }
}

static struct STRTAB *
build_norms(struct v4l2_handle *h)
{
    struct STRTAB *norms;
    int i;

    norms = malloc(sizeof(struct STRTAB) * (h->nstds+1));
    for (i = 0; i < h->nstds; i++) {
	norms[i].nr  = i;
	norms[i].str = (const char*)h->std[i].name;
    }
    norms[i].nr  = -1;
    norms[i].str = NULL;
    return norms;
}

static struct STRTAB *
build_inputs(struct v4l2_handle *h)
{
    struct STRTAB *inputs;
    int i;

    inputs = malloc(sizeof(struct STRTAB) * (h->ninputs+1));
    for (i = 0; i < h->ninputs; i++) {
	inputs[i].nr  = i;
	inputs[i].str = (const char*)h->inp[i].name;
    }
    inputs[i].nr  = -1;
    inputs[i].str = NULL;
    return inputs;
}

/* ---------------------------------------------------------------------- */

static struct V4L2_ATTR {
    unsigned int id;
    unsigned int v4l2;
} v4l2_attr[] = {
    { ATTR_ID_VOLUME,   V4L2_CID_AUDIO_VOLUME },
    { ATTR_ID_MUTE,     V4L2_CID_AUDIO_MUTE   },
    { ATTR_ID_COLOR,    V4L2_CID_SATURATION   },
    { ATTR_ID_BRIGHT,   V4L2_CID_BRIGHTNESS   },
    { ATTR_ID_HUE,      V4L2_CID_HUE          },
    { ATTR_ID_CONTRAST, V4L2_CID_CONTRAST     },
};
#define NUM_ATTR (sizeof(v4l2_attr)/sizeof(struct V4L2_ATTR))

static struct STRTAB*
v4l2_menu(int fd, const struct v4l2_queryctrl *ctl)
{
    struct STRTAB *menu;
    struct v4l2_querymenu item;
    int i;

    menu = malloc(sizeof(struct STRTAB) * (ctl->maximum-ctl->minimum+2));
    for (i = ctl->minimum; i <= ctl->maximum; i++) {
	item.id = ctl->id;
	item.index = i;
	if (-1 == xioctl(fd, VIDIOC_QUERYMENU, &item, 0)) {
	    free(menu);
	    return NULL;
	}
	menu[i-ctl->minimum].nr  = i;
	menu[i-ctl->minimum].str = (const char*)strdup((const char*)item.name);
    }
    menu[i-ctl->minimum].nr  = -1;
    menu[i-ctl->minimum].str = NULL;
    return menu;
}

static void
v4l2_add_attr(struct v4l2_handle *h, struct v4l2_queryctrl *ctl,
	      int id, struct STRTAB *choices)
{
    static int private_ids = ATTR_ID_COUNT;
    unsigned int i;
    
    h->attr = realloc(h->attr,(h->nattr+2) * sizeof(struct ng_attribute));
    memset(h->attr+h->nattr,0,sizeof(struct ng_attribute)*2);
    if (ctl) {
	for (i = 0; i < NUM_ATTR; i++)
	    if (v4l2_attr[i].v4l2 == ctl->id)
		break;
	if (i != NUM_ATTR) {
	    h->attr[h->nattr].id  = v4l2_attr[i].id;
	} else {
	    h->attr[h->nattr].id  = private_ids++;
	}
	h->attr[h->nattr].name    = (const char*)ctl->name;
	h->attr[h->nattr].priv    = ctl;
	h->attr[h->nattr].defval  = ctl->default_value;
	switch (ctl->type) {
	case V4L2_CTRL_TYPE_INTEGER:
	    h->attr[h->nattr].type    = ATTR_TYPE_INTEGER;
	    h->attr[h->nattr].defval  = ctl->default_value;
	    h->attr[h->nattr].min     = ctl->minimum;
	    h->attr[h->nattr].max     = ctl->maximum;
	    break;
	case V4L2_CTRL_TYPE_BOOLEAN:
	    h->attr[h->nattr].type    = ATTR_TYPE_BOOL;
	    break;
	case V4L2_CTRL_TYPE_MENU:
	    h->attr[h->nattr].type    = ATTR_TYPE_CHOICE;
	    h->attr[h->nattr].choices = v4l2_menu(h->fd, ctl);
	    break;
	default:
	    return;
	}
    } else {
	/* for norms + inputs */
	h->attr[h->nattr].id      = id;
	if (-1 == h->attr[h->nattr].id)
	    h->attr[h->nattr].id  = private_ids++;
	h->attr[h->nattr].defval  = 0;
	h->attr[h->nattr].type    = ATTR_TYPE_CHOICE;
	h->attr[h->nattr].choices = choices;
    }
    if (h->attr[h->nattr].id < ATTR_ID_COUNT)
	h->attr[h->nattr].name = ng_attr_to_desc[h->attr[h->nattr].id];

    h->attr[h->nattr].read    = v4l2_read_attr;
    h->attr[h->nattr].write   = v4l2_write_attr;
    h->attr[h->nattr].handle  = h;
    h->nattr++;
}

static int v4l2_read_attr(struct ng_attribute *attr)
{
    struct v4l2_handle *h = attr->handle;
    const struct v4l2_queryctrl *ctl = attr->priv;
    struct v4l2_control c;
    struct v4l2_tuner tuner;
    v4l2_std_id std;
    int value = 0;
    int i;

    if (NULL != ctl) {
	c.id = ctl->id;
	xioctl(h->fd,VIDIOC_G_CTRL,&c,0);
	value = c.value;
	
    } else if (attr->id == ATTR_ID_NORM) {
	value = -1;
	xioctl(h->fd,VIDIOC_G_STD,&std,0);
	for (i = 0; i < h->nstds; i++)
	    if (std & h->std[i].id)
		value = i;
	
    } else if (attr->id == ATTR_ID_INPUT) {
	xioctl(h->fd,VIDIOC_G_INPUT,&value,0);

    } else if (attr->id == ATTR_ID_AUDIO_MODE) {
	memset(&tuner,0,sizeof(tuner));
	xioctl(h->fd,VIDIOC_G_TUNER,&tuner,0);
	value = tuner.audmode;
#if 1
	if (ng_debug) {
	    fprintf(stderr,"v4l2:   tuner cap:%s%s%s\n",
		    (tuner.capability&V4L2_TUNER_CAP_STEREO) ? " STEREO" : "",
		    (tuner.capability&V4L2_TUNER_CAP_LANG1)  ? " LANG1"  : "",
		    (tuner.capability&V4L2_TUNER_CAP_LANG2)  ? " LANG2"  : "");
	    fprintf(stderr,"v4l2:   tuner rxs:%s%s%s%s\n",
		    (tuner.rxsubchans&V4L2_TUNER_SUB_MONO)   ? " MONO"   : "",
		    (tuner.rxsubchans&V4L2_TUNER_SUB_STEREO) ? " STEREO" : "",
		    (tuner.rxsubchans&V4L2_TUNER_SUB_LANG1)  ? " LANG1"  : "",
		    (tuner.rxsubchans&V4L2_TUNER_SUB_LANG2)  ? " LANG2"  : "");
	    fprintf(stderr,"v4l2:   tuner cur:%s%s%s%s\n",
		    (tuner.audmode==V4L2_TUNER_MODE_MONO)   ? " MONO"   : "",
		    (tuner.audmode==V4L2_TUNER_MODE_STEREO) ? " STEREO" : "",
		    (tuner.audmode==V4L2_TUNER_MODE_LANG1)  ? " LANG1"  : "",
		    (tuner.audmode==V4L2_TUNER_MODE_LANG2)  ? " LANG2"  : "");
	}
#endif
    }
    return value;
}

static void v4l2_write_attr(struct ng_attribute *attr, int value)
{
    struct v4l2_handle *h = attr->handle;
    const struct v4l2_queryctrl *ctl = attr->priv;
    struct v4l2_control c;
    struct v4l2_tuner tuner;

    if (NULL != ctl) {
	c.id = ctl->id;
	c.value = value;
	xioctl(h->fd,VIDIOC_S_CTRL,&c,0);
	
    } else if (attr->id == ATTR_ID_NORM) {
	xioctl(h->fd,VIDIOC_S_STD,&h->std[value].id,0);
	
    } else if (attr->id == ATTR_ID_INPUT) {
	xioctl(h->fd,VIDIOC_S_INPUT,&value,0);

    } else if (attr->id == ATTR_ID_AUDIO_MODE) {
	memset(&tuner,0,sizeof(tuner));
	xioctl(h->fd,VIDIOC_G_TUNER,&tuner,0);
	tuner.audmode = value;
	xioctl(h->fd,VIDIOC_S_TUNER,&tuner,0);
    }
}

/* ---------------------------------------------------------------------- */

static void*
v4l2_open(char *device)
{
    struct v4l2_handle *h;
    int i;

    h = malloc(sizeof(*h));
    if (NULL == h)
	return NULL;
    memset(h,0,sizeof(*h));
    
    if (-1 == (h->fd = open(device, O_RDWR))) {
	fprintf(stderr,"v4l2: open %s: %s\n",device,strerror(errno));
	goto err;
    }

    if (-1 == xioctl(h->fd,VIDIOC_QUERYCAP,&h->cap,EINVAL))
	goto err;
    if (ng_debug)
	fprintf(stderr, "v4l2: open\n");
    fcntl(h->fd,F_SETFD,FD_CLOEXEC);
    if (ng_debug)
	fprintf(stderr,"v4l2: device info:\n"
		"  %s %d.%d.%d / %s @ %s\n",
		h->cap.driver,
		(h->cap.version >> 16) & 0xff,
		(h->cap.version >>  8) & 0xff,
		h->cap.version         & 0xff,
		h->cap.card,h->cap.bus_info);
    get_device_capabilities(h);

    /* attributes */
    v4l2_add_attr(h, NULL, ATTR_ID_NORM,  build_norms(h));
    v4l2_add_attr(h, NULL, ATTR_ID_INPUT, build_inputs(h));
    if (h->cap.capabilities & V4L2_CAP_TUNER)
	v4l2_add_attr(h, NULL, ATTR_ID_AUDIO_MODE, stereo);
    for (i = 0; i < MAX_CTRL*2; i++) {
	if (h->ctl[i].id == UNSET)
	    continue;
	v4l2_add_attr(h, &h->ctl[i], 0, NULL);
    }

    /* capture buffers */
    for (i = 0; i < WANTED_BUFFERS; i++) {
    	ng_init_video_buf(h->buf_me+i);
	h->buf_me[i].release = ng_wakeup_video_buf;
    }

    return h;

 err:
    if (h->fd != -1)
	close(h->fd);
    if (h)
	free(h);
    return NULL;
}

static int
v4l2_close(void *handle)
{
    struct v4l2_handle *h = handle;

    if (ng_debug)
	fprintf(stderr, "v4l2: close\n");

	close(h->fd);

	if (NULL != h->attr) {
		int i;
		for (i = 0; i < h->nattr; ++i) {
			if (NULL != h->attr[i].choices) {
				free(h->attr[i].choices);
				h->attr[i].choices = NULL;
			}
		}
		free(h->attr);
		h->attr = NULL;
	}
	
	free(h);
	h = NULL;
    
	return 0;
}

static char*
v4l2_devname(void *handle)
{
    struct v4l2_handle *h = handle;
    return (char*)h->cap.card;
}

static int v4l2_flags(void *handle)
{
    struct v4l2_handle *h = handle;
    int ret = 0;

    if (h->cap.capabilities & V4L2_CAP_VIDEO_OVERLAY && !h->ov_error)
	ret |= CAN_OVERLAY;
    if (h->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
	ret |= CAN_CAPTURE;
    if (h->cap.capabilities & V4L2_CAP_TUNER)
	ret |= CAN_TUNE;
    return ret;
}

static struct ng_attribute* v4l2_attrs(void *handle)
{
    struct v4l2_handle *h = handle;
    return h->attr;
}

/* ---------------------------------------------------------------------- */

static unsigned long
v4l2_getfreq(void *handle)
{
    struct v4l2_handle *h = handle;
    struct v4l2_frequency f;

    memset(&f,0,sizeof(f));
    xioctl(h->fd, VIDIOC_G_FREQUENCY, &f, 0);
    return f.frequency;
}

static void
v4l2_setfreq(void *handle, unsigned long freq)
{
    struct v4l2_handle *h = handle;
    struct v4l2_frequency f;

    if (ng_debug)
	fprintf(stderr,"v4l2: freq: %.3f\n",(float)freq/16);
    memset(&f,0,sizeof(f));
    f.type = V4L2_TUNER_ANALOG_TV;
    f.frequency = freq;
    xioctl(h->fd, VIDIOC_S_FREQUENCY, &f, 0);
}

static int
v4l2_tuned(void *handle)
{
    struct v4l2_handle *h = handle;
    struct v4l2_tuner tuner;

    usleep(10000);
    memset(&tuner,0,sizeof(tuner));
    if (-1 == xioctl(h->fd,VIDIOC_G_TUNER,&tuner,0))
	return 0;
    return tuner.signal ? 1 : 0;
}

/* ---------------------------------------------------------------------- */
/* overlay                                                                */

static int
v4l2_setupfb(void *handle, struct ng_video_fmt *fmt, void *base)
{
    struct v4l2_handle *h = handle;

    if (-1 == xioctl(h->fd, VIDIOC_G_FBUF, &h->ov_fb, 0))
	return -1;
    
    /* double-check settings */
    if (NULL != base && h->ov_fb.base != base) {
	fprintf(stderr,"v4l2: WARNING: framebuffer base address mismatch\n");
	fprintf(stderr,"v4l2: me=%p v4l=%p\n",base,h->ov_fb.base);
	h->ov_error = 1;
	return -1;
    }
    if (h->ov_fb.fmt.width  != fmt->width ||
	h->ov_fb.fmt.height != fmt->height) {
	fprintf(stderr,"v4l2: WARNING: framebuffer size mismatch\n");
	fprintf(stderr,"v4l2: me=%dx%d v4l=%dx%d\n",
		fmt->width,fmt->height,h->ov_fb.fmt.width,h->ov_fb.fmt.height);
	h->ov_error = 1;
	return -1;
    }
    if (fmt->bytesperline > 0 &&
	fmt->bytesperline != h->ov_fb.fmt.bytesperline) {
	fprintf(stderr,"v4l2: WARNING: framebuffer bpl mismatch\n");
	fprintf(stderr,"v4l2: me=%d v4l=%d\n",
		fmt->bytesperline,h->ov_fb.fmt.bytesperline);
	h->ov_error = 1;
	return -1;
    }
    return 0;
}

static int
v4l2_overlay(void *handle, struct ng_video_fmt *fmt, int x, int y,
	     struct OVERLAY_CLIP *oc, int count, int aspect)
{
    struct v4l2_handle *h = handle;
    struct v4l2_format win;
    int rc,i;

    if (h->ov_error)
	return -1;
    
    if (NULL == fmt) {
	if (ng_debug)
	    fprintf(stderr,"v4l2: overlay off\n");
	if (h->ov_enabled) {
	    h->ov_enabled = 0;
	    h->ov_on = 0;
	    xioctl(h->fd, VIDIOC_OVERLAY, &h->ov_on, 0);
	}
	return 0;
    }

    if (ng_debug)
	fprintf(stderr,"v4l2: overlay win=%dx%d+%d+%d, %d clips\n",
		fmt->width,fmt->height,x,y,count);
    memset(&win,0,sizeof(win));
    win.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
    win.fmt.win.w.left    = x;
    win.fmt.win.w.top     = y;
    win.fmt.win.w.width   = fmt->width;
    win.fmt.win.w.height  = fmt->height;

    /* check against max. size */
    xioctl(h->fd,VIDIOC_TRY_FMT,&win,0);
    if (win.fmt.win.w.width != (int)fmt->width)
	win.fmt.win.w.left = x + (fmt->width - win.fmt.win.w.width)/2;
    if (win.fmt.win.w.height != (int)fmt->height)
	win.fmt.win.w.top = y + (fmt->height - win.fmt.win.w.height)/2;
    if (aspect)
	ng_ratio_fixup(&win.fmt.win.w.width,&win.fmt.win.w.height,
		       &win.fmt.win.w.left,&win.fmt.win.w.top);

    /* fixups */
    ng_check_clipping(win.fmt.win.w.width, win.fmt.win.w.height,
		      x - win.fmt.win.w.left, y - win.fmt.win.w.top,
		      oc, &count);

    h->ov_win = win;
    if (h->ov_fb.capability & V4L2_FBUF_CAP_LIST_CLIPPING) {
	h->ov_win.fmt.win.clips      = h->ov_clips;
	h->ov_win.fmt.win.clipcount  = count;
	
	for (i = 0; i < count; i++) {
	    h->ov_clips[i].next = (i+1 == count) ? NULL : &h->ov_clips[i+1];
	    h->ov_clips[i].c.left   = oc[i].x1;
	    h->ov_clips[i].c.top    = oc[i].y1;
	    h->ov_clips[i].c.width  = oc[i].x2-oc[i].x1;
	    h->ov_clips[i].c.height = oc[i].y2-oc[i].y1;
	}
    }
    rc = xioctl(h->fd, VIDIOC_S_FMT, &h->ov_win, 0);

    h->ov_enabled = (0 == rc) ? 1 : 0;
    h->ov_on      = (0 == rc) ? 1 : 0;
    xioctl(h->fd, VIDIOC_OVERLAY, &h->ov_on, 0);

    return 0;
}

/* ---------------------------------------------------------------------- */
/* capture helpers                                                        */

static int
v4l2_queue_buffer(struct v4l2_handle *h)
{
    int frame = h->queue % h->reqbufs.count;
    int rc;

    if (0 != h->buf_me[frame].refcount) {
	if (0 != h->queue - h->waiton)
	    return -1;
	fprintf(stderr,"v4l2: waiting for a free buffer\n");
	ng_waiton_video_buf(h->buf_me+frame);
    }

    rc = xioctl(h->fd,VIDIOC_QBUF,&h->buf_v4l2[frame], 0);
    if (0 == rc)
	h->queue++;
    return rc;
}

static void
v4l2_queue_all(struct v4l2_handle *h)
{
    for (;;) {
	if (h->queue - h->waiton >= h->reqbufs.count)
	    return;
	if (0 != v4l2_queue_buffer(h))
	    return;
    }
}

static int
v4l2_waiton(struct v4l2_handle *h)
{
    struct v4l2_buffer buf;
    struct timeval tv;
    fd_set rdset;
    
    /* wait for the next frame */
 again:
    tv.tv_sec  = 5;
    tv.tv_usec = 0;
    FD_ZERO(&rdset);
    FD_SET(h->fd, &rdset);
    switch (select(h->fd + 1, &rdset, NULL, NULL, &tv)) {
    case -1:
	if (EINTR == errno)
	    goto again;
	perror("v4l2: select");
	return -1;
    case  0:
	fprintf(stderr,"v4l2: oops: select timeout\n");
	return -1;
    }

    /* get it */
    memset(&buf,0,sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(h->fd,VIDIOC_DQBUF,&buf, 0))
	return -1;
    h->waiton++;
    h->buf_v4l2[buf.index] = buf;

    return buf.index;
}

static int
v4l2_start_streaming(struct v4l2_handle *h, int buffers)
{
    int disable_overlay = 0;
    unsigned int i;
    
    /* setup buffers */
    h->reqbufs.count  = buffers;
    h->reqbufs.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    h->reqbufs.memory = V4L2_MEMORY_MMAP;
    if (-1 == xioctl(h->fd, VIDIOC_REQBUFS, &h->reqbufs, 0))
	return -1;
    for (i = 0; i < h->reqbufs.count; i++) {
	h->buf_v4l2[i].index  = i;
	h->buf_v4l2[i].type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	h->buf_v4l2[i].memory = V4L2_MEMORY_MMAP;
	if (-1 == xioctl(h->fd, VIDIOC_QUERYBUF, &h->buf_v4l2[i], 0))
	    return -1;
	h->buf_me[i].fmt  = h->fmt_me;
	h->buf_me[i].size = h->buf_me[i].fmt.bytesperline *
	    h->buf_me[i].fmt.height;
	h->buf_me[i].data = mmap(NULL, h->buf_v4l2[i].length,
				 PROT_READ | PROT_WRITE, MAP_SHARED,
				 h->fd, h->buf_v4l2[i].m.offset);
	if (MAP_FAILED == h->buf_me[i].data) {
	    perror("mmap");
	    return -1;
	}
	if (ng_debug)
	    print_bufinfo(&h->buf_v4l2[i]);
    }

    /* queue up all buffers */
    v4l2_queue_all(h);

 try_again:
    /* turn off preview (if needed) */
    if (disable_overlay) {
	h->ov_on = 0;
	xioctl(h->fd, VIDIOC_OVERLAY, &h->ov_on, 0);
	if (ng_debug)
	    fprintf(stderr,"v4l2: overlay off (start_streaming)\n");
    }

    /* start capture */
    if (-1 == xioctl(h->fd,VIDIOC_STREAMON,&h->fmt_v4l2.type,
		     h->ov_on ? EBUSY : 0)) {
	if (h->ov_on && errno == EBUSY) {
	    disable_overlay = 1;
	    goto try_again;
	}
	return -1;
    }
    return 0;
}

static void
v4l2_stop_streaming(struct v4l2_handle *h)
{
    unsigned int i;
    
    /* stop capture */
    if (-1 == ioctl(h->fd,VIDIOC_STREAMOFF,&h->fmt_v4l2.type))
	perror("ioctl VIDIOC_STREAMOFF");
    
    /* free buffers */
    for (i = 0; i < h->reqbufs.count; i++) {
	if (0 != h->buf_me[i].refcount)
	    ng_waiton_video_buf(&h->buf_me[i]);
	if (ng_debug)
	    print_bufinfo(&h->buf_v4l2[i]);
	if (-1 == munmap(h->buf_me[i].data,h->buf_me[i].size))
	    perror("munmap");
    }
    h->queue = 0;
    h->waiton = 0;

    /* turn on preview (if needed) */
    if (h->ov_on != h->ov_enabled) {
	h->ov_on = h->ov_enabled;
	xioctl(h->fd, VIDIOC_OVERLAY, &h->ov_on, 0);
	if (ng_debug)
	    fprintf(stderr,"v4l2: overlay on (stop_streaming)\n");
    }
}

/* ---------------------------------------------------------------------- */
/* capture interface                                                      */

/* set capture parameters */
static int
v4l2_setformat(void *handle, struct ng_video_fmt *fmt)
{
    struct v4l2_handle *h = handle;
    
    h->fmt_v4l2.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    h->fmt_v4l2.fmt.pix.pixelformat  = xawtv_pixelformat[fmt->fmtid];
    h->fmt_v4l2.fmt.pix.width        = fmt->width;
    h->fmt_v4l2.fmt.pix.height       = fmt->height;
    h->fmt_v4l2.fmt.pix.field        = V4L2_FIELD_ANY;
    //h->fmt_v4l2.fmt.pix.field        = V4L2_FIELD_ALTERNATE;
    if (fmt->bytesperline != fmt->width * ng_vfmt_to_depth[fmt->fmtid]/8)
	h->fmt_v4l2.fmt.pix.bytesperline = fmt->bytesperline;
    else
	h->fmt_v4l2.fmt.pix.bytesperline = 0;

    if (-1 == xioctl(h->fd, VIDIOC_S_FMT, &h->fmt_v4l2, EINVAL))
	return -1;
    if (h->fmt_v4l2.fmt.pix.pixelformat != xawtv_pixelformat[fmt->fmtid])
	return -1;
    fmt->width        = h->fmt_v4l2.fmt.pix.width;
    fmt->height       = h->fmt_v4l2.fmt.pix.height;
    fmt->bytesperline = h->fmt_v4l2.fmt.pix.bytesperline;
    if (0 == fmt->bytesperline)
	fmt->bytesperline = fmt->width * ng_vfmt_to_depth[fmt->fmtid] / 8;
    h->fmt_me = *fmt;
    if (ng_debug)
	fprintf(stderr,"v4l2: new capture params (%dx%d, %c%c%c%c, %d byte)\n",
		fmt->width,fmt->height,
		h->fmt_v4l2.fmt.pix.pixelformat & 0xff,
		(h->fmt_v4l2.fmt.pix.pixelformat >>  8) & 0xff,
		(h->fmt_v4l2.fmt.pix.pixelformat >> 16) & 0xff,
		(h->fmt_v4l2.fmt.pix.pixelformat >> 24) & 0xff,
		h->fmt_v4l2.fmt.pix.sizeimage);
    return 0;
}

/* start/stop video */
static int
v4l2_startvideo(void *handle, int fps, unsigned int buffers)
{
    struct v4l2_handle *h = handle;

    if (0 != h->fps)
	fprintf(stderr,"v4l2_startvideo: oops: fps!=0\n");
    h->fps = fps;
    h->first = 1;
    h->start = 0;

    if (h->cap.capabilities & V4L2_CAP_STREAMING)
	return v4l2_start_streaming(h,buffers);
    return 0;
}

static void
v4l2_stopvideo(void *handle)
{
    struct v4l2_handle *h = handle;

    if (0 == h->fps)
	fprintf(stderr,"v4l2_stopvideo: oops: fps==0\n");
    h->fps = 0;

    if (h->cap.capabilities & V4L2_CAP_STREAMING)
	v4l2_stop_streaming(h);
}

/* read images */
static struct ng_video_buf*
v4l2_nextframe(void *handle)
{
    struct v4l2_handle *h = handle;
    struct ng_video_buf *buf = NULL;
    int rc,size,frame = 0;

    if (h->cap.capabilities & V4L2_CAP_STREAMING) {
	v4l2_queue_all(h);
	frame = v4l2_waiton(h);
	if (-1 == frame)
	    return NULL;
	h->buf_me[frame].refcount++;
	buf = &h->buf_me[frame];
	memset(&buf->info,0,sizeof(buf->info));
	buf->info.ts = ng_tofday_to_timestamp(&h->buf_v4l2[frame].timestamp);
    } else {
	size = h->fmt_me.bytesperline * h->fmt_me.height;
	buf = ng_malloc_video_buf(&h->fmt_me,size);
	rc = read(h->fd,buf->data,size);
	if (rc != size) {
	    if (-1 == rc) {
		perror("v4l2: read");
	    } else {
		fprintf(stderr, "v4l2: read: rc=%d/size=%d\n",rc,size);
	    }
	    ng_release_video_buf(buf);
	    return NULL;
	}
	memset(&buf->info,0,sizeof(buf->info));
	buf->info.ts = ng_get_timestamp();
    }

    if (h->first) {
	h->first = 0;
	h->start = buf->info.ts;
	if (ng_debug)
	    fprintf(stderr,"v4l2: start ts=%lld\n",h->start);
    }
    buf->info.ts -= h->start;
    return buf;
}

static struct ng_video_buf*
v4l2_getimage(void *handle)
{
    struct v4l2_handle *h = handle;
    struct ng_video_buf *buf; 
    int size,frame,rc;

    size = h->fmt_me.bytesperline * h->fmt_me.height;
    buf = ng_malloc_video_buf(&h->fmt_me,size);
    if (h->cap.capabilities & V4L2_CAP_READWRITE) {
	rc = read(h->fd,buf->data,size);
	if (-1 == rc  &&  EBUSY == errno  &&  h->ov_on) {
	    h->ov_on = 0;
	    xioctl(h->fd, VIDIOC_OVERLAY, &h->ov_on, 0);
	    rc = read(h->fd,buf->data,size);
	    h->ov_on = 1;
	    xioctl(h->fd, VIDIOC_OVERLAY, &h->ov_on, 0);
	}
	if (rc != size) {
	    if (-1 == rc) {
		perror("v4l2: read");
	    } else {
		fprintf(stderr, "v4l2: read: rc=%d/size=%d\n",rc,size);
	    }
	    ng_release_video_buf(buf);
	    return NULL;
	}
    } else {
	if (-1 == v4l2_start_streaming(h,1)) {
	    v4l2_stop_streaming(h);
	    return NULL;
	}
	frame = v4l2_waiton(h);
	if (-1 == frame) {
	    v4l2_stop_streaming(h);
	    return NULL;
	}
	memcpy(buf->data,h->buf_me[0].data,size);
	v4l2_stop_streaming(h);
    }
    return buf;
}

/* ---------------------------------------------------------------------- */

void ng_plugin_v4l2_init(void)
{
    ng_vid_driver_register(NG_PLUGIN_MAGIC,__FILE__,&v4l2_driver);
}
