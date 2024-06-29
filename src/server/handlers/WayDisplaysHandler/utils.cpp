#include "server/handlers/WayDisplaysHandler/utils.hpp"

#include <regex>
#include <string>

using string = std::string;
template <class T> using vector = std::vector<T>;

bool does_display_match(const DisplayInfo display, const string pattern) {
  if (pattern[0] != '!') {
    return display.name == pattern || display.description == pattern;
  }
  std::regex regex = std::regex(pattern.substr(1));
  return std::regex_search(display.name, regex) || std::regex_search(display.description, regex);
}

/**
 * Find display in vector that is matched fully by name or description.
 */
int find_display(const vector<DisplayInfo> displays, const string pattern) {
  for (int i = 0; i < displays.size(); i++) {
    auto display = displays.at(i);
    if (does_display_match(display, pattern)) {
      return i;
    }
  }
  return -1;
}

std::pair<vector<DisplayInfo>, vector<DisplayInfo>>
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

bool does_profile_match(const vector<DisplayInfo> displays, const Profile profile) {
  auto [matched, unmatched] = match_displays(displays, profile.displays);
  return unmatched.size() == 0;
}

/**
 * Find profile that matches the current display configuration.
 */
Profile find_matching_profile(const vector<Profile> profiles, const vector<DisplayInfo> displays) {
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
 * Find profile that matches name fully
 */
Profile get_profile_by_name(const vector<Profile> profiles, const string name) {
  Profile matched;
  for (Profile profile : profiles) {
    if (profile.name == name) {
      matched = profile;
      break;
    }
  }

  return matched;
}
