#pragma once

#include <yaml-cpp/yaml.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

static char *socket_read_raw(int fd_sock) {
  recv(fd_sock, NULL, 0, MSG_PEEK);

  int n = 0;
  ioctl(fd_sock, FIONREAD, &n);

  char *buf = (char *)calloc(n + 1, sizeof(char));
  recv(fd_sock, buf, n, 0);

  return buf;
}

static void socket_write_raw(int fd_sock, std::string s) {
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
  socket_write_raw(fd_sock, s + "\n");
}
