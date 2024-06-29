#pragma once

#include "argparse/argparse.h"
#include "common/ipc.cpp"
#include <yaml-cpp/node/node.h>

void run_ctl(Namespace args) {
  // TODO: This will crash if --switch not given
  // Use std::optional for argparse
  std::string profile_name = args.get<std::string>("switch");
  if (true) {
    YAML::Node node;
    node["OP"] = "SWITCH";
    node["PROFILE"] = profile_name;
    YAML::Node response = send_ipc_request(node);
  }
}
