/*
 * next generation[tm] xawtv capture interfaces
 *
 * (c) 2001-03 Gerd Knorr <kraxel@bytesex.org>
 *
 */
#ifndef GRAB_NG_H
#define GRAB_NG_H

#include <pthread.h>
#include <sys/types.h>

#include "devices.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int  ng_debug;
extern int  ng_chromakey;
extern int  ng_jpeg_quality;
extern int  ng_ratio_x;
extern int  ng_ratio_y;
extern char ng_v4l_conf[256];

#define BUG_ON(condition,message)	if (condition) {\
	fprintf(stderr,"BUG: %s [%s:%d]\n",\
		message,__FILE__,__LINE__);\
	exit(1);}

#if __STDC_VERSION__ < 199901
# define restrict
//# define bool int
#endif

#define UNSET          (-1U)
#define DIMOF(array)   (sizeof(array)/sizeof(array[0]))
#define SDIMOF(array)  ((signed int)(sizeof(array)/sizeof(array[0])))
#define GETELEM(array,index,default) \
	(index < sizeof(array)/sizeof(array[0]) ? array[index] : default)

/* --------------------------------------------------------------------- */
/* defines                                                               */

#define VIDEO_NONE           0
#define VIDEO_RGB08          1  /* bt848 dithered */
#define VIDEO_GRAY           2
#define VIDEO_RGB15_LE       3  /* 15 bpp little endian */
#define VIDEO_RGB16_LE       4  /* 16 bpp little endian */
#define VIDEO_RGB15_BE       5  /* 15 bpp big endian */
#define VIDEO_RGB16_BE       6  /* 16 bpp big endian */
#define VIDEO_BGR24          7  /* bgrbgrbgrbgr (LE) */
#define VIDEO_BGR32          8  /* bgr-bgr-bgr- (LE) */
#define VIDEO_RGB24          9  /* rgbrgbrgbrgb (BE) */
#define VIDEO_RGB32         10  /* -rgb-rgb-rgb (BE) */
#define VIDEO_LUT2          11  /* lookup-table 2 byte depth */
#define VIDEO_LUT4          12  /* lookup-table 4 byte depth */
#define VIDEO_YUYV      13  /* 4:2:2 */
#define VIDEO_YUV422P       14  /* YUV 4:2:2 (planar) */
#define VIDEO_YUV420P       15  /* YUV 4:2:0 (planar) */
#define VIDEO_MJPEG     16  /* MJPEG (AVI) */
#define VIDEO_JPEG      17  /* JPEG (JFIF) */
#define VIDEO_UYVY      18  /* 4:2:2 */
#define VIDEO_FMT_COUNT     19


#if BYTE_ORDER == BIG_ENDIAN
# define VIDEO_RGB15_NATIVE      VIDEO_RGB15_BE
# define VIDEO_RGB16_NATIVE      VIDEO_RGB16_BE
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
# define VIDEO_RGB15_NATIVE      VIDEO_RGB15_LE
# define VIDEO_RGB16_NATIVE      VIDEO_RGB16_LE
#endif

#define ATTR_TYPE_INTEGER    1   /*  range 0 - 65535  */
#define ATTR_TYPE_CHOICE     2   /*  multiple choice  */
#define ATTR_TYPE_BOOL       3   /*  yes/no           */

#define ATTR_ID_NORM         1
#define ATTR_ID_INPUT        2
#define ATTR_ID_VOLUME       3
#define ATTR_ID_MUTE         4
#define ATTR_ID_AUDIO_MODE   5
#define ATTR_ID_COLOR        6
#define ATTR_ID_BRIGHT       7
#define ATTR_ID_HUE          8
#define ATTR_ID_CONTRAST     9
#define ATTR_ID_COUNT       10

#define CAN_OVERLAY          1
#define CAN_CAPTURE          2
#define CAN_TUNE             4
#define NEEDS_CHROMAKEY      8

/* --------------------------------------------------------------------- */


extern const unsigned int   ng_vfmt_to_depth[VIDEO_FMT_COUNT];
extern const char*          ng_vfmt_to_desc[VIDEO_FMT_COUNT];
extern const char*          ng_attr_to_desc[ATTR_ID_COUNT];


extern void ng_plugin_v4l2_init(void);
extern void ng_plugin_v4l_init(void);



/* --------------------------------------------------------------------- */

struct STRTAB {
    long nr;
    const char *str;
};

struct OVERLAY_CLIP {
    int x1,x2,y1,y2;
};

/* --------------------------------------------------------------------- */
/* video data structures                                                 */

