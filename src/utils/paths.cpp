#include <string>
#include <unistd.h>

std::string get_socket_path() {
  char file_path[256];
  const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
  snprintf(file_path, sizeof(file_path), "%s/%s", runtime_dir, "wayland-displays.sock");
  return file_path;
};
