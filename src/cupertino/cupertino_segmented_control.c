/**
 * @file cupertino_segmented_control.c
 * @brief Cupertino Segmented Control implementation wrapping
 * ui_segmented_control_base.
 */

/* clang-format off */
#include "cupertino/cupertino_segmented_control.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_segmented_control_mock_base_destroy_fail = 0;
int g_cupertino_segmented_control_mock_button_destroy_fail = 0;

static ui_error_t mock_ui_segmented_control_base_destroy(
    struct ui_segmented_control_base *control) {
  if (g_cupertino_segmented_control_mock_base_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_segmented_control_base_destroy(control);
}
#undef ui_segmented_control_base_destroy
/** @cond */
#define ui_segmented_control_base_destroy mock_ui_segmented_control_base_destroy
/** @endcond */

static ui_error_t
mock_ui_segmented_button_base_destroy(struct ui_segmented_button_base *btn) {
  if (g_cupertino_segmented_control_mock_button_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_segmented_button_base_destroy(btn);
}
#undef ui_segmented_button_base_destroy
/** @cond */
#define ui_segmented_button_base_destroy mock_ui_segmented_button_base_destroy
/** @endcond */
#endif

static ui_error_t
cupertino_segmented_control_cva_write_value(void *component,
                                            union ui_signal_payload value) {
  struct cupertino_segmented_control *control;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  control = (struct cupertino_segmented_control *)component;
  return cupertino_segmented_control_select_index(control, (int)value.int_val);
}

static ui_error_t
cupertino_segmented_control_cva_set_disabled_state(void *component,
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
 * @brief Creates a new Cupertino Segmented Control component.
 */
ui_error_t cupertino_segmented_control_create(
    struct ui_engine *engine, struct cupertino_segmented_control **out_control,
    struct ui_control_value_accessor **out_cva) {
  struct cupertino_segmented_control *control;
  ui_error_t rc;

  if (!engine || !out_control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  control = (struct cupertino_segmented_control *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_segmented_control));
  if (!control) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(control, 0, sizeof(struct cupertino_segmented_control));
  control->selected_index = -1;
  control->segment_count = 0;
  control->thumb_offset_x = 0.0f;
  control->thumb_width = 80.0f;
  control->bg_color = UI_COLOR_ARGB(255, 0xE5, 0xE5, 0xEA);    /* SystemGray5 */
  control->thumb_color = UI_COLOR_ARGB(255, 0xFF, 0xFF, 0xFF); /* White */

  rc = ui_segmented_control_base_create(&control->base, NULL);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(control);
    return rc;
  }

  control->cva.component = control;
  control->cva.write_value = cupertino_segmented_control_cva_write_value;
  control->cva.set_disabled_state =
      cupertino_segmented_control_cva_set_disabled_state;
  control->cva.register_on_change = NULL;
  control->cva.register_on_touched = NULL;

  if (out_cva) {
    *out_cva = &control->cva;
  }
  *out_control = control;

  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a Cupertino segmented control and its underlying base.
 */
ui_error_t cupertino_segmented_control_destroy(
    struct cupertino_segmented_control *control) {
  ui_error_t rc;

  if (!control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (control->base) {
    rc = ui_segmented_control_base_destroy(control->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    control->base = NULL;
  }

  C_MULTIPLATFORM_FREE(control);
  return UI_ERROR_NONE;
}

/**
 * @brief Appends a new text segment to the control.
 */
ui_error_t cupertino_segmented_control_add_segment(
    struct cupertino_segmented_control *control, const char *label,
    int *out_index) {
  struct ui_segmented_button_base *btn;
  ui_error_t rc;

  if (!control || !label || !out_index ||
      control->segment_count >= CUPERTINO_SEGMENTED_CONTROL_MAX_SEGMENTS) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_segmented_button_base_create(&btn);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_segmented_control_base_append_segment(control->base, btn);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_segmented_button_base_destroy(btn);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    return rc;
  }

  *out_index = control->segment_count++;
  if (control->selected_index < 0) {
    control->selected_index = 0;
    control->thumb_offset_x = 0.0f;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Selects segment by index, updating slider thumb position.
 */
ui_error_t cupertino_segmented_control_select_index(
    struct cupertino_segmented_control *control, int index) {
  if (!control || index < 0 || index >= control->segment_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  control->selected_index = index;
  control->thumb_offset_x = (float)index * control->thumb_width;

  return UI_ERROR_NONE;
}

/**
 * @brief Queries currently selected segment index.
 */
ui_error_t cupertino_segmented_control_get_selected_index(
    const struct cupertino_segmented_control *control, int *out_index) {
  if (!control || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_index = control->selected_index;
  return UI_ERROR_NONE;
}

/**
 * @brief Queries number of segments.
 */
ui_error_t cupertino_segmented_control_get_segment_count(
    const struct cupertino_segmented_control *control, int *out_count) {
  if (!control || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = control->segment_count;
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves underlying ui_segmented_control_base handle.
 */
ui_error_t cupertino_segmented_control_get_base(
    struct cupertino_segmented_control *control,
    struct ui_segmented_control_base **out_base) {
  if (!control || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = control->base;
  return UI_ERROR_NONE;
}
