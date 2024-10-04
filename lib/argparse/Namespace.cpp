#pragma once

#include <any>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>

class Namespace {
private:
  std::map<std::string, std::any> storage;

public:
  template <typename T> void set(const std::string &name, T value) {
    storage[name] = value;
  }

  template <typename T> std::optional<T> get(const std::string &name) const {
    auto it = storage.find(name);
    if (it == storage.end()) {
      throw std::runtime_error("Bug: No such key: " + name);
    }
    std::any possibleValue = it->second;
    if (!possibleValue.has_value()) {
      return std::nullopt;
    }
    if (possibleValue.type() != typeid(T)) {
      throw std::runtime_error(std::string("Bug: type to args.get is invalid: ") +
                               possibleValue.type().name());
    }
    return std::any_cast<T>(possibleValue);
  }
};
