#ifndef DEVICES_H
#define DEVICES_H

#ifdef __cplusplus
extern "C" {
#endif
	
struct ng_device_config {
    char *video;
    char *radio;
    char *vbi;
    char *dsp;
    char *mixer;
    char *video_scan[32];
    char *mixer_scan[32];
};
extern struct ng_device_config ng_dev;

void ng_device_init(void);

#ifdef __cplusplus
}
#endif

#endif /* DEVICES_H */
