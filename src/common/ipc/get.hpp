#pragma once

#include "common/ipc/base.hpp"
#include "common/shapes.hpp"
#include <yaml-cpp/node/node.h>

struct IpcGetRequest : IpcBaseRequest {
  IpcGetRequest() : IpcBaseRequest("GET") {}
};

struct IpcGetResponse : IpcBaseResponse {
  std::vector<DisplayInfo> heads;

  IpcGetResponse() : IpcBaseResponse("GET") {}
  IpcGetResponse(std::vector<DisplayInfo> heads) : IpcGetResponse() {
    this->heads = heads;
  }
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

template <> struct convert<IpcGetResponse> {
  static Node encode(const IpcGetResponse &rhs) {
    printf("Encoding IpcGetResponse\n");
    Node node = convert<IpcBaseResponse>::encode(rhs);
    node["HEADS"] = convert<std::vector<DisplayInfo>>::encode(rhs.heads);
    return node;
  }
  static bool decode(const Node &node, IpcGetResponse &rhs) {
    printf("Decoding IpcGetResponse\n");
    convert<IpcBaseResponse>::decode(node, rhs);
    printf("Going to index\n");
    auto heads = node["HEADS"].as<std::vector<DisplayInfo>>();
    rhs = IpcGetResponse(heads);
    return true;
  }
};
} // namespace YAML
