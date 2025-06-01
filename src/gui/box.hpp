#pragma once

#include <string>

/* Scaling factor between compositor pixels and canvas pixels */
#define CANVAS_FAC 0.15

struct Box {
  std::string name;

  float x;
  float y;
  float width;
  float height;

  int transform;

  bool within(float pt_x, float pt_y) {
    return x <= pt_x && pt_x <= x + width && y <= pt_y && pt_y <= y + height;
  }
};
