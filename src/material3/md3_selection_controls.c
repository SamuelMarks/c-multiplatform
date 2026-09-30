/**
 * @file md3_selection_controls.c
 * @brief Material 3 Selection & Input Controls Implementation.
 */

/* clang-format off */
#include "material3/md3_selection_controls.h"
#include "ui_engine.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/* ========================================================================= */
/* md3_autocomplete                                                          */
/* ========================================================================= */

ui_error_t md3_autocomplete_create(struct ui_engine *engine,
                                   enum md3_text_field_variant variant,
                                   struct md3_autocomplete **out_autocomplete,
                                   struct ui_control_value_accessor *out_cva) {
  struct md3_autocomplete *auto_c;
  ui_error_t rc;

  if (!engine || !out_autocomplete) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  auto_c = (struct md3_autocomplete *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_autocomplete));
  if (!auto_c) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(auto_c, 0, sizeof(struct md3_autocomplete));

  rc = ui_autocomplete_base_create(&auto_c->base, out_cva);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(auto_c);
    return rc;
  }

  auto_c->variant = variant;
  auto_c->is_open = 0;
  auto_c->query[0] = '\0';
  auto_c->dropdown_elevation = 2.0f;
  auto_c->corner_radius = (variant == MD3_TEXT_FIELD_FILLED) ? 4.0f : 8.0f;
  auto_c->expressive_spring_enabled = 1;

  *out_autocomplete = auto_c;
  return UI_ERROR_NONE;
}

ui_error_t md3_autocomplete_destroy(struct md3_autocomplete *autocomplete) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!autocomplete) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_autocomplete_base_destroy(autocomplete->base);

  C_MULTIPLATFORM_FREE(autocomplete);
  return rc;
}

ui_error_t md3_autocomplete_set_query(struct md3_autocomplete *autocomplete,
                                      const char *query) {
  size_t len;

  if (!autocomplete || !query) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  len = strlen(query);
  if (len >= sizeof(autocomplete->query)) {
    len = sizeof(autocomplete->query) - 1;
  }

#if defined(_MSC_VER)
  strncpy_s(autocomplete->query, sizeof(autocomplete->query), query, len);
#else
  strncpy(autocomplete->query, query, len);
#endif
  autocomplete->query[len] = '\0';

  return UI_ERROR_NONE;
}

ui_error_t
md3_autocomplete_get_query(const struct md3_autocomplete *autocomplete,
                           char *out_query, size_t query_capacity) {
  size_t len;

  if (!autocomplete || !out_query || query_capacity == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  len = strlen(autocomplete->query);
  if (len >= query_capacity) {
    len = query_capacity - 1;
  }

#if defined(_MSC_VER)
  strncpy_s(out_query, query_capacity, autocomplete->query, len);
#else
  strncpy(out_query, autocomplete->query, len);
#endif
  out_query[len] = '\0';

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_select                                                                */
/* ========================================================================= */

ui_error_t md3_select_create(struct ui_engine *engine,
                             enum md3_text_field_variant variant,
                             struct md3_select **out_select,
                             struct ui_control_value_accessor *out_cva) {
  struct md3_select *sel;
  ui_error_t rc;

  if (!engine || !out_select) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sel = (struct md3_select *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_select));
  if (!sel) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(sel, 0, sizeof(struct md3_select));

  rc = ui_select_base_create(&sel->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(sel);
    return rc;
  }

  if (out_cva) {
    rc = ui_select_base_get_cva(sel->base, out_cva);
  }

  sel->variant = variant;
  sel->is_multi = 0;
  sel->is_open = 0;
  sel->arrow_rotation_deg = 0.0f;
  sel->option_count = 0;
  sel->selected_index = -1;
  sel->placeholder[0] = '\0';

  *out_select = sel;
  return UI_ERROR_NONE;
}

ui_error_t md3_select_destroy(struct md3_select *select) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!select) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_select_base_destroy(select->base);

  C_MULTIPLATFORM_FREE(select);
  return rc;
}

