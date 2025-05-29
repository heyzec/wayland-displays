#include "wlr-screencopy-unstable-v1.h"
#include <string>
#include <vector>

struct ScreencopyFrame {
  /* Name of output of the source of frame */
  const std::string name;
  /* Raw pixel data in XRGB format */
  const void *pixels;
  /* Horizontal size of frame in pixels */
  const uint width;
  /* Vertical size of frame in pixels */
  const uint height;
};

struct ScreencopyObject {
  /* const */ int id;
  /* const */ std::vector<ScreencopyFrame> frames;
};

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

ScreencopyObject screencopy_get();
/**
 * Destroy the container of frames to free memory after use.
 */
void screencopy_destroy();
