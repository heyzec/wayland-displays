#include "server/handlers/WayDisplaysHandler/utils.hpp"

#include <catch2/catch_test_macros.hpp>

static DisplayInfo create_display(string name, string description) {
  DisplayInfo display = DisplayInfo{};
  display.name = strdup(name.c_str());
  display.description = strdup(description.c_str());
  return display;
}

TEST_CASE("does_display_match() correctly matches plain strings") {
  SECTION("should match full name") {
    DisplayInfo display = create_display("eDP-1", "Description of eDP-1");
    REQUIRE(does_display_match(display, "eDP-1"));
  }
  SECTION("should match full description") {
    DisplayInfo display = create_display("eDP-1", "Description of eDP-1");
    REQUIRE(does_display_match(display, "Description of eDP-1"));
  }
  SECTION("should not match substring of name") {
    DisplayInfo display = create_display("Name of eDP-1", "Description of eDP-1");
    REQUIRE(!does_display_match(display, "Name"));
  }
  SECTION("should not match substring of description") {
    DisplayInfo display = create_display("eDP-1", "Description of eDP-1");
    REQUIRE(!does_display_match(display, "Description"));
  }
}

TEST_CASE("does_display_match() correctly matches regexes") {
  SECTION("should match a regex") {
    DisplayInfo display = create_display("eDP-1", "Description of eDP-1");
    REQUIRE(does_display_match(display, "!\\w+ .{2} eDP-\\d"));
  }
  SECTION("should match substrings by default") {
    DisplayInfo display = create_display("eDP-1", "desc1 desc2 desc3");
    REQUIRE(does_display_match(display, "!desc2"));
  }
  SECTION("should not match substring when anchors present") {
    DisplayInfo display = create_display("eDP-1", "desc1 desc2 desc3");
    REQUIRE(!does_display_match(display, "!^desc2$"));
  }
  SECTION("should not match when regex prefix is missing") {
    DisplayInfo display = create_display("eDP-1", "Description of eDP-1");
    REQUIRE(!does_display_match(display, ".+"));
  }
}

TEST_CASE("find_display()") {
  SECTION("should return -1 in empty vector") {
    vector<DisplayInfo> displays;
    REQUIRE(find_display(displays, "") == -1);
  }
  SECTION("should find display by name even if names are substrings of another") {
    vector<DisplayInfo> displays;

    DisplayInfo display;
    display = create_display("eDP-1", "Description of eDP-1");
    displays.push_back(display);
    display = create_display("DP-1", "Description of DP-1");
    displays.push_back(display);

    REQUIRE(find_display(displays, "DP-1") == 1);
  }
}
