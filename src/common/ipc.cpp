#include "common/ipc.hpp"
#include "common/ipc/union.hpp"
#include "common/logger.hpp"
#include "common/paths.hpp"
#include "common/socket.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <yaml-cpp/yaml.h>

IpcResponse send_ipc_request(IpcRequest request) {
  std::visit([](auto &&req) { log_debug("Sending request: {}", req.op); }, request);

  // Create a UNIX domain socket
  int fd_client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd_client_sock < 0) {
    log_critical("Error creating socket: {}", strerror(errno));
    exit(1);
  }

  // Set up the address structure
  struct sockaddr_un addr = {};
  addr.sun_family = AF_UNIX;
  std::string path = get_socket_path();
  strncpy(addr.sun_path, get_socket_path().c_str(), sizeof(addr.sun_path) - 1);

  // Connect to server
  if (connect(fd_client_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    log_critical("Error connecting to socket: {}", strerror(errno));
    exit(1);
  }

  YAML::Node node = YAML::convert<IpcRequest>::encode(request);

  // Send request
  socket_write(fd_client_sock, node);

  // Read response
  YAML::Node response_node = socket_read(fd_client_sock);
  IpcResponse response = response_node.as<IpcResponse>();
  return response;
}
