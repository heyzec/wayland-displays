#pragma once

struct Box {
  float x;
  float y;
  float width;
  float height;

  bool within(float pt_x, float pt_y) {
    return x <= pt_x && pt_x <= x + width && y <= pt_y && pt_y <= y + height;
  }
};
