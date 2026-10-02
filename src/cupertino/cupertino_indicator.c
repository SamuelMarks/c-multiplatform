/**
 * @file cupertino_indicator.c
 * @brief Cupertino Activity Indicator & Progress Bar implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_indicator.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_indicator_mock_create_fail = 0;
int g_cupertino_indicator_mock_destroy_fail = 0;
int g_cupertino_indicator_mock_set_indet_fail = 0;
int g_cupertino_indicator_mock_set_deter_fail = 0;

static ui_error_t mock_progress_base_create(struct ui_progress_base **b) {
  if (g_cupertino_indicator_mock_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  return ui_progress_base_create(b);
}
#undef ui_progress_base_create
/** @cond */
#define ui_progress_base_create mock_progress_base_create
/** @endcond */

static ui_error_t mock_progress_base_destroy(struct ui_progress_base *b) {
  if (g_cupertino_indicator_mock_destroy_fail) {
    (ui_progress_base_destroy)(b);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_progress_base_destroy)(b);
}
#undef ui_progress_base_destroy
/** @cond */
#define ui_progress_base_destroy mock_progress_base_destroy
/** @endcond */

static ui_error_t
mock_progress_base_set_indeterminate(struct ui_progress_base *b) {
  if (g_cupertino_indicator_mock_set_indet_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_progress_base_set_indeterminate(b);
}
#undef ui_progress_base_set_indeterminate
/** @cond */
#define ui_progress_base_set_indeterminate mock_progress_base_set_indeterminate
/** @endcond */

static ui_error_t mock_progress_base_set_determinate(struct ui_progress_base *b,
                                                     float v, float mn,
                                                     float mx) {
  if (g_cupertino_indicator_mock_set_deter_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_progress_base_set_determinate(b, v, mn, mx);
}
#undef ui_progress_base_set_determinate
/** @cond */
#define ui_progress_base_set_determinate mock_progress_base_set_determinate
/** @endcond */
#endif

static void
cupertino_indicator_update_spokes(struct cupertino_indicator *indicator) {
  int i;
  int count;

  count = indicator->spoke_count;
  for (i = 0; i < count; i++) {
    int dist;
    dist = (count + indicator->current_spoke_step - i) % count;
    indicator->spoke_opacities[i] =
        1.0f - (0.92f * ((float)dist / (float)count));
  }
}

ui_error_t
cupertino_indicator_create(struct ui_engine *engine,
                           const struct cupertino_indicator_descriptor *desc,
                           struct cupertino_indicator **out_indicator) {
  struct cupertino_indicator *ind;
  ui_error_t rc;

  if (!engine || !desc || !out_indicator) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ind = (struct cupertino_indicator *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_indicator));
  if (!ind) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(ind, 0, sizeof(*ind));
  ind->type = desc->type;
  ind->size = desc->size;
  ind->is_animating = desc->is_animating ? 1 : 0;
  ind->value = desc->initial_value;
  ind->spoke_count = (desc->size == CUPERTINO_INDICATOR_SIZE_LARGE) ? 12 : 8;

  rc = ui_progress_base_create(&ind->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(ind);
    return rc;
  }

  if (desc->is_indeterminate) {
    rc = ui_progress_base_set_indeterminate(ind->base);
    if (rc != UI_ERROR_NONE) {
      ui_error_t destroy_rc;
      destroy_rc = ui_progress_base_destroy(ind->base);
      if (destroy_rc != UI_ERROR_NONE) {
        /* Keep original error */
      }
      C_MULTIPLATFORM_FREE(ind);
      return rc;
    }
  } else {
    rc = ui_progress_base_set_determinate(ind->base, desc->initial_value, 0.0f,
                                          1.0f);
    if (rc != UI_ERROR_NONE) {
      ui_error_t destroy_rc;
      destroy_rc = ui_progress_base_destroy(ind->base);
      if (destroy_rc != UI_ERROR_NONE) {
        /* Keep original error */
      }
      C_MULTIPLATFORM_FREE(ind);
      return rc;
    }
  }

  cupertino_indicator_update_spokes(ind);

  *out_indicator = ind;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_indicator_destroy(struct cupertino_indicator *indicator) {
  ui_error_t rc;

  if (!indicator) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (indicator->base) {
    rc = ui_progress_base_destroy(indicator->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    indicator->base = NULL;
  }

  C_MULTIPLATFORM_FREE(indicator);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_indicator_start(struct cupertino_indicator *indicator) {
  if (!indicator) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  indicator->is_animating = 1;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_indicator_stop(struct cupertino_indicator *indicator) {
  if (!indicator) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  indicator->is_animating = 0;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_indicator_is_animating(const struct cupertino_indicator *indicator,
                                 int *out_animating) {
  if (!indicator || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_animating = indicator->is_animating;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_indicator_tick(struct cupertino_indicator *indicator,
                                    float delta_ms) {
  if (!indicator || delta_ms < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!indicator->is_animating) {
    return UI_ERROR_NONE;
  }

  indicator->step_timer_ms += delta_ms;
  while (indicator->step_timer_ms >= CUPERTINO_INDICATOR_STEP_INTERVAL_MS) {
    indicator->step_timer_ms -= CUPERTINO_INDICATOR_STEP_INTERVAL_MS;
    indicator->current_spoke_step =
        (indicator->current_spoke_step + 1) % indicator->spoke_count;
  }

  cupertino_indicator_update_spokes(indicator);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_indicator_get_spoke_opacity(
    const struct cupertino_indicator *indicator, size_t spoke_index,
    float *out_opacity) {
  if (!indicator || spoke_index >= (size_t)indicator->spoke_count ||
      !out_opacity) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_opacity = indicator->spoke_opacities[spoke_index];
  return UI_ERROR_NONE;
}

ui_error_t cupertino_indicator_set_value(struct cupertino_indicator *indicator,
                                         float value) {
  ui_error_t rc;

  if (!indicator) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (value < 0.0f) {
    value = 0.0f;
  } else if (value > 1.0f) {
    value = 1.0f;
  }

  indicator->value = value;

  if (indicator->base) {
    rc = ui_progress_base_set_determinate(indicator->base, value, 0.0f, 1.0f);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_indicator_get_value(const struct cupertino_indicator *indicator,
                              float *out_value) {
  if (!indicator || !out_value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_value = indicator->value;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_indicator_get_dimensions(const struct cupertino_indicator *indicator,
                                   float *out_width, float *out_height) {
  if (!indicator || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (indicator->type == CUPERTINO_INDICATOR_TYPE_ACTIVITY) {
    if (indicator->size == CUPERTINO_INDICATOR_SIZE_LARGE) {
      *out_width = CUPERTINO_INDICATOR_LARGE_DIM;
      *out_height = CUPERTINO_INDICATOR_LARGE_DIM;
    } else {
      *out_width = CUPERTINO_INDICATOR_SMALL_DIM;
      *out_height = CUPERTINO_INDICATOR_SMALL_DIM;
    }
  } else {
    *out_width = 0.0f; /* Flexible */
    *out_height = CUPERTINO_INDICATOR_BAR_HEIGHT;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_indicator_get_base(struct cupertino_indicator *indicator,
                                        struct ui_progress_base **out_base) {
  if (!indicator || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = indicator->base;
  return UI_ERROR_NONE;
}
