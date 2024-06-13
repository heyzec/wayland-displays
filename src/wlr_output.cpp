#pragma once

#include "utils/fixed24_8.hpp"
#include "wlr_output/config.hpp"
#include "wlr_output/head.hpp"
#include "wlr_output/shapes.hpp"

#include "wlr-output-management-unstable-v1.h"
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-util.h>

#include <cstring>
#include <iostream>
#include <ostream>
#include <stdio.h>
#include <vector>

struct WlrHead {
  struct zwlr_output_head_v1 *head;
  struct wl_list link;
};

/* State required for us to get and set display configs via the protocol */
struct WlrState {
  struct wl_display *display;
  struct zwlr_output_manager_v1 *manager;
  uint32_t serial;

  struct std::vector<Head> heads;
};

WlrState *state = new WlrState{};

// ============================================================
// Handlers and listeners for zwlr_output_manager
// ============================================================

static void head(void *data, struct zwlr_output_manager_v1 *manager,
                 struct zwlr_output_head_v1 *wlr_head) {
  printf("==Head==\n");
  auto state = (WlrState *)data;

  int index = state->heads.size();
  Head head = Head{};
  head.wlr_head = wlr_head;
  state->heads.push_back(head);

  // int index = displays->size();
  // displays->push_back(Display{});
  zwlr_output_head_v1_add_listener(wlr_head, get_head_listener(), &state->heads.at(index));
}

static void done(void *data, struct zwlr_output_manager_v1 *manager, uint32_t serial) {
  printf("==Done==\n");
  state->serial = serial;
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
static void global(void *data, struct wl_registry *registry, uint32_t name, const char *interface,
                   uint32_t version) {
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
// Setup and teardown of connection to compositor
// ============================================================

void wlr_output_init() {
  // Connect to compositor and get the Wayland display singleton
  wl_display *display = wl_display_connect(NULL);
  if (!display) {
    fprintf(stderr, "Failed to connect to Wayland display.\n");
    exit(1);
  }
  state->display = display;

  // Get the global registry singleton
  struct wl_registry *registry = wl_display_get_registry(display);
  // Bind a listener to the registry
  wl_registry_add_listener(registry, &registry_listener, NULL);

  // TODO: Binding the manager should be done conditional to whether compositor supports this
  // protocol Consider moving this to listener (code structure confusing?) or do a roundtrip first
  // TODO: Don't hardcode name and version
  struct zwlr_output_manager_v1 *manager = (zwlr_output_manager_v1 *)wl_registry_bind(
      registry, 20, &zwlr_output_manager_v1_interface, 4);
  state->manager = manager;
  zwlr_output_manager_v1_add_listener(manager, &manager_listener, state);

  // Block until all pending requests/events are sent/received and all listeners executed:
  // - Handle global events (globals available on this compositor)
  wl_display_roundtrip(display);
}

void wlr_output_deinit() {
  wl_display_disconnect(state->display);
}

// ============================================================
// Wrapper functions for encapsulating wl_display
// ============================================================

int get_wl_display_fd() {
  return wl_display_get_fd(state->display);
}

void prepare_dispatch_events() {
  // See Wayland Protocol docs Appendix B wl_display_prepare_read_queue

  // Prepare to read events from the display's file descriptor
  // This function must be called before using wl_display_read_events()
  // It ensures in the meantime no other thread will read from the file descriptor
  while (wl_display_prepare_read(state->display) != 0) {
    // Dispatch default queue events without reading from the display fd
    wl_display_dispatch_pending(state->display);
  }
  // Send all buffered requests on the display to the server
  wl_display_flush(state->display);
}

void dispatch_events() {
  // See Wayland Protocol docs Appendix B wl_display_prepare_read_queue

  // Read events from display file descriptor
  wl_display_read_events(state->display);
  // Dispatch default queue events without reading from the display fd
  wl_display_dispatch_pending(state->display);
}

void cancel_dispatch_events() {
  // See Wayland Protocol docs Appendix B wl_display_prepare_read_queue

  // Cancel read intention on display's fd
  // If the threads do not follow this rule it will lead to deadlock
  wl_display_cancel_read(state->display);
}

/* Momentarily connect to compositor to get display info */
std::vector<HeadAllInfo> get_displays() {
  auto displays = std::vector<HeadAllInfo>{};
  wlr_output_init();

  for (auto head : state->heads) {
    displays.push_back(head.info);
  }

  // wlr_output_cleanup();
  return displays;
}

void apply_configurations(std::vector<HeadDyanamicInfo> configs) {
  if (state->display == nullptr || state->manager == nullptr) {
    printf("wl_display or zwlr_output_manager is null!");
    return;
  }

  struct zwlr_output_configuration_v1 *zwlr_config =
      zwlr_output_manager_v1_create_configuration(state->manager, state->serial);
  zwlr_output_configuration_v1_add_listener(zwlr_config, get_config_listener(), state->display);

  for (HeadDyanamicInfo config : configs) {
    for (Head head : state->heads) {
      if (config.name == head.info.name) {
        if (!config.enabled) {
          zwlr_output_configuration_v1_disable_head(zwlr_config, head.wlr_head);
          break;
        }

        zwlr_output_configuration_head_v1 *config_head =
            zwlr_output_configuration_v1_enable_head(zwlr_config, head.wlr_head);

        zwlr_output_configuration_head_v1_set_position(config_head, config.pos_x, config.pos_y);
        zwlr_output_configuration_head_v1_set_custom_mode(config_head, config.size_x, config.size_y,
                                                          (int)config.rate);
        zwlr_output_configuration_head_v1_set_scale(config_head, float_to_fixed(config.scale));
        zwlr_output_configuration_head_v1_set_transform(config_head, config.transform);

        printf("Setting %s: Position (%d,%d) Size %dx%d Scale %f Rate %d Transform %d\n",
               config.name, config.pos_x, config.pos_y, config.size_x, config.size_y, config.scale,
               config.rate, config.transform);
        break;
      }
    }
  }

  zwlr_output_configuration_v1_apply(zwlr_config);

  // We may lose some events here
  wl_display_roundtrip(state->display);

  // TODO: Return success code
}
