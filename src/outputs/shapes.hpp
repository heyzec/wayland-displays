#pragma once

#include "display.hpp"

#include "wlr-output-management-unstable-v1.h"

#include "vector"

// Forward declarations
struct WlrState;
struct Head;
struct Mode;

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

  void (*on_done)(std::vector<DisplayInfo> displays);

  std::vector<Head *> heads;
};

/* Container for zwlr_output_head_v1 */
struct Head {
  WlrState *state;
  zwlr_output_head_v1 *wlr_head;

  std::vector<Mode *> modes;
  DisplayInfo info;
};

/* Container for zwlr_output_mode_v1 */
struct Mode {
  Head *head;
  zwlr_output_mode_v1 *wlr_mode;
  ModeInfo info;
};
