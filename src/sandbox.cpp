#include "wlr-output.cpp"
#include "wlr-output-management-unstable-v1.h"
#include <nlohmann/json.hpp>

int sandbox() {
  auto displays = std::vector<struct Display>{};
  wlr_output_init(&displays);

  // Print the elements of the vector
  for (auto &display : displays) {
    json j = display;
    std::cout << j.dump(4) << std::endl;
  }

  wlr_output_cleanup();
  return 0;
}
