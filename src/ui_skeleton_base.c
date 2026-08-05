/* clang-format off */
#include "ui_skeleton_base.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
/* clang-format on */

struct ui_skeleton_base {
  enum ui_skeleton_shape shape;
  int width;
  int height;
  float elapsed_ms;
  struct ui_skeleton_animation_config config;
  struct ui_signal *active_signal;
};

ui_error_t ui_skeleton_base_create(struct ui_skeleton_base **out_skeleton) {
  struct ui_skeleton_base *skel;
  ui_error_t rc = UI_ERROR_NONE;

  if (!out_skeleton) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  skel = (struct ui_skeleton_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_skeleton_base));
  if (!skel) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  skel->shape = UI_SKELETON_SHAPE_RECTANGLE;
  skel->width = 100;
  skel->height = 20;
  skel->elapsed_ms = 0.0f;

  skel->config.duration_ms = 1500.0f;
  skel->config.base_opacity = 0.1f;
  skel->config.highlight_opacity = 0.3f;
  skel->config.highlight_width = 0.5f;

  *out_skeleton = skel;

cleanup:
  return rc;
}

ui_error_t ui_skeleton_base_destroy(struct ui_skeleton_base *skeleton) {
  if (!skeleton) {
    return UI_ERROR_NONE;
  }
  C_MULTIPLATFORM_FREE(skeleton);
  return UI_ERROR_NONE;
}

ui_error_t ui_skeleton_base_set_shape(struct ui_skeleton_base *skeleton,
                                      enum ui_skeleton_shape shape) {
  if (!skeleton) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  skeleton->shape = shape;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_skeleton_base_get_shape(const struct ui_skeleton_base *skeleton,
                                      enum ui_skeleton_shape *out_shape) {
  if (!skeleton || !out_shape) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_shape = skeleton->shape;
  return UI_ERROR_NONE;
}

ui_error_t ui_skeleton_base_set_dimensions(struct ui_skeleton_base *skeleton,
                                           int width, int height) {
  if (!skeleton) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (width < 0)
    width = 0;
  if (height < 0)
    height = 0;

  skeleton->width = width;
  skeleton->height = height;

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_skeleton_base_get_dimensions(const struct ui_skeleton_base *skeleton,
                                int *out_width, int *out_height) {
  if (!skeleton || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_width = skeleton->width;
  *out_height = skeleton->height;

  return UI_ERROR_NONE;
}

ui_error_t ui_skeleton_base_tick(struct ui_skeleton_base *skeleton,
                                 float delta_time_ms) {
  if (!skeleton) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  skeleton->elapsed_ms += delta_time_ms;

  /* Loop cleanly */
  while (skeleton->elapsed_ms >= skeleton->config.duration_ms &&
         skeleton->config.duration_ms > 0.0f) {
    skeleton->elapsed_ms -= skeleton->config.duration_ms;
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_skeleton_base_get_animation_phase(const struct ui_skeleton_base *skeleton,
                                     float *out_phase) {
  if (!skeleton || !out_phase) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (skeleton->config.duration_ms <= 0.0f) {
    *out_phase = 0.0f;
  } else {
    *out_phase = skeleton->elapsed_ms / skeleton->config.duration_ms;
  }

  return UI_ERROR_NONE;
}

/** \brief ui_skeleton_base_get_animation_config */
ui_error_t ui_skeleton_base_get_animation_config(
    const struct ui_skeleton_base *skeleton,
    struct ui_skeleton_animation_config **out_config) {
  if (!skeleton || !out_config) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_config = (struct ui_skeleton_animation_config *)&skeleton->config;
  return UI_ERROR_NONE;
}

ui_error_t ui_skeleton_base_bind_active(struct ui_skeleton_base *widget,
                                        struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->active_signal = signal;
  return UI_ERROR_NONE;
}
