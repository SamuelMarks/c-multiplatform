/* clang-format off */
#include "ui_textarea_autosize.h"
/* clang-format on */

ui_error_t ui_textarea_autosize_config_init(
    struct ui_textarea_autosize_config *out_config) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!out_config) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  out_config->min_height = 0.0f;
  out_config->max_height = 0.0f;
  out_config->line_height = 16.0f;
  out_config->padding_top = 8.0f;
  out_config->padding_bottom = 8.0f;

cleanup:
  return rc;
}

/** \brief ui_error */
ui_error_t
ui_textarea_autosize_calculate(const struct ui_textarea_autosize_config *config,
                               size_t num_lines,
                               struct ui_textarea_autosize_state *out_state) {
  ui_error_t rc = UI_ERROR_NONE;
  float content_height;
  float target_height;
  int has_scrollbar;

  if (!config || !out_state) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  content_height = config->padding_top + config->padding_bottom +
                   ((float)num_lines * config->line_height);
  target_height = content_height;
  has_scrollbar = 0;

  if (config->min_height > 0.0f && target_height < config->min_height) {
    target_height = config->min_height;
  }

  if (config->max_height > 0.0f && target_height > config->max_height) {
    target_height = config->max_height;
    has_scrollbar = 1;
  }

  out_state->content_height = content_height;
  out_state->target_height = target_height;
  out_state->has_scrollbar = has_scrollbar;

cleanup:
  return rc;
}
