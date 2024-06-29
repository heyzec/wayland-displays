#include "server/handlers/DefaultHandler.cpp"

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

YAML::Node handle_switch(YAML::Node yaml, YAML::Node config) {
  string profile_name = yaml["PROFILE"].as<string>();
  auto handler = DefaultHandler();
  std::vector<DisplayInfo> heads = get_head_infos();
  std::vector<DisplayConfig> *changes =
      handler.handle_command("switch", profile_name, &heads, config);
  if (changes != nullptr) {
    apply_configurations(*changes);
  }
  return YAML::Node{};
}

YAML::Node handle_ipc_request(YAML::Node request, YAML::Node config) {
  printf("Someone's knocking...\n");
  YAML::Node null = YAML::Node();

  if (!request.IsMap()) {
    // Invalid yaml
    printf("IPC request is invalid, ignoring\n");
    return null;
  }

  std::string op = request["OP"].as<std::string>();
  if (op == "GET") {
    return handle_get(request);
  }
  if (op == "SET") {
    return handle_set(request);
  }

  if (op == "SWITCH") {
    return handle_switch(request, config);
  }

  // Unable to reply now, keep socket open and we will reply later
  return null;
}
