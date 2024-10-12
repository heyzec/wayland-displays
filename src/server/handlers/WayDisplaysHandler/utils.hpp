#pragma once

#include "common/shapes.hpp"
#include "server/handlers/WayDisplaysHandler/shapes.cpp"

#include <string>
#include <vector>

using string = std::string;
template <class T> using vector = std::vector<T>;

bool does_display_match(const DisplayInfo display, const string pattern);

int find_display(const vector<DisplayInfo> displays, const string pattern);

std::map<string, string> assign_displays(const vector<DisplayInfo> displays,
                                         const vector<string> patterns);

// vector<DisplayInfo> find_displays(const vector<DisplayInfo> displays,
//                                   const vector<string> patterns);

// bool does_profile_match(const vector<DisplayInfo> displays, const Profile profile);

std::optional<std::pair<Profile, std::map<string, string>>>
find_matching_profile(const vector<Profile> profiles, const vector<DisplayInfo> displays);

Profile get_profile_by_name(const vector<Profile> profiles, const string name);
