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

/**
 * Assign displays to patterns.
 * Each display can only be matched once.
 */
std::map<string, string> assign_displays(const vector<DisplayInfo> displays,
                                         const vector<string> patterns) {
  vector<DisplayInfo> unmatched = displays;
  std::map<string, string> assignments;

  for (auto pattern : patterns) {
    int index = find_display(unmatched, pattern);
    if (index != -1) {
      assignments[unmatched.at(index).name] = pattern;
      log_debug("assign_displays: matched pattern <<{}>> to display \"{}\"", pattern,
                unmatched.at(index).name);
      unmatched.erase(unmatched.begin() + index);
    } else {
      log_debug("assign_displays: no match for pattern <<{}>>", pattern);
    }
  }

  for (auto it = assignments.begin(); it != assignments.end(); it++) {
    auto [name, pattern] = *it;
  }

  return assignments;
}

// /**
//  * Finds and returns a subset of displays from the given vector that match the provided patterns.
//  * Each display can only be matched once.
//  * If no match is found for a pattern, an empty vector is returned.
//  */
// vector<DisplayInfo> find_displays(const vector<DisplayInfo> displays,
//                                   const vector<string> patterns) {
//   std::map<string, string> assignments = assign_displays(displays, patterns);
//   return matched;
// }

// bool does_profile_match(const vector<DisplayInfo> displays, const Profile profile) {
//   vector<string> patterns;
//   for (const auto &pair : profile.displays) {
//     string pattern = pair.first;
//     patterns.push_back(pattern);
//   }
//   std::map<string, string> assignments = assign_displays(displays, patterns);
//   printf("assignments.size() = %lu, patterns.size() = %lu\n", assignments.size(),
//   patterns.size()); return assignments.size() == patterns.size();
// }

/**
 * Find profile that (exactly) matches the current display configuration.
 * Also return the assignment of displays.
 */
std::optional<std::pair<Profile, std::map<string, string>>>
find_matching_profile(const vector<Profile> profiles, const vector<DisplayInfo> displays) {
  for (Profile profile : profiles) {
    vector<string> patterns;
    for (const auto &pair : profile.displays) {
      string pattern = pair.first;
      patterns.push_back(pattern);
    }
    log_debug("find_matching_profile: testing profile \"{}\"", profile.name);
    std::map<string, string> assignments = assign_displays(displays, patterns);
    // Criteria for a match: all displays are matched
    // Can consider relaxing this requirement, e.g. score based partial matches
    if (assignments.size() == patterns.size()) {
      log_debug("find_matching_profile: found profile \"{}\" with {} assignments", profile.name,
                assignments.size());
      return std::make_pair(profile, assignments);
    }
  }
  log_debug("find_matching_profile: no matching profile found");
  return std::nullopt;
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
