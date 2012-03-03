#ifndef STRUCT_V4L2_H
#define STRUCT_V4L2_H

#ifdef __cplusplus
extern "C" {
#endif
	

extern char *desc_v4l2_field[];
extern char *desc_v4l2_buf_type[];
extern char *desc_v4l2_ctrl_type[];
extern char *desc_v4l2_tuner_type[];
extern char *desc_v4l2_memory[];
extern char *desc_v4l2_colorspace[];
extern char *bits_capabilities[32];
extern char *bits_standard[64];
extern char *bits_buf_flags[32];
extern char *bits_fbuf_cap[32];
extern char *bits_fbuf_flags[32];
extern char *desc_input_type[32];
extern char *bits_input_status[32];
extern char *bits_tuner_cap[32];
extern char *bits_tuner_rx[32];
extern char *desc_tuner2_mode[];

extern struct struct_desc desc_v4l2_rect[];
extern struct struct_desc desc_v4l2_fract[];
extern struct struct_desc desc_v4l2_capability[];
extern struct struct_desc desc_v4l2_pix_format[];
extern struct struct_desc desc_v4l2_fmtdesc[];
extern struct struct_desc desc_v4l2_timecode[];
extern struct struct_desc desc_v4l2_compression[];
extern struct struct_desc desc_v4l2_jpegcompression[];
extern struct struct_desc desc_v4l2_requestbuffers[];
extern struct struct_desc desc_v4l2_buffer[];
extern struct struct_desc desc_v4l2_framebuffer[];
extern struct struct_desc desc_v4l2_clip[];
extern struct struct_desc desc_v4l2_window[];
extern struct struct_desc desc_v4l2_captureparm[];
extern struct struct_desc desc_v4l2_outputparm[];
extern struct struct_desc desc_v4l2_cropcap[];
extern struct struct_desc desc_v4l2_crop[];
extern struct struct_desc desc_v4l2_standard[];
extern struct struct_desc desc_v4l2_input[];
extern struct struct_desc desc_v4l2_output[];
extern struct struct_desc desc_v4l2_control[];
extern struct struct_desc desc_v4l2_queryctrl[];
extern struct struct_desc desc_v4l2_querymenu[];
extern struct struct_desc desc_v4l2_tuner[];
extern struct struct_desc desc_v4l2_modulator[];
extern struct struct_desc desc_v4l2_frequency[];
extern struct struct_desc desc_v4l2_audio[];
extern struct struct_desc desc_v4l2_audioout[];
extern struct struct_desc desc_v4l2_vbi_format[];
extern struct struct_desc desc_v4l2_format[];
extern struct struct_desc desc_v4l2_streamparm[];

extern struct ioctl_desc ioctls_v4l2[256];

#ifdef __cplusplus
}
#endif

#endif /* STRUCT_V4L2_H */
