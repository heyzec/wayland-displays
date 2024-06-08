#include <cstring>
#include <iostream>
#include <ostream>
#include <vector>
#include <stdio.h>
#include <wayland-client.h>

#include "display.cpp"
#include "wlr-output-management-unstable-v1.h"


typedef int32_t fixed24_8;

// ============================================================
// Handlers and listeners for zwlr_output_mode
// ============================================================

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

// ============================================================
// Handlers and listeners for zwlr_output_head
// ============================================================

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
  zwlr_output_mode_v1_add_listener(wl_mode, &mode_listener, &display->modes.at(index));
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

// ============================================================
// Handlers and listeners for zwlr_output_manager
// ============================================================

static void head(void *data, struct zwlr_output_manager_v1 *manager,
                 struct zwlr_output_head_v1 *head) {
  printf("==Head==\n");
  std::vector<struct Display> *displays = (std::vector<struct Display>*) data;
  int index = displays->size();
  displays->push_back(Display{});
  zwlr_output_head_v1_add_listener(head, &head_listener, &displays->at(index));
}

static void done(void *data, struct zwlr_output_manager_v1 *manager,
                 uint32_t serial) {
  printf("==Done==\n");
}

static void finished(void *data, struct zwlr_output_manager_v1 *manager) {
  printf("==Finished==\n");
}

static const struct zwlr_output_manager_v1_listener manager_listener = {
    .head = head,
    .done = done,
    .finished = finished,
};

// ============================================================
// Handlers and listeners for wl_registry
// ============================================================

// https://wayland.app/protocols/wayland#wl_registry:event:global
static void global(void *data, struct wl_registry *registry, uint32_t name,
                                   const char *interface, uint32_t version) {
  if (strcmp(interface, zwlr_output_manager_v1_interface.name) == 0) {
    printf("interface: '%s', version: %d, name: %d\n", interface, version, name);
  }
}

static void global_remove(void *data, struct wl_registry *registry, uint32_t name) {
  // This space deliberately left blank
}

static const struct wl_registry_listener registry_listener = {
    .global = global,
    .global_remove = global_remove,
};

// ============================================================
// Entry point
// ============================================================

void wlr_output_init(std::vector<struct Display> *displays) {
  // Connect to compositor and get the Wayland display singleton
  struct wl_display *display = wl_display_connect(NULL);
  if (!display) {
    fprintf(stderr, "Failed to connect to Wayland display.\n");
    exit(1);
  }

  // Get the global registry singleton
  struct wl_registry *registry = wl_display_get_registry(display);
  // Bind a listener to the registry
  wl_registry_add_listener(registry, &registry_listener, NULL);

  // TODO: Binding the manager should be done conditional to whether compositor supports this protocol
  // Consider moving this to listener (code structure confusing?) or do a roundtrip first
  // TODO: Don't hardcode name and version
  struct zwlr_output_manager_v1 *manager = (zwlr_output_manager_v1 *)wl_registry_bind(
      registry, 20, &zwlr_output_manager_v1_interface, 4);

  zwlr_output_manager_v1_add_listener(manager, &manager_listener, displays);

  // Block until all pending requests/events are sent/received and all listeners executed:
  // - Handle global events (globals available on this compositor)
  wl_display_roundtrip(display);


  // while (wl_display_dispatch(display) != -1) {
  //   /* This space deliberately left blank */
  // }

  wl_display_disconnect(display);
  return;
}

