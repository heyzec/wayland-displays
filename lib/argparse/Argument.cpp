#pragma once

#include <any>
#include <cassert>
#include <optional>
#include <string>

enum ArgumentAction {
  STORE,
  STORE_CONST,
  STORE_TRUE,
};

class Argument {
public:
  std::string opt_short;
  std::string opt_long;

  // Refer to https://docs.python.org/3/library/argparse.html#the-add-argument-method

  // Specify how the command-line arguments should be handled
  std::optional<ArgumentAction> _action = STORE;
  // Specify what value should be used if the command-line argument is not present
  std::optional<std::any> _fallback;
  // Brief description of the argument
  std::optional<std::string> _help;

  bool is_required = false;

  Argument(std::string opt_short, std::string opt_long) {
    assert(opt_short.length() == 2 && opt_short[0] == '-');
    assert(opt_long.length() > 2 && opt_long.substr(0, 2) == "--");
    this->opt_short = opt_short.substr(1);
    this->opt_long = opt_long.substr(2);
  }

  Argument *action(ArgumentAction action) {
    this->_action = action;
    if (action == STORE_TRUE) {
      _fallback = true;
    }
    return this;
  }

  // Equivalent to python.argparse's default parameter
  Argument *fallback(std::any fallback) {
    this->_fallback = fallback;
    return this;
  }

  Argument *help(std::string help) {
    this->_help = help;
    return this;
  }

  Argument *required() {
    is_required = true;
    return this;
  }
};
