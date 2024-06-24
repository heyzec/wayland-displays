#include "server/handlers/WayDisplaysHandler/utils.hpp"

#include <string>

using string = std::string;
template <class T> using vector = std::vector<T>;

bool does_display_match(const DisplayInfo display, const string pattern) {
  return display.name == pattern or display.description == pattern;
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

Profile find_matching_profile(const vector<DisplayInfo> displays, const vector<Profile> profiles) {
  Profile matched;
  for (Profile profile : profiles) {
    if (does_profile_match(displays, profile)) {
      matched = profile;
      break;
    }
  }

  return matched;
}