struct ng_video_fmt {
    unsigned int   fmtid;         /* VIDEO_* */
    unsigned int   width;
    unsigned int   height;
    unsigned int   bytesperline;  /* zero for compressed formats */
};

struct ng_video_buf {
    struct ng_video_fmt  fmt;
    size_t               size;
    unsigned char        *data;

    /* meta info for frame */
    struct {
	int64_t          ts;      /* time stamp */
	int              seq;
	int              twice;
    } info;

    /*
     * the lock is for the reference counter.
     * if the reference counter goes down to zero release()
     * should be called.  priv is for the owner of the
     * buffer (can be used by the release callback)
     */
    pthread_mutex_t      lock;
    pthread_cond_t       cond;
    int                  refcount;
    void                 (*release)(struct ng_video_buf *buf);
    void                 *priv;
};

void ng_init_video_buf(struct ng_video_buf *buf);
void ng_release_video_buf(struct ng_video_buf *buf);
struct ng_video_buf* ng_malloc_video_buf(struct ng_video_fmt *fmt,
					 int size);
void ng_wakeup_video_buf(struct ng_video_buf *buf);
void ng_waiton_video_buf(struct ng_video_buf *buf);



/* --------------------------------------------------------------------- */
/* attributes                                                            */

struct ng_attribute {
    int                  id;
    const char           *name;
    int                  type;
    int                  defval;
    struct STRTAB        *choices;    /* ATTR_TYPE_CHOICE  */
    int                  min,max;     /* ATTR_TYPE_INTEGER */
    int                  points;      /* ATTR_TYPE_INTEGER -- fixed point */
    const void           *priv;
    void                 *handle;
    int         (*read)(struct ng_attribute*);
    void        (*write)(struct ng_attribute*, int val);
};

struct ng_attribute* ng_attr_byid(struct ng_attribute *attrs, int id);
struct ng_attribute* ng_attr_byname(struct ng_attribute *attrs, char *name);
const char* ng_attr_getstr(struct ng_attribute *attr, int value);
int ng_attr_getint(struct ng_attribute *attr, char *value);
void ng_attr_listchoices(struct ng_attribute *attr);
int ng_attr_int2percent(struct ng_attribute *attr, int value);
int ng_attr_percent2int(struct ng_attribute *attr, int percent);
int ng_attr_parse_int(struct ng_attribute *attr, char *str);

/* --------------------------------------------------------------------- */

void ng_ratio_fixup(int *width, int *height, int *xoff, int *yoff);
void ng_ratio_fixup2(int *width, int *height, int *xoff, int *yoff,
		     int ratio_x, int ratio_y, int up);

/* --------------------------------------------------------------------- */
/* device informations                                                   */

struct ng_devinfo {
    char  device[32];
    char  name[64];
    int   flags;
};

/* --------------------------------------------------------------------- */
/* capture/overlay interface driver                                      */

struct ng_vid_driver {
    const char *name;

    /* open/close */
    void*  (*open)(char *device);
    int    (*close)(void *handle);

    /* attributes */
    char* (*get_devname)(void *handle);
    int   (*capabilities)(void *handle);
    struct ng_attribute* (*list_attrs)(void *handle);
    
    /* overlay */
    int   (*setupfb)(void *handle, struct ng_video_fmt *fmt, void *base);
    int   (*overlay)(void *handle, struct ng_video_fmt *fmt, int x, int y,
		     struct OVERLAY_CLIP *oc, int count, int aspect);
    
    /* capture */
    int   (*setformat)(void *handle, struct ng_video_fmt *fmt);
    int   (*startvideo)(void *handle, int fps, unsigned int buffers);
    void  (*stopvideo)(void *handle);
    struct ng_video_buf* (*nextframe)(void *handle); /* video frame */
    struct ng_video_buf* (*getimage)(void *handle);  /* single image */

    /* tuner */
    unsigned long (*getfreq)(void *handle);
    void  (*setfreq)(void *handle, unsigned long freq);
    int   (*is_tuned)(void *handle);

    struct list_head list;
};


/* --------------------------------------------------------------------- */
/* color space converters                                                */

struct ng_video_conv {
    unsigned int          fmtid_in;
    unsigned int          fmtid_out;
    void*                 (*init)(struct ng_video_fmt *out,
				  void *priv);
    void                  (*frame)(void *handle,
				   struct ng_video_buf *out,
				   struct ng_video_buf *in);
    void                  (*fini)(void *handle);
    void                  *priv;

    struct list_head      list;
};

struct ng_convert_handle {
    struct ng_video_fmt     ifmt;
    struct ng_video_fmt     ofmt;
    int                     isize;
    int                     osize;
    struct ng_video_conv    *conv;
    void                    *chandle;
};

