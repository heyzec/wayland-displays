#include "wlr_output.cpp"

#include <csignal>
#include <cstdio>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>

/* File descriptor of server socket */
int fd_server_sock;
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

std::string get_socket_path() {
  char file_path[256];
  const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
  snprintf(file_path, sizeof(file_path), "%s/%s", runtime_dir, "wayland-displays.sock");
  return file_path;
};

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
  std::string socket_path = get_socket_path();
  fd_server_sock = ipc_socket_create(socket_path.c_str());
  fd_signal = setup_signals();
}

void server_deinit() {
  std::string socket_path = get_socket_path();
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

void handle_client_request(int client_sock) {
  char buf[100];
  int bytes_received;

  // Handle the client connection
  while ((bytes_received = read(client_sock, buf, sizeof(buf) - 1)) > 0) {
    buf[bytes_received] = '\0';
    printf("Received: %s\n", buf);
    write(client_sock, buf, bytes_received); // Echo back the message
  }

  close(client_sock);
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

    if (pfd_ipc->revents) {
      int client_sock = accept(fd_server_sock, nullptr, nullptr);
      if (client_sock < 0) {
        perror("Error accepting a connection");
        continue;
      }
      handle_client_request(client_sock);
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

/* Start the daemon */
void run_server() {
  // Setup
  wlr_output_init();
  server_init();

  // Handle events in a loop until signalled
  server_loop();

  // Release what resources we can
  server_deinit();
  wlr_output_deinit();
}
