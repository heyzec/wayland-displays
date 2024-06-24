#include "server/handlers/WayDisplaysHandler/utils.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("find_display") {
  SECTION("should return -1 in empty vector") {
    vector<DisplayInfo> displays;
    REQUIRE(find_display(displays, "") == -1);
  }

  SECTION("should find display by name in a vector of size 1") {
    vector<DisplayInfo> displays;
    DisplayInfo display = DisplayInfo{};
    display.name = strdup("eDP-1");
    display.description = strdup("Description of eDP-1");
    displays.push_back(display);
    REQUIRE(find_display(displays, "eDP-1") == 0);
  }

  SECTION("should find display by description in a vector of size 1") {
    vector<DisplayInfo> displays;
    DisplayInfo display = DisplayInfo{};
    display.name = strdup("eDP-1");
    display.description = strdup("Description of eDP-1");
    displays.push_back(display);
    REQUIRE(find_display(displays, "Description of eDP-1") == 0);
  }

  SECTION("should not match substring incorrectly") {
    vector<DisplayInfo> displays;
    DisplayInfo display = DisplayInfo{};
    display.name = strdup("eDP-1");
    display.description = strdup("Description of eDP-1");
    displays.push_back(display);
    REQUIRE(find_display(displays, "DP-1") == -1);
  }

  SECTION("should find display by name even if names are substrings") {
    vector<DisplayInfo> displays;

    DisplayInfo display = DisplayInfo{};
    display.name = strdup("eDP-1");
    display.description = strdup("Description of eDP-1");
    displays.push_back(display);

    display = DisplayInfo{};
    display.name = strdup("DP-1"); // substring of eDP-1
    display.description = strdup("Description of DP-1");
    displays.push_back(display);

    REQUIRE(find_display(displays, "DP-1") == 1);
  }
}
