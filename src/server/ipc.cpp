#include "common/ipc.hpp"
#include "common/ipc/get.hpp"
#include "common/ipc/set.hpp"
#include "common/ipc/switch.hpp"
#include "server/handlers/DefaultHandler.cpp"

#include "common/shapes.hpp"
#include "outputs/outputs.hpp"

#include <optional>
#include <variant>
#include <yaml-cpp/yaml.h>

IpcGetResponse handle_get(IpcGetRequest yaml) {
  std::vector<DisplayInfo> heads = get_head_infos();
  IpcGetResponse response = IpcGetResponse(heads);
  return response;
}

IpcSetResponse handle_set(IpcSetRequest request) {
  apply_configurations(request.heads);
  return IpcSetResponse(true);
}

IpcSwitchResponse handle_switch(IpcSwitchRequest request, std::optional<Config> config) {
  string profile_name = request.profile_name;
  auto handler = DefaultHandler();
  std::vector<DisplayInfo> heads = get_head_infos();
  std::vector<DisplayConfig> *changes =
      handler.handle_command("switch", profile_name, &heads, config);
  if (changes != nullptr) {
    apply_configurations(*changes);
  }
  return IpcSwitchResponse(true);
}

IpcResponse handle_ipc_request(IpcRequest request, std::optional<Config> config) {
  std::visit([](auto &&req) { log_debug("Handling request: {}", req.op); }, request);

  if (auto *req = std::get_if<IpcGetRequest>(&request)) {
    return handle_get(*req);
  }
  if (auto *req = std::get_if<IpcSetRequest>(&request)) {
    return handle_set(*req);
  }
  if (auto *req = std::get_if<IpcSwitchRequest>(&request)) {
    return handle_switch(*req, config);
  }

  // This should not occur
  return IpcResponse();

  // // Unable to reply now, keep socket open and we will reply later
  // return null;
}
