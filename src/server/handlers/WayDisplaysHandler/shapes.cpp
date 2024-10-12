#pragma once

#include "common/logger.hpp"
#include "common/shapes.hpp"

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
  std::map<string, DisplaySettable> displays;
  // vector<string> displays;
  // vector<string> order;
  // vector<string> disabled;
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

    for (YAML::Node matcher : node["DISPLAYS"]) {
      string name;
      DisplaySettable settable;
      if (matcher.IsScalar()) {
        name = matcher.as<string>();
      } else {
        name = matcher["NAME"].as<string>();
        settable = matcher.as<DisplaySettable>();
        settable.show();
      }
      rhs.displays[name] = settable;
    }

    return true;
  }
};

template <> struct convert<Config> {
  static bool decode(const Node &node, Config &rhs) {
    YAML::Node profiles;
    try {
      profiles = node["PROFILES"];
    } catch (YAML::Exception) {
      return false;
    }
    if (!profiles.IsMap()) {
      return false;
    }

    for (YAML::const_iterator it = profiles.begin(); it != profiles.end(); it++) {
      std::string profile_name = it->first.as<std::string>();
      YAML::Node yaml_profile = it->second;
      Profile profile;
      try {
        profile = yaml_profile.as<Profile>();
      } catch (YAML::Exception e) {
        log_warn("Profile \"{}\" is malformed, ignoring: {}", profile_name, e.what());
        continue;
      }
      profile.name = profile_name;
      rhs.profiles.push_back(profile);
    }

    return true;
  }
};
} // namespace YAML
