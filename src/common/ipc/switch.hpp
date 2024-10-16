#pragma once

#include "common/ipc/base.hpp"
#include <yaml-cpp/node/node.h>

struct IpcSwitchRequest : IpcBaseRequest {
  std::string profile_name;

  IpcSwitchRequest() : IpcBaseRequest("SWITCH") {}
  IpcSwitchRequest(std::string profile_name) : IpcSwitchRequest() {
    this->profile_name = profile_name;
  }
};

struct IpcSwitchResponse : IpcBaseResponse {
  bool success;

  IpcSwitchResponse() : IpcBaseResponse("SET") {}
  IpcSwitchResponse(bool success) : IpcSwitchResponse() {
    this->success = success;
  }
};

namespace YAML {
template <> struct convert<IpcSwitchRequest> {
  static Node encode(const IpcSwitchRequest &rhs) {
    Node node = convert<IpcBaseRequest>::encode(rhs);
    node["PROFILE"] = rhs.profile_name;
    return node;
  }
  static bool decode(const Node &node, IpcSwitchRequest &rhs) {
    std::string profile_name = node["PROFILE"].as<std::string>();
    rhs = IpcSwitchRequest(profile_name);
    return true;
  }
};

template <> struct convert<IpcSwitchResponse> {
  static Node encode(const IpcSwitchResponse &rhs) {
    Node node = convert<IpcBaseResponse>::encode(rhs);
    node["SUCCESS"] = rhs.success;
    return node;
  }
  static bool decode(const Node &node, IpcSwitchResponse &rhs) {
    auto success = node["SUCCESS"].as<bool>();
    rhs = IpcSwitchResponse(success);
    return true;
  }
};
} // namespace YAML
