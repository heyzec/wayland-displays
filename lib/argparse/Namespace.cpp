#pragma once

#include <any>
#include <map>
#include <stdexcept>
#include <string>

class Namespace {
private:
  std::map<std::string, std::any> storage;

public:
  template <typename T> void set(const std::string &name, T value) {
    storage[name] = value;
  }

  template <typename T> T get(const std::string &name) const {
    auto it = storage.find(name);
    if (it != storage.end()) {
      return std::any_cast<T>(it->second);
    }
    throw std::runtime_error("No such key: " + name);
  }
};
