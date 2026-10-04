/**
 * @file ui_scroll_field_base.c
 * @brief Implementation of the base unstyled scroll field component.
 */

/* clang-format off */
#include "ui_scroll_field_base.h"
#include "ui_dom_node.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct ui_scroll_field_base {
  struct ui_component *component;
  struct ui_dom_node *root_node;
  struct ui_dom_node *value_node;
  struct ui_dom_node *text_node;
  int min_val;
  int max_val;
  int step;
  int current_value;
  int looping;
};

static ui_error_t
sampler_update_spinbutton_attributes(struct ui_scroll_field_base *field) {
  char num_buf[32];
  ui_error_t rc;

#if defined(_MSC_VER)
  sprintf_s(num_buf, sizeof(num_buf), "%d", field->current_value);
#else
  sprintf(num_buf, "%d", field->current_value);
#endif
  rc = ui_dom_node_set_attribute(field->root_node, "aria-valuenow", num_buf);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

#if defined(_MSC_VER)
  sprintf_s(num_buf, sizeof(num_buf), "%d", field->min_val);
#else
  sprintf(num_buf, "%d", field->min_val);
#endif
  rc = ui_dom_node_set_attribute(field->root_node, "aria-valuemin", num_buf);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

#if defined(_MSC_VER)
  sprintf_s(num_buf, sizeof(num_buf), "%d", field->max_val);
#else
  sprintf(num_buf, "%d", field->max_val);
#endif
  rc = ui_dom_node_set_attribute(field->root_node, "aria-valuemax", num_buf);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

#if defined(_MSC_VER)
  sprintf_s(num_buf, sizeof(num_buf), "%d", field->current_value);
#else
  sprintf(num_buf, "%d", field->current_value);
#endif
  rc = ui_dom_node_set_text_content(field->text_node, num_buf);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t
ui_scroll_field_base_create(struct ui_scroll_field_base **out_field) {
  struct ui_scroll_field_base *field = NULL;
  ui_error_t rc;

  if (out_field == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  field = (struct ui_scroll_field_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_scroll_field_base));
  if (field == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(field, 0, sizeof(*field));

  field->min_val = 0;
  field->max_val = 100;
  field->step = 1;
  field->current_value = 0;
  field->looping = 0;

  rc = ui_component_create(&field->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(field);
    return rc;
  }

  /* Root container */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->root_node);
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(field->component);
    C_MULTIPLATFORM_FREE(field);
    return rc;
  }

  rc = ui_dom_node_set_tag_name(field->root_node, "div");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(field->root_node);
    ui_component_destroy(field->component);
    C_MULTIPLATFORM_FREE(field);
    return rc;
  }

  rc = ui_dom_node_set_attribute(field->root_node, "role", "spinbutton");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(field->root_node);
    ui_component_destroy(field->component);
    C_MULTIPLATFORM_FREE(field);
    return rc;
  }

  rc = ui_dom_node_set_attribute(field->root_node, "class", "ui-scroll-field");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(field->root_node);
    ui_component_destroy(field->component);
    C_MULTIPLATFORM_FREE(field);
    return rc;
  }

  rc = ui_dom_node_set_attribute(field->root_node, "tabindex", "0");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(field->root_node);
    ui_component_destroy(field->component);
    C_MULTIPLATFORM_FREE(field);
    return rc;
  }

  /* Value display node */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->value_node);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(field->root_node);
    ui_component_destroy(field->component);
    C_MULTIPLATFORM_FREE(field);
    return rc;
  }

  rc = ui_dom_node_set_tag_name(field->value_node, "span");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(field->value_node);
    ui_dom_node_destroy(field->root_node);
    ui_component_destroy(field->component);
    C_MULTIPLATFORM_FREE(field);
    return rc;
  }

  rc = ui_dom_node_set_attribute(field->value_node, "class",
                                 "ui-scroll-field-value");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(field->value_node);
    ui_dom_node_destroy(field->root_node);
    ui_component_destroy(field->component);
    C_MULTIPLATFORM_FREE(field);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &field->text_node);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(field->value_node);
    ui_dom_node_destroy(field->root_node);
    ui_component_destroy(field->component);
    C_MULTIPLATFORM_FREE(field);
    return rc;
  }

  rc = ui_dom_node_append_child(field->value_node, field->text_node);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(field->text_node);
    ui_dom_node_destroy(field->value_node);
    ui_dom_node_destroy(field->root_node);
    ui_component_destroy(field->component);
    C_MULTIPLATFORM_FREE(field);
    return rc;
  }

  rc = ui_dom_node_append_child(field->root_node, field->value_node);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(field->value_node);
    ui_dom_node_destroy(field->root_node);
    ui_component_destroy(field->component);
    C_MULTIPLATFORM_FREE(field);
    return rc;
  }

  rc = sampler_update_spinbutton_attributes(field);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(field->root_node);
    ui_component_destroy(field->component);
    C_MULTIPLATFORM_FREE(field);
    return rc;
  }

  field->component->shadow_root = field->root_node;
  *out_field = field;
  return UI_ERROR_NONE;
}

