#pragma once

#include "wlr_output/head.hpp"
#include "display.hpp"

#include "wlr-output-management-unstable-v1.h"

#include <cstring>
#include <iostream>
#include <ostream>
#include <vector>
#include <stdio.h>
#include <wayland-client.h>



struct wl_display *display;


// ============================================================
// Handlers and listeners for zwlr_output_manager
// ============================================================

static void head(void *data, struct zwlr_output_manager_v1 *manager,
                 struct zwlr_output_head_v1 *head) {
  printf("==Head==\n");
  std::vector<struct Display> *displays = (std::vector<struct Display>*) data;
  int index = displays->size();
  displays->push_back(Display{});
  zwlr_output_head_v1_add_listener(head, get_head_listener(), &displays->at(index));
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
  display = wl_display_connect(NULL);
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
}

void wlr_output_cleanup() {
  wl_display_disconnect(display);
}

/* Momentarily connect to compositor to get display info */
std::vector<struct Display> get_displays() {
  auto displays = std::vector<struct Display>{};
  wlr_output_init(&displays);
  wlr_output_cleanup();
  return displays;
}
