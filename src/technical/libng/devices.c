#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "devices.h"
/*
 * default devices names
 */
#if defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__)
struct ng_device_config ng_dev = {
    video:  "/dev/bktr0",
    radio:  NULL,
    vbi:    "/dev/vbi0",
    dsp:    "/dev/dsp",
    mixer:  "/dev/mixer",
    video_scan: {
	"/dev/bktr0",
	"/dev/bktr1",
	"/dev/cxm0",
	"/dev/cxm1",
	NULL
    },
    mixer_scan: {
	"/dev/mixer",
	"/dev/mixer1", 
	"/dev/mixer2",
	"/dev/mixer3",
	NULL
    }
};
#endif
#if defined(__linux__)
struct ng_device_config ng_dev = {
    video:  "/dev/video0", /* <rant>thank you redhat breaking
			    * /dev/video as symbolic link to the
			    * default video device ... </rant> */
    radio:  "/dev/radio",
    vbi:    "/dev/vbi",
    dsp:    "/dev/dsp",
    mixer:  "/dev/mixer",
    video_scan:   {
	"/dev/video0",
	"/dev/video1",
	"/dev/video2",
	"/dev/video3",
	NULL
    },
    mixer_scan: {
	"/dev/mixer",
	"/dev/mixer1", 
	"/dev/mixer2",
	"/dev/mixer3",
	NULL
    }
};

struct ng_device_config ng_dev_devfs = {
    video:  "/dev/v4l/video0",
    radio:  "/dev/v4l/radio0",
    vbi:    "/dev/v4l/vbi0",
    dsp:    "/dev/sound/dsp",
    mixer:  "/dev/sound/mixer",
    video_scan:   {
	"/dev/v4l/video0",
	"/dev/v4l/video1",
	"/dev/v4l/video2",
	"/dev/v4l/video3",
	NULL
    },
    mixer_scan: {
	"/dev/sound/mixer",
	"/dev/sound/mixer1", 
	"/dev/sound/mixer2",
	"/dev/sound/mixer3",
	NULL
    }
};
#endif

void
ng_device_init(void)
{
#if defined(__linux__)
    struct stat st;

    if (-1 == lstat("/dev/.devfsd",&st))
	return;
    if (!S_ISCHR(st.st_mode))
	return;
    ng_dev = ng_dev_devfs;
#endif
}
