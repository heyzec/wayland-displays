#include "common/ipc.hpp"

#include "common/ipc_request.hpp"
#include "common/paths.hpp"
#include "common/socket.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <yaml-cpp/yaml.h>

YAML::Node send_ipc_request(IpcRequest request) {
  // Create a UNIX domain socket
  int fd_client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd_client_sock < 0) {
    perror("Error creating socket");
    exit(1);
  }

  // Set up the address structure
  struct sockaddr_un addr = {};
  addr.sun_family = AF_UNIX;
  std::string path = get_socket_path();
  strncpy(addr.sun_path, get_socket_path().c_str(), sizeof(addr.sun_path) - 1);

  // Connect to server
  if (connect(fd_client_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("connect");
    exit(1);
  }

  YAML::Node node = YAML::convert<IpcRequest>::encode(request);

  // Send request
  socket_write(fd_client_sock, node);

  // Read response
  YAML::Node response = socket_read(fd_client_sock);
  return response;
}
