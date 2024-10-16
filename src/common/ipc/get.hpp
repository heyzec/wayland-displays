#pragma once

#include "common/ipc/base.hpp"
#include <yaml-cpp/node/node.h>

struct IpcGetRequest : IpcBaseRequest {
  IpcGetRequest() : IpcBaseRequest("GET") {}
};

namespace YAML {
template <> struct convert<IpcGetRequest> {
  static Node encode(const IpcGetRequest &rhs) {
    Node node = convert<IpcBaseRequest>::encode(rhs);
    return node;
  }
  static bool decode(const Node &node, IpcGetRequest &rhs) {
    convert<IpcBaseRequest>::decode(node, rhs);
    return true;
  }
};
} // namespace YAML
