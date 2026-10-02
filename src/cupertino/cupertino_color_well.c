/**
 * @file cupertino_color_well.c
 * @brief macOS/iPadOS Color Well Swatch Control implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_color_well.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

static ui_error_t color_well_cva_write_value(void *component,
                                             union ui_signal_payload value) {
  struct cupertino_color_well *well = (struct cupertino_color_well *)component;

  if (!well) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  well->current_color = (ui_color_t)value.int_val;
  return UI_ERROR_NONE;
}

static ui_error_t color_well_cva_set_disabled(void *component,
                                              ui_bool_t is_disabled) {
  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (is_disabled != UI_TRUE && is_disabled != UI_FALSE) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_color_well_create(struct ui_engine *engine,
                            const struct cupertino_color_well_descriptor *desc,
                            struct cupertino_color_well **out_well) {
  struct cupertino_color_well *well;
  ui_error_t rc;

  if (!engine || !desc || !out_well) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  well = (struct cupertino_color_well *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_color_well));
  if (!well) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(well, 0, sizeof(*well));
  well->current_color = desc->initial_color;
  well->width =
      (desc->width > 0.0f) ? desc->width : CUPERTINO_COLOR_WELL_DEFAULT_WIDTH;
  well->height = (desc->height > 0.0f) ? desc->height
                                       : CUPERTINO_COLOR_WELL_DEFAULT_HEIGHT;
  well->is_panel_open = 0;
  well->is_dragging = 0;

  rc = ui_color_picker_base_create(&well->base, NULL);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(well);
    return rc;
  }

  /* Setup CVA */
  well->cva.component = well;
  well->cva.write_value = color_well_cva_write_value;
  well->cva.set_disabled_state = color_well_cva_set_disabled;

  *out_well = well;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_color_well_destroy(struct cupertino_color_well *well) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!well) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (well->base) {
    rc = ui_color_picker_base_destroy(well->base);
    well->base = NULL;
  }

  C_MULTIPLATFORM_FREE(well);
  return rc;
}

ui_error_t cupertino_color_well_set_color(struct cupertino_color_well *well,
                                          ui_color_t color) {
  if (!well) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  well->current_color = color;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_color_well_get_color(const struct cupertino_color_well *well,
                               ui_color_t *out_color) {
  if (!well || !out_color) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_color = well->current_color;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_color_well_open_panel(struct cupertino_color_well *well) {
  if (!well) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  well->is_panel_open = 1;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_color_well_close_panel(struct cupertino_color_well *well) {
  if (!well) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  well->is_panel_open = 0;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_color_well_is_panel_open(const struct cupertino_color_well *well,
                                   int *out_is_open) {
  if (!well || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_open = well->is_panel_open;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_color_well_start_drag(struct cupertino_color_well *well) {
  if (!well) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  well->is_dragging = 1;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_color_well_end_drag(struct cupertino_color_well *well) {
  if (!well) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  well->is_dragging = 0;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_color_well_get_dimensions(const struct cupertino_color_well *well,
                                    float *out_width, float *out_height) {
  if (!well || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_width = well->width;
  *out_height = well->height;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_color_well_get_cva(struct cupertino_color_well *well,
                             struct ui_control_value_accessor **out_cva) {
  if (!well || !out_cva) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_cva = &well->cva;
  return UI_ERROR_NONE;
}