ui_error_t md3_select_add_option(struct md3_select *select, const char *label,
                                 const char *value) {
  size_t idx;
  ui_error_t rc;

  if (!select || !label || !value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (select->option_count >=
      sizeof(select->options) / sizeof(select->options[0])) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  idx = select->option_count;
#if defined(_MSC_VER)
  strncpy_s(select->options[idx].label, sizeof(select->options[idx].label),
            label, _TRUNCATE);
  strncpy_s(select->options[idx].value, sizeof(select->options[idx].value),
            value, _TRUNCATE);
#else
  strncpy(select->options[idx].label, label,
          sizeof(select->options[idx].label) - 1);
  select->options[idx].label[sizeof(select->options[idx].label) - 1] = '\0';
  strncpy(select->options[idx].value, value,
          sizeof(select->options[idx].value) - 1);
  select->options[idx].value[sizeof(select->options[idx].value) - 1] = '\0';
#endif
  select->options[idx].is_selected = 0;
  select->options[idx].is_disabled = 0;
  select->option_count++;

  rc = ui_select_base_add_option(select->base, label, value);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return ui_select_base_set_item_count(select->base, (int)select->option_count);
}

ui_error_t md3_select_set_multiselect(struct md3_select *select, int is_multi) {
  if (!select) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  select->is_multi = is_multi ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_select_set_open(struct md3_select *select, int is_open) {
  if (!select) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  select->is_open = is_open ? 1 : 0;
  select->arrow_rotation_deg = is_open ? 180.0f : 0.0f;

  return ui_select_base_set_open(select->base, select->is_open);
}

ui_error_t md3_select_set_selected_index(struct md3_select *select, int index) {
  size_t i;

  if (!select) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (index >= 0 && (size_t)index >= select->option_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  select->selected_index = index;
  for (i = 0; i < select->option_count; ++i) {
    select->options[i].is_selected = ((int)i == index) ? 1 : 0;
  }

  return ui_select_base_set_selected_index(select->base, index);
}

ui_error_t md3_select_get_selected_index(const struct md3_select *select,
                                         int *out_index) {
  if (!select || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_index = select->selected_index;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_pin_input                                                             */
/* ========================================================================= */

ui_error_t md3_pin_input_create(struct ui_engine *engine, int length,
                                struct md3_pin_input **out_pin_input,
                                struct ui_control_value_accessor *out_cva) {
  struct md3_pin_input *pin;
  ui_error_t rc;

  if (!engine || !out_pin_input || length <= 0 || length > 15) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  pin = (struct md3_pin_input *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_pin_input));
  if (!pin) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(pin, 0, sizeof(struct md3_pin_input));

  rc = ui_pin_input_base_create(&pin->base, length, out_cva);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(pin);
    return rc;
  }

  pin->length = length;
  pin->is_masked = 0;
  pin->focused_index = 0;
  pin->cell_width = 48.0f;
  pin->cell_height = 56.0f;
  pin->is_valid = 1;

  *out_pin_input = pin;
  return UI_ERROR_NONE;
}

ui_error_t md3_pin_input_destroy(struct md3_pin_input *pin_input) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!pin_input) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_pin_input_base_destroy(pin_input->base);

  C_MULTIPLATFORM_FREE(pin_input);
  return rc;
}

ui_error_t md3_pin_input_set_masked(struct md3_pin_input *pin_input,
                                    int is_masked) {
  if (!pin_input) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  pin_input->is_masked = is_masked ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_pin_input_on_input(struct md3_pin_input *pin_input, int index,
                                  const char *c) {
  if (!pin_input || !c || index < 0 || index >= pin_input->length) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  pin_input->values[index] = c[0];
  if (index + 1 < pin_input->length) {
    pin_input->focused_index = index + 1;
  }

  return ui_pin_input_base_on_input(pin_input->base, index, c);
}

ui_error_t md3_pin_input_on_backspace(struct md3_pin_input *pin_input,
                                      int index) {
  if (!pin_input || index < 0 || index >= pin_input->length) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  pin_input->values[index] = '\0';
  if (index > 0) {
    pin_input->focused_index = index - 1;
  }

  return ui_pin_input_base_on_backspace(pin_input->base, index);
}

ui_error_t md3_pin_input_on_paste(struct md3_pin_input *pin_input,
                                  const char *pasted_text) {
  size_t len, i;

  if (!pin_input || !pasted_text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  len = strlen(pasted_text);
  for (i = 0; i < (size_t)pin_input->length && i < len; ++i) {
    pin_input->values[i] = pasted_text[i];
  }
  if (len < (size_t)pin_input->length) {
    pin_input->focused_index = (int)len;
  } else {
    pin_input->focused_index = pin_input->length - 1;
  }

  return ui_pin_input_base_on_paste(pin_input->base, pasted_text);
}

ui_error_t md3_pin_input_get_value(const struct md3_pin_input *pin_input,
                                   char *out_text, size_t text_capacity) {
  int i;
  size_t count;

  if (!pin_input || !out_text || text_capacity == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  count = 0;
  for (i = 0; i < pin_input->length && count + 1 < text_capacity; ++i) {
    if (pin_input->values[i] != '\0') {
      out_text[count++] = pin_input->values[i];
    }
  }
  out_text[count] = '\0';

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_rating                                                                */
/* ========================================================================= */

ui_error_t md3_rating_create(struct ui_engine *engine, int max_rating,
                             struct md3_rating **out_rating,
                             struct ui_control_value_accessor *out_cva) {
  struct md3_rating *rating;
  ui_error_t rc;

  if (!engine || !out_rating || max_rating <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rating =
      (struct md3_rating *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_rating));
  if (!rating) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(rating, 0, sizeof(struct md3_rating));

  rc = ui_rating_base_create(&rating->base, out_cva);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(rating);
    return rc;
  }

  rc = ui_rating_base_set_max(rating->base, (unsigned int)max_rating);

  rating->max_rating = max_rating;
  rating->value = 0.0f;
  rating->hover_preview_value = 0.0f;
  rating->is_read_only = 0;
  rating->allow_half_stars = 1;

  *out_rating = rating;
  return rc;
}

ui_error_t md3_rating_destroy(struct md3_rating *rating) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!rating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_rating_base_destroy(rating->base);

  C_MULTIPLATFORM_FREE(rating);
  return rc;
}

ui_error_t md3_rating_set_value(struct md3_rating *rating, float value) {
  if (!rating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (value < 0.0f) {
    value = 0.0f;
  } else if (value > (float)rating->max_rating) {
    value = (float)rating->max_rating;
  }

  rating->value = value;
  return ui_rating_base_set_value(rating->base, value);
}

ui_error_t md3_rating_get_value(const struct md3_rating *rating,
                                float *out_value) {
  if (!rating || !out_value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_value = rating->value;
  return UI_ERROR_NONE;
}

ui_error_t md3_rating_set_hover_preview(struct md3_rating *rating,
                                        float preview_value) {
  if (!rating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (preview_value < 0.0f) {
    preview_value = 0.0f;
  } else if (preview_value > (float)rating->max_rating) {
    preview_value = (float)rating->max_rating;
  }
  rating->hover_preview_value = preview_value;
  return UI_ERROR_NONE;
}

ui_error_t md3_rating_set_read_only(struct md3_rating *rating,
                                    int is_read_only) {
  if (!rating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  rating->is_read_only = is_read_only ? 1 : 0;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_spin_button                                                           */
/* ========================================================================= */

ui_error_t md3_spin_button_create(struct ui_engine *engine, double min,
                                  double max, double step,
                                  struct md3_spin_button **out_spin_button,
                                  struct ui_control_value_accessor *out_cva) {
  struct md3_spin_button *spin;
  ui_error_t rc;

  if (!engine || !out_spin_button || min > max || step <= 0.0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  spin = (struct md3_spin_button *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_spin_button));
  if (!spin) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(spin, 0, sizeof(struct md3_spin_button));

  rc = ui_spin_button_base_create(&spin->base, out_cva);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(spin);
    return rc;
  }

  rc = ui_spin_button_base_set_min(spin->base, min);
  rc = ui_spin_button_base_set_max(spin->base, max);
  rc = ui_spin_button_base_set_step(spin->base, step);
  rc = ui_spin_button_base_set_value(spin->base, min);

  spin->min_val = min;
  spin->max_val = max;
  spin->step_val = step;
  spin->current_val = min;
  spin->precision = 2;
  spin->prefix[0] = '\0';
  spin->suffix[0] = '\0';
  spin->is_horizontal = 0;

  *out_spin_button = spin;
  return rc;
}

ui_error_t md3_spin_button_destroy(struct md3_spin_button *spin_button) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!spin_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_spin_button_base_destroy(spin_button->base);

  C_MULTIPLATFORM_FREE(spin_button);
  return rc;
}

ui_error_t md3_spin_button_set_value(struct md3_spin_button *spin_button,
                                     double value) {
  if (!spin_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (value < spin_button->min_val) {
    value = spin_button->min_val;
  } else if (value > spin_button->max_val) {
    value = spin_button->max_val;
  }

  spin_button->current_val = value;
  return ui_spin_button_base_set_value(spin_button->base, value);
}

ui_error_t md3_spin_button_get_value(const struct md3_spin_button *spin_button,
                                     double *out_value) {
  if (!spin_button || !out_value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_value = spin_button->current_val;
  return UI_ERROR_NONE;
}

ui_error_t md3_spin_button_step(struct md3_spin_button *spin_button,
                                int step_count) {
  double new_val;

  if (!spin_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  new_val =
      spin_button->current_val + (double)step_count * spin_button->step_val;
  return md3_spin_button_set_value(spin_button, new_val);
}
