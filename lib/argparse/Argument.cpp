#pragma once

#include <any>
#include <cassert>
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

  ArgumentAction _action = STORE;
  std::any _store;
  std::string _help;

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
      _store = true;
    }
    return this;
  }

  Argument *store(std::any value) {
    this->_store = value;
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
