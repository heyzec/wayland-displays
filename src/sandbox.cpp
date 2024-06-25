#include "argparse/argparse.h"

int sandbox(int argc, char *argv[]) {
  ArgumentParser parser =
      ArgumentParser("ProgramName", "What the program does", "Text at the bottom of help");

  parser.add_argument("-c", "--count");                       // option that takes a value
  parser.add_argument("-v", "--verbose")->action(STORE_TRUE); // on/off flag

  Namespace args = parser.parse_args(argc, argv);
  // std::cout << args.get<std::string>("filename") << "\n"
  std::cout << args.get<std::string>("count") << "\n";
  std::cout << args.get<bool>("verbose") << "\n";
  return 0;
}