ui_error_t ui_scroll_field_base_destroy(struct ui_scroll_field_base *field) {
  if (field == NULL) {
    return UI_ERROR_NONE;
  }

  if (field->component != NULL) {
    ui_component_destroy(field->component);
    field->component = NULL;
  }

  C_MULTIPLATFORM_FREE(field);
  return UI_ERROR_NONE;
}

ui_error_t
ui_scroll_field_base_get_component(struct ui_scroll_field_base *field,
                                   struct ui_component **out_component) {
  if (field == NULL || out_component == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_component = field->component;
  return UI_ERROR_NONE;
}

ui_error_t ui_scroll_field_base_set_range(struct ui_scroll_field_base *field,
                                          int min_val, int max_val, int step) {
  if (field == NULL || min_val >= max_val || step <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  field->min_val = min_val;
  field->max_val = max_val;
  field->step = step;

  if (field->current_value < min_val) {
    field->current_value = min_val;
  } else if (field->current_value > max_val) {
    field->current_value = max_val;
  }

  return sampler_update_spinbutton_attributes(field);
}

ui_error_t
ui_scroll_field_base_get_range(const struct ui_scroll_field_base *field,
                               int *out_min_val, int *out_max_val,
                               int *out_step) {
  if (field == NULL || out_min_val == NULL || out_max_val == NULL ||
      out_step == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_min_val = field->min_val;
  *out_max_val = field->max_val;
  *out_step = field->step;

  return UI_ERROR_NONE;
}

ui_error_t ui_scroll_field_base_set_value(struct ui_scroll_field_base *field,
                                          int value) {
  if (field == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (field->looping) {
    int range_span = (field->max_val - field->min_val) + 1;
    while (value < field->min_val) {
      value += range_span;
    }
    while (value > field->max_val) {
      value -= range_span;
    }
  } else {
    if (value < field->min_val) {
      value = field->min_val;
    } else if (value > field->max_val) {
      value = field->max_val;
    }
  }

  field->current_value = value;
  return sampler_update_spinbutton_attributes(field);
}

ui_error_t
ui_scroll_field_base_get_value(const struct ui_scroll_field_base *field,
                               int *out_value) {
  if (field == NULL || out_value == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_value = field->current_value;
  return UI_ERROR_NONE;
}

ui_error_t ui_scroll_field_base_step_up(struct ui_scroll_field_base *field) {
  if (field == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_scroll_field_base_set_value(field,
                                        field->current_value + field->step);
}

ui_error_t ui_scroll_field_base_step_down(struct ui_scroll_field_base *field) {
  if (field == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_scroll_field_base_set_value(field,
                                        field->current_value - field->step);
}

ui_error_t ui_scroll_field_base_set_looping(struct ui_scroll_field_base *field,
                                            int looping) {
  if (field == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  field->looping = looping ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t
ui_scroll_field_base_is_looping(const struct ui_scroll_field_base *field,
                                int *out_looping) {
  if (field == NULL || out_looping == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_looping = field->looping;
  return UI_ERROR_NONE;
}
