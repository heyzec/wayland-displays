#include "outputs/outputs.hpp"
#include "outputs/config.hpp"
#include "outputs/head.hpp"
#include "outputs/shapes.hpp"

#include "server/handlers/DefaultHandler.cpp"
#include "utils/fixed24_8.hpp"
#include "utils/time.cpp"

#include "wlr-output-management-unstable-v1.h"
#include <unistd.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-util.h>

#include <cstring>
#include <iostream>
#include <ostream>
#include <stdio.h>
#include <vector>

using std::vector;

struct WlrHead {
  struct zwlr_output_head_v1 *head;
  struct wl_list link;
};

/* State required for us to get and set display configs via the protocol */
struct WlrState {
  struct wl_display *display;
  struct zwlr_output_manager_v1 *manager;
  uint32_t serial;

  /* True iff we initiated a zwlr_output_configuration_v1::apply() request and are waiting for the
   * next zwlr_output_manager_v1::done() event.
   * To keep track of events so that we do not respond to config changes initiated by us
   * (infinite loop).
   */
  bool is_updating = false;
  long long last_updated = 0;
  /* Keep track of how many updates that occured too fast */
  int n_bursty_update = 0;

  std::vector<Head *> heads;
};

WlrState *state = new WlrState{};

// ============================================================
// Handlers and listeners for zwlr_output_manager
// ============================================================

static void head(void *data, struct zwlr_output_manager_v1 *manager,
                 struct zwlr_output_head_v1 *wlr_head) {
  auto state = (WlrState *)data;

  int index = state->heads.size();
  Head *head = new Head{};
  head->wlr_head = wlr_head;
  state->heads.push_back(head);

  zwlr_output_head_v1_add_listener(wlr_head, get_head_listener(), (Head *)state->heads.at(index));
}

static void done(void *data, struct zwlr_output_manager_v1 *manager, uint32_t serial) {
  printf("==Done==\n");
  state->serial = serial;
  if (state->is_updating) {
    // We ignore this event as it is caused by us requesting a config change previously
    state->is_updating = false;
    return;
  }
  state->is_updating = false;

  auto displays = get_head_infos();
  // Call the default handler
  auto handler = DefaultHandler();
  vector<HeadDyanamicInfo> *changes = handler.handle(&displays);
  if (changes != nullptr) {
    // TODO: Sleep for a short time since there can be multiple DONE events, e.g.
    // another display outputs manager is setting heads too
    // But we need to retrieve the new serials too, else our request will be invalid.
    usleep(200);
    apply_configurations(*changes);
  }
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

// ============================================================
// Main(?) functions
// ============================================================

/* Get the current configuration of all displays */
std::vector<HeadAllInfo> get_head_infos() {
  auto displays = std::vector<HeadAllInfo>{};
  for (auto head : state->heads) {
    displays.push_back(head->info);
  }
  return displays;
}

/* Momentarily connect to compositor to get display info */
std::vector<HeadAllInfo> get_displays() {
  wlr_output_init();
  auto displays = get_head_infos();
  // TODO: GUI shouldn't rely on this function
  // wlr_output_deinit();
  return displays;
}

void apply_configurations(std::vector<HeadDyanamicInfo> configs) {
  printf("Apply new configuration changes...\n");

  if (state->display == nullptr || state->manager == nullptr) {
    printf("wl_display or zwlr_output_manager is null!");
    return;
  }

  struct zwlr_output_configuration_v1 *zwlr_config =
      zwlr_output_manager_v1_create_configuration(state->manager, state->serial);
  zwlr_output_configuration_v1_add_listener(zwlr_config, get_config_listener(), state->display);

  for (HeadDyanamicInfo config : configs) {
    for (Head *head : state->heads) {
      if (config.name == head->info.name) {
        if (!config.enabled) {
          zwlr_output_configuration_v1_disable_head(zwlr_config, head->wlr_head);
          break;
        }

        zwlr_output_configuration_head_v1 *config_head =
            zwlr_output_configuration_v1_enable_head(zwlr_config, head->wlr_head);

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

  state->is_updating = true;

  // Safety mechanism
  if ((time_in_ms() - state->last_updated) < 2000) {
    if (state->n_bursty_update > 3) {
      printf("Changes being applied too quickly, exiting to not break stuff...\n");
      // TODO: Proper cleanup
      exit(1);
    }
    state->n_bursty_update += 1;
  } else {
    state->n_bursty_update = 0;
  }
  state->last_updated = time_in_ms();

  // We may lose some events here
  wl_display_roundtrip(state->display);

  // TODO: Return success code
}
