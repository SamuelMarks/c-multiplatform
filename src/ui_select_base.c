/* clang-format off */
#include "ui_select_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include "ui_web_bridge.h"
#include <stddef.h>
#include <stdio.h>
/* clang-format on */

/*
 * \file ui_select_base.c
 * \brief Select base component implementation.
 */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

/*
 * \brief Default CSS for select base.
 */
static const char *ui_select_base_default_css =
    "select { "
    "background: var(--select-bg, #fff); "
    "border: var(--select-border, 1px solid #ccc); "
    "padding: var(--select-padding, 4px 8px); "
    "color: var(--select-color, #000); "
    "cursor: pointer; "
    "} "
    "select[aria-disabled=\"true\"] { "
    "opacity: var(--select-disabled-opacity, 0.5); "
    "cursor: not-allowed; "
    "}";

/**
 * @struct ui_select_base
 * \brief ui_select_base structure.
 * \details Internal state for the select base component.
 */
struct ui_select_base {
  struct ui_component *component;                   /**< component */
  struct ui_gesture_recognizer *gesture_recognizer; /**< gesture_recognizer */
  int is_open;                                      /**< is_open */
  int disabled;                                     /**< disabled */
  int num_items;                                    /**< num_items */
  int highlighted_index;                            /**< highlighted_index */
  int selected_index;                               /**< selected_index */
  ui_select_on_change_t on_change;                  /**< on_change */
  void *change_user_data;                           /**< change_user_data */
  ui_select_on_open_change_t on_open_change;        /**< on_open_change */
  void *open_change_user_data; /**< open_change_user_data */
};

/*
 * \brief Updates the DOM state of the select component.
 * \param select The select base component.
 * \return UI_ERROR_NONE on success, or an error code.
 */
static ui_error_t update_dom_state(struct ui_select_base *select) {

#if defined(__EMSCRIPTEN__)
  if (select && select->component && select->component->host_node) {
  }
#endif
  if (select->is_open) {
    {
      ui_error_t set_rc1 = ui_dom_node_set_attribute(
          select->component->shadow_root, "aria-expanded", "true");
      if (set_rc1 != UI_ERROR_NONE) {
        return set_rc1;
      }
    }
  } else {
    {
      ui_error_t set_rc2 = ui_dom_node_set_attribute(
          select->component->shadow_root, "aria-expanded", "false");
      if (set_rc2 != UI_ERROR_NONE) {
        return set_rc2;
      }
    }
  }

  if (select->disabled) {
    {
      ui_error_t set_rc4 = ui_dom_node_set_attribute(
          select->component->shadow_root, "aria-disabled", "true");
#ifdef UI_TEST_MOCK_ALLOC
      extern int g_malloc_fail_countdown;
      if (g_malloc_fail_countdown == 0) {
        set_rc4 = UI_ERROR_OUT_OF_MEMORY;
        g_malloc_fail_countdown = -1;
      }
#endif
      if (set_rc4 != UI_ERROR_NONE) {
        return set_rc4;
      }
    }
  } else {
    {
      (void)ui_dom_node_remove_attribute(select->component->shadow_root,
                                         "disabled");
    }
    {
      (void)ui_dom_node_remove_attribute(select->component->shadow_root,
                                         "aria-disabled");
    }
  }
  return UI_ERROR_NONE;
}

/*
 * \brief Creates a new select base component.
 * \param out_select Pointer to store the created component.
 * \return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_select_base_create(struct ui_select_base **out_select) {
  struct ui_select_base *sel;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_select) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sel = (struct ui_select_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_select_base));
  if (!sel) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  sel->component = NULL;
  sel->gesture_recognizer = NULL;
  sel->is_open = 0;
  sel->disabled = 0;
  sel->num_items = 0;
  sel->highlighted_index = -1;
  sel->selected_index = -1;
  sel->on_change = NULL;
  sel->change_user_data = NULL;
  sel->on_open_change = NULL;
  sel->open_change_user_data = NULL;

  rc = ui_component_create(&sel->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_gesture_recognizer_create(&sel->gesture_recognizer);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(root_node, "select");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "tabindex", "0");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_css_parse_stylesheet(ui_select_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  {

    ui_error_t _ign_rc =
        ui_component_set_default_style(sel->component, default_style);

    (void)_ign_rc;
  }

  sel->component->shadow_root = root_node;
  root_node = NULL; /* Owned by component now */

  {
    ui_error_t upd_rc = update_dom_state(sel);
    if (upd_rc != UI_ERROR_NONE) {
      return upd_rc;
    }
  }

  *out_select = sel;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    (void)ui_dom_node_destroy(root_node);
  }
  if (sel->gesture_recognizer) {
    (void)ui_gesture_recognizer_destroy(sel->gesture_recognizer);
  }
  if (sel->component) {
    (void)ui_component_destroy(sel->component);
  }
  C_MULTIPLATFORM_FREE(sel);
  return rc;
}

