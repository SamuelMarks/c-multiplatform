/**
 * @file cupertino_magnifier.c
 * @brief Cupertino Text Magnifier (optical loupe) implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_magnifier.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

ui_error_t
cupertino_magnifier_create(struct ui_engine *engine,
                           const struct cupertino_magnifier_descriptor *desc,
                           struct cupertino_magnifier **out_magnifier) {
  struct cupertino_magnifier *mag;

  if (!engine || !desc || !out_magnifier) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (desc->width < 0.0f || desc->height < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  mag = (struct cupertino_magnifier *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_magnifier));
  if (!mag) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(mag, 0, sizeof(*mag));
  mag->width =
      (desc->width > 0.0f) ? desc->width : CUPERTINO_MAGNIFIER_DEFAULT_WIDTH;
  mag->height =
      (desc->height > 0.0f) ? desc->height : CUPERTINO_MAGNIFIER_DEFAULT_HEIGHT;
  mag->vertical_offset = (desc->vertical_offset != 0.0f)
                             ? desc->vertical_offset
                             : CUPERTINO_MAGNIFIER_DEFAULT_OFFSET_Y;
  mag->magnification = (desc->magnification > 0.0f)
                           ? desc->magnification
                           : CUPERTINO_MAGNIFIER_DEFAULT_ZOOM;
  mag->corner_radius = (desc->corner_radius > 0.0f)
                           ? desc->corner_radius
                           : CUPERTINO_MAGNIFIER_DEFAULT_CORNER_RADIUS;
  mag->current_scale = 0.0f;
  mag->is_active = 0;

  *out_magnifier = mag;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_magnifier_destroy(struct cupertino_magnifier *magnifier) {
  if (!magnifier) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(magnifier);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_magnifier_show(struct cupertino_magnifier *magnifier,
                                    float touch_x, float touch_y) {
  if (!magnifier) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  magnifier->is_active = 1;
  magnifier->touch_x = touch_x;
  magnifier->touch_y = touch_y;
  magnifier->lens_x = touch_x - (magnifier->width * 0.5f);
  magnifier->lens_y =
      touch_y + magnifier->vertical_offset - (magnifier->height * 0.5f);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_magnifier_update_position(struct cupertino_magnifier *magnifier,
                                    float touch_x, float touch_y) {
  if (!magnifier) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  magnifier->touch_x = touch_x;
  magnifier->touch_y = touch_y;
  magnifier->lens_x = touch_x - (magnifier->width * 0.5f);
  magnifier->lens_y =
      touch_y + magnifier->vertical_offset - (magnifier->height * 0.5f);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_magnifier_clamp_bounds(struct cupertino_magnifier *magnifier,
                                 float screen_width, float screen_height) {
  float min_margin;

  if (!magnifier || screen_width <= 0.0f || screen_height <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  min_margin = 8.0f;

  if (magnifier->lens_x < min_margin) {
    magnifier->lens_x = min_margin;
  } else if (magnifier->lens_x + magnifier->width > screen_width - min_margin) {
    magnifier->lens_x = screen_width - min_margin - magnifier->width;
  }

  if (magnifier->lens_y < min_margin) {
    magnifier->lens_y = min_margin;
  } else if (magnifier->lens_y + magnifier->height >
             screen_height - min_margin) {
    magnifier->lens_y = screen_height - min_margin - magnifier->height;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_magnifier_hide(struct cupertino_magnifier *magnifier) {
  if (!magnifier) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  magnifier->is_active = 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_magnifier_tick(struct cupertino_magnifier *magnifier,
                                    float delta_ms) {
  float target;
  float step;

  if (!magnifier || delta_ms < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  target = magnifier->is_active ? 1.0f : 0.0f;
  step = delta_ms / 150.0f;

  if (magnifier->current_scale < target) {
    magnifier->current_scale += step;
    if (magnifier->current_scale > target) {
      magnifier->current_scale = target;
    }
  } else if (magnifier->current_scale > target) {
    magnifier->current_scale -= step;
    if (magnifier->current_scale < target) {
      magnifier->current_scale = target;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_magnifier_get_lens_rect(const struct cupertino_magnifier *magnifier,
                                  float *out_x, float *out_y, float *out_w,
                                  float *out_h) {
  if (!magnifier || !out_x || !out_y || !out_w || !out_h) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_x = magnifier->lens_x;
  *out_y = magnifier->lens_y;
  *out_w = magnifier->width * magnifier->current_scale;
  *out_h = magnifier->height * magnifier->current_scale;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_magnifier_get_sample_center(
    const struct cupertino_magnifier *magnifier, float *out_sample_x,
    float *out_sample_y) {
  if (!magnifier || !out_sample_x || !out_sample_y) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_sample_x = magnifier->touch_x;
  *out_sample_y = magnifier->touch_y;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_magnifier_get_scale(const struct cupertino_magnifier *magnifier,
                              float *out_scale) {
  if (!magnifier || !out_scale) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_scale = magnifier->current_scale;
  return UI_ERROR_NONE;
}
