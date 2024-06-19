#include "server/handlers/DefaultHandler.cpp"
#include "server/ipc.cpp"

#include "outputs/outputs.hpp"
#include "utils/paths.hpp"
#include "utils/socket.cpp"

#include <string>
#include <sys/poll.h>
#include <yaml-cpp/yaml.h>

#include <csignal>
#include <cstdio>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>

using string = std::string;

YAML::Node config;

/* File descriptor of server socket */
int fd_server_sock;
/* File descriptor of client socket, if client IPC request not handled within in one loop */
int fd_client_sock;
/* File descriptor for signals */
int fd_signal;

// Create vector of pollfd structures and named references to them in vector
std::vector<pollfd> all_pfds;
pollfd *pfd_ipc;
pollfd *pfd_wayland;
pollfd *pfd_signal;

// ============================================================
// Helper functions (avoid using global variables)
// ============================================================

static YAML::Node get_config() {
  std::string config_path = get_config_path();
  bool ok = std::filesystem::exists(config_path);
  if (!ok) {
    return YAML::Node{};
  }

  YAML::Node config = YAML::LoadFile(get_config_path());
  return config;
}

int ipc_socket_create(const char *socket_path) {
  // Create a UNIX domain socket
  int fd_server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd_server_sock < 0) {
    perror("Error creating socket");
    exit(1);
  }

  // Set up the address structure
  struct sockaddr_un addr;
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

  // Bind the socket
  if (bind(fd_server_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("Error binding socket");
    close(fd_server_sock);
    exit(1);
  }

  // Listen for connections
  if (listen(fd_server_sock, 5) < 0) {
    perror("Error listening on socket");
    close(fd_server_sock);
    exit(1);
  }

  return fd_server_sock;
}

void ipc_socket_destroy(int fd_server_sock, const char *socket_path) {
  close(fd_server_sock);
  unlink(socket_path);
}

int setup_signals() {
  // Block the signals we're interested in
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigprocmask(SIG_BLOCK, &mask, NULL);

  // Use signalfd to receive signals as file descriptor events
  int fd_signal = signalfd(-1, &mask, 0);
  if (fd_signal == -1) {
    perror("Error creating fd_signal");
    exit(1);
  }

  return fd_signal;
}

// ============================================================
// Stateful helper routines
// ============================================================

void server_init() {
  string socket_path = get_socket_path();
  fd_server_sock = ipc_socket_create(socket_path.c_str());

  fd_signal = setup_signals();

  config = get_config();
}

void server_deinit() {
  string socket_path = get_socket_path();
  ipc_socket_destroy(fd_server_sock, socket_path.c_str());
}

void reset_all_pfds() {
  all_pfds.clear();

  all_pfds.push_back({.fd = fd_server_sock, .events = POLLIN});
  pfd_ipc = &all_pfds.back();

  all_pfds.push_back({.fd = get_wl_display_fd(), .events = POLLIN});
  pfd_wayland = &all_pfds.back();

  all_pfds.push_back({.fd = fd_signal, .events = POLLIN});
  pfd_signal = &all_pfds.back();
}

// ============================================================
// Handlers for events from the loop
// ============================================================

bool handle_socket(int client_sock) {
  YAML::Node request = socket_read(client_sock);
  YAML::Node response = handle_ipc_request(request);
  if (response.IsNull()) {
    return true;
  }

  socket_write(client_sock, response);
  return true;
}

void complete_socket(int client_sock) {
  YAML::Node node;
  socket_write(client_sock, node);
}

// ============================================================
// Meat of the code
// ============================================================

void server_loop() {
  while (1) {
    reset_all_pfds();

    prepare_dispatch_events();

    // Block with infinite timeout until event occurs
    int num_events = poll(all_pfds.data(), all_pfds.size(), -1);
    if (num_events < 0) {
      perror("Error polling for events");
      close(fd_server_sock);
      exit(1);
    }

    if (pfd_wayland->revents) {
      dispatch_events();
    } else {
      cancel_dispatch_events();
    }

    // TODO: This flow is incomplete
    if (fd_client_sock != 0) {
      complete_socket(fd_client_sock);
      close(fd_client_sock);
      fd_client_sock = 0;
    }

    if (pfd_ipc->revents) {
      // If false, another pending request not been resolved, don't accept this yet
      if (fd_client_sock == 0) {
        int client_sock = accept(fd_server_sock, nullptr, nullptr);
        printf("Server: Accepted connection\n");
        if (client_sock < 0) {
          perror("Error accepting a connection");
          continue;
        }
        bool completed = handle_socket(client_sock);
        if (completed) {
          close(client_sock);
        } else {
          fd_client_sock = client_sock;
        }
      }
    }

    if (pfd_signal->revents) {
      struct signalfd_siginfo si;
      read(fd_signal, &si, sizeof(si));
      if (si.ssi_signo == SIGINT) {
        printf("Received SIGINT\n");
        break;
      }
    }
  }
}

static void on_done(std::vector<DisplayInfo> displays) {
  auto handler = DefaultHandler();
  std::vector<DisplayConfig> *changes = handler.handle(&displays, config);
  if (changes != nullptr) {
    // TODO: Sleep for a short time since there can be multiple DONE events, e.g.
    // another display outputs manager is setting heads too
    // But we need to retrieve the new serials too, else our request will be invalid.
    usleep(200);
    apply_configurations(*changes);
  }
}

/* Start the daemon */
void run_server() {
  // TODO: Rethink ordering
  attach_on_done(on_done);

  // Setup
  server_init();
  wlr_output_init();

  // Handle events in a loop until signalled
  server_loop();

  // Release what resources we can
  server_deinit();
  wlr_output_deinit();
}
