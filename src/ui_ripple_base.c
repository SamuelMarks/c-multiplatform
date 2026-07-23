/* clang-format off */
#include "ui_ripple_base.h"
/* clang-format on */

enum ui_error ui_ripple_config_init(struct ui_ripple_config *out_config) {
  enum ui_error rc = UI_ERROR_NONE;

  if (!out_config) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  out_config->color[0] = 0.0f;
  out_config->color[1] = 0.0f;
  out_config->color[2] = 0.0f;
  out_config->color[3] = 0.1f;
  out_config->duration_ms = 300.0f;
  out_config->max_radius = 0.0f;
  out_config->center_origin = 0;

cleanup:
  return rc;
}

enum ui_error ui_ripple_start(const struct ui_ripple_config *config, float x,
                              float y, struct ui_ripple_state *out_state) {
  enum ui_error rc = UI_ERROR_NONE;

  if (!config || !out_state) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  out_state->center_x = x;
  out_state->center_y = y;
  out_state->radius = 0.0f;
  out_state->opacity = config->color[3];
  out_state->elapsed_ms = 0.0f;
  out_state->active = 1;

cleanup:
  return rc;
}

enum ui_error ui_ripple_update(const struct ui_ripple_config *config,
                               float delta_ms, struct ui_ripple_state *state) {
  enum ui_error rc = UI_ERROR_NONE;
  float progress;

  if (!config || !state) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  if (!state->active) {
    goto cleanup;
  }

  state->elapsed_ms += delta_ms;
  progress = state->elapsed_ms / config->duration_ms;

  if (progress >= 1.0f) {
    progress = 1.0f;
    state->active = 0;
  }

  state->radius = config->max_radius * progress;
  state->opacity = config->color[3] * (1.0f - progress);

cleanup:
  return rc;
}
