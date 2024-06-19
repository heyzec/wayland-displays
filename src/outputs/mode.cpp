/*
 * Handlers and listeners for zwlr_output_mode
 * https://wayland.app/protocols/wlr-output-management-unstable-v1#zwlr_output_mode_v1
 */
#include "outputs/mode.hpp"
#include "outputs/shapes.hpp"

#include "wlr-output-management-unstable-v1.h"

static void size(void *data, struct zwlr_output_mode_v1 *wlr_mode, const int32_t width,
                 const int32_t height) {
  auto *mode = (struct Mode *)data;
  mode->info.size_x = width;
  mode->info.size_y = height;
}

static void refresh(void *data, struct zwlr_output_mode_v1 *wlr_mode, const int32_t refresh) {
  auto *mode = (struct Mode *)data;
  mode->info.rate = refresh;
}

static void preferred(void *data, struct zwlr_output_mode_v1 *wlr_mode) {
  auto *mode = (struct Mode *)data;
  mode->info.preferred = true;
}

static void finished(void *data, struct zwlr_output_mode_v1 *wlr_mode) {
  auto mode = (Mode *)data;
  auto modes = &mode->head->modes;

  modes->erase(std::remove(modes->begin(), modes->end(), mode), modes->end());
  free(mode);
  // Protocol requires us to send a destroy request to release resources
  zwlr_output_mode_v1_destroy(wlr_mode);
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