/*
 * \brief Destroys a select base component.
 * \param select The select base component to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_select_base_destroy(struct ui_select_base *select) {
  if (!select)
    return UI_ERROR_NONE;
  if (select->gesture_recognizer)
    (void)ui_gesture_recognizer_destroy(select->gesture_recognizer);
  if (select->component)
    (void)ui_component_destroy(select->component);
  C_MULTIPLATFORM_FREE(select);
  return UI_ERROR_NONE;
}

/*
 * \brief Sets the disabled state of the select.
 * \param select The select component.
 * \param disabled The disabled state (1 for disabled, 0 for enabled).
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_select_base_set_disabled(struct ui_select_base *select,
                                       int disabled) {
  if (!select)
    return UI_ERROR_INVALID_ARGUMENT;
  select->disabled = disabled;
  if (disabled && select->is_open) {
    ui_error_t rc = ui_select_base_set_open(select, 0);
    if (rc != UI_ERROR_NONE)
      return rc;
  } else {
    ui_error_t rc = update_dom_state(select);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  return UI_ERROR_NONE;
}

/*
 * \brief Sets the item count of the select.
 * \param select The select component.
 * \param num_items The number of items.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_select_base_set_item_count(struct ui_select_base *select,
                                         int num_items) {
  if (!select)
    return UI_ERROR_INVALID_ARGUMENT;
  if (num_items < 0)
    num_items = 0;
  select->num_items = num_items;

  /* Re-clamp indices if necessary */
  if (select->highlighted_index >= num_items) {
    select->highlighted_index = num_items - 1;
  }
  if (select->selected_index >= num_items) {
    select->selected_index = num_items - 1;
  }

  return UI_ERROR_NONE;
}

/*
 * \brief Sets whether the select is open.
 * \param select The select component.
 * \param is_open 1 to open, 0 to close.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_select_base_set_open(struct ui_select_base *select, int is_open) {
  if (!select)
    return UI_ERROR_INVALID_ARGUMENT;
  if (select->disabled && is_open)
    return UI_ERROR_NONE;

  is_open = is_open ? 1 : 0;
  if (select->is_open != is_open) {
    select->is_open = is_open;

    /* Reset highlight to selected when opened */
    if (is_open) {
      select->highlighted_index = select->selected_index >= 0
                                      ? select->selected_index
                                      : (select->num_items > 0 ? 0 : -1);
    }

    {
      ui_error_t upd_rc2 = update_dom_state(select);
      if (upd_rc2 != UI_ERROR_NONE) {
        return upd_rc2;
      }
    }
    if (select->on_open_change) {
      {
        ui_error_t cb_rc = select->on_open_change(
            select, select->is_open, select->open_change_user_data);
        if (cb_rc != UI_ERROR_NONE) {
          return cb_rc;
        }
      }
    }
  }
  return UI_ERROR_NONE;
}

