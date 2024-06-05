#include <poll.h>
#include <string>
#include <sys/signalfd.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-util.h>

#include "wlr-output-management-unstable-v1.h"

struct Displ {
  // global
  struct wl_registry *registry;
  struct wl_display *display;

  // wlroots output manager
  struct zwlr_output_manager_v1 *zwlr_output_manager;
  uint32_t zwlr_output_manager_name;
  uint32_t zwlr_output_manager_version;
  char *zwlr_output_manager_interface;
  uint32_t zwlr_output_manager_serial;

  // // wayland output manager
  // struct zxdg_output_manager_v1 *zxdg_output_manager;
  // uint32_t zxdg_output_manager_name;
  // uint32_t zxdg_output_manager_version;
  // char *zxdg_output_manager_interface;

  // enum ConfigState config_state;
};

// https://wayland.app/protocols/wlr-output-management-unstable-v1#zwlr_output_manager_v1:event:head
static void head(void *data, struct zwlr_output_manager_v1 *zwlr_output_manager_v1,
                 struct zwlr_output_head_v1 *zwlr_output_head_v1) {
  printf("Head");
}
// https://wayland.app/protocols/wlr-output-management-unstable-v1#zwlr_output_manager_v1:event:done
static void done(void *data, struct zwlr_output_manager_v1 *zwlr_output_manager_v1,
                 uint32_t serial) {
  printf("Done");
}

// https://wayland.app/protocols/wlr-output-management-unstable-v1#zwlr_output_manager_v1:event:finished
static void finished(void *data, struct zwlr_output_manager_v1 *zwlr_output_manager_v1) {
  printf("Fin");
}

const struct zwlr_output_manager_v1_listener *zwlr_output_manager_listener(void) {
  struct zwlr_output_manager_v1_listener listener = {
      .head = head,
      .done = done,
      .finished = finished,
  };
  return &listener;
}

static void bind_zwlr_output_manager(struct Displ *displ_bad, struct wl_registry *wl_registry,
                                     uint32_t name, const char *interface, uint32_t version) {

  Displ *displ = new Displ{};

  displ->zwlr_output_manager_name = name;
  displ->zwlr_output_manager_version = version;
  displ->zwlr_output_manager_interface = strdup(interface);
  displ->zwlr_output_manager = (zwlr_output_manager_v1 *)wl_registry_bind(
      wl_registry, name, &zwlr_output_manager_v1_interface, displ->zwlr_output_manager_version);
  // zwlr_output_manager_v1 manager = wl_registry_bind(wl_registry, name,
  // &zwlr_output_manager_v1_interface, version);

  zwlr_output_manager_v1_add_listener(displ->zwlr_output_manager, zwlr_output_manager_listener(),
                                      displ);
  printf("YAYAAY");
}

static void global(void *data, struct wl_registry *wl_registry, uint32_t name,
                   const char *interface, uint32_t version) {
  // Don't hardcode, use zwlr_output_manager_v1_interface.name...
  if (strcmp(interface, "zwlr_output_manager_v1") == 0) {
    printf("Sup\n");
    bind_zwlr_output_manager(NULL, wl_registry, name, interface, version);
    // } else if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
    // 	bind_zxdg_output_manager(data, wl_registry, name, interface, version);
    // } else if (strcmp(interface, wl_output_interface.name) == 0) {
    // 	bind_wl_output(data, wl_registry, name, interface, version);
    // } else if (strcmp(interface, wp_fractional_scale_manager_v1_interface.name) == 0) {
    // 	printf("\nCompositor supports %s version %d", interface, version);
  }
  printf("\nCompositor supports %s version %d", interface, version);
}

static void global_remove(void *data, struct wl_registry *wl_registry, uint32_t name) {
  // struct Displ *displ = data;

  // output_destroy_by_wl_output_name(name);
  //
  // // a "who cares?" situation in the WLR examples
  // if (displ && displ->zwlr_output_manager_name == name) {
  // 	log_info("\nDisplay's output manager has been removed, exiting");
  // 	wd_exit(EXIT_SUCCESS);
  // }
}

static const struct wl_registry_listener listener = {
    .global = global,
    .global_remove = global_remove,
};

const struct wl_registry_listener *registry_listener(void) { return &listener; }

nfds_t npfds = 2;
struct pollfd pfds[5];
struct pollfd *pfd_signal = NULL;
struct pollfd *pfd_ipc = NULL;
struct pollfd *pfd_wayland = NULL;

int fd_signal = -1;
#define FL __FILE__, __LINE__
int loop(wl_display *display) {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGQUIT);
  // sigaddset(&mask, SIGTERM);
  sigaddset(&mask, SIGPIPE);
  sigprocmask(SIG_BLOCK, &mask, NULL);
  fd_signal = signalfd(-1, &mask, 0);

  int i = 0;

  pfd_signal = &pfds[i++];
  pfd_signal->fd = fd_signal;
  pfd_signal->events = POLLIN;

  pfd_wayland = &pfds[i++];
  pfd_wayland->fd = wl_display_get_fd(display);
  pfd_wayland->events = POLLIN;

  for (;;) {
    // prepare for reading wayland events
    while (wl_display_prepare_read(display)) {
      wl_display_dispatch_pending(display);
    }
    wl_display_flush(display);

    // poll for all events
    if (poll(pfds, npfds, -1) < 0) {
      printf("\npoll failed, exiting");
      return 1;
    }

    wl_display_read_events(display);
    wl_display_dispatch_pending(display);
  }
}

void experiment() {
  wl_display *display = wl_display_connect(NULL);
  if (!display) {
    printf("\nUnable to connect to the compositor. Check or set the WAYLAND_DISPLAY environment "
           "variable. exiting");
    return;
  } else {
    printf("Yooooo");
  }
  auto registry = wl_display_get_registry(display);

  wl_registry_listener listener = {
      .global = global,
      .global_remove = global_remove,
  };

  wl_registry_add_listener(registry, registry_listener(), display);

  if (wl_display_roundtrip(display) == -1) {
    printf("\nwl_display_roundtrip failed -1, exiting");
    exit(1);
    return;
  }

  // if (display->zwlr_output_manager) {
  //   printf("\ncompositor does not support WLR output manager protocol, exiting");
  //   exit(1);
  //   return;
  // }
  loop(display);
}
