#include <cstdlib>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

std::string get_socket_path() {
  const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
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
    config_dir = fs::path(home) / ".config";
  }

  return config_dir / "wayland-displays.yml";
};
