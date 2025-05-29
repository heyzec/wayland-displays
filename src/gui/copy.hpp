#include "wlr-screencopy-unstable-v1.h"
#include <vector>

struct OutputState {
  // OutputState
  wl_output *output;
  char *name;
  // FrameState
  uint id;
  // output
  wl_buffer *buffer;
  void *pixels;
  uint width;
  uint height;
  uint stride;
  uint size; // Obtained from stride * height
  int fd;
  // remove
  bool copied = false;
};

/**
 * Initialize this module (by connecting to the Wayland compositor).
 * This function must be called before other functions in this module.
 */

void screencopy_init();
/**
 * Get a container of frames of all outputs.
 */

std::vector<OutputState *> *screencopy_get();
/**
 * Destroy the container of frames to free memory after use.
 */
void screencopy_destroy();
