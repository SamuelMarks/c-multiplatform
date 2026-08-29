/**
 * @file ui_checkbox_base.c
 * @brief Implementation of ui_checkbox_base.c.
 */
#ifdef _MSC_VER
#endif

/* clang-format off */
#include "ui_checkbox_base.h"
#include "ui_internal_mem.h"
#include "ui_web_bridge.h"
#include "ui_dom_node.h"
#include "ui_component.h"
#include <stddef.h>
/* clang-format on */
#ifdef UI_TEST_MOCK_ALLOC

int g_checkbox_mock_fail = 0;
int g_checkbox_mock_target = 0;
int g_checkbox_mock_current = 0;

/**
 * @brief mock_dom_node_set_attribute.
 * @param node Parameter node.
 * @param k Parameter k.
 * @param v Parameter v.
 * @return Return value.
 */
static ui_error_t mock_dom_node_set_attribute(struct ui_dom_node *node,
                                              const char *k, const char *v) {
  if (g_checkbox_mock_fail == 5) {
    if (++g_checkbox_mock_current == g_checkbox_mock_target)
      return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_set_attribute)(node, k, v);
}
#undef ui_dom_node_set_attribute
/** @cond */
#define ui_dom_node_set_attribute mock_dom_node_set_attribute
/** @endcond */

/**
 * @brief mock_dom_node_remove_attribute.
 * @param node Parameter node.
 * @param k Parameter k.
 * @return Return value.
 */
static ui_error_t mock_dom_node_remove_attribute(struct ui_dom_node *node,
                                                 const char *k) {
  if (g_checkbox_mock_fail == 5) {
    if (++g_checkbox_mock_current == g_checkbox_mock_target)
      return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_remove_attribute)(node, k);
}
#undef ui_dom_node_remove_attribute
/** @cond */
#define ui_dom_node_remove_attribute mock_dom_node_remove_attribute
/** @endcond */

#endif

/**
 * @struct ui_checkbox_base
 * \brief ui_checkbox_base
 */
struct ui_checkbox_base {
  struct ui_component *component; /**< component */
  enum ui_checkbox_state state;   /**< state */
  ui_signal_t *state_signal;      /**< state_signal */
  ui_signal_t *disabled_signal;   /**< disabled_signal */
};

/**
 * @brief update_dom_state.
 * @param checkbox Parameter checkbox.
 * @return Return value.
 */
static ui_error_t update_dom_state(struct ui_checkbox_base *checkbox) {
#if defined(__EMSCRIPTEN__)
  ui_web_bridge_set_property(
      (uint32_t)(uintptr_t)checkbox->component->shadow_root, "checked",
      checkbox->state == UI_CHECKBOX_STATE_CHECKED ? "true" : "false");
  ui_web_bridge_set_property(
      (uint32_t)(uintptr_t)checkbox->component->shadow_root, "indeterminate",
      checkbox->state == UI_CHECKBOX_STATE_INDETERMINATE ? "true" : "false");
#endif
  if (checkbox->state == UI_CHECKBOX_STATE_CHECKED) {
    ui_error_t rc = ui_dom_node_set_attribute(checkbox->component->shadow_root,
                                              "checked", "");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_dom_node_set_attribute(checkbox->component->shadow_root,
                                   "aria-checked", "true");
    if (rc != UI_ERROR_NONE)
      return rc;
  } else if (checkbox->state == UI_CHECKBOX_STATE_INDETERMINATE) {
    ui_error_t rc = ui_dom_node_remove_attribute(
        checkbox->component->shadow_root, "checked");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_dom_node_set_attribute(checkbox->component->shadow_root,
                                   "aria-checked", "mixed");
    if (rc != UI_ERROR_NONE)
      return rc;
  } else {
    ui_error_t rc = ui_dom_node_remove_attribute(
        checkbox->component->shadow_root, "checked");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_dom_node_set_attribute(checkbox->component->shadow_root,
                                   "aria-checked", "false");
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_checkbox_base_create.
 * @param out_checkbox Parameter out_checkbox.
 * @return Return value.
 */
ui_error_t ui_checkbox_base_create(struct ui_checkbox_base **out_checkbox) {
  struct ui_checkbox_base *checkbox;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;

  if (!out_checkbox) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  checkbox = (struct ui_checkbox_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_checkbox_base));
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

  rc = ui_dom_node_set_attribute(root_node, "role", "checkbox");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_attribute(root_node, "tabindex", "0");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  checkbox->component->shadow_root = root_node;
  root_node = NULL;

  rc = update_dom_state(checkbox);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  *out_checkbox = checkbox;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    (void)ui_dom_node_destroy(root_node);
  }
  if (checkbox->component) {
    (void)ui_component_destroy(checkbox->component);
  }
  C_MULTIPLATFORM_FREE(checkbox);
  return rc;
}

