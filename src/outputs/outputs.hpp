#include "outputs/shapes.hpp"

#include <vector>

// ============================================================
// Setup and teardown of connection to compositor
// ============================================================

void wlr_output_init();

void wlr_output_deinit();

// ============================================================
// Wrapper functions for encapsulating wl_display
// ============================================================

int get_wl_display_fd();

void prepare_dispatch_events();

void dispatch_events();

void cancel_dispatch_events();

// ============================================================
// Main(?) functions
// ============================================================

std::vector<HeadAllInfo> get_head_infos();

std::vector<HeadAllInfo> get_displays();

void apply_configurations(std::vector<HeadDyanamicInfo> configs);
