#pragma once

#include "display.hpp"

#include "wlr-output-management-unstable-v1.h"

#include "vector"
#include <cstdio>

/* Container for zwlr_output_mode_v1 */
struct Mode {
  zwlr_output_mode_v1 *wlr_mode;

  // Attributes
  /* Width of the output in hardware units */
  int size_x;
  /* Height of the output in hardware units */
  int size_y;
  /* Fixed vertical hardware refresh rate in mHz */
  int rate;
};

struct Head {
  zwlr_output_head_v1 *wlr_head;
  std::vector<Mode> modes;
  DisplayInfo info;
};
