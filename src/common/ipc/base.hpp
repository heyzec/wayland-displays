#pragma once

#include <string>
#include <yaml-cpp/node/node.h>

struct IpcBaseRequest {
  std::string op;

  IpcBaseRequest(const std::string &op) : op(op) {}
};

namespace YAML {
template <> struct convert<IpcBaseRequest> {
  static Node encode(const IpcBaseRequest &rhs) {
    Node node;
    node["OP"] = rhs.op;
    return node;
  }
  static bool decode(const Node &node, IpcBaseRequest &rhs) {
    rhs.op = node["OP"].as<std::string>();
    return true;
  }
};
} // namespace YAML
