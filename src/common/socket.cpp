#include "common/socket.hpp"

#include <arpa/inet.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <yaml-cpp/yaml.h>

static char *socket_read_raw(int fd_sock) {
  uint32_t len_net;
  recv(fd_sock, &len_net, sizeof(len_net), MSG_WAITALL);
  uint32_t len = ntohl(len_net);

  char *buf = (char *)calloc(len + 1, sizeof(char));
  recv(fd_sock, buf, len, MSG_WAITALL);
  return buf;
}

static void socket_write_raw(int fd_sock, std::string s) {
  // fixed-length protocol: [ 4 bytes: uint32 payload length ][ N bytes: YAML payload ]
  uint32_t len_net = htonl(s.length());
  write(fd_sock, &len_net, sizeof(len_net));
  write(fd_sock, s.c_str(), s.length());
}

YAML::Node socket_read(int fd_sock) {
  char *buf = socket_read_raw(fd_sock);
  YAML::Node node = YAML::Load(buf);
  free(buf);
  return node;
}

void socket_write(int fd_sock, YAML::Node yaml) {
  YAML::Emitter out;
  out << yaml;
  std::string s = out.c_str();
  socket_write_raw(fd_sock, s);
}
