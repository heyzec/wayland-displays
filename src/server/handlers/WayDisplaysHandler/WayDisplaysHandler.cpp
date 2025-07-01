#include "common/logger.hpp"
#include "common/shapes.hpp"
#include "server/handlers/BaseHandler.cpp"
#include "server/handlers/WayDisplaysHandler/shapes.cpp"
#include "server/handlers/WayDisplaysHandler/utils.hpp"

#include <string>

using string = std::string;
template <class T> using vector = std::vector<T>;

/**
 * Removes displays meant to be disabled from map of displays
 */
static std::pair<std::map<string, DisplaySettable>, vector<string>>
sort_by_is_enabled(std::map<string, DisplaySettable> displays) {
  vector<string> disabled;
  for (auto it = displays.begin(); it != displays.end();) {
    auto [name, setting] = *it;
    if (!setting.enabled.value()) {
      disabled.push_back(name);
      it = displays.erase(it);
    }
  }
  std::map<string, DisplaySettable> enabled = displays;
  return std::make_pair(enabled, disabled);
}

/**
 * Reorder display_names based on assignments.
 * Unmatched displays are placed last.
 */
static vector<string> order_display_names(const vector<string> display_names,
                                          std::map<string, string> assignments) {
  // Because assign_displays now is strict, we can assume that all displays are matched and hence
  // return just the keys of the map In the future, move the unmatched displays to the back
  vector<string> ordered_names;
  for (auto [name, setting] : assignments) {
    ordered_names.push_back(name);
  }
  return ordered_names;
  // Append the remaining heads
  // ordered.reserve(ordered.size() + distance(unordered.begin(), unordered.end()));
  // ordered.insert(ordered.end(), unordered.begin(), unordered.end());
  // return ordered;
}

/**
  Select the best mode.
  Currently based on a naive scoring method that maximises size_x, size_y and rate
 */
static ModeInfo set_mode_for_displays(vector<ModeInfo> modes) {
  ModeInfo best;
  int best_score = 0;
  for (auto mode : modes) {
    int score = mode.size_x * mode.size_y + mode.rate / 1000;
    if (score > best_score) {
      best = mode;
      best_score = score;
    }
  }

  return best;
}

/**
 * For each display, set size_x, size_y and rate (attributes of mode)
 */
static vector<DisplaySettable> set_mode_for_displays(vector<DisplayInfo> heads,
                                                     vector<DisplaySettable> settings) {
  for (int i = 0; i < settings.size(); i++) {
    const DisplayInfo head = heads[i];
    DisplaySettable &setting = settings[i];

    // // Don't override user-defined attributes
    // if (setting.size_x.has_value() || setting.size_y.has_value() || setting.rate.has_value()) {
    //   continue;
    // }

    ModeInfo best_mode = set_mode_for_displays(head.modes);
    setting.size_x = best_mode.size_x;
    setting.size_y = best_mode.size_y;
    setting.rate = best_mode.rate;
  }
  return settings;
}

/**
 * Arrange and align displays.
 * Displays with coordinates set possibly taken out of the flow
 */
static vector<DisplaySettable> arrange_displays(vector<DisplaySettable> displays, Arrange arrange,
                                                Align align) {
  // Follow CSS terminology for main and cross axis

  // Find maximum width/height among all displays along the cross axis
  int size_cross_max = 0;
  for (const auto display : displays) {
    bool is_main_x = (arrange == ROW) ^ (display.transform.value() % 2 == 1);
    int size_cross = is_main_x ? display.size_y.value() : display.size_x.value();
    size_cross_max = size_cross > size_cross_max ? size_cross : size_cross_max;
  }

  int pos_main = 0;
  for (auto &display : displays) {
    bool is_main_x = (arrange == ROW) ^ (display.transform.value() % 2 == 1);
    // 1. Assign coordinate on main axis
    // If main axis set, skip it
    if (is_main_x) {
      if (display.pos_x.has_value()) {
        display.pos_y = 0;
        continue;
      }
    } else {
      if (display.pos_y.has_value()) {
        display.pos_x = 0;
        continue;
      }
    }
    // Otherwise, use calculated value, based on accumulation so far
    if (is_main_x) {
      display.pos_x = pos_main;
      pos_main += display.size_x.value();
    } else {
      display.pos_y = pos_main;
      pos_main += display.size_y.value();
    }

    // 2. Assign coordinate on cross axis
    // If cross axis set (and main axis not set), use this overriding value instead
    if (is_main_x && display.pos_y.has_value() || !is_main_x && display.pos_x.has_value()) {
      continue;
    }
    // Otherwise, use calculated value, based on maximum
    int size_cross = (is_main_x ? display.size_y.value() : display.size_x.value());
    int pos_cross;
    if (align == TOP_OR_LEFT) {
      pos_cross = 0;
    } else if (align == MIDDLE) {
      pos_cross = (size_cross_max - size_cross) / 2;
    } else {
      pos_cross = size_cross_max - size_cross;
    }
    if (is_main_x) {
      display.pos_y = pos_cross;
    } else {
      display.pos_x = pos_cross;
    }
  }

  return displays;
}