struct ng_convert_handle* ng_convert_alloc(struct ng_video_conv *conv,
					   struct ng_video_fmt *i,
					   struct ng_video_fmt *o);
void ng_convert_init(struct ng_convert_handle *h);
struct ng_video_buf* ng_convert_frame(struct ng_convert_handle *h,
				      struct ng_video_buf *dest,
				      struct ng_video_buf *buf);
void ng_convert_fini(struct ng_convert_handle *h);
struct ng_video_buf* ng_convert_single(struct ng_convert_handle *h,
				       struct ng_video_buf *in);

/* --------------------------------------------------------------------- */
/* filters                                                               */

struct ng_filter {
    char                  *name;
    int                   fmts;
    struct ng_attribute*  attrs;
    void*                 (*init)(struct ng_video_fmt *fmt);
    struct ng_video_buf*  (*frame)(void *handle,
				   struct ng_video_buf *in);
    void                  (*fini)(void *handle);

    struct list_head      list;
};

/* --------------------------------------------------------------------- */

/* must be changed if we break compatibility */
#define NG_PLUGIN_MAGIC 0x20030129

extern struct list_head ng_conv;
extern struct list_head ng_filters;
extern struct list_head ng_vid_drivers;

int ng_conv_register(int magic, char *plugname,
		     struct ng_video_conv *list, int count);
int ng_filter_register(int magic, char *plugname,
		       struct ng_filter *filter);
int ng_vid_driver_register(int magic, char *plugname,
			   struct ng_vid_driver *driver);

struct ng_video_conv* ng_conv_find_to(unsigned int out, int *i);
struct ng_video_conv* ng_conv_find_from(unsigned int out, int *i);
struct ng_video_conv* ng_conv_find_match(unsigned int in, unsigned int out);

const struct ng_vid_driver* ng_vid_open(char *device, char *driver,
					struct ng_video_fmt *screen,
					void *base, void **handle);

int64_t ng_tofday_to_timestamp(struct timeval *tv);
int64_t ng_get_timestamp(void);
void ng_check_clipping(int width, int height, int xadjust, int yadjust,
		       struct OVERLAY_CLIP *oc, int *count);
struct ng_video_buf* ng_filter_single(struct ng_filter *filter,
				      struct ng_video_buf *in);

/* --------------------------------------------------------------------- */

void ng_init(void);
void ng_lut_init(unsigned long red_mask, unsigned long green_mask,
		 unsigned long blue_mask, unsigned int fmtid, int swap);

void ng_rgb24_to_lut2(unsigned char *dest, unsigned char *src, int p);
void ng_rgb24_to_lut4(unsigned char *dest, unsigned char *src, int p);

/* --------------------------------------------------------------------- */
/* internal stuff starts here                                            */

#ifdef NG_PRIVATE

/* init functions */
//void ng_color_packed_init(void);
//void ng_color_yuv2rgb_init(void);
//void ng_writefile_init(void);


/* for yuv2rgb using lookup tables (color_lut.c, color_yuv2rgb.c) */
unsigned long   ng_lut_red[256];
unsigned long   ng_lut_green[256];
unsigned long   ng_lut_blue[256];
void ng_yuv422_to_lut2(unsigned char *dest, unsigned char *s, int p);
void ng_yuv422_to_lut4(unsigned char *dest, unsigned char *s, int p);
void ng_yuv420p_to_lut2(void *h, struct ng_video_buf *out,
			struct ng_video_buf *in);
void ng_yuv420p_to_lut4(void *h, struct ng_video_buf *out,
			struct ng_video_buf *in);
void ng_yuv422p_to_lut2(void *h, struct ng_video_buf *out,
			struct ng_video_buf *in);
void ng_yuv422p_to_lut4(void *h, struct ng_video_buf *out,
			struct ng_video_buf *in);

/* color_common.c stuff */
void* ng_packed_init(struct ng_video_fmt *out, void *priv);
void  ng_packed_frame(void *handle, struct ng_video_buf *out,
		      struct ng_video_buf *in);
void* ng_conv_nop_init(struct ng_video_fmt *out, void *priv);
void  ng_conv_nop_fini(void *handle);

#define NG_GENERIC_PACKED			\
	init:           ng_packed_init,		\
	frame:          ng_packed_frame,       	\
	fini:           ng_conv_nop_fini

#endif /* NG_PRIVATE */

/* --------------------------------------------------------------------- */
/*
 * Local variables:
 * compile-command: "(cd ..; make)"
 * End:
 */
#ifdef __cplusplus
}
#endif

#endif /* GRAB_NG_H */
