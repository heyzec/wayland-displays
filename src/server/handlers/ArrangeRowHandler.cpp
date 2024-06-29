#include "server/handlers/BaseHandler.cpp"

#include "common/shapes.hpp"

/* Arrange discovered displays in a row. */
class ArrangeRowHandler : BaseHandler {
public:
  std::vector<DisplayConfig> *handle(std::vector<DisplayInfo> *heads, YAML::Node config) {
    for (int i = 0; i < heads->size(); i++) {
      auto head = heads->at(i);
      head.show();
    }

    int x = 0;

    std::vector<DisplayConfig> *changes = new std::vector<DisplayConfig>();
    changes->reserve(heads->size());
    for (DisplayConfig &head : *heads) {
      head.pos_x = x;
      head.pos_y = 0;
      changes->push_back(head);
      x += head.size_x;
    }

    for (auto &head : *changes) {
      head.show();
    }

    return changes;
  }
};
