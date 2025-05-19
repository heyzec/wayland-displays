#include "wlr-screencopy-unstable-v1.h"

// ============================================================
// Handlers and listeners for wl_registry
// ============================================================

#include "gui/copy.hpp"

// https://wayland.app/protocols/wayland#wl_registry:event:global
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

zwlr_screencopy_manager_v1 *manager;
zwlr_screencopy_frame_v1 *frame;
wl_output *output;
wl_shm *shm;
void *pixels;
uint global_width;
uint global_height;
uint global_stride;

static void geometry(void *data, struct wl_output *output, int x, int y, int physical_width,
                     int physical_height, int subpixel, const char *make, const char *model,
                     int transform) {
  /*printf("I got name %s\n");*/
}

static void mode(void *data, struct wl_output *output, uint flags, int width, int height,
                 int refresh) {
  /*printf("I got name %s\n", name);*/
}

static void done(void *data, struct wl_output *output) {
  printf("==DONE==\n");
}

static void scale(void *data, struct wl_output *output, int scale) {
  // printf("==DONE==");
}

static void name(void *data, struct wl_output *output, const char *name) {
  printf("I got name %s\n", name);
}

static void description(void *data, struct wl_output *output, const char *description) {
  printf("I got desc %s\n", description);
}

static const struct wl_output_listener output_listener = {
    .geometry = geometry,
    .mode = mode,
    .done = done,
    .scale = scale,
    .name = name,
    .description = description,
};

static void global(void *data, struct wl_registry *registry, uint32_t name, const char *interface,
                   uint32_t version) {
  // printf("interface: '%s', version: %d, name: %d\n", interface, version, name);
  if (strcmp(interface, wl_shm_interface.name) == 0) {
    shm = (wl_shm *)wl_registry_bind(registry, name, &wl_shm_interface, version);
    /**/
    /*zwlr_screencopy_frame_v1 *frame =*/
    /*    zwlr_screencopy_manager_v1_capture_output(manager, 1, output);*/
    /* h */
    /**/
    /*zwlr_screencopy_frame_v1_copy(copy_frame, frame->buffer);*/
    /*frame->stride = stride;*/
    /*frame->width = width;*/
    /*frame->height = height;*/
    /*frame->swap_rgb = format == WL_SHM_FORMAT_ABGR8888 || format == WL_SHM_FORMAT_XBGR8888;*/
  }
  if (strcmp(interface, wl_output_interface.name) == 0) {
    output = (wl_output *)wl_registry_bind(registry, name, &wl_output_interface, version);
    wl_output_add_listener(output, &output_listener, NULL);
    printf("Set output\n");

    /*state->manager = manager;*/
    /*zwlr_output_manager_v1_add_listener(manager, &manager_listener, state);*/
  }
  if (strcmp(interface, zwlr_screencopy_manager_v1_interface.name) == 0) {
    manager = (zwlr_screencopy_manager_v1 *)wl_registry_bind(
        registry, name, &zwlr_screencopy_manager_v1_interface, version);
    printf("Set maanger\n");
    /*state->manager = manager;*/
    /*zwlr_output_manager_v1_add_listener(manager, &manager_listener, state);*/
  }
}

static void global_remove(void *data, struct wl_registry *registry, uint32_t name) {
  // This space deliberately left blank
}

static const struct wl_registry_listener registry_listener = {
    .global = global,
    .global_remove = global_remove,
};

// ============================================================
// Setup and teardown of connection to compositor
// ============================================================

static void buffer(void *data, struct zwlr_screencopy_frame_v1 *frame, uint format, uint width,
                   uint height, uint stride) {
  printf("Got a buffer event of format %d\n", format);
  int fd = shm_open("/my_shm14", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  printf("Fd %d\n", fd);
  size_t size = stride * height;
  int stat = ftruncate(fd, size);
  printf("Stat %d\n", stat);
  wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
  wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, format);
  printf("Created size %zu\n", size);

  zwlr_screencopy_frame_v1_copy(frame, buffer);

  pixels = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  printf("Bytes (in buffer): %.*s\n", 100, (char *)pixels);

  // zwlr_screencopy_frame_v1_destroy(frame);

  global_width = width;
  global_height = height;
  global_stride = stride;
}

static void flags(void *data, struct zwlr_screencopy_frame_v1 *frame, uint flags) {
  printf("Got a flags event\n");
  zwlr_screencopy_frame_v1_destroy(frame);
}

static void ready(void *data, struct zwlr_screencopy_frame_v1 *frame, uint tv_sec_hi,
                  uint tv_sec_lo, uint tv_nsec) {
  printf("Got a ready event\n");
}

static void failed(void *data, struct zwlr_screencopy_frame_v1 *frame) {
  printf("Got a failed event\n");
}

static void damage(void *data, struct zwlr_screencopy_frame_v1 *frame, uint x, uint y, uint width,
                   uint height) {
  printf("Got a dam event\n");
}

static void linux_dmabuf(void *data, struct zwlr_screencopy_frame_v1 *frame, uint format,
                         uint width, uint height) {
  // printf("Got a linux dmabuf event\n");
  // uint fourcc = format;
  // printf("FourCC: %c%c%c%c\n", (fourcc & 0xFF), (fourcc >> 8) & 0xFF, (fourcc >> 16) & 0xFF,
  //        (fourcc >> 24) & 0xFF);
}

static void buffer_done(void *data, struct zwlr_screencopy_frame_v1 *frame) {
  printf("Got a buf done event\n");
}

static const struct zwlr_screencopy_frame_v1_listener frame_listener = {
    .buffer = buffer,
    .flags = flags,
    .failed = failed,
    .damage = damage,
    .linux_dmabuf = linux_dmabuf,
    .buffer_done = buffer_done,
};

void wlr_screencopy_init() {
  // state = new WlrState{};
  // Connect to compositor and get the Wayland display singleton
  wl_display *display = wl_display_connect(NULL);
  if (!display) {
    fprintf(stderr, "Failed to connect to Wayland display.\n");
    exit(1);
  }
  display = display;

  // Get the global registry singleton
  struct wl_registry *registry = wl_display_get_registry(display);
  // Bind a listener to the registry
  wl_registry_add_listener(registry, &registry_listener, NULL);

  // Registry listener will bind the zwlr manager if it is supported
  // TODO: Quit app if compositor does not support the protocol

  // Block until all pending requests/events are sent/received and all listeners executed:
  // - Handle global events (globals available on this compositor)
  printf("Roundtrip 1\n");
  for (int i = 0; i < 100; i++) {
    wl_display_roundtrip(display);
  }

  frame = zwlr_screencopy_manager_v1_capture_output(manager, 0, output);
  zwlr_screencopy_frame_v1_add_listener(frame, &frame_listener, NULL);

  printf("Roundtrip 2\n");
  for (int i = 0; i < 100; i++) {
    wl_display_roundtrip(display);
  }
  printf("Bytes (in init): %.*s\n", 100, (char *)pixels);
}
