#include <cstdlib>
#include <filesystem>
#include <string>

#include "common/logger.hpp"

namespace fs = std::filesystem;

std::string get_socket_path() {
  const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
  if (runtime_dir == nullptr) {
    log_critical("XDG_RUNTIME_DIR is not set");
    exit(1);
  }
  fs::path path = fs::path(runtime_dir) / "wayland-displays.sock";
  return path.string();
};

std::string get_config_path() {
  fs::path config_dir;

  char *xdg_config_home = getenv("XDG_CONFIG_HOME");
  if (xdg_config_home != nullptr) {
    config_dir = fs::path(xdg_config_home);
  } else {
    char *home = getenv("HOME");
    if (home == nullptr) {
      log_critical("HOME is not set");
      exit(1);
    }
    config_dir = fs::path(home) / ".config";
  }

  std::string output =  config_dir / "wayland-displays.yml";
  log_debug(output.c_str());
  return output;
};
