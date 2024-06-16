#pragma once

#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

#include <iostream>
#include <string>
#include <vector>

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

namespace YAML {
template <> struct convert<DisplayConfig> {
  static Node encode(const DisplayConfig &rhs) {
    Node node;
    node["NAME"] = rhs.name;
    node["ENABLED"] = rhs.enabled;
    node["POSITION_X"] = rhs.pos_x;
    node["POSITION_Y"] = rhs.pos_y;
    node["SCALE"] = rhs.scale;
    node["TRANSFORM"] = rhs.transform;

    // Properties of the current mode
    node["WIDTH"] = rhs.size_x;
    node["HEIGHT"] = rhs.size_y;
    node["HZ"] = rhs.rate;

    return node;
  }

  static bool decode(const Node &node, DisplayConfig &rhs) {
    rhs.name = strdup(node["NAME"].as<std::string>().c_str());
    rhs.enabled = node["ENABLED"].as<bool>();
    rhs.pos_x = node["POSITION_X"].as<int>();
    rhs.pos_y = node["POSITION_Y"].as<int>();
    rhs.scale = node["SCALE"].as<float>();
    rhs.transform = node["TRANSFORM"].as<int>();

    // Properties of the current mode
    rhs.size_x = node["WIDTH"].as<int>();
    rhs.size_y = node["HEIGHT"].as<int>();
    rhs.rate = node["HZ"].as<int>();

    return true;
  }
};

template <> struct convert<DisplayInfo> {
  static Node encode(const DisplayInfo &rhs) {
    Node node = convert<DisplayConfig>::encode(rhs);
    node["DESCRIPTION"] = rhs.description;
    node["WIDTH_MM"] = rhs.phy_x;
    node["HEIGHT_MM"] = rhs.phy_y;
    return node;
  }

  static bool decode(const Node &node, DisplayInfo &rhs) {
    convert<DisplayConfig>::decode(node, rhs);
    rhs.description = strdup(node["DESCRIPTION"].as<std::string>().c_str());
    rhs.phy_x = node["WIDTH_MM"].as<int>();
    rhs.phy_y = node["HEIGHT_MM"].as<int>();

    return true;
  }
};
} // namespace YAML

// void to_json(json &j, const Mode &m);
// void to_json(json &j, const Display &d);
