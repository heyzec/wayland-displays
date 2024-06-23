#include "server/handlers/WayDisplaysHandler/shapes.cpp"

#include "server/handlers/BaseHandler.cpp"
#include <string>

using string = std::string;
template <class T> using vector = std::vector<T>;

// ============================================================
// Utilities
// ============================================================

static bool does_display_match(const DisplayInfo display, const string pattern) {
  return display.name == pattern or display.description == pattern;
}

/**
 * Find display in vector that is matched fully by name or description.
 */
static int find_display(const vector<DisplayInfo> displays, const string pattern) {
  for (int i = 0; i < displays.size(); i++) {
    auto display = displays.at(i);
    if (does_display_match(display, pattern)) {
      return i;
    }
  }
  return -1;
}

static std::pair<vector<DisplayInfo>, vector<DisplayInfo>>
match_displays(const vector<DisplayInfo> displays, const vector<string> patterns) {
  vector<DisplayInfo> unmatched = displays;
  vector<DisplayInfo> matched;

  for (auto pattern : patterns) {
    int index = find_display(unmatched, pattern);
    if (index != -1) {
      matched.push_back(unmatched.at(index));
      unmatched.erase(unmatched.begin() + index);
    }
  }

  return std::pair(matched, unmatched);
}

/**
 * Finds and returns a subset of displays from the given vector that match the provided patterns.
 * Each display can only be matched once.
 * If no match is found for a pattern, an empty vector is returned.
 */
vector<DisplayInfo> find_displays(const vector<DisplayInfo> displays,
                                  const vector<string> patterns) {
  auto [matched, unmatched] = match_displays(displays, patterns);
  return matched;
}

static bool does_profile_match(const vector<DisplayInfo> displays, const Profile profile) {
  auto [matched, unmatched] = match_displays(displays, profile.displays);
  return unmatched.size() == 0;
}

// ============================================================
// Helpers
// ============================================================

static Profile find_matching_profile(const vector<DisplayInfo> displays,
                                     const vector<Profile> profiles) {
  Profile matched;
  for (Profile profile : profiles) {
    if (does_profile_match(displays, profile)) {
      matched = profile;
      break;
    }
  }

  return matched;
}

/**
 * Removes displays meant to be disabled from original and place then in the disabled vector.
 * Each display is matched fully by name or description.
 */
static void filter_disabled(vector<DisplayInfo> *original, vector<DisplayInfo> *disabled,
                            const vector<string> patterns) {
  for (auto pattern : patterns) {
    int index = find_display(*original, pattern);
    if (index != -1) {
      disabled->push_back(original->at(index));
      original->erase(original->begin() + index);
      break;
    }
  }
}

/* Create a new vector of displays from the original vector, based on order.
 * Each display is matched fully by name or description.
 * Unmatched displays are placed last.
 */
static vector<DisplayInfo> order_displays(const vector<DisplayInfo> displays,
                                          vector<string> patterns) {
  auto [ordered, unordered] = match_displays(displays, patterns);

  // Append the remaining heads
  ordered.reserve(ordered.size() + distance(unordered.begin(), unordered.end()));
  ordered.insert(ordered.end(), unordered.begin(), unordered.end());
  return ordered;
}

/* For each display, select the best mode based on a naive scoring method */
static vector<DisplayInfo> set_mode_for_displays(vector<DisplayInfo> displays) {
  for (auto &display : displays) {
    ModeInfo best;
    int best_score = 0;
    for (auto mode : display.modes) {
      int score = mode.size_x * mode.size_y + mode.rate / 1000;
      if (score > best_score) {
        best = mode;
        best_score = score;
      }
    }
    display.size_x = best.size_x;
    display.size_y = best.size_y;
    display.rate = best.rate;
  }
  return displays;
}

/* Arrange and align displays. */
static vector<DisplayInfo> arrange_displays(vector<DisplayInfo> displays, Arrange arrange,
                                            Align align) {
  // Follow CSS terminology for main and cross axis

  // Find maximum width/height among all displays along the cross axis
  int size_cross_max = 0;
  for (const auto display : displays) {
    int size_cross = arrange == ROW ? display.size_y : display.size_x;
    size_cross_max = size_cross > size_cross_max ? size_cross : size_cross_max;
  }

  int pos_main = 0;
  for (auto &display : displays) {
    // 1. Assign coordinate on main axis
    if (arrange == ROW) {
      display.pos_x = pos_main;
      pos_main += display.size_x;
    } else {
      display.pos_y = pos_main;
      pos_main += display.size_y;
    }

    // 2. Assign coordinate on cross axis
    int size_cross = (arrange == ROW ? display.size_y : display.size_x);
    int pos_cross;
    if (align == TOP_OR_LEFT) {
      pos_cross = 0;
    } else if (align == MIDDLE) {
      pos_cross = (size_cross_max - size_cross) / 2;
    } else {
      pos_cross = size_cross_max - size_cross;
    }
    if (arrange == ROW) {
      display.pos_y = pos_cross;
    } else {
      display.pos_x = pos_cross;
    }
  }

  return displays;
}

class WayDisplaysHandler : BaseHandler {
public:
  vector<DisplayConfig> *handle(vector<DisplayInfo> *heads, YAML::Node node) override {
    Config config = node.as<Config>();

    Profile profile = find_matching_profile(*heads, config.profiles);
    // TODO: Don't use hacky way to test for non-match
    if (profile.name == "") {
      return new vector<DisplayConfig>();
    }
    printf("Matched profile: %s\n", profile.name.c_str());

    vector<DisplayInfo> enabled = *heads;
    vector<DisplayInfo> disabled;

    // 1) Determine whether to enable or disable each display
    filter_disabled(&enabled, &disabled, profile.disabled);

    // 2) Determine the mode for each display
    enabled = set_mode_for_displays(enabled);

    // 3) Determine the position for each display
    enabled = order_displays(enabled, profile.order);
    enabled = arrange_displays(enabled, profile.arrange, profile.align);

    vector<DisplayConfig> *changes = new vector<DisplayConfig>();
    changes->reserve(heads->size());
    for (DisplayConfig display : enabled) {
      display.enabled = true;
      // Hardcode to 1.0 for now
      display.scale = 1.0;
      changes->push_back(display);
    }
    for (DisplayConfig display : disabled) {
      display.enabled = false;
      changes->push_back(display);
    }

    return changes;
  }
};
