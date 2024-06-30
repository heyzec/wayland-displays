#pragma once

#include "common/shapes.hpp"
#include <string>
#include <variant>
#include <vector>

struct IpcBaseRequest {
  std::string op;

  IpcBaseRequest(const std::string &op) : op(op) {}
};

struct IpcGetRequest : IpcBaseRequest {
  IpcGetRequest() : IpcBaseRequest("GET") {}
};

struct IpcSetRequest : IpcBaseRequest {
  std::vector<DisplayConfig> heads;

  IpcSetRequest() : IpcBaseRequest("SET") {}
  IpcSetRequest(std::vector<DisplayConfig> heads) : IpcSetRequest() {
    this->heads = heads;
  }
};

struct IpcSwitchRequest : IpcBaseRequest {
  std::string profile_name;

  IpcSwitchRequest() : IpcBaseRequest("SWITCH") {}
  IpcSwitchRequest(std::string profile_name) : IpcSwitchRequest() {
    this->profile_name = profile_name;
  }
};

typedef std::variant<IpcGetRequest, IpcSetRequest, IpcSwitchRequest> IpcRequest;

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