/**
 * @brief ui_checkbox_base_destroy.
 * @param checkbox Parameter checkbox.
 * @return Return value.
 */
ui_error_t ui_checkbox_base_destroy(struct ui_checkbox_base *checkbox) {
  if (!checkbox) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  (void)ui_component_destroy(checkbox->component);
  C_MULTIPLATFORM_FREE(checkbox);
  return UI_ERROR_NONE;
}

/**
 * @brief ui_checkbox_base_get_state.
 * @param checkbox Parameter checkbox.
 * @param out_state Parameter out_state.
 * @return Return value.
 */
ui_error_t ui_checkbox_base_get_state(struct ui_checkbox_base *checkbox,
                                      enum ui_checkbox_state *out_state) {
  if (!checkbox || !out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_state = checkbox->state;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_checkbox_base_set_state.
 * @param checkbox Parameter checkbox.
 * @param state Parameter state.
 * @return Return value.
 */
ui_error_t ui_checkbox_base_set_state(struct ui_checkbox_base *checkbox,
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
  return update_dom_state(checkbox);
}

/**
 * @brief checkbox_cva_write_value.
 * @param component Parameter component.
 * @param value Parameter value.
 * @return Return value.
 */
static ui_error_t checkbox_cva_write_value(void *component,
                                           union ui_signal_payload value) {
  struct ui_checkbox_base *checkbox = (struct ui_checkbox_base *)component;
  return ui_checkbox_base_set_state(checkbox,
                                    (enum ui_checkbox_state)value.int_val);
}

/* No change callback exposed in ui_checkbox_base.h yet! We will mock it */
/**
 * @brief checkbox_cva_register_on_change.
 * @param component Parameter component.
 * @param callback Parameter callback.
 * @param user_data Parameter user_data.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t checkbox_cva_register_on_change(
    void *component, ui_error_t (*callback)(union ui_signal_payload, void *),
    void *user_data) {
  (void)component;
  (void)callback;
  (void)user_data;
  return UI_ERROR_NONE;
}

/* \brief checkbox_cva_register_on_touched
 */
/**
 * @brief checkbox_cva_register_on_touched.
 * @param component Parameter component.
 * @param ) Parameter ).
 * @param user_data Parameter user_data.
 * @return Return value.
 * @param callback Parameter callback.
 */
static ui_error_t checkbox_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *), void *user_data) {
  (void)component;
  (void)callback;
  (void)user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief checkbox_cva_set_disabled_state.
 * @param component Parameter component.
 * @param is_disabled Parameter is_disabled.
 * @return Return value.
 */
static ui_error_t checkbox_cva_set_disabled_state(void *component,
                                                  ui_bool_t is_disabled) {
  struct ui_checkbox_base *checkbox = (struct ui_checkbox_base *)component;
  if (!checkbox)
    return UI_ERROR_INVALID_ARGUMENT;

  if (is_disabled) {
    ui_error_t rc = ui_dom_node_set_attribute(checkbox->component->shadow_root,
                                              "disabled", "");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_dom_node_set_attribute(checkbox->component->shadow_root,
                                   "aria-disabled", "true");
    if (rc != UI_ERROR_NONE)
      return rc;
  } else {
    ui_error_t rc = ui_dom_node_remove_attribute(
        checkbox->component->shadow_root, "disabled");
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
/**
 * @brief ui_checkbox_base_get_cva.
 * @param checkbox Parameter checkbox.
 * @param out_cva Parameter out_cva.
 * @return Return value.
 */
ui_error_t ui_checkbox_base_get_cva(struct ui_checkbox_base *checkbox,
                                    struct ui_control_value_accessor *out_cva) {
  if (!checkbox || !out_cva)
    return UI_ERROR_INVALID_ARGUMENT;
  out_cva->write_value = checkbox_cva_write_value;
  out_cva->register_on_change = checkbox_cva_register_on_change;
  out_cva->register_on_touched = checkbox_cva_register_on_touched;
  out_cva->set_disabled_state = checkbox_cva_set_disabled_state;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_checkbox_base_toggle.
 * @param checkbox Parameter checkbox.
 * @return Return value.
 */
ui_error_t ui_checkbox_base_toggle(struct ui_checkbox_base *checkbox) {
  if (!checkbox) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (checkbox->state == UI_CHECKBOX_STATE_CHECKED) {
    checkbox->state = UI_CHECKBOX_STATE_UNCHECKED;
  } else {
    checkbox->state = UI_CHECKBOX_STATE_CHECKED;
  }
  return update_dom_state(checkbox);
}
