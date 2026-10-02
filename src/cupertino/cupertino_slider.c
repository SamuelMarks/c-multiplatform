/**
 * @file cupertino_slider.c
 * @brief Cupertino Slider component wrapping ui_slider_base implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_slider.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_slider_mock_set_min_fail = 0;
int g_cupertino_slider_mock_set_max_fail = 0;
int g_cupertino_slider_mock_destroy_fail = 0;

static ui_error_t mock_slider_base_set_min(struct ui_slider_base *s, float m) {
  if (g_cupertino_slider_mock_set_min_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_slider_base_set_min(s, m);
}
#undef ui_slider_base_set_min
/** @cond */
#define ui_slider_base_set_min mock_slider_base_set_min
/** @endcond */

static ui_error_t mock_slider_base_set_max(struct ui_slider_base *s, float m) {
  if (g_cupertino_slider_mock_set_max_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_slider_base_set_max(s, m);
}
#undef ui_slider_base_set_max
/** @cond */
#define ui_slider_base_set_max mock_slider_base_set_max
/** @endcond */

static ui_error_t mock_slider_base_destroy(struct ui_slider_base *s) {
  if (g_cupertino_slider_mock_destroy_fail) {
    (ui_slider_base_destroy)(s);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_slider_base_destroy)(s);
}
#undef ui_slider_base_destroy
/** @cond */
#define ui_slider_base_destroy mock_slider_base_destroy
/** @endcond */
#endif

static ui_error_t
cupertino_slider_cva_write_value(void *component,
                                 union ui_signal_payload value) {
  struct cupertino_slider *slider;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  slider = (struct cupertino_slider *)component;
  return cupertino_slider_set_value(slider, value.float_val);
}

static ui_error_t
cupertino_slider_cva_set_disabled_state(void *component,
                                        ui_bool_t is_disabled) {
  struct cupertino_slider *slider;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  slider = (struct cupertino_slider *)component;
  return cupertino_slider_set_disabled(slider, is_disabled ? 1 : 0);
}

/**
 * @brief Creates a new Cupertino Slider component.
 */
ui_error_t cupertino_slider_create(struct ui_engine *engine, float min,
                                   float max,
                                   struct cupertino_slider **out_slider,
                                   struct ui_control_value_accessor **out_cva) {
  struct cupertino_slider *slider;
  ui_error_t rc;

  if (!engine || !out_slider || min >= max) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  slider = (struct cupertino_slider *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_slider));
  if (!slider) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(slider, 0, sizeof(struct cupertino_slider));
  slider->min = min;
  slider->max = max;
  slider->step = 0.0f;
  slider->active_color = UI_COLOR_ARGB(255, 0x00, 0x7A, 0xFF); /* SystemBlue */
  slider->track_color = UI_COLOR_ARGB(255, 0xE5, 0xE5, 0xEA);  /* SystemGray5 */
  slider->thumb_color = UI_COLOR_ARGB(255, 0xFF, 0xFF, 0xFF);  /* White */

  rc = ui_slider_base_create(&slider->base, NULL);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(slider);
    return rc;
  }

  rc = ui_slider_base_set_min(slider->base, min);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_slider_base_destroy(slider->base);
    C_MULTIPLATFORM_FREE(slider);
    if (destroy_rc != UI_ERROR_NONE) {
      return destroy_rc;
    }
    return rc;
  }

  rc = ui_slider_base_set_max(slider->base, max);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_slider_base_destroy(slider->base);
    C_MULTIPLATFORM_FREE(slider);
    if (destroy_rc != UI_ERROR_NONE) {
      return destroy_rc;
    }
    return rc;
  }

  slider->cva.component = slider;
  slider->cva.write_value = cupertino_slider_cva_write_value;
  slider->cva.set_disabled_state = cupertino_slider_cva_set_disabled_state;
  slider->cva.register_on_change = NULL;
  slider->cva.register_on_touched = NULL;

  if (out_cva) {
    *out_cva = &slider->cva;
  }
  *out_slider = slider;

  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a Cupertino slider and its underlying base.
 */
ui_error_t cupertino_slider_destroy(struct cupertino_slider *slider) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!slider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (slider->base) {
    rc = ui_slider_base_destroy(slider->base);
    slider->base = NULL;
  }

  C_MULTIPLATFORM_FREE(slider);
  return rc;
}

/**
 * @brief Sets current slider value.
 */
ui_error_t cupertino_slider_set_value(struct cupertino_slider *slider,
                                      float value) {
  if (!slider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (value < slider->min) {
    value = slider->min;
  } else if (value > slider->max) {
    value = slider->max;
  }

  return ui_slider_base_set_value(slider->base, value);
}

/**
 * @brief Queries current slider value.
 */
ui_error_t cupertino_slider_get_value(const struct cupertino_slider *slider,
                                      float *out_value) {
  if (!slider || !out_value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_slider_base_get_value(slider->base, out_value);
}

/**
 * @brief Sets discrete step increment with magnetic snapping.
 */
ui_error_t cupertino_slider_set_step(struct cupertino_slider *slider,
                                     float step) {
  if (!slider || step < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  slider->step = step;
  return ui_slider_base_set_step(slider->base, step);
}

/**
 * @brief Sets disabled state of the slider.
 */
ui_error_t cupertino_slider_set_disabled(struct cupertino_slider *slider,
                                         int disabled) {
  if (!slider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_slider_base_set_disabled(slider->base, disabled ? 1 : 0);
}

/**
 * @brief Retrieves underlying ui_slider_base handle.
 */
ui_error_t cupertino_slider_get_base(struct cupertino_slider *slider,
                                     struct ui_slider_base **out_base) {
  if (!slider || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_base = slider->base;
  return UI_ERROR_NONE;
}
