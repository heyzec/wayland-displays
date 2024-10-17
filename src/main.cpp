#include "argparse/argparse.h"
#include "common/logger.hpp"
#include "ctl/ctl.cpp"
#include "gui/gui.cpp"
#include "sandbox.cpp"
#include "server/server.cpp"

int main(int argc, char *argv[]) {
  init_logger();
  ArgumentParser parser;

  parser.add_argument("-s", "--server")->action(STORE_TRUE)->help("start the daemon");
  parser.add_argument("-g", "--gui")->action(STORE_TRUE)->help("start the GUI app");
  parser.add_argument("-x", "--switch")->help("activate a profile by name");

  Namespace args = parser.parse_args(argc, argv);

  if (args.get<bool>("server").value_or(false)) {
    run_server();
    exit(1);
  } else if (args.get<bool>("gui").value_or(false)) {
    // TODO: Fork a temporary server if no server exists
    // int pid = fork();
    // if (pid == 0) {
    //   run_server();
    //   exit(0);
    // }
    // printf("Server PID %d, GUI PID %d\n", pid, getpid());

    run_gui();

    // kill(pid, SIGINT);

    exit(0);
  } else {
    run_ctl(args);
  }

  return 0;
}
