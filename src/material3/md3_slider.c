/**
 * @file md3_slider.c
 * @brief Material 3 Slider and Range Slider implementation wrapping CDK base
 * components.
 */

/* clang-format off */
#include "material3/md3_slider.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

ui_error_t md3_slider_create(struct ui_engine *engine,
                             enum md3_slider_type type, float min, float max,
                             struct md3_slider **out_slider,
                             struct ui_control_value_accessor **out_cva) {
  struct md3_slider *s;
  ui_error_t rc;

  if (!engine || !out_slider || min >= max) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  s = (struct md3_slider *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_slider));
  if (!s) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(s, 0, sizeof(struct md3_slider));
  s->type = type;
  s->orientation = MD3_SLIDER_HORIZONTAL;
  s->min = min;
  s->max = max;
  s->step = (type == MD3_SLIDER_DISCRETE) ? 1.0f : 0.0f;
  s->has_value_indicator = 1;

  rc = ui_slider_base_create(&s->base, &s->cva);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(s);
    return rc;
  }

  rc = ui_slider_base_set_min(s->base, min);
  rc = ui_slider_base_set_max(s->base, max);

  if (s->step > 0.0f) {
    rc = ui_slider_base_set_step(s->base, s->step);
  }

  if (out_cva) {
    *out_cva = &s->cva;
  }

  *out_slider = s;
  return rc;
}

ui_error_t md3_slider_destroy(struct md3_slider *slider) {
  ui_error_t rc;

  if (!slider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_slider_base_destroy(slider->base);
  C_MULTIPLATFORM_FREE(slider);
  return rc;
}

ui_error_t md3_slider_set_value(struct md3_slider *slider, float value) {
  if (!slider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_slider_base_set_value(slider->base, value);
}

ui_error_t md3_slider_get_value(const struct md3_slider *slider,
                                float *out_value) {
  if (!slider || !out_value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_slider_base_get_value(slider->base, out_value);
}

ui_error_t md3_slider_set_step(struct md3_slider *slider, float step) {
  if (!slider || step < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  slider->step = step;
  return ui_slider_base_set_step(slider->base, step);
}

ui_error_t md3_slider_set_disabled(struct md3_slider *slider, int disabled) {
  if (!slider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_slider_base_set_disabled(slider->base, disabled);
}

ui_error_t md3_slider_set_on_change(struct md3_slider *slider,
                                    ui_slider_on_change_t on_change,
                                    void *user_data) {
  if (!slider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_slider_base_set_on_change(slider->base, on_change, user_data);
}

ui_error_t md3_slider_get_base(struct md3_slider *slider,
                               struct ui_slider_base **out_base) {
  if (!slider || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = slider->base;
  return UI_ERROR_NONE;
}

ui_error_t md3_range_slider_create(struct ui_engine *engine, float min,
                                   float max,
                                   struct md3_range_slider **out_slider) {
  struct md3_range_slider *rs;
  ui_error_t rc;

  if (!engine || !out_slider || min >= max) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rs = (struct md3_range_slider *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_range_slider));
  if (!rs) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(rs, 0, sizeof(struct md3_range_slider));
  rs->min = min;
  rs->max = max;

  rc = ui_range_slider_base_create(&rs->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(rs);
    return rc;
  }

  rc = ui_range_slider_base_set_min(rs->base, min);
  rc = ui_range_slider_base_set_max(rs->base, max);

  *out_slider = rs;
  return rc;
}

ui_error_t md3_range_slider_destroy(struct md3_range_slider *slider) {
  ui_error_t rc;

  if (!slider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_range_slider_base_destroy(slider->base);
  C_MULTIPLATFORM_FREE(slider);
  return rc;
}

ui_error_t md3_range_slider_set_values(struct md3_range_slider *slider,
                                       float low, float high) {
  if (!slider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_range_slider_base_set_values(slider->base, low, high);
}

ui_error_t md3_range_slider_get_values(const struct md3_range_slider *slider,
                                       float *out_low, float *out_high) {
  if (!slider || !out_low || !out_high) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_range_slider_base_get_values(slider->base, out_low, out_high);
}

ui_error_t md3_range_slider_get_base(struct md3_range_slider *slider,
                                     struct ui_range_slider_base **out_base) {
  if (!slider || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = slider->base;
  return UI_ERROR_NONE;
}
