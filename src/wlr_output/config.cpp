/*
 * Handlers and listeners for zwlr_output_configuration
 * https://wayland.app/protocols/wlr-output-management-unstable-v1#zwlr_output_configuration_v1
 */
#include "wlr-output-management-unstable-v1.h"
#include <cstdio>

static void succeeded(void *data, struct zwlr_output_configuration_v1 *config) {
  printf("==Succeeded==\n");
}

static void failed(void *data, struct zwlr_output_configuration_v1 *config) {
  printf("==Failed==\n");
}

static void cancelled(void *data, struct zwlr_output_configuration_v1 *config) {
  printf("==Cancelled==\n");
}

static const struct zwlr_output_configuration_v1_listener config_listener = {
    .succeeded = succeeded,
    .failed = failed,
    .cancelled = cancelled,
};

const struct zwlr_output_configuration_v1_listener *get_config_listener() {
  return &config_listener;
}
