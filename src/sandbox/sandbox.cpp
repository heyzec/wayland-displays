#include <stdio.h>
#include <wayland-client.h>

// https://wayland.app/protocols/wayland#wl_registry:event:global
static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name,
                                   const char *interface, uint32_t version) {
  printf("interface: '%s', version: %d, name: %d\n", interface, version, name);
}

// https://wayland.app/protocols/wayland#wl_registry:event:global_remove
static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name) {
  // This space deliberately left blank
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

int sandbox() {
  // Connect to compositor and get the Wayland display singleton
  struct wl_display *display = wl_display_connect(NULL);
  if (!display) {
    fprintf(stderr, "Failed to connect to Wayland display.\n");
    return 1;
  }
  fprintf(stderr, "Connection established!\n");

  // Get the global registry singleton
  struct wl_registry *registry = wl_display_get_registry(display);
  // Bind a listener to the registry
  wl_registry_add_listener(registry, &registry_listener, NULL);

  // Handle global events (globals available on this compositor)
  wl_display_roundtrip(display);


  // while (wl_display_dispatch(display) != -1) {
  //   /* This space deliberately left blank */
  // }

  wl_display_disconnect(display);
  return 0;
}
