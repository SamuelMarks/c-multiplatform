#ifdef _MSC_VER
#pragma warning(disable : 4716)
#endif

/* clang-format off */
#include "ui_checkbox_base.h"
#include "ui_internal_mem.h"
#include "ui_web_bridge.h"
#include "ui_dom_node.h"
#include "ui_component.h"
#include <stddef.h>
/* clang-format on */

struct ui_checkbox_base {
  struct ui_component *component;
  enum ui_checkbox_state state;
};

static enum ui_error update_dom_state(struct ui_checkbox_base *checkbox) {
#if defined(__EMSCRIPTEN__)
  ui_web_bridge_set_property(
      (uint32_t)(uintptr_t)checkbox->component->shadow_root, "checked",
      checkbox->state == UI_CHECKBOX_STATE_CHECKED ? "true" : "false");
  ui_web_bridge_set_property(
      (uint32_t)(uintptr_t)checkbox->component->shadow_root, "indeterminate",
      checkbox->state == UI_CHECKBOX_STATE_INDETERMINATE ? "true" : "false");
#endif
  if (checkbox->state == UI_CHECKBOX_STATE_CHECKED) {
    ui_dom_node_set_attribute(checkbox->component->shadow_root, "checked", "");
    ui_dom_node_set_attribute(checkbox->component->shadow_root, "aria-checked",
                              "true");
  } else if (checkbox->state == UI_CHECKBOX_STATE_INDETERMINATE) {
    ui_dom_node_remove_attribute(checkbox->component->shadow_root, "checked");
    ui_dom_node_set_attribute(checkbox->component->shadow_root, "aria-checked",
                              "mixed");
  } else {
    ui_dom_node_remove_attribute(checkbox->component->shadow_root, "checked");
    ui_dom_node_set_attribute(checkbox->component->shadow_root, "aria-checked",
                              "false");
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_checkbox_base_create(struct ui_checkbox_base **out_checkbox) {
  struct ui_checkbox_base *checkbox;
  enum ui_error rc;
  struct ui_dom_node *root_node = NULL;

  if (!out_checkbox) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  checkbox =
      (struct ui_checkbox_base *)UI_MALLOC(sizeof(struct ui_checkbox_base));
  if (!checkbox) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  checkbox->component = NULL;
  checkbox->state = UI_CHECKBOX_STATE_UNCHECKED;

  rc = ui_component_create(&checkbox->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(root_node, "input");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "type", "checkbox");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  ui_dom_node_set_attribute(root_node, "role", "checkbox");
  ui_dom_node_set_attribute(root_node, "tabindex", "0");

  checkbox->component->shadow_root = root_node;
  root_node = NULL;

  update_dom_state(checkbox);

  *out_checkbox = checkbox;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    ui_dom_node_destroy(root_node);
  }
  if (checkbox->component) {
    ui_component_destroy(checkbox->component);
  }
  UI_FREE(checkbox);
  return rc;
}

enum ui_error ui_checkbox_base_destroy(struct ui_checkbox_base *checkbox) {
  if (!checkbox) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ui_component_destroy(checkbox->component);
  UI_FREE(checkbox);
  return UI_ERROR_NONE;
}

enum ui_error ui_checkbox_base_get_state(struct ui_checkbox_base *checkbox,
                                         enum ui_checkbox_state *out_state) {
  if (!checkbox || !out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_state = checkbox->state;
  return UI_ERROR_NONE;
}

enum ui_error ui_checkbox_base_set_state(struct ui_checkbox_base *checkbox,
                                         enum ui_checkbox_state state) {
  if (!checkbox) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (state != UI_CHECKBOX_STATE_UNCHECKED &&
      state != UI_CHECKBOX_STATE_CHECKED &&
      state != UI_CHECKBOX_STATE_INDETERMINATE) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  checkbox->state = state;
  update_dom_state(checkbox);
  return UI_ERROR_NONE;
}

static enum ui_error checkbox_cva_write_value(void *component,
                                              union ui_signal_payload value) {
  struct ui_checkbox_base *checkbox = (struct ui_checkbox_base *)component;
  return ui_checkbox_base_set_state(checkbox,
                                    (enum ui_checkbox_state)value.int_val);
}

/* No change callback exposed in ui_checkbox_base.h yet! We will mock it */
static enum ui_error checkbox_cva_register_on_change(
    void *component, enum ui_error (*callback)(union ui_signal_payload, void *),
    void *user_data) {
  (void)component;
  (void)callback;
  (void)user_data;
  return UI_ERROR_NONE;
}

/** \brief checkbox_cva_register_on_touched */
static enum ui_error checkbox_cva_register_on_touched(
    void *component, enum ui_error (*callback)(void *), void *user_data) {
  (void)component;
  (void)callback;
  (void)user_data;
  return UI_ERROR_NONE;
}

static enum ui_error checkbox_cva_set_disabled_state(void *component,
                                                     ui_bool_t is_disabled) {
  struct ui_checkbox_base *checkbox = (struct ui_checkbox_base *)component;
  if (!checkbox)
    return UI_ERROR_INVALID_ARGUMENT;

  if (is_disabled) {
    ui_dom_node_set_attribute(checkbox->component->shadow_root, "disabled", "");
    ui_dom_node_set_attribute(checkbox->component->shadow_root, "aria-disabled",
                              "true");
  } else {
    ui_dom_node_remove_attribute(checkbox->component->shadow_root, "disabled");
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_checkbox_base_get_cva(struct ui_checkbox_base *checkbox,
                         struct ui_control_value_accessor *out_cva) {
  if (!checkbox || !out_cva)
    return UI_ERROR_INVALID_ARGUMENT;
  out_cva->write_value = checkbox_cva_write_value;
  out_cva->register_on_change = checkbox_cva_register_on_change;
  out_cva->register_on_touched = checkbox_cva_register_on_touched;
  out_cva->set_disabled_state = checkbox_cva_set_disabled_state;
  return UI_ERROR_NONE;
}

enum ui_error ui_checkbox_base_toggle(struct ui_checkbox_base *checkbox) {
  if (!checkbox) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (checkbox->state == UI_CHECKBOX_STATE_CHECKED) {
    checkbox->state = UI_CHECKBOX_STATE_UNCHECKED;
  } else {
    checkbox->state = UI_CHECKBOX_STATE_CHECKED;
  }
  update_dom_state(checkbox);
  return UI_ERROR_NONE;
}
