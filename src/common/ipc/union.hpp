#pragma once

#include "common/ipc/get.hpp"
#include "common/ipc/set.hpp"
#include "common/ipc/switch.hpp"

#include <yaml-cpp/node/node.h>

#include <variant>

typedef std::variant<IpcGetRequest, IpcSetRequest, IpcSwitchRequest> IpcRequest;

namespace YAML {
template <> struct convert<IpcRequest> {
  static Node encode(const IpcRequest &rhs) {
    return std::visit(
        [](const auto &arg) -> Node { return convert<std::decay_t<decltype(arg)>>::encode(arg); },
        rhs);
  }
  static bool decode(const Node &node, IpcRequest &rhs) {
    std::string op = node["OP"].as<std::string>();
    if (op == "GET") {
      rhs = node.as<IpcGetRequest>();
      return true;
    }
    if (op == "SET") {
      rhs = node.as<IpcSetRequest>();
      return true;
    }
    if (op == "SWITCH") {
      rhs = node.as<IpcSwitchRequest>();
      return true;
    }
    return false;
  }
};
} // namespace YAML
