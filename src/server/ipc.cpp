#include "common/ipc_request.hpp"
#include "server/handlers/DefaultHandler.cpp"

#include "common/shapes.hpp"
#include "outputs/outputs.hpp"

#include <variant>
#include <yaml-cpp/yaml.h>

YAML::Node handle_get(IpcGetRequest yaml) {
  std::vector<DisplayInfo> heads = get_head_infos();
  YAML::Node node;
  node["STATE"]["HEADS"] = heads;
  return node;
}

YAML::Node handle_set(IpcSetRequest request) {
  apply_configurations(request.heads);
  return YAML::Node{};
}

YAML::Node handle_switch(IpcSwitchRequest request, YAML::Node config) {
  string profile_name = request.profile_name;
  auto handler = DefaultHandler();
  std::vector<DisplayInfo> heads = get_head_infos();
  std::vector<DisplayConfig> *changes =
      handler.handle_command("switch", profile_name, &heads, config);
  if (changes != nullptr) {
    apply_configurations(*changes);
  }
  return YAML::Node{};
}

YAML::Node handle_ipc_request(IpcRequest request, YAML::Node config) {
  // TODO: SLAP this function by returning IpcResponse

  printf("Someone's knocking...\n");

  if (auto *v = std::get_if<IpcGetRequest>(&request)) {
    return handle_get(*v);
  }
  if (auto *v = std::get_if<IpcSetRequest>(&request)) {
    return handle_set(*v);
  }
  if (auto *v = std::get_if<IpcSwitchRequest>(&request)) {
    return handle_switch(*v, config);
  }

  // This should not occur
  return YAML::Node{};

  // // Unable to reply now, keep socket open and we will reply later
  // return null;
}
