#include "wlr-screencopy-unstable-v1.h"

#include "common/logger.hpp"
#include "gui/screencopy.hpp"

// https://wayland.app/protocols/wayland#wl_registry:event:global
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

static wl_display *display;
static zwlr_screencopy_manager_v1 *manager;
static wl_shm *shm;

struct OutputState {
  wl_output *output;
  char *name;
};

struct FrameState {
  uint id;
  zwlr_screencopy_frame_v1 *frame; // unused
  OutputState *output;
  // variables for creating buffer
  uint format, width, height, stride;
  // buffer-related state that require cleanup
  wl_buffer *buffer;
  void *pixels;
  uint size;
  int fd;
};

static uint next_frame_id = 1;
static std::vector<OutputState *> output_states;
static std::vector<FrameState *> frame_states;

// ============================================================
// Handlers for zwlr_screencopy_frame
// ============================================================

static void buffer(void *data, struct zwlr_screencopy_frame_v1 *frame, uint format, uint width,
                   uint height, uint stride) {
  FrameState *state = (FrameState *)data;
  state->format = format;
  state->width = width;
  state->height = height;
  state->stride = stride;
  // Refer to buffer_done listener
}

static void flags(void *data, struct zwlr_screencopy_frame_v1 *frame, uint flags) {}

static void ready(void *data, struct zwlr_screencopy_frame_v1 *frame, uint tv_sec_hi,
                  uint tv_sec_lo, uint tv_nsec) {
  zwlr_screencopy_frame_v1_destroy(frame);
}

static void failed(void *data, struct zwlr_screencopy_frame_v1 *frame) {
  FrameState *frame_state = (FrameState *)data;
  log_warn("Failed to copy frame for {}", frame_state->output->name);
  zwlr_screencopy_frame_v1_destroy(frame);
}

static void damage(void *data, struct zwlr_screencopy_frame_v1 *frame, uint x, uint y, uint width,
                   uint height) {}

static void linux_dmabuf(void *data, struct zwlr_screencopy_frame_v1 *frame, uint format,
                         uint width, uint height) {}

static void buffer_done(void *data, struct zwlr_screencopy_frame_v1 *frame) {
  FrameState *state = (FrameState *)data;
  std::string name = state->output->name;

  char shm_name[32];
  sprintf(shm_name, "/wayland-displays_%s", name.c_str());
  int fd = shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  shm_unlink(shm_name);

  size_t size = state->stride * state->height;
  ftruncate(fd, size);
  void *pixels = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (pixels == MAP_FAILED) {
    log_warn("Failed to mmap shared memory for {}", name.c_str());
    return;
  }

  wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
  wl_buffer *buffer =
      wl_shm_pool_create_buffer(pool, 0, state->width, state->height, state->stride, state->format);
  wl_shm_pool_destroy(pool); // protocol says we can destroy pool after creating buffer

  state->pixels = pixels;
  state->size = size;
  state->buffer = buffer;
  state->fd = fd;

  zwlr_screencopy_frame_v1_copy(frame, buffer);
  // Refer to ready or failed listener
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
// Handlers for wl_output
// ============================================================

static void geometry(void *data, struct wl_output *output, int x, int y, int physical_width,
                     int physical_height, int subpixel, const char *make, const char *model,
                     int transform) {}

static void mode(void *data, struct wl_output *output, uint flags, int width, int height,
                 int refresh) {}

static void done(void *data, struct wl_output *output) {}

static void scale(void *data, struct wl_output *output, int scale) {}

static void name(void *data, struct wl_output *output_, const char *name) {
  OutputState *output_state = (OutputState *)data;
  output_state->name = strdup(name);
}

static void description(void *data, struct wl_output *output, const char *description) {}

static const struct wl_output_listener output_listener = {
    .geometry = geometry,
    .mode = mode,
    .done = done,
    .scale = scale,
    .name = name,
    .description = description,
};

// ============================================================
// Handlers for wl_registry
// ============================================================

static void global(void *data, struct wl_registry *registry, uint32_t name, const char *interface,
                   uint32_t version) {
  if (strcmp(interface, wl_shm_interface.name) == 0) {
    shm = (wl_shm *)wl_registry_bind(registry, name, &wl_shm_interface, version);
  }
  if (strcmp(interface, wl_output_interface.name) == 0) {
    wl_output *output =
        (wl_output *)wl_registry_bind(registry, name, &wl_output_interface, version);

    OutputState *output_state = new OutputState{
        .output = output,
    };
    output_states.push_back(output_state);
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
// Entry point
// ============================================================

void screencopy_init() {
  // Connect to compositor and get the Wayland display singleton
  display = wl_display_connect(NULL);
  // TODO: Allow GUI to still run even if display is NULL
  // display = NULL;
  if (!display) {
    log_critical("Failed to connect to Wayland display.");
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
  wl_display_roundtrip(display);
}

ScreencopyFrames screencopy_get() {
  std::vector<FrameState *> new_frame_states;
  for (auto &output_state : output_states) {
    FrameState *frame_state = new FrameState{
        .id = next_frame_id,
        .output = output_state,
    };
    next_frame_id++;
    new_frame_states.push_back(frame_state);
    frame_states.push_back(frame_state);

    zwlr_screencopy_frame_v1 *frame =
        zwlr_screencopy_manager_v1_capture_output(manager, 0, output_state->output);
    zwlr_screencopy_frame_v1_add_listener(frame, &frame_listener, frame_state);
    // Refer to buffer handler
  }

  for (int _ = 0; _ < 3; _++) {
    wl_display_roundtrip(display);
  }

  auto screencopy_frames = std::vector<ScreencopyFrame>();
  for (FrameState *frame_state : new_frame_states) {
    screencopy_frames.push_back(ScreencopyFrame{
        .id = frame_state->id,
        .name = frame_state->output->name,
        .pixels = frame_state->pixels,
        .width = frame_state->width,
        .height = frame_state->height,
    });
  }
  return screencopy_frames;
}

void screencopy_destroy(ScreencopyFrames frames) {
  for (auto it = frame_states.begin(); it != frame_states.end();) {
    auto frame_state = *it;
    bool found = false;
    for (const auto &frame : frames) {
      if (frame_state->id == frame.id) {
        found = true;
        break;
      }
    }
    if (found) {
      // This is important, otherwise memory will leak fast
      it = frame_states.erase(it);
      munmap(frame_state->pixels, frame_state->size);
      wl_buffer_destroy(frame_state->buffer);
      close(frame_state->fd);
      delete frame_state;
    } else {
      ++it;
    }
  }
}
