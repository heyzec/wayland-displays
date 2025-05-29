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
#include <vector>

wl_display *display;
zwlr_screencopy_manager_v1 *manager;
wl_shm *shm;

struct OutputState {
  wl_output *output;
  std::string name;
};

struct FrameState {
  uint id;
  OutputState *output;
  wl_buffer *buffer;
  uint size;
  int fd;
  ScreencopyFrame screencopy_frame;
};

std::vector<FrameState> frames;

std::vector<OutputState *> outputs;

// ============================================================
// ???
// ============================================================

static void buffer(void *data, struct zwlr_screencopy_frame_v1 *frame, uint format, uint width,
                   uint height, uint stride) {
  printf("==BUFFER==\n");
  FrameState *state = (FrameState *)data;

  char shm_name[32];
  sprintf(shm_name, "/my_shm%s", state->output->name.c_str());
  int fd = shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  shm_unlink(shm_name);

  size_t size = stride * height;
  ftruncate(fd, size);
  wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
  wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, format);
  wl_shm_pool_destroy(pool); // protocol says we can destroy pool after creating buffer

  void *pixels = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  zwlr_screencopy_frame_v1_copy(frame, state->buffer);

  state->fd = fd;
  state->buffer = buffer;
  state->size = size;

  if (pixels == NULL) {
    fprintf(stderr, "Failed to mmap shared memory: %s\n", strerror(errno));
    exit(1);
  }

  ScreencopyFrame screencopy_frame = {
      .name = state->output->name,
      .pixels = pixels,
      .width = width,
      .height = height,
  };
}

static void flags(void *data, struct zwlr_screencopy_frame_v1 *frame, uint flags) {
  CopyOutput *out = (CopyOutput *)data;
  out->copied = true;
  // printf("Got a flags event\n");
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
                         uint width, uint height) {}

static void buffer_done(void *data, struct zwlr_screencopy_frame_v1 *frame) {
  // printf("Got a buf done event\n");
}

static const struct zwlr_screencopy_frame_v1_listener frame_listener = {
    .buffer = buffer,
    .flags = flags,
    .ready = ready,
    .failed = failed,
    .damage = damage,
    .linux_dmabuf = linux_dmabuf,
    .buffer_done = buffer_done,
};

// ============================================================
// ???
// ============================================================

static void geometry(void *data, struct wl_output *output, int x, int y, int physical_width,
                     int physical_height, int subpixel, const char *make, const char *model,
                     int transform) {}

static void mode(void *data, struct wl_output *output, uint flags, int width, int height,
                 int refresh) {}

static void done(void *data, struct wl_output *output) {
  printf("==DONE==\n");
}

static void scale(void *data, struct wl_output *output, int scale) {}

static void name(void *data, struct wl_output *_, const char *name) {
  OutputState *output = (OutputState *)data;
  output->name = std::string(name);
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
  if (strcmp(interface, wl_shm_interface.name) == 0) {
    shm = (wl_shm *)wl_registry_bind(registry, name, &wl_shm_interface, version);
  }
  if (strcmp(interface, wl_output_interface.name) == 0) {
    wl_output *output =
        (wl_output *)wl_registry_bind(registry, name, &wl_output_interface, version);
    OutputState *output_state = new OutputState();
    outputs.push_back(output_state);
    wl_output_add_listener(output, &output_listener, output_state);
  }
  if (strcmp(interface, zwlr_screencopy_manager_v1_interface.name) == 0) {
    manager = (zwlr_screencopy_manager_v1 *)wl_registry_bind(
        registry, name, &zwlr_screencopy_manager_v1_interface, version);
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
// ???
// ============================================================

void screencopy_init() {
  printf("screencopy_init called\n");
  // state = new WlrState{};
  // Connect to compositor and get the Wayland display singleton
  display = wl_display_connect(NULL);
  if (!display) {
    fprintf(stderr, "Failed to connect to Wayland display.\n");
    exit(1);
  }

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

  // frame = zwlr_screencopy_manager_v1_capture_output(manager, 0, output);
  // zwlr_screencopy_frame_v1_add_listener(frame, &frame_listener, NULL);
  //
  // printf("Roundtrip 2\n");
  // for (int i = 0; i < 100; i++) {
  //   wl_display_roundtrip(display);
  // }
  // printf("Bytes (in init): %.*s\n", 100, (char *)pixels);
}

ScreencopyObject screencopy_get() {
  std::vector<FrameState *> frame_states;
  for (OutputState *output : outputs) {
    zwlr_screencopy_frame_v1 *frame =
        zwlr_screencopy_manager_v1_capture_output(manager, 0, output->output);
    FrameState *frame_state = new FrameState{
        .output = output,
    };
    frame_states.push_back(frame_state);
    zwlr_screencopy_frame_v1_add_listener(frame, &frame_listener, frame_state);
    for (int i = 0; i < 1000; i++) {
      wl_display_roundtrip(display);
    }
    printf("called\n");
  }

  std::vector<ScreencopyFrame> frames = std::vector<ScreencopyFrame>();
  for (auto frame_state : frame_states) {
    frames.push_back(frame_state->screencopy_frame);
  }

  ScreencopyObject object = {
      .id = 0, // Not used, but required by the interface
      .frames = frames,
  };

  // if (frames.at(0).pixels == NULL) {
  //   fprintf(stderr, "No frames captured.\n");
  //   exit(1);
  // }

  return object;
}

void screencopy_destroy(ScreencopyObject obj) {}
