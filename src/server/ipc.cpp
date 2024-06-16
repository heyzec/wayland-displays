#include "display.hpp"
#include "outputs/outputs.hpp"

#include <yaml-cpp/yaml.h>

YAML::Node handle_get(YAML::Node yaml) {
  std::vector<DisplayInfo> heads = get_head_infos();
  YAML::Node node;
  node["STATE"]["HEADS"] = heads;
  return node;
}

YAML::Node handle_set(YAML::Node yaml) {
  std::vector<DisplayConfig> displays = yaml["HEADS"].as<std::vector<DisplayConfig>>();
  apply_configurations(displays);
  return YAML::Node{};
}

YAML::Node handle_ipc_request(YAML::Node yaml) {
  printf("Someone's knocking...\n");
  YAML::Node null = YAML::Node();

  if (!yaml.IsMap()) {
    // Invalid yaml
    printf("IPC request is invalid, ignoring\n");
    return null;
  }

  std::string op = yaml["OP"].as<std::string>();
  if (strcmp(op.c_str(), "GET") == 0) {
    return handle_get(yaml);
  }
  if (strcmp(op.c_str(), "SET") == 0) {
    return handle_set(yaml);
  }

  // Unable to reply now, keep socket open and we will reply later
  return null;
}
