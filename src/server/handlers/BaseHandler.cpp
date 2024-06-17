#include "display.hpp"
#include <vector>

/* Override this class to define custom handlers */
class BaseHandler {
public:
  /* Callback to call when event occurs.
   * Changes will be apply based on the config of returned value.
   * Otherwise, return NULL if no changes are required.
   */
  virtual std::vector<DisplayConfig> *handle(std::vector<DisplayInfo> *heads) = 0;
};
