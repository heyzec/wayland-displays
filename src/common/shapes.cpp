#include "common/shapes.hpp"

#include <yaml-cpp/yaml.h>

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
