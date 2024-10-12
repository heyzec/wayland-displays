#include "common/shapes.hpp"
#include "server/handlers/WayDisplaysHandler/shapes.cpp"

#include <optional>
#include <vector>
#include <yaml-cpp/yaml.h>

/* Override this class to define custom handlers */
class BaseHandler {
public:
  /**
   * Callback for when a change to display configuration occurs.
   * Changes will be apply based on the config of returned value.
   * Otherwise, return NULL if no changes are required.
   */
  virtual std::vector<DisplayConfig> *handle_change(std::vector<DisplayInfo> *heads,
                                                    std::optional<Config> config) = 0;

  /**
   * Callback for when an IPC command requested.
   * Changes will be apply based on the config of returned value.
   * Otherwise, return NULL if no changes are required.
   */
  virtual std::vector<DisplayConfig> *handle_command(string command, string param,
                                                     std::vector<DisplayInfo> *heads,
                                                     std::optional<Config> node) = 0;
};
