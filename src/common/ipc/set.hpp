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
} // namespace YAML
