#pragma once

#include "argparse/argparse.h"
#include "common/ipc.hpp"
#include "common/ipc/switch.hpp"
#include <yaml-cpp/node/node.h>

void run_ctl(Namespace args) {
  std::optional<std::string> profile_name = args.get<std::string>("switch");
  if (profile_name.has_value()) {
    IpcSwitchRequest request = IpcSwitchRequest(profile_name.value());
    IpcSwitchResponse response = std::get<IpcSwitchResponse>(send_ipc_request(request));
  } else {
    printf("Please provide more arguments. Run with --help to see what options are available\n");
  }
}
