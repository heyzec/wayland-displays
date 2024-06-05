#include <getopt.h>
#include <string>

#include "gui.cpp"
#include "experiment.cpp"
#include "wlr-output-management-unstable-v1.h"

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
  static struct option long_options[] = {
      {"help", no_argument, 0, 'h'}, {"gui", no_argument, 0, 'g'}, {0, 0, 0, 0}};

  printf("hi");

  while (1) {
    int option_index = 0;
    opt = getopt_long(argc, argv, "hg:", long_options, &option_index);

    if (opt == -1) {
      break; // No more options
    }

    switch (opt) {
    case 'h':
      print_usage();
      exit(0);
    case 'g':
      start_gui();
      exit(1);
    case '?':
      // getopt_long already printed an error message
      print_usage();
      exit(1);
    default:
      printf("Exp");
      experiment();
      exit(1);
    }
  }

  experiment();

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
