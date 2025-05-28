#include "wlr-screencopy-unstable-v1.h"
#include <vector>

extern zwlr_screencopy_manager_v1 *manager;
extern zwlr_screencopy_frame_v1 *frame;
extern wl_output *output;
extern wl_shm *shm;

struct CopyOutput {
  zwlr_screencopy_manager_v1 *manager;
  zwlr_screencopy_frame_v1 *frame;
  wl_output *output;
  wl_shm *shm;
  wl_buffer *buffer;
  void *pixels;
  uint width;
  uint height;
  uint stride;
  char *name;
  uint size; // Obtained from stride * height
  int fd;
};

void wlr_screencopy_init();
std::vector<CopyOutput *> *get_pixels();
