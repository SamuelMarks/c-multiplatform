/**
 * @file cupertino_stepper.c
 * @brief Cupertino Stepper component wrapping ui_spin_button_base
 * implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_stepper.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_stepper_mock_set_min_fail = 0;
int g_cupertino_stepper_mock_set_max_fail = 0;
int g_cupertino_stepper_mock_set_step_fail = 0;
int g_cupertino_stepper_mock_set_value_fail = 0;
int g_cupertino_stepper_mock_destroy_fail = 0;
int g_cupertino_stepper_mock_increment_fail = 0;
int g_cupertino_stepper_mock_decrement_fail = 0;
int g_cupertino_stepper_mock_set_disabled_fail = 0;
int g_cupertino_stepper_mock_on_tick_fail = 0;

static ui_error_t mock_spin_button_base_set_min(struct ui_spin_button_base *sb,
                                                double min) {
  if (g_cupertino_stepper_mock_set_min_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_spin_button_base_set_min(sb, min);
}
#undef ui_spin_button_base_set_min
/** @cond */
#define ui_spin_button_base_set_min mock_spin_button_base_set_min
/** @endcond */

static ui_error_t mock_spin_button_base_set_max(struct ui_spin_button_base *sb,
                                                double max) {
  if (g_cupertino_stepper_mock_set_max_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_spin_button_base_set_max(sb, max);
}
#undef ui_spin_button_base_set_max
/** @cond */
#define ui_spin_button_base_set_max mock_spin_button_base_set_max
/** @endcond */

static ui_error_t mock_spin_button_base_set_step(struct ui_spin_button_base *sb,
                                                 double step) {
  if (g_cupertino_stepper_mock_set_step_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_spin_button_base_set_step(sb, step);
}
#undef ui_spin_button_base_set_step
/** @cond */
#define ui_spin_button_base_set_step mock_spin_button_base_set_step
/** @endcond */

static ui_error_t
mock_spin_button_base_set_value(struct ui_spin_button_base *sb, double val) {
  if (g_cupertino_stepper_mock_set_value_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_spin_button_base_set_value(sb, val);
}
#undef ui_spin_button_base_set_value
/** @cond */
#define ui_spin_button_base_set_value mock_spin_button_base_set_value
/** @endcond */

static ui_error_t
mock_spin_button_base_destroy(struct ui_spin_button_base *sb) {
  if (g_cupertino_stepper_mock_destroy_fail) {
    ui_spin_button_base_destroy(sb);
    return UI_ERROR_UNKNOWN;
  }
  return ui_spin_button_base_destroy(sb);
}
#undef ui_spin_button_base_destroy
/** @cond */
#define ui_spin_button_base_destroy mock_spin_button_base_destroy
/** @endcond */

static ui_error_t
mock_spin_button_base_increment(struct ui_spin_button_base *sb) {
  if (g_cupertino_stepper_mock_increment_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_spin_button_base_increment(sb);
}
#undef ui_spin_button_base_increment
/** @cond */
#define ui_spin_button_base_increment mock_spin_button_base_increment
/** @endcond */

static ui_error_t
mock_spin_button_base_decrement(struct ui_spin_button_base *sb) {
  if (g_cupertino_stepper_mock_decrement_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_spin_button_base_decrement(sb);
}
#undef ui_spin_button_base_decrement
/** @cond */
#define ui_spin_button_base_decrement mock_spin_button_base_decrement
/** @endcond */

static ui_error_t
mock_spin_button_base_set_disabled(struct ui_spin_button_base *sb,
                                   int disabled) {
  if (g_cupertino_stepper_mock_set_disabled_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_spin_button_base_set_disabled(sb, disabled);
}
#undef ui_spin_button_base_set_disabled
/** @cond */
#define ui_spin_button_base_set_disabled mock_spin_button_base_set_disabled
/** @endcond */

static ui_error_t mock_spin_button_base_on_tick(struct ui_spin_button_base *sb,
                                                double delta_ms) {
  if (g_cupertino_stepper_mock_on_tick_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_spin_button_base_on_tick(sb, delta_ms);
}
#undef ui_spin_button_base_on_tick
/** @cond */
#define ui_spin_button_base_on_tick mock_spin_button_base_on_tick
/** @endcond */
#endif

static ui_error_t
cupertino_stepper_cva_write_value(void *component,
                                  union ui_signal_payload value) {
  struct cupertino_stepper *stepper;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  stepper = (struct cupertino_stepper *)component;
  return cupertino_stepper_set_value(stepper, (double)value.float_val);
}

static ui_error_t
cupertino_stepper_cva_set_disabled_state(void *component,
                                         ui_bool_t is_disabled) {
  struct cupertino_stepper *stepper;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  stepper = (struct cupertino_stepper *)component;
  return cupertino_stepper_set_disabled(stepper, is_disabled ? 1 : 0);
}

/**
 * @brief Creates a new Cupertino Stepper component.
 */
ui_error_t
cupertino_stepper_create(struct ui_engine *engine, double min, double max,
                         double step, struct cupertino_stepper **out_stepper,
                         struct ui_control_value_accessor **out_cva) {
  struct cupertino_stepper *stepper;
  ui_error_t rc;

  if (!engine || !out_stepper || min > max || step <= 0.0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  stepper = (struct cupertino_stepper *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_stepper));
  if (!stepper) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(stepper, 0, sizeof(struct cupertino_stepper));
  stepper->min = min;
  stepper->max = max;
  stepper->step = step;
  stepper->value = min;
  stepper->is_disabled = 0;
  stepper->auto_repeat = 1;
  stepper->background_color =
      UI_COLOR_ARGB(255, 0xE5, 0xE5, 0xEA);                   /* SystemGray5 */
  stepper->tint_color = UI_COLOR_ARGB(255, 0x00, 0x7A, 0xFF); /* SystemBlue */
  stepper->separator_color =
      UI_COLOR_ARGB(77, 0x3C, 0x3C, 0x43); /* Separator */

  rc = ui_spin_button_base_create(&stepper->base, NULL);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(stepper);
    return rc;
  }

  rc = ui_spin_button_base_set_min(stepper->base, min);
  if (rc != UI_ERROR_NONE) {
    ui_spin_button_base_destroy(stepper->base);
    C_MULTIPLATFORM_FREE(stepper);
    return rc;
  }

  rc = ui_spin_button_base_set_max(stepper->base, max);
  if (rc != UI_ERROR_NONE) {
    ui_spin_button_base_destroy(stepper->base);
    C_MULTIPLATFORM_FREE(stepper);
    return rc;
  }

  rc = ui_spin_button_base_set_step(stepper->base, step);
  if (rc != UI_ERROR_NONE) {
    ui_spin_button_base_destroy(stepper->base);
    C_MULTIPLATFORM_FREE(stepper);
    return rc;
  }

  rc = ui_spin_button_base_set_value(stepper->base, min);
  if (rc != UI_ERROR_NONE) {
    ui_spin_button_base_destroy(stepper->base);
    C_MULTIPLATFORM_FREE(stepper);
    return rc;
  }

  stepper->cva.component = stepper;
  stepper->cva.write_value = cupertino_stepper_cva_write_value;
  stepper->cva.set_disabled_state = cupertino_stepper_cva_set_disabled_state;
  stepper->cva.register_on_change = NULL;
  stepper->cva.register_on_touched = NULL;

  if (out_cva) {
    *out_cva = &stepper->cva;
  }
  *out_stepper = stepper;

  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a Cupertino stepper and releases all resources.
 */
ui_error_t cupertino_stepper_destroy(struct cupertino_stepper *stepper) {
  ui_error_t rc;

  if (!stepper) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (stepper->base) {
    rc = ui_spin_button_base_destroy(stepper->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    stepper->base = NULL;
  }

  C_MULTIPLATFORM_FREE(stepper);
  return UI_ERROR_NONE;
}

/**
 * @brief Sets current stepper value (clamped between min and max).
 */
ui_error_t cupertino_stepper_set_value(struct cupertino_stepper *stepper,
                                       double value) {
  ui_error_t rc;

  if (!stepper) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (value < stepper->min) {
    value = stepper->min;
  } else if (value > stepper->max) {
    value = stepper->max;
  }

  rc = ui_spin_button_base_set_value(stepper->base, value);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  stepper->value = value;
  return UI_ERROR_NONE;
}

/**
 * @brief Queries current stepper value.
 */
ui_error_t cupertino_stepper_get_value(const struct cupertino_stepper *stepper,
                                       double *out_value) {
  if (!stepper || !out_value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_spin_button_base_get_value(stepper->base, out_value);
}

/**
 * @brief Increments current stepper value by step.
 */
ui_error_t cupertino_stepper_increment(struct cupertino_stepper *stepper) {
  ui_error_t rc;

  if (!stepper) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (stepper->is_disabled) {
    return UI_ERROR_NONE;
  }

  rc = ui_spin_button_base_increment(stepper->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return ui_spin_button_base_get_value(stepper->base, &stepper->value);
}

/**
 * @brief Decrements current stepper value by step.
 */
ui_error_t cupertino_stepper_decrement(struct cupertino_stepper *stepper) {
  ui_error_t rc;

  if (!stepper) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (stepper->is_disabled) {
    return UI_ERROR_NONE;
  }

  rc = ui_spin_button_base_decrement(stepper->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return ui_spin_button_base_get_value(stepper->base, &stepper->value);
}

/**
 * @brief Sets step amount.
 */
ui_error_t cupertino_stepper_set_step(struct cupertino_stepper *stepper,
                                      double step) {
  ui_error_t rc;

  if (!stepper || step <= 0.0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_spin_button_base_set_step(stepper->base, step);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  stepper->step = step;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets minimum and maximum range bounds.
 */
ui_error_t cupertino_stepper_set_range(struct cupertino_stepper *stepper,
                                       double min, double max) {
  ui_error_t rc;

  if (!stepper || min > max) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_spin_button_base_set_min(stepper->base, min);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_spin_button_base_set_max(stepper->base, max);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  stepper->min = min;
  stepper->max = max;

  if (stepper->value < min) {
    rc = cupertino_stepper_set_value(stepper, min);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else if (stepper->value > max) {
    rc = cupertino_stepper_set_value(stepper, max);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Enables or disables auto-repeat on continuous press-and-hold.
 */
ui_error_t cupertino_stepper_set_auto_repeat(struct cupertino_stepper *stepper,
                                             int auto_repeat) {
  if (!stepper) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  stepper->auto_repeat = auto_repeat ? 1 : 0;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets disabled state of the stepper.
 */
ui_error_t cupertino_stepper_set_disabled(struct cupertino_stepper *stepper,
                                          int disabled) {
  ui_error_t rc;

  if (!stepper) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  stepper->is_disabled = disabled ? 1 : 0;
  rc = ui_spin_button_base_set_disabled(stepper->base, stepper->is_disabled);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Advances continuous hold timer by delta_ms.
 */
ui_error_t cupertino_stepper_on_tick(struct cupertino_stepper *stepper,
                                     double delta_ms) {
  ui_error_t rc;

  if (!stepper || delta_ms < 0.0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (stepper->is_disabled || !stepper->auto_repeat) {
    return UI_ERROR_NONE;
  }

  rc = ui_spin_button_base_on_tick(stepper->base, delta_ms);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return ui_spin_button_base_get_value(stepper->base, &stepper->value);
}

/**
 * @brief Retrieves underlying ui_spin_button_base CDK handle.
 */
ui_error_t cupertino_stepper_get_base(struct cupertino_stepper *stepper,
                                      struct ui_spin_button_base **out_base) {
  if (!stepper || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = stepper->base;
  return UI_ERROR_NONE;
}
