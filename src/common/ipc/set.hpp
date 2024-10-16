#pragma once

#include "common/ipc/base.hpp"
#include "common/shapes.hpp"
#include <yaml-cpp/node/node.h>

struct IpcSetRequest : IpcBaseRequest {
  std::vector<DisplayConfig> heads;

  IpcSetRequest() : IpcBaseRequest("SET") {}
  IpcSetRequest(std::vector<DisplayConfig> heads) : IpcSetRequest() {
    this->heads = heads;
  }
};

struct IpcSetResponse : IpcBaseResponse {
  bool success;

  IpcSetResponse() : IpcBaseResponse("SET") {}
  IpcSetResponse(bool success) : IpcSetResponse() {
    this->success = success;
  }
};

namespace YAML {
template <> struct convert<IpcSetRequest> {
  static Node encode(const IpcSetRequest &rhs) {
    Node node = convert<IpcBaseRequest>::encode(rhs);
    node["HEADS"] = convert<std::vector<DisplayConfig>>::encode(rhs.heads);
    return node;
  }
  static bool decode(const Node &node, IpcSetRequest &rhs) {
    auto heads = node["HEADS"].as<std::vector<DisplayConfig>>();
    rhs = IpcSetRequest(heads);
    return true;
  }
};

template <> struct convert<IpcSetResponse> {
  static Node encode(const IpcSetResponse &rhs) {
    Node node = convert<IpcBaseResponse>::encode(rhs);
    node["SUCCESS"] = rhs.success;
    return node;
  }
  static bool decode(const Node &node, IpcSetResponse &rhs) {
    auto success = node["SUCCESS"].as<bool>();
    rhs = IpcSetResponse(success);
    return true;
  }
};
} // namespace YAML
