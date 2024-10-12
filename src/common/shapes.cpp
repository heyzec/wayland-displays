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

void DisplaySettable::show() {
  printf("Enabled %s Position (%s,%s) Size %sx%s Scale %s Rate %s Transform %s\n",
         enabled.has_value() ? (enabled.value() ? "1" : "0") : "NULL",
         pos_x.has_value() ? std::to_string(pos_x.value()).c_str() : "NULL",
         pos_y.has_value() ? std::to_string(pos_y.value()).c_str() : "NULL",
         size_x.has_value() ? std::to_string(size_x.value()).c_str() : "NULL",
         size_y.has_value() ? std::to_string(size_y.value()).c_str() : "NULL",
         scale.has_value() ? std::to_string(scale.value()).c_str() : "NULL",
         rate.has_value() ? std::to_string(rate.value()).c_str() : "NULL",
         transform.has_value() ? std::to_string(transform.value()).c_str() : "NULL");
}
