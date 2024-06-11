#include "wlr_output.cpp"
#include "wlr-output-management-unstable-v1.h"
#include "wlr_output/shapes.hpp"
#include <nlohmann/json.hpp>

int sandbox() {
  auto displays = std::vector<HeadDyanamicInfo>{};
  wlr_output_init();

  // // Print the elements of the vector
  // for (auto &display : displays) {
  //   json j = display;
  //   std::cout << j.dump(4) << std::endl;
  // }
  //
  // wlr_output_cleanup();
  //
  // set(displays);
  return 0;
}
