#pragma once

#include "wlr-output-management-unstable-v1.h"

#include "vector"

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

struct HeadDyanamicInfo {
  char *name;
  bool enabled;
  int pos_x;
  int pos_y;
  float scale;
  int transform;

  // Properties of the current mode
  int size_x;
  int size_y;
  int rate;
};

struct HeadAllInfo : HeadDyanamicInfo {
  char *description;

  int phy_x;
  int phy_y;
};

struct Head {
  zwlr_output_head_v1 *wlr_head;
  std::vector<Mode> modes;
  HeadAllInfo info;
};

typedef HeadAllInfo Display;
// typedef HeadDyanamicInfo Display;
