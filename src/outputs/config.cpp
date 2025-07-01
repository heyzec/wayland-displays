/*
 * Handlers and listeners for zwlr_output_configuration
 * https://wayland.app/protocols/wlr-output-management-unstable-v1#zwlr_output_configuration_v1
 */
#include "common/logger.hpp"

#include "wlr-output-management-unstable-v1.h"

static void succeeded(void *data, struct zwlr_output_configuration_v1 *config) {
  log_debug("event: succeeded");
}

static void failed(void *data, struct zwlr_output_configuration_v1 *config) {
  log_debug("event: failed");
}

static void cancelled(void *data, struct zwlr_output_configuration_v1 *config) {
  log_debug("event: cancelled");
}

static const struct zwlr_output_configuration_v1_listener config_listener = {
    .succeeded = succeeded,
    .failed = failed,
    .cancelled = cancelled,
};

const struct zwlr_output_configuration_v1_listener *get_config_listener() {
  return &config_listener;
}