class WayDisplaysHandler : BaseHandler {
  vector<DisplayConfig> *generate_changes(Profile profile, vector<DisplayInfo> *heads,
                                          std::map<string, string> assignments) {
    vector<string> enabled_names;
    vector<string> disabled_names;
    std::map<string, std::pair<DisplayInfo, DisplaySettable>>
        map_name_to_pair; // Pair of (current, desired) settings
    for (auto head : *heads) {
      DisplaySettable &setting = profile.displays[assignments[head.name]];
      if (!setting.enabled.has_value()) {
        setting.enabled = true;
      }
      if (setting.enabled.value()) {
        enabled_names.push_back(head.name);
      } else {
        disabled_names.push_back(head.name);
      }
      map_name_to_pair[head.name] = std::make_pair(head, setting);
    }

    // Assert all settings have enable is value
    for (auto [name, pair] : map_name_to_pair) {
      DisplaySettable setting = pair.second;
      if (!setting.enabled.has_value()) {
        log_warn("Enable for %s is null!\n", name.c_str());
      }
    }

    // 2. Reorder displays based on order of matches in profile
    vector<DisplayInfo> enabled_info;
    vector<DisplaySettable> enabled_settings;
    for (auto [pattern, _] : profile.displays) {
      // find the assignment with the pattern
      for (auto [name, setting] : map_name_to_pair) {
        // TODO: Avoid double loop, can we convert assignment from map to list?
        if (pattern == assignments[name]) {
          if (!setting.second.enabled.value()) {
            continue;
          }
          enabled_info.push_back(setting.first);
          enabled_settings.push_back(setting.second);
        }
      }
      // auto [info, setting] = map_name_to_pair[name];
      // printf("Going to opt access %s\n", name.c_str());
      // if (setting.enabled.value()) {
      //   enabled_info.push_back(info);
      //   enabled_settings.push_back(setting);
      // }
      // printf("Done opt access %s\n", name.c_str());
    }

    // 1. Apply defaults setting for transform, scale
    for (DisplaySettable &setting : enabled_settings) {
      if (!setting.scale.has_value()) {
        setting.scale = 1.0;
      }
      if (!setting.transform.has_value()) {
        setting.transform = 0;
      }
    }

    // 3) Determine the mode for each display
    enabled_settings = set_mode_for_displays(enabled_info, enabled_settings);

    // Ensure all settingsd, the size_x, size_y and rate are set
    for (DisplaySettable setting : enabled_settings) {
      if (!setting.size_x.has_value()) {
        printf("Setting for size_x does not have size_x value!!!!!!\n");
      }
      if (!setting.size_y.has_value()) {
        printf("Setting for size_y does not have size_y value!!!!!!\n");
      }
      if (!setting.rate.has_value()) {
        printf("Setting for rate does not have rate value!!!!!!\n");
      }
    }

    // 3) Determine the position for each display
    enabled_settings = arrange_displays(enabled_settings, profile.arrange, profile.align);

    // 3) Encode the changes to output format
    vector<DisplayConfig> *changes = new vector<DisplayConfig>();
    changes->reserve(heads->size());
    // for (DisplaySettable display : enabled_settings) {
    for (int i = 0; i < enabled_settings.size(); i++) {
      DisplayInfo info = enabled_info[i];
      DisplaySettable setting = enabled_settings[i];
      DisplayConfig change = {.name = info.name,
                              .enabled = true,
                              .pos_x = setting.pos_x.value(),
                              .pos_y = setting.pos_y.value(),
                              .scale = setting.scale.value(),
                              .transform = setting.transform.value(),
                              .size_x = setting.size_x.value(),
                              .size_y = setting.size_y.value(),
                              .rate = setting.rate.value()};
      changes->push_back(change);
    }
    for (string name : disabled_names) {
      DisplayConfig change = {
          .name = strdup(name.c_str()),
          .enabled = false,
      };
      changes->push_back(change);
    }

    return changes;
  }

public:
  vector<DisplayConfig> *handle_change(vector<DisplayInfo> *heads,
                                       std::optional<Config> config) override {
    if (!config.has_value()) {
      return new vector<DisplayConfig>();
    }
    auto match = find_matching_profile(config->profiles, *heads);
    if (!match.has_value()) {
      return new vector<DisplayConfig>();
    }
    Profile profile = match->first;
    std::map<string, string> assignments = match->second;
    log_info("Matched profile: {}", profile.name);
    return generate_changes(profile, heads, assignments);
  }

  vector<DisplayConfig> *handle_command(string command, string param, vector<DisplayInfo> *heads,
                                        std::optional<Config> config) override {
    if (!config.has_value()) {
      return new vector<DisplayConfig>();
    }
    // Only "switch" command supported
    if (command != "switch") {
      return new vector<DisplayConfig>();
    }

    Profile profile = get_profile_by_name(config->profiles, param);
    // TODO: Don't use hacky way to test for non-match
    if (profile.name == "") {
      return new vector<DisplayConfig>();
    }
    return new vector<DisplayConfig>();
    // TODO: Generate an assignment mapping with a method that is not as strict
    // log_info("Switching to profile: {}", profile.name);
    // return generate_changes(profile, heads);
  }
};
