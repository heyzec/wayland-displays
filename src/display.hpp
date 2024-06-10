#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

using string = std::string;
using json = nlohmann::ordered_json;

/* Output mode advertised by the display. Based on zwlr_output_mode_v1. */
class Mode {
public:
  /* Width of the mode in hardware units */
  int width;
  /* Height of the mode in hardware units */
  int height;
  /* Vertical refresh rate in mHz */
  int refresh;
  bool preferred;
};
void to_json(json &j, const Mode &m);


/* Based on zwlr_output_mode_v1. */
class Display {
public:
  string name;
  string description;

  bool enabled;
  int width;
  int height;
  int pos_x;
  int pos_y;

  std::vector<Mode> modes;

  string make;
  string model;
  string serial_number;
};
void to_json(json &j, const Display &d);
