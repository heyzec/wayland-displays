/*
 * Handlers and listeners for zwlr_output_mode
 * https://wayland.app/protocols/wlr-output-management-unstable-v1#zwlr_output_mode_v1
 */
#include "wlr_output/mode.hpp"
#include "display.hpp"
#include "wlr-output-management-unstable-v1.h"
#include <cstdio>


static void size(void *data, struct zwlr_output_mode_v1 *wl_mode, const int32_t width, const int32_t height) {
  auto *mode = (struct Mode *) data;
  mode->width = width;
  mode->height = height;
}

static void refresh(void *data, struct zwlr_output_mode_v1 *wl_mode, const int32_t refresh) {
  auto *mode = (struct Mode *) data;
  mode->refresh = refresh;
}

static void preferred(void *data, struct zwlr_output_mode_v1 *wl_mode) {
  auto *mode = (struct Mode *) data;
  mode->preferred = true;
}

static void finished(void *data, struct zwlr_output_mode_v1 *mode) {
  printf("Finished\n");
}

static const struct zwlr_output_mode_v1_listener mode_listener = {
  .size = size,
  .refresh = refresh,
  .preferred = preferred,
  .finished = finished,
};

const struct zwlr_output_mode_v1_listener *get_mode_listener() {
  return &mode_listener;
}
