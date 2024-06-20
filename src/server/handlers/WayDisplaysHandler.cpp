#include "server/handlers/BaseHandler.cpp"
#include <string>

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

/* Removes displays meant to be disabled from original and place then in the disabled vector.
 * Each display is matched fully by name.
 */
static void filter_disabled(vector<DisplayInfo> *original, vector<DisplayInfo> *disabled,
                            YAML::Node specified) {
  for (auto d : specified) {
    string match = d.as<string>();
    for (int i = 0; i < original->size(); i++) {
      auto head = original->at(i);
      if (match == head.name) {
        disabled->push_back(head);
        original->erase(original->begin() + i);
        break;
      }
    }
  }
}

/* Create a new vector of displays from the original vector, based on order.
 * Each display is matched fully by name or description.
 * Unmatched displays are placed last.
 */
static vector<DisplayInfo> order_displays(vector<DisplayInfo> original, YAML::Node order) {
  vector<DisplayInfo> sorted;

  for (auto o : order) {
    string match = o.as<string>();
    for (int i = 0; i < original.size(); i++) {
      auto head = original.at(i);
      if (match == head.name || match == head.description) {
        sorted.push_back(head);
        original.erase(original.begin() + i);
        break;
      }
    }
  }

  // Append the remaining heads
  sorted.reserve(sorted.size() + distance(original.begin(), original.end()));
  sorted.insert(sorted.end(), original.begin(), original.end());
  return sorted;
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
  vector<DisplayConfig> *handle(vector<DisplayInfo> *heads, YAML::Node config) override {
    vector<DisplayInfo> enabled = *heads;
    vector<DisplayInfo> disabled;

    // 1) Determine whether to enable or disable each display
    if (config["DISABLED"].IsSequence()) {
      filter_disabled(&enabled, &disabled, config["DISABLED"]);
    }

    // 2) Determine the mode for each display
    enabled = set_mode_for_displays(enabled);

    // 3) Determine the position for each display
    if (config["ORDER"].IsSequence()) {
      enabled = order_displays(enabled, config["ORDER"]);
    }
    Arrange arrange = ROW;
    if (config["ARRANGE"].IsScalar() && config["ARRANGE"].as<string>() == "COLUMN") {
      arrange = COLUMN;
    }
    Align align = MIDDLE;
    if (config["ALIGN"].IsScalar()) {
      string arrange_s = config["ALIGN"].as<string>();
      if (arrange_s == "TOP" || arrange_s == "LEFT") {
        align = TOP_OR_LEFT;
      } else if (arrange_s == "BOTTOM" || arrange_s == "RIGHT") {
        align = BOTTOM_OR_RIGHT;
      }
    }
    enabled = arrange_displays(enabled, arrange, align);

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
