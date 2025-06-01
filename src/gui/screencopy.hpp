#include <string>
#include <vector>

struct ScreencopyFrame {
  const uint id;
  /* Name of output of the source of frame */
  std::string name;
  /* Raw pixel data in XRGB format */
  void *pixels;
  /* Horizontal size of frame in pixels */
  uint width;
  /* Vertical size of frame in pixels */
  uint height;
};

typedef const std::vector<ScreencopyFrame> ScreencopyFrames;

/**
 * Initialize this module (by connecting to the Wayland compositor).
 * This function must be called before other functions in this module.
 */
void screencopy_init();

/**
 * Get a container of frames of all outputs.
 */
ScreencopyFrames screencopy_get();

/**
 * Destroy the container of frames to free memory after use.
 */
void screencopy_destroy(ScreencopyFrames frames);
