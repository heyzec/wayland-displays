#include <yaml-cpp/yaml.h>

YAML::Node socket_read(int fd_sock);

void socket_write(int fd_sock, YAML::Node yaml);
