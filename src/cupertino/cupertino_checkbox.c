/**
 * @file cupertino_checkbox.c
 * @brief Cupertino Checkbox component wrapping ui_checkbox_base
 * implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_checkbox.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_checkbox_mock_get_state_fail = 0;
static ui_error_t
mock_checkbox_base_get_state(struct ui_checkbox_base *base,
                             enum ui_checkbox_state *out_state) {
  if (g_cupertino_checkbox_mock_get_state_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_checkbox_base_get_state(base, out_state);
}
#undef ui_checkbox_base_get_state
/** @cond */
#define ui_checkbox_base_get_state mock_checkbox_base_get_state
/** @endcond */
#endif

static ui_error_t
cupertino_checkbox_cva_write_value(void *component,
                                   union ui_signal_payload value) {
  struct cupertino_checkbox *cb;
  enum ui_checkbox_state st;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  cb = (struct cupertino_checkbox *)component;
  st = value.bool_val ? UI_CHECKBOX_STATE_CHECKED : UI_CHECKBOX_STATE_UNCHECKED;
  return cupertino_checkbox_set_state(cb, st);
}

static ui_error_t
cupertino_checkbox_cva_set_disabled_state(void *component,
                                          ui_bool_t is_disabled) {
  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (is_disabled != UI_TRUE && is_disabled != UI_FALSE) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Creates a new Cupertino Checkbox component.
 */
ui_error_t
cupertino_checkbox_create(struct ui_engine *engine,
                          struct cupertino_checkbox **out_checkbox,
                          struct ui_control_value_accessor **out_cva) {
  struct cupertino_checkbox *cb;
  ui_error_t rc;

  if (!engine || !out_checkbox) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  cb = (struct cupertino_checkbox *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_checkbox));
  if (!cb) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(cb, 0, sizeof(struct cupertino_checkbox));
  cb->state = UI_CHECKBOX_STATE_UNCHECKED;
  cb->active_color = UI_COLOR_ARGB(255, 0x00, 0x7A, 0xFF); /* SystemBlue */
  cb->border_color = UI_COLOR_ARGB(255, 0xD1, 0xD1, 0xD6); /* SystemGray4 */

  rc = ui_checkbox_base_create(&cb->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(cb);
    return rc;
  }

  cb->cva.component = cb;
  cb->cva.write_value = cupertino_checkbox_cva_write_value;
  cb->cva.set_disabled_state = cupertino_checkbox_cva_set_disabled_state;
  cb->cva.register_on_change = NULL;
  cb->cva.register_on_touched = NULL;

  if (out_cva) {
    *out_cva = &cb->cva;
  }
  *out_checkbox = cb;

  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a Cupertino checkbox and its underlying base.
 */
ui_error_t cupertino_checkbox_destroy(struct cupertino_checkbox *cb) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!cb) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (cb->base) {
    rc = ui_checkbox_base_destroy(cb->base);
    cb->base = NULL;
  }

  C_MULTIPLATFORM_FREE(cb);
  return rc;
}

/**
 * @brief Sets tri-state status of the checkbox.
 */
ui_error_t cupertino_checkbox_set_state(struct cupertino_checkbox *cb,
                                        enum ui_checkbox_state state) {
  ui_error_t rc;

  if (!cb) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_checkbox_base_set_state(cb->base, state);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  cb->state = state;
  cb->stroke_progress = (state == UI_CHECKBOX_STATE_CHECKED) ? 1.0f : 0.0f;
  return UI_ERROR_NONE;
}

/**
 * @brief Gets current tri-state status of the checkbox.
 */
ui_error_t cupertino_checkbox_get_state(struct cupertino_checkbox *cb,
                                        enum ui_checkbox_state *out_state) {
  if (!cb || !out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_checkbox_base_get_state(cb->base, out_state);
}

/**
 * @brief Toggles checkbox state between checked and unchecked.
 */
ui_error_t cupertino_checkbox_toggle(struct cupertino_checkbox *cb) {
  ui_error_t rc;

  if (!cb) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_checkbox_base_toggle(cb->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_checkbox_base_get_state(cb->base, &cb->state);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  cb->stroke_progress = (cb->state == UI_CHECKBOX_STATE_CHECKED) ? 1.0f : 0.0f;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets optional label text for the checkbox.
 */
ui_error_t cupertino_checkbox_set_label(struct cupertino_checkbox *cb,
                                        const char *label) {
  ui_error_t rc;

  if (!cb || !label) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(cb->label, sizeof(cb->label), label, sizeof(cb->label) - 1);
#else
  strncpy(cb->label, label, sizeof(cb->label) - 1);
  cb->label[sizeof(cb->label) - 1] = '\0';
#endif

  rc = ui_checkbox_base_set_label(cb->base, label);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves underlying ui_checkbox_base handle.
 */
ui_error_t cupertino_checkbox_get_base(struct cupertino_checkbox *cb,
                                       struct ui_checkbox_base **out_base) {
  if (!cb || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_base = cb->base;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_checkbox_set_stroke_progress(struct cupertino_checkbox *cb,
                                                  float progress) {
  if (!cb || progress < 0.0f || progress > 1.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  cb->stroke_progress = progress;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_checkbox_get_stroke_progress(const struct cupertino_checkbox *cb,
                                       float *out_progress) {
  if (!cb || !out_progress) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_progress = cb->stroke_progress;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_checkbox_compute_checkmark_path(
    const struct cupertino_checkbox *cb, float *out_x1, float *out_y1,
    float *out_x2, float *out_y2, float *out_x3, float *out_y3) {
  float p;
  float seg1_frac;
  float seg2_frac;

  if (!cb || !out_x1 || !out_y1 || !out_x2 || !out_y2 || !out_x3 || !out_y3) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  p = cb->stroke_progress;
  *out_x1 = 4.5f;
  *out_y1 = 9.0f;

  seg1_frac = 1.0f / 3.0f;
  if (p <= seg1_frac) {
    float t = p / seg1_frac;
    *out_x2 = 4.5f + (7.5f - 4.5f) * t;
    *out_y2 = 9.0f + (13.0f - 9.0f) * t;
    *out_x3 = *out_x2;
    *out_y3 = *out_y2;
  } else {
    float t;
    *out_x2 = 7.5f;
    *out_y2 = 13.0f;
    seg2_frac = 2.0f / 3.0f;
    t = (p - seg1_frac) / seg2_frac;
    *out_x3 = 7.5f + (13.5f - 7.5f) * t;
    *out_y3 = 13.0f + (5.0f - 13.0f) * t;
  }

  return UI_ERROR_NONE;
}
