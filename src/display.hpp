#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

using string = std::string;
using json = nlohmann::ordered_json;

struct DisplayConfig {
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

  /* For easy debugging */
  void show();
};

struct DisplayInfo : DisplayConfig {
  char *description;

  int phy_x;
  int phy_y;

  /* For easy debugging */
  void show();
};

typedef std::vector<DisplayConfig> DisplayConfigVector;
typedef std::vector<DisplayInfo> DisplayInfoVector;

// void to_json(json &j, const Mode &m);
// void to_json(json &j, const Display &d);
