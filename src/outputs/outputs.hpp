#include "display.hpp"
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

std::vector<DisplayInfo> get_head_infos();

std::vector<DisplayInfo> get_displays();

void attach_on_done(void (*on_done)(std::vector<DisplayInfo>));

void apply_configurations(std::vector<DisplayConfig> configs);
