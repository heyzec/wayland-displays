/*
 * Handlers and listeners for zwlr_output_head
 * https://wayland.app/protocols/wlr-output-management-unstable-v1#zwlr_output_head_v1
 */
#include "wlr_output/head.hpp"
#include "display.hpp"
#include "utils/fixed24_8.hpp"
#include "wlr-output-management-unstable-v1.h"
#include "wlr_output/mode.hpp"
#include "wlr_output/shapes.hpp"
#include <wayland-client-protocol.h>

static void name(void *data, struct zwlr_output_head_v1 *wlr_head, const char *name) {
  auto head = (Head *)data;
  head->info.name = strdup(name);
}

static void description(void *data, struct zwlr_output_head_v1 *wlr_head, const char *description) {
  auto head = (Head *)data;
  head->info.description = strdup(description);
}

static void physical_size(void *data, struct zwlr_output_head_v1 *wlr_head, const int32_t width,
                          const int32_t height) {
  auto head = (Head *)data;
  head->info.phy_x = width;
  head->info.phy_y = height;
}

static void mode(void *data, struct zwlr_output_head_v1 *wlr_head, zwlr_output_mode_v1 *wlr_mode) {
  // We need to store the zwlr_output_mode_v1 objects in order to determine which mode is current
  auto head = (Head *)data;
  int index = head->modes.size();
  Mode mode = Mode{};
  mode.wlr_mode = wlr_mode;
  head->modes.push_back(mode);
  zwlr_output_mode_v1_add_listener(wlr_mode, get_mode_listener(), &head->modes.at(index));
}

static void enabled(void *data, struct zwlr_output_head_v1 *wlr_head, const int32_t enabled) {
  auto head = (Head *)data;
  head->info.enabled = enabled;
}

static void current_mode(void *data, struct zwlr_output_head_v1 *wlr_head,
                         zwlr_output_mode_v1 *wlr_mode) {
  // TODO: This function relies on the mode events occuring before the current_mode event,
  // which the protocol actually does not specify as a requirement!
  auto head = (Head *)data;
  for (Mode mode : head->modes) {
    if (mode.wlr_mode == wlr_mode) {
      head->info.size_x = mode.size_x;
      head->info.size_y = mode.size_y;
      head->info.rate = mode.rate;
      break;
    }
  }
}

static void position(void *data, struct zwlr_output_head_v1 *wlr_head, const int32_t x,
                     const int32_t y) {
  auto head = (Head *)data;
  head->info.pos_x = x;
  head->info.pos_y = y;
}

static void transform(void *data, struct zwlr_output_head_v1 *wlr_head, const int32_t transform) {
  auto head = (Head *)data;
  head->info.transform = transform;
}

static void scale(void *data, struct zwlr_output_head_v1 *wlr_head, const fixed24_8 scale) {
  auto head = (Head *)data;
  head->info.scale = fixed_to_float(scale);
  printf("Scale %f\n", fixed_to_float(scale));
}

static void make(void *data, struct zwlr_output_head_v1 *head, const char *make) {
  // auto *display = (struct Display *) data;
  // display->make = make;
}

static void model(void *data, struct zwlr_output_head_v1 *head, const char *model) {
  // auto *display = (struct Display *) data;
  // display->model = model;
}

static void serial_number(void *data, struct zwlr_output_head_v1 *head, const char *serial_number) {
  // auto *display = (struct Display *) data;
  // display->serial_number = serial_number;
}

static void adaptive_sync(void *data, struct zwlr_output_head_v1 *head, uint adaptive_sync) {
  printf("Adaptive Sync: %d\n", adaptive_sync);
}

static const struct zwlr_output_head_v1_listener head_listener = {
    .name = name,
    .description = description,
    .physical_size = physical_size,
    .mode = mode,
    .enabled = enabled,
    .current_mode = current_mode,
    .position = position,
    .transform = transform,
    .scale = scale,
    .make = make,
    .model = model,
    .serial_number = serial_number,
    .adaptive_sync = adaptive_sync,
};

const struct zwlr_output_head_v1_listener *get_head_listener() {
  return &head_listener;
}
