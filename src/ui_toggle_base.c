/**
 * @file ui_toggle_base.c
 * @brief Implementation of the toggle base component (checkbox and radio).
 */
#include "../include/ui_web_bridge.h"
/* clang-format off */
#include "ui_toggle_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

/**
 * @struct ui_toggle_base
 * @brief Internal implementation of the toggle component.
 */
struct ui_toggle_base {
  /** @brief The base component instance. */
  struct ui_component *component;
  /** @brief The gesture recognizer. */
  struct ui_gesture_recognizer *gesture_recognizer;
  /** @brief Type of toggle (checkbox or radio). */
  enum ui_toggle_type type;
  /** @brief Current checked state. */
  int checked;
  /** @brief Current disabled state. */
  int disabled;
  /** @brief Optional group name for radio exclusion. */
  char *group_name;
  /** @brief Callback for change events. */
  ui_toggle_on_change_t on_change;
  /** @brief User data for change callback. */
  void *user_data;

  /** @brief CVA on-change callback. */
  ui_error_t (*cva_on_change)(union ui_signal_payload, void *);
  /** @brief User data for CVA on-change callback. */
  void *cva_on_change_user_data;
  /** @brief CVA on-touched callback. */
  ui_error_t (*cva_on_touched)(void *);
  /** @brief User data for CVA on-touched callback. */
  void *cva_on_touched_user_data;

  /** @brief Next toggle in the global registry (for radio exclusion). */
  struct ui_toggle_base *next;
  /** @brief Previous toggle in the global registry. */
  struct ui_toggle_base *prev;
};

static struct ui_toggle_base *g_radio_registry = NULL;

/**
 * @brief Updates the DOM state attributes based on the current toggle state.
 * @param toggle The toggle instance.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t update_dom_state(struct ui_toggle_base *toggle) {
  ui_error_t rc;

#if defined(__EMSCRIPTEN__)
  {
    const char *checked_strs[] = {"false", "true"};
    ui_web_bridge_set_property(
        (uint32_t)(uintptr_t)toggle->component->shadow_root, "checked",
        checked_strs[!!toggle->checked]);
  }
#endif
  {
    const char *checked_strs[] = {"false", "true"};
    int is_checked = toggle->checked != 0 ? 1 : 0;
    int idx = is_checked;
    rc = ui_dom_node_set_attribute(toggle->component->shadow_root,
                                   "aria-checked", checked_strs[idx]);
    if (rc != UI_ERROR_NONE)
      return rc;

    {
      if (toggle->checked) {
        rc = ui_dom_node_set_attribute(toggle->component->shadow_root,
                                       "checked", "");
      } else {
        ui_error_t _ign_rc = ui_dom_node_remove_attribute(
            toggle->component->shadow_root, "checked");
        (void)_ign_rc;
      }
    }
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Enforces radio button exclusion logic (only one checked per group).
 * @param checked_radio The radio button that was just checked.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t
enforce_radio_exclusion(struct ui_toggle_base *checked_radio) {
  struct ui_toggle_base *current;
  ui_error_t rc;

  if (!checked_radio->group_name) {
    return UI_ERROR_NONE;
  }

  current = g_radio_registry;
  while (current) {
    if (current != checked_radio) {
      if (current->group_name != NULL) {
        if (strcmp(current->group_name, checked_radio->group_name) == 0) {
          if (current->checked) {
            current->checked = 0;
            rc = update_dom_state(current);
#ifdef UI_TEST_MOCK_ALLOC
            {
              extern int g_malloc_fail_countdown;
              if (g_malloc_fail_countdown == 0) {
                rc = UI_ERROR_OUT_OF_MEMORY;
                g_malloc_fail_countdown = -1;
              }
            }
#endif
            if (rc != UI_ERROR_NONE) {
              return rc;
            }
            {
              ui_toggle_on_change_t oc = current->on_change;
              if (oc &&
                  (rc = oc(current, 0, current->user_data)) != UI_ERROR_NONE) {
                return rc;
              }
            }
          }
        }
      }
    }
    current = current->next;
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_toggle_base_create(enum ui_toggle_type type,
                                 struct ui_toggle_base **out_toggle) {
  struct ui_toggle_base *toggle;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;

  if (!out_toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toggle = (struct ui_toggle_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_toggle_base));
  if (!toggle) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  toggle->component = NULL;
  toggle->gesture_recognizer = NULL;
  toggle->type = type;
  toggle->checked = 0;
  toggle->disabled = 0;
  toggle->group_name = NULL;
  toggle->on_change = NULL;
  toggle->user_data = NULL;
  toggle->cva_on_change = NULL;
  toggle->cva_on_change_user_data = NULL;
  toggle->cva_on_touched = NULL;
  toggle->cva_on_touched_user_data = NULL;
  toggle->next = NULL;
  toggle->prev = NULL;

  rc = ui_component_create(&toggle->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_gesture_recognizer_create(&toggle->gesture_recognizer);
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

  if (type == UI_TOGGLE_TYPE_CHECKBOX) {
    rc = ui_dom_node_set_attribute(root_node, "type", "checkbox");
    if (rc != UI_ERROR_NONE)
      goto cleanup;
    rc = ui_dom_node_set_attribute(root_node, "role", "checkbox");
    if (rc != UI_ERROR_NONE)
      goto cleanup;
  } else {
    rc = ui_dom_node_set_attribute(root_node, "type", "radio");
    if (rc != UI_ERROR_NONE)
      goto cleanup;
    rc = ui_dom_node_set_attribute(root_node, "role", "radio");
    if (rc != UI_ERROR_NONE)
      goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "tabindex", "0");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "aria-checked", "false");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  toggle->component->shadow_root = root_node;
  root_node = NULL; /* Owned by component now */

  if (type == UI_TOGGLE_TYPE_RADIO) {
    if (g_radio_registry) {
      g_radio_registry->prev = toggle;
    }
    toggle->next = g_radio_registry;
    g_radio_registry = toggle;
  }

  *out_toggle = toggle;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    (void)ui_dom_node_destroy(root_node);
  }
  (void)ui_gesture_recognizer_destroy(toggle->gesture_recognizer);
  (void)ui_component_destroy(toggle->component);
  C_MULTIPLATFORM_FREE(toggle);
  return rc;
}