/*
 * \brief Checks if the select is open.
 * \param select The select component.
 * \param out_is_open Pointer to store the result.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_select_base_is_open(const struct ui_select_base *select,
                                  int *out_is_open) {
  if (!select)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_is_open = select->is_open;
  return UI_ERROR_NONE;
}

/*
 * \brief Sets the highlighted index.
 * \param select The select component.
 * \param index The index to highlight.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_select_base_set_highlighted_index(struct ui_select_base *select,
                                                int index) {
  if (!select)
    return UI_ERROR_INVALID_ARGUMENT;
  if (index < -1)
    index = -1;
  if (select->num_items > 0 && index >= select->num_items)
    index = select->num_items - 1;
  if (select->num_items == 0)
    index = -1;

  select->highlighted_index = index;
  return UI_ERROR_NONE;
}

/*
 * \brief Gets the highlighted index.
 * \param select The select component.
 * \param out_index Pointer to store the index.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_select_base_get_highlighted_index(const struct ui_select_base *select,
                                     int *out_index) {
  if (!select)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_index = select->highlighted_index;
  return UI_ERROR_NONE;
}

/*
 * \brief Sets the selected index.
 * \param select The select component.
 * \param index The selected index.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_select_base_set_selected_index(struct ui_select_base *select,
                                             int index) {
  if (!select)
    return UI_ERROR_INVALID_ARGUMENT;
  if (index < -1)
    index = -1;
  if (select->num_items > 0 && index >= select->num_items)
    index = select->num_items - 1;
  if (select->num_items == 0)
    index = -1;

  if (select->selected_index != index) {
    select->selected_index = index;

#if defined(__EMSCRIPTEN__)
    if (select->component && select->component->shadow_root) {
      char idx_str[32];
#if defined(_MSC_VER)
      sprintf_s(idx_str, sizeof(idx_str), "%d", index);
#else
      sprintf(idx_str, "%d", index);
#endif
      ui_web_bridge_set_property(
          (uint32_t)(uintptr_t)select->component->shadow_root, "selectedIndex",
          idx_str);
    }
#endif

    if (select->on_change) {
      {
        ui_error_t cb_rc2 = select->on_change(select, select->selected_index,
                                              select->change_user_data);
        if (cb_rc2 != UI_ERROR_NONE) {
          return cb_rc2;
        }
      }
    }
  }
  return UI_ERROR_NONE;
}

/*
 * \brief Gets the selected index.
 * \param select The select component.
 * \param out_index Pointer to store the index.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_select_base_get_selected_index(const struct ui_select_base *select,
                                  int *out_index) {
  if (!select)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_index = select->selected_index;
  return UI_ERROR_NONE;
}

/*
 * \brief Sets the on change callback.
 * \param select The select component.
 * \param on_change The callback function.
 * \param user_data User data for the callback.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_select_base_set_on_change(struct ui_select_base *select,
                                        ui_select_on_change_t on_change,
                                        void *user_data) {
  if (!select)
    return UI_ERROR_INVALID_ARGUMENT;
  select->on_change = on_change;
  select->change_user_data = user_data;
  return UI_ERROR_NONE;
}

/*
 * \brief Sets the on open change callback.
 * \param select The select component.
 * \param on_open_change The callback.
 * \param user_data User data for the callback.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_select_base_set_on_open_change(struct ui_select_base *select,
                                  ui_select_on_open_change_t on_open_change,
                                  void *user_data) {
  if (!select)
    return UI_ERROR_INVALID_ARGUMENT;
  select->on_open_change = on_open_change;
  select->open_change_user_data = user_data;
  return UI_ERROR_NONE;
}

/*
 * \brief Processes an event for the select.
 * \param select The select component.
 * \param event The event to process.
 * \param timestamp_ms The event timestamp.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_select_base_process_event(struct ui_select_base *select,
                                        const struct ui_event *event,
                                        double timestamp_ms) {
  (void)timestamp_ms;
  if (!select || !event)
    return UI_ERROR_INVALID_ARGUMENT;
  if (select->disabled)
    return UI_ERROR_NONE;

  if (event->type == UI_EVENT_KEY_DOWN) {
    if (!select->is_open) {
      if (event->event_data.keyboard.key_code == UI_KEY_ENTER ||
          event->event_data.keyboard.key_code == UI_KEY_SPACE ||
          event->event_data.keyboard.key_code == UI_KEY_DOWN ||
          event->event_data.keyboard.key_code == UI_KEY_UP) {
        return ui_select_base_set_open(select, 1);
      }
    } else {
      if (event->event_data.keyboard.key_code == UI_KEY_ESCAPE) {
        return ui_select_base_set_open(select, 0);
      } else if (event->event_data.keyboard.key_code == UI_KEY_ENTER ||
                 event->event_data.keyboard.key_code == UI_KEY_SPACE) {
        if (select->highlighted_index >= 0) {
          {
            ui_error_t set_rc = ui_select_base_set_selected_index(
                select, select->highlighted_index);
            if (set_rc != UI_ERROR_NONE) {
              return set_rc;
            }
          }
        }
        return ui_select_base_set_open(select, 0);
      } else if (event->event_data.keyboard.key_code == UI_KEY_DOWN) {
        int next = select->highlighted_index + 1;
        if (next >= select->num_items)
          next = select->num_items > 0 ? select->num_items - 1 : -1;
        return ui_select_base_set_highlighted_index(select, next);
      } else if (event->event_data.keyboard.key_code == UI_KEY_UP) {
        int prev = select->highlighted_index - 1;
        if (prev < 0)
          prev = select->num_items > 0 ? 0 : -1;
        return ui_select_base_set_highlighted_index(select, prev);
      } else if (event->event_data.keyboard.key_code == UI_KEY_HOME) {
        return ui_select_base_set_highlighted_index(
            select, select->num_items > 0 ? 0 : -1);
      } else if (event->event_data.keyboard.key_code == UI_KEY_END) {
        return ui_select_base_set_highlighted_index(
            select, select->num_items > 0 ? select->num_items - 1 : -1);
      }
    }
  }

  return UI_ERROR_NONE;
}

/*
 * \brief Writes a value to the select component (CVA).
 * \param component The select component.
 * \param value The value to write.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t select_cva_write_value(void *component,
                                         union ui_signal_payload value) {
  struct ui_select_base *select = (struct ui_select_base *)component;
  return ui_select_base_set_selected_index(select, value.int_val);
}

/**
 * @struct select_cva_wrapper
 * \brief select_cva_wrapper structure.
 * \details Wrapper for CVA change callbacks.
 */
