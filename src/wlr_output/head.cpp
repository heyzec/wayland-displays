/*
 * Handlers and listeners for zwlr_output_head
 * https://wayland.app/protocols/wlr-output-management-unstable-v1#zwlr_output_head_v1
 */
#include "wlr_output/head.hpp"
#include "wlr_output/mode.hpp"
#include "display.hpp"
#include "wlr-output-management-unstable-v1.h"

typedef int32_t fixed24_8;

static void name(void *data, struct zwlr_output_head_v1 *head, const char *name) {
  auto *display = (struct Display *) data;
  display->name = name;
}

static void description(void *data, struct zwlr_output_head_v1 *head, const char *description) {
  auto *display = (struct Display *) data;
  display->description = description;
}

static void physical_size(void *data, struct zwlr_output_head_v1 *head, const int32_t width,
                          const int32_t height) {
  auto *display = (struct Display *) data;
  display->width = width;
  display->height = height;
}

static void mode(void *data, struct zwlr_output_head_v1 *head, zwlr_output_mode_v1 *wl_mode) {
  auto *display = (struct Display *) data;
  int index = display->modes.size();
  display->modes.push_back(Mode{});
  zwlr_output_mode_v1_add_listener(wl_mode, get_mode_listener(), &display->modes.at(index));
}

static void enabled(void *data, struct zwlr_output_head_v1 *head, const int32_t enabled) {
  auto *display = (struct Display *) data;
  display->enabled = enabled;
}

static void current_mode(void *data, struct zwlr_output_head_v1 *head, zwlr_output_mode_v1 *mode) {
  // TODO: We need to store the zwlr_output_mode_v1 objects in order to determine which mode is current
  // zwlr_output_mode_v1_add_listener(mode, &mode_listener, NULL);
}

static void position(void *data, struct zwlr_output_head_v1 *head, const int32_t x, const int32_t y) {
  auto *display = (struct Display *) data;
  display->pos_x = x;
  display->pos_y = y;
}

static void transform(void *data, struct zwlr_output_head_v1 *head, const int32_t transform) {
  printf("Transform: %d\n", transform);
}

static void scale(void *data, struct zwlr_output_head_v1 *head, const fixed24_8 scale) {
  printf("Scale: %d\n", scale);
}

static void make(void *data, struct zwlr_output_head_v1 *head, const char *make) {
  auto *display = (struct Display *) data;
  display->make = make;
}

static void model(void *data, struct zwlr_output_head_v1 *head, const char *model) {
  auto *display = (struct Display *) data;
  display->model = model;
}

static void serial_number(void *data, struct zwlr_output_head_v1 *head, const char *serial_number) {
  auto *display = (struct Display *) data;
  display->serial_number = serial_number;
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
