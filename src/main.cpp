#include "argparse/argparse.h"
#include "gui/gui.cpp"
#include "sandbox.cpp"

int main(int argc, char *argv[]) {
  ArgumentParser parser;
  parser.add_argument("-s", "--server")->action(STORE_TRUE)->help("start the daemon");
  parser.add_argument("-g", "--gui")->action(STORE_TRUE)->help("start the GUI app");
  Namespace args = parser.parse_args(argc, argv);

  if (args.get<bool>("server")) {
    run_server();
    exit(1);
  } else if (args.get<bool>("gui")) {
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
  }

  return 0;
}
