#pragma once

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

using string = std::string;
template <class T> using vector = std::vector<T>;

enum Arrange {
  ROW,
  COLUMN,
};

enum Align {
  TOP_OR_LEFT,
  MIDDLE,
  BOTTOM_OR_RIGHT,
};

struct Profile {
  string name;
  Arrange arrange;
  Align align;
  vector<string> displays;
  vector<string> order;
  vector<string> disabled;
};

struct Config {
  vector<Profile> profiles;
};

namespace YAML {
template <> struct convert<Profile> {
  static bool decode(const Node &node, Profile &rhs) {
    if (!node["ARRANGE"].IsScalar()) {
      return false;
    }
    string arrange = node["ARRANGE"].as<string>();
    if (arrange == "ROW") {
      rhs.arrange = ROW;
    } else if (arrange == "COLUMN") {
      rhs.arrange = COLUMN;
    } else {
      return false;
    }

    if (!node["ALIGN"].IsScalar()) {
      return false;
    }
    string align = node["ALIGN"].as<string>();
    if (align == "TOP" || align == "LEFT") {
      rhs.align = TOP_OR_LEFT;
    } else if (align == "MIDDLE") {
      rhs.align = MIDDLE;
    } else if (align == "BOTTOM" || align == "RIGHT") {
      rhs.align = BOTTOM_OR_RIGHT;
    } else {
      return false;
    }

    if (!node["DISPLAYS"]) {
      rhs.order = vector<string>();
    } else {
      rhs.displays = node["DISPLAYS"].as<vector<string>>();
    }

    if (!node["ORDER"]) {
      rhs.order = vector<string>();
    } else {
      rhs.order = node["ORDER"].as<vector<string>>();
    }

    if (!node["DISABLED"]) {
      rhs.disabled = vector<string>();
    } else {
      rhs.disabled = node["DISABLED"].as<vector<string>>();
    }

    return true;
  }
};

template <> struct convert<Config> {
  static bool decode(const Node &node, Config &rhs) {
    YAML::Node profiles = node["PROFILES"];
    if (!profiles.IsMap()) {
      return false;
    }

    for (YAML::const_iterator it = profiles.begin(); it != profiles.end(); it++) {
      std::string profile_name = it->first.as<std::string>();
      YAML::Node yaml_profile = it->second;
      Profile profile = yaml_profile.as<Profile>();
      profile.name = profile_name;
      rhs.profiles.push_back(profile);
    }

    return true;
  }
};
} // namespace YAML
