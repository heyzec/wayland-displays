#include "wlr-screencopy-unstable-v1.h"
#include <vector>

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
  bool copied = false;
};

void screencopy_init();
std::vector<CopyOutput *> *screencopy_get();

/**
 * Initialize this module (by connecting to the Wayland compositor).
 * This function must be called before other functions in this module.
 */

void screencopy_init();
/**
 * Get a container of frames of all outputs.
 */

std::vector<CopyOutput *> *screencopy_get();
/**
 * Destroy the container of frames to free memory after use.
 */
void screencopy_destroy();
