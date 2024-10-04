#pragma once

#include "Argument.cpp"
#include "Namespace.cpp"

#include <getopt.h>
#include <iostream>
#include <sstream>
#include <vector>

class ArgumentParser {
private:
  std::vector<Argument *> arguments;
  std::map<char, Argument *> table;

  std::string prog;
  std::string description;
  std::string epilog;

  void add_help_argument() {
    // By default, the help option is always available
    add_argument("-h", "--help")->action(STORE_TRUE)->help("show this help message and exit");
  }

  std::string get_usage() {
    std::ostringstream oss;

    oss << "usage: ";
    oss << prog << ":";

    for (Argument *&arg : arguments) {
      oss << " ";
      oss << "["
          << "-" << arg->opt_short;
      if (arg->_action != STORE_CONST && arg->_action != STORE_TRUE) {
        oss << " " << arg->opt_long;
      }
      oss << "]";
    }
    oss << "\n";

    return oss.str();
  }

  void show_help() {
    std::cout << get_usage();
    std::cout << "\n";

    if (description != "") {
      std::cout << description << "\n\n";
    }

    std::cout << "options:"
              << "\n";
    for (Argument *&arg : arguments) {
      std::cout << "  ";
      std::cout << "-" << arg->opt_short << ", ";
      std::cout << "--" << arg->opt_long;
      if (arg->_help) {
        std::cout << "\t" << arg->_help.value();
      }
      std::cout << "\n";
    }
    std::cout << "\n";

    if (epilog != "") {
      std::cout << epilog << "\n";
    }
  }

  Namespace parse(int argc, char **argv, std::vector<option> options) {
    Namespace ns;

    // Set default values
    if (prog == "") {
      prog = std::string(argv[0]);
    }
    for (Argument *&arg : arguments) {
      std::any value;
      if (arg->_action == STORE_TRUE) {
        value = false;
      } else if (arg->_action == STORE_CONST && arg->_fallback) {
        value = arg->_fallback;
      }
      ns.set<std::any>(arg->opt_short, value);
      ns.set<std::any>(arg->opt_long, value);
    }

    std::string short_options;
    for (option option : options) {
      short_options += (char)option.val;
    }

    options.push_back({0, 0, 0, 0});

    // opterr is an extern, set it to prevent the error message when option unrecognised
    opterr = 0;

    while (1) {
      char opt = getopt_long(argc, argv, short_options.c_str(), options.data(), NULL);

      if (opt == -1) {
        break; // No more options
      }

      if (opt == '?') {
        // If option unrecognised
        std::cerr << get_usage();

        std::cerr << prog << ": ";
        std::cerr << "error: ";

        // optopt is an extern, the option char attempted to parse
        if (!optopt) {
          std::cerr << "unrecognised arguments: ";
          // optind is an extern, the index of the next element to be processed in argv
          std::cerr << argv[optind - 1];
        } else {
          Argument *arg = table.find(optopt)->second;
          std::cerr << "argument "
                    << "-" << arg->opt_short << "/"
                    << "--" << arg->opt_long << ": ";
          std::cerr << "invalid";
        }
        std::cerr << "\n";

        exit(1);
      }

      auto it = table.find(opt);
      if (it == table.end()) {
        // Unknown flags are ignored
        continue;
      }

      Argument arg = *it->second;
      std::any value;
      if (arg._action == STORE) {
        // optarg is an extern containing the value of the option
        value = std::string(optarg);
      } else if (arg._action == STORE_CONST) {
        // TODO: Implement by adding a Argument.const()
      } else if (arg._action == STORE_TRUE) {
        value = true;
      }

      ns.set<std::any>(arg.opt_short, value);
      ns.set<std::any>(arg.opt_long, value);
    }

    // Handle non-option arguments
    for (int i = optind; i < argc; i++) {
      // printf("Non-option argument %s\n", argv[i]);
    }

    // for (auto &arg : arguments) {
    //   if (arg->isRequired && !ns.get<std::vector<int>>(arg->arg_short).size()) {
    //     throw std::runtime_error("Required argument missing: " + arg->arg_short);
    //   }
    // }

    return ns;
  }

public:
  ArgumentParser() {
    add_help_argument();
  }

  ArgumentParser(std::string prog, std::string description, std::string epilog) {
    this->prog = prog;
    this->description = description;
    this->epilog = epilog;
    add_help_argument();
  }

  Argument *add_argument(std::string name, std::string description) {
    Argument *arg = new Argument(name, description);
    table[arg->opt_short[0]] = arg;
    arguments.push_back(arg);
    return arg;
  }

  Namespace parse_args(int argc, char **argv) {
    std::vector<option> options;
    for (Argument *&arg : arguments) {
      int has_arg = required_argument;
      if (arg->_action == STORE_CONST || arg->_action == STORE_TRUE) {
        has_arg = optional_argument;
      }
      options.push_back({
          .name = arg->opt_long.c_str(), // name of long option
          .has_arg = has_arg,            // enum
          .flag = NULL,                  // specify NULL to return val
          .val = arg->opt_short[0]       // we set val to the equivalent short option char
      });
    }

    Namespace ns = parse(argc, argv, options);

    if (ns.get<bool>("help").value_or(false)) {
      // Display help and exit
      show_help();
      exit(0);
    }

    return ns;
  }

  ~ArgumentParser() {
    for (auto arg : arguments) {
      delete arg;
    }
  }
};
