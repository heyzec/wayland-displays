#include "wlr-screencopy-unstable-v1.h"

extern zwlr_screencopy_manager_v1 *manager;
extern zwlr_screencopy_frame_v1 *frame;
extern wl_output *output;
extern wl_shm *shm;
extern void *pixels;
extern uint global_width;
extern uint global_height;
extern uint global_stride;

void wlr_screencopy_init();