struct select_cva_wrapper {
  ui_error_t (*callback)(union ui_signal_payload, void *); /**< ) */
  void *user_data;                                         /**< user_data */
};

/*
 * \brief Wrapper for on change callback.
 * \param select The select component.
 * \param index The new index.
 * \param user_data User data containing the wrapper.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t select_on_change_wrapper(struct ui_select_base *select,
                                           int index, void *user_data) {
  struct select_cva_wrapper *wrap = (struct select_cva_wrapper *)user_data;
  (void)select;
  if (wrap->callback) {
    union ui_signal_payload p;
    p.int_val = index;
    { (void)wrap->callback(p, wrap->user_data); }
  }
  return UI_ERROR_NONE;
}

/*
 * \brief Registers an on change callback for CVA.
 * \param component The select component.
 * \param callback The callback to register.
 * \param user_data User data for the callback.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t select_cva_register_on_change(
    void *component, ui_error_t (*callback)(union ui_signal_payload, void *),
    void *user_data) {
  struct ui_select_base *select = (struct ui_select_base *)component;
  struct select_cva_wrapper *wrap;
  if (!select)
    return UI_ERROR_INVALID_ARGUMENT;
  wrap = (struct select_cva_wrapper *)C_MULTIPLATFORM_MALLOC(sizeof(*wrap));
  if (!wrap)
    return UI_ERROR_OUT_OF_MEMORY;
  wrap->callback = callback;
  wrap->user_data = user_data;
  return ui_select_base_set_on_change(select, select_on_change_wrapper, wrap);
}

/*
 * \brief Registers an on touched callback for CVA.
 * \param component The select component.
 * \param callback The callback to register.
 * \param user_data User data for the callback.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t select_cva_register_on_touched(void *component,
                                                 ui_error_t (*callback)(void *),
                                                 void *user_data) {
  (void)component;
  (void)callback;
  (void)user_data;
  return UI_ERROR_NONE;
}

/*
 * \brief Sets the disabled state for CVA.
 * \param component The select component.
 * \param is_disabled The disabled state.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t select_cva_set_disabled_state(void *component,
                                                ui_bool_t is_disabled) {
  return ui_select_base_set_disabled((struct ui_select_base *)component,
                                     is_disabled);
}

/*
 * \brief Gets the Control Value Accessor for the select.
 * \param select The select component.
 * \param out_cva Pointer to store the CVA.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_select_base_get_cva(struct ui_select_base *select,
                                  struct ui_control_value_accessor *out_cva) {
  if (!select || !out_cva)
    return UI_ERROR_INVALID_ARGUMENT;
  out_cva->write_value = select_cva_write_value;
  out_cva->register_on_change = select_cva_register_on_change;
  out_cva->register_on_touched = select_cva_register_on_touched;
  out_cva->set_disabled_state = select_cva_set_disabled_state;
  return UI_ERROR_NONE;
}
/*
 * \brief Gets the base component for the select.
 * \param select The select component.
 * \param out_component Pointer to store the component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_select_base_get_component(struct ui_select_base *select,
                                        struct ui_component **out_component) {
  if (!select || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = select->component;
  return UI_ERROR_NONE;
}
/*
 * \brief Adds an option to the select.
 * \param select The select component.
 * \param label The option label.
 * \param value The option value.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_select_base_add_option(struct ui_select_base *select,
                                     const char *label, const char *value) {
  struct ui_dom_node *option_node = NULL;
  struct ui_dom_node *text_node = NULL;
  ui_error_t rc;

  if (!select || !label) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &option_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(option_node, "option");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  if (value) {
    rc = ui_dom_node_set_attribute(option_node, "value", value);
    if (rc != UI_ERROR_NONE) {
      goto cleanup;
    }
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_text_content(text_node, label);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  {

    ui_error_t _ign_rc = ui_dom_node_append_child(option_node, text_node);

    (void)_ign_rc;
  }
  text_node = NULL; /* Owned by option_node */

  {

    ui_error_t _ign_rc =
        ui_dom_node_append_child(select->component->shadow_root, option_node);

    (void)_ign_rc;
  }
  option_node = NULL; /* Owned by select */

  select->num_items++;
  return UI_ERROR_NONE;

cleanup:
  if (text_node) {
    (void)ui_dom_node_destroy(text_node);
  }
  if (option_node) {
    (void)ui_dom_node_destroy(option_node);
  }
  return rc;
}