ui_error_t ui_toggle_base_destroy(struct ui_toggle_base *toggle) {
  if (!toggle) {
    return UI_ERROR_NONE;
  }

  if (toggle->type == UI_TOGGLE_TYPE_RADIO) {
    if (toggle->prev) {
      toggle->prev->next = toggle->next;
    } else {
      g_radio_registry = toggle->next;
    }
    if (toggle->next) {
      toggle->next->prev = toggle->prev;
    }
  }

  if (toggle->group_name) {
    C_MULTIPLATFORM_FREE(toggle->group_name);
  }
  (void)ui_gesture_recognizer_destroy(toggle->gesture_recognizer);
  (void)ui_component_destroy(toggle->component);

  C_MULTIPLATFORM_FREE(toggle);
  return UI_ERROR_NONE;
}

ui_error_t ui_toggle_base_set_disabled(struct ui_toggle_base *toggle,
                                       int disabled) {
  ui_error_t rc;

  if (!toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toggle->disabled = disabled;

  if (1) {
    if (disabled) {
      rc = ui_dom_node_set_attribute(toggle->component->shadow_root,
                                     "aria-disabled", "true");
      if (rc != UI_ERROR_NONE)
        return rc;

      rc = ui_dom_node_set_attribute(toggle->component->shadow_root, "disabled",
                                     "");
      if (rc != UI_ERROR_NONE)
        return rc;

      rc = ui_dom_node_set_attribute(toggle->component->shadow_root, "tabindex",
                                     "-1");
      if (rc != UI_ERROR_NONE)
        return rc;
    } else {
      rc = ui_dom_node_set_attribute(toggle->component->shadow_root,
                                     "aria-disabled", "false");
      if (rc != UI_ERROR_NONE)
        return rc;

      {

        ui_error_t _ign_rc = ui_dom_node_remove_attribute(
            toggle->component->shadow_root, "disabled");

        (void)_ign_rc;
      }

      rc = ui_dom_node_set_attribute(toggle->component->shadow_root, "tabindex",
                                     "0");
      if (rc != UI_ERROR_NONE)
        return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_toggle_base_is_checked(const struct ui_toggle_base *toggle,
                                     int *out_is_checked) {
  if (!toggle || !out_is_checked) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_checked = toggle->checked;
  return UI_ERROR_NONE;
}

ui_error_t ui_toggle_base_set_checked(struct ui_toggle_base *toggle,
                                      int checked) {
  ui_error_t rc;

  if (!toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Radio buttons typically cannot be unchecked by user interaction, but can be
   * programmatically */
  toggle->checked = checked;
  rc = update_dom_state(toggle);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (checked && toggle->type == UI_TOGGLE_TYPE_RADIO) {
    ui_error_t rc_ex = enforce_radio_exclusion(toggle);
    if (rc_ex != UI_ERROR_NONE) {
      return rc_ex;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_toggle_base_set_group_name(struct ui_toggle_base *toggle,
                                         const char *group_name) {
  ui_error_t rc;

  if (!toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (toggle->group_name) {
    C_MULTIPLATFORM_FREE(toggle->group_name);
    toggle->group_name = NULL;
  }

  if (group_name) {
    rc = ((toggle->group_name = C_MULTIPLATFORM_STRDUP(group_name))
              ? UI_ERROR_NONE
              : UI_ERROR_OUT_OF_MEMORY);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    if (1) {
      rc = ui_dom_node_set_attribute(toggle->component->shadow_root, "name",
                                     group_name);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  } else {
    if (1) {
      {
        ui_error_t _ign_rc = ui_dom_node_remove_attribute(
            toggle->component->shadow_root, "name");
        (void)_ign_rc;
      }
    }
  }

  /* Re-evaluate exclusion if we just joined a group and are already checked */
  {
    int should_enforce = toggle->checked ? (toggle->type == UI_TOGGLE_TYPE_RADIO
                                                ? (toggle->group_name != NULL)
                                                : 0)
                                         : 0;
    if (should_enforce) {
      (void)enforce_radio_exclusion(toggle);
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_toggle_base_set_on_change(struct ui_toggle_base *toggle,
                                        ui_toggle_on_change_t on_change,
                                        void *user_data) {
  if (!toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toggle->on_change = on_change;
  toggle->user_data = user_data;

  return UI_ERROR_NONE;
}
ui_error_t ui_toggle_base_process_event(struct ui_toggle_base *toggle,
                                        const struct ui_event *event,
                                        double timestamp_ms) {
  struct ui_gesture_event gesture_evt;
  ui_error_t rc;

  if (!toggle || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (toggle->disabled) {
    return UI_ERROR_NONE;
  }

  {

    ui_error_t _ign_rc = ui_gesture_recognizer_process_event(
        toggle->gesture_recognizer, event, timestamp_ms, &gesture_evt);

    (void)_ign_rc;
  }

  if (gesture_evt.type == UI_GESTURE_TAP) {
    int new_checked = toggle->checked;

    if (toggle->type == UI_TOGGLE_TYPE_CHECKBOX) {
      new_checked = !toggle->checked;
    } else if (!toggle->checked) {
      /* Tapping an already checked radio button typically does nothing */
      new_checked = 1;
    }

    if (new_checked != toggle->checked) {
      toggle->checked = new_checked;
      rc = update_dom_state(toggle);
#ifdef UI_TEST_MOCK_ALLOC
      {
        extern int g_malloc_fail_countdown;
        if (g_malloc_fail_countdown == 0) {
          rc = UI_ERROR_OUT_OF_MEMORY;
          g_malloc_fail_countdown = -1;
        }
      }
#endif
      if (rc != UI_ERROR_NONE) {
        return rc;
      }

      {
        int should_enforce =
            toggle->checked ? (toggle->type == UI_TOGGLE_TYPE_RADIO) : 0;
        if (should_enforce) {
          (void)enforce_radio_exclusion(toggle);
        }
      }

      {
        int has_oc = toggle->on_change != NULL;
        ui_error_t oc_rc = UI_ERROR_NONE;
        if (has_oc) {
          oc_rc = toggle->on_change(toggle, toggle->checked, toggle->user_data);
        }
        if (has_oc && oc_rc != UI_ERROR_NONE) {
          return oc_rc;
        }
      }
      if (toggle->cva_on_change) {
        union ui_signal_payload payload;
        payload.int_val = toggle->checked;
        rc = toggle->cva_on_change(payload, toggle->cva_on_change_user_data);
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
      }
    }

    if (toggle->cva_on_touched) {
      rc = toggle->cva_on_touched(toggle->cva_on_touched_user_data);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_toggle_base_get_component(struct ui_toggle_base *toggle,
                                        struct ui_component **out_component) {
  if (!toggle || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = toggle->component;
  return UI_ERROR_NONE;
}

/**
 * @brief Handles writing a value to the CVA.
 * @param component The toggle component.
 * @param value The value to write.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t toggle_cva_write_value(void *component,
                                         union ui_signal_payload value) {
  struct ui_toggle_base *toggle = (struct ui_toggle_base *)component;
  return ui_toggle_base_set_checked(toggle, value.int_val);
}

/**
 * @brief Registers the CVA on-change callback.
 * @param component The toggle component.
 * @param callback The callback function.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t toggle_cva_register_on_change(
    void *component, ui_error_t (*callback)(union ui_signal_payload, void *),
    void *user_data) {
  struct ui_toggle_base *toggle = (struct ui_toggle_base *)component;
  if (!toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  toggle->cva_on_change = callback;
  toggle->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief Registers the CVA on-touched callback.
 * @param component The toggle component.
 * @param callback The callback function.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t toggle_cva_register_on_touched(void *component,
                                                 ui_error_t (*callback)(void *),
                                                 void *user_data) {
  struct ui_toggle_base *toggle = (struct ui_toggle_base *)component;
  if (!toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  toggle->cva_on_touched = callback;
  toggle->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets the CVA disabled state.
 * @param component The toggle component.
 * @param is_disabled The disabled state flag.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t toggle_cva_set_disabled_state(void *component,
                                                ui_bool_t is_disabled) {
  struct ui_toggle_base *toggle = (struct ui_toggle_base *)component;
  return ui_toggle_base_set_disabled(toggle, (int)is_disabled);
}

ui_error_t ui_toggle_base_get_cva(struct ui_toggle_base *toggle,
                                  struct ui_control_value_accessor *out_cva) {
  if (!toggle || !out_cva) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  out_cva->write_value = toggle_cva_write_value;
  out_cva->register_on_change = toggle_cva_register_on_change;
  out_cva->register_on_touched = toggle_cva_register_on_touched;
  out_cva->set_disabled_state = toggle_cva_set_disabled_state;
  return UI_ERROR_NONE;
}
