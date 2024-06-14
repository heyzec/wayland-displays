#include "display.hpp"
#include <iostream>
#include <string>
#include <vector>

// #include <nlohmann/json.hpp>

// using json = nlohmann::ordered_json;

static void show_(DisplayConfig *head) {
  printf("Name %s Enabled %d Position (%d,%d) Size %dx%d Scale %f Rate %d Transform %d\n",
         head->name, head->enabled, head->pos_x, head->pos_y, head->size_x, head->size_y,
         head->scale, head->rate, head->transform);
}

void DisplayConfig::show() {
  show_(this);
}

void DisplayInfo::show() {
  show_((DisplayConfig *)this);
}

// void to_json(json &j, const Mode &m) {
//   j = json{
//       {"width", m.width}, {"height", m.height}, {"refresh", m.refresh}, {"preferred",
// }

// void to_json(json &j, const Display &d) {
//   j = json{
//       {"name", d.name},
//       {"description", d.description},
//       {"enabled", d.enabled},
//       {"width", d.width},
//       {"height", d.height},
//       {"pos_x", d.pos_x},
//       {"pos_y", d.pos_y},
//       // {"modes", d.modes},
//       {"make", d.make},
//       {"model", d.model},
//       {"serial_number", d.serial_number},
//   };
// }
