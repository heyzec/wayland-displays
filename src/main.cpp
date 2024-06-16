#include "gui/gui.cpp"
#include "sandbox.cpp"

#include <yaml-cpp/yaml.h>

#include <getopt.h>
#include <string>

void print_usage() {
  printf("Usage: my_program [options]\n");
  printf("Options:\n");
  printf("  -h, --help            Display this help message\n");
}

int main(int argc, char *argv[]) {
  int opt;
  int verbose_flag = 0;
  char *output_file = NULL;

  // Define long options
  static struct option long_options[] = {{"help", no_argument, 0, 'h'},
                                         {"server", no_argument, 0, 's'},
                                         {"gui", no_argument, 0, 'g'},
                                         {0, 0, 0, 0}};

  int n_opt = 0;

  while (1) {
    int option_index = 0;
    opt = getopt_long(argc, argv, "hsg", long_options, &option_index);

    if (opt == -1) {
      break; // No more options
    }
    n_opt++;

    switch (opt) {
    case 'h':
      print_usage();
      exit(0);
    case 's':
      run_server();
      exit(1);
    case 'g': {
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
    };
    case '?':
      // getopt_long already printed an error message
      print_usage();
      exit(1);
    default:
      print_usage();
      exit(1);
    }
  }

  // If no arguments, run code sandbox
  if (n_opt == 0) {
    int status = sandbox();
    exit(status);
  }

  // Process any remaining arguments (not options)
  if (optind < argc) {
    printf("Non-option arguments: ");
    while (optind < argc) {
      printf("%s ", argv[optind++]);
    }
    printf("\n");
  }

  // Example usage of parsed options
  if (verbose_flag) {
    printf("Verbose mode enabled.\n");
  }
  if (output_file) {
    printf("Output file: %s\n", output_file);
  }

  return 0;
}
