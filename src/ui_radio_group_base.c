/*
 * \file ui_radio_group_base.c
 * \brief Implementation of the UI Radio Group Base component.
 */

/* clang-format off */
#include "ui_radio_group_base.h"
#include "ui_internal_mem.h"
#include "ui_toggle_base.h"
#include <string.h>
/* clang-format on */

/**
 * @struct ui_radio_group_base
 * \brief Internal structure representing a radio group.
 */
struct ui_radio_group_base {
  struct ui_toggle_base **toggles;      /**< Array of toggles in the group */
  size_t count;                         /**< Number of toggles */
  size_t capacity;                      /**< Capacity of the array */
  struct ui_toggle_base *active_toggle; /**< Currently active toggle */
  ui_radio_group_on_change_t on_change; /**< Change callback */
  void *user_data;                      /**< User data for change callback */

  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data); /**< CVA change callback */
  void *cva_on_change_user_data;                /**< CVA change user data */

  ui_error_t (*cva_on_touched)(void *user_data); /**< CVA touched callback */
  void *cva_on_touched_user_data;                /**< CVA touched user data */
  int is_disabled;                               /**< Non-zero if disabled */
};

/*
 * \brief Triggers the CVA change callback.
 *
 * \param group The group.
 * \param active_index The active index.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t trigger_cva_change(struct ui_radio_group_base *group,
                                     int active_index) {
  if (group->cva_on_change) {
    union ui_signal_payload payload;
    payload.int_val = active_index;
    return group->cva_on_change(payload, group->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

/*
 * \brief Triggers the CVA touched callback.
 *
 * \param group The group.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t trigger_cva_touched(struct ui_radio_group_base *group) {
  if (group->cva_on_touched) {
    return group->cva_on_touched(group->cva_on_touched_user_data);
  }
  return UI_ERROR_NONE;
}

/*
 * \brief CVA method to write a value.
 *
 * \param component The radio group component.
 * \param value The payload containing the index.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t radio_group_cva_write_value(void *component,
                                              union ui_signal_payload value) {
  struct ui_radio_group_base *group = (struct ui_radio_group_base *)component;
  int index;

  if (!group)
    return UI_ERROR_INVALID_ARGUMENT;

  index = value.int_val;

  if (index >= 0 && index < (int)group->count) {
    (void)ui_radio_group_base_set_active(group, group->toggles[index]);
  } else {
    (void)ui_radio_group_base_set_active(group, NULL);
  }

  return UI_ERROR_NONE;
}

/*
 * \brief CVA method to register an on-change callback.
 *
 * \param component The radio group component.
 * \param callback The callback.
 * \param user_data Opaque user data.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t radio_group_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_radio_group_base *group = (struct ui_radio_group_base *)component;
  if (!group)
    return UI_ERROR_INVALID_ARGUMENT;
  group->cva_on_change = callback;
  group->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

/*
 * \brief CVA method to register an on-touched callback.
 *
 * \param component The radio group component.
 * \param callback The callback.
 * \param user_data Opaque user data.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t radio_group_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_radio_group_base *group = (struct ui_radio_group_base *)component;
  if (!group)
    return UI_ERROR_INVALID_ARGUMENT;
  group->cva_on_touched = callback;
  group->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

/*
 * \brief CVA method to set the disabled state.
 *
 * \param component The radio group component.
 * \param is_disabled The disabled state.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t radio_group_cva_set_disabled_state(void *component,
                                                     ui_bool_t is_disabled) {
  struct ui_radio_group_base *group = (struct ui_radio_group_base *)component;
  size_t i;
  if (!group)
    return UI_ERROR_INVALID_ARGUMENT;

  group->is_disabled = is_disabled;
  /* Disable all underlying toggles as well */
  for (i = 0; i < group->count; ++i) {
    /* Not exposing a dedicated disable in toggle_base public API yet,
       but we should probably just store it and prevent processing in group. */
  }

  return UI_ERROR_NONE;
}

/*
 * \brief Callback invoked when a child toggle changes.
 *
 * \param toggle The toggle that changed.
 * \param checked The new checked state.
 * \param user_data Pointer to the radio group.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t on_child_toggle_change(struct ui_toggle_base *toggle,
                                         int checked, void *user_data) {
  struct ui_radio_group_base *group = (struct ui_radio_group_base *)user_data;
  size_t i;
  ui_error_t rc = UI_ERROR_NONE;

  if (!checked) {
    /* In a standard radio group, you cannot uncheck an item by clicking it.
       If it got unchecked, it's either programmatic or another radio got
       checked. */
    group->active_toggle = NULL;
    return UI_ERROR_NONE;
  }

  /* An item was checked. Ensure all others are unchecked. */
  for (i = 0; i < group->count; ++i) {
    int is_checked = 0;
    if (group->toggles[i] != toggle) {
      (void)ui_toggle_base_is_checked(group->toggles[i], &is_checked);
      if (is_checked) {
        (void)ui_toggle_base_set_checked(group->toggles[i], 0);
      }
    }
  }

  group->active_toggle = toggle;

  if (group->on_change) {
    rc = group->on_change(group, toggle, group->user_data);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  for (i = 0; i < group->count; ++i) {
    if (group->toggles[i] == toggle) {
      (void)trigger_cva_change(group, (int)i);
    }
  }
  return UI_ERROR_NONE;
}

/*
 * \brief Creates a new radio group manager.
 *
 * \param out_group Pointer to receive the allocated radio group.
 * \param out_cva Optional pointer to receive the CVA interface.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_radio_group_base_create(struct ui_radio_group_base **out_group,
                           struct ui_control_value_accessor *out_cva) {
  struct ui_radio_group_base *group;

  if (!out_group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  group = (struct ui_radio_group_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_radio_group_base));
  if (!group) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  group->toggles = NULL;
  group->count = 0;
  group->capacity = 0;
  group->active_toggle = NULL;
  group->on_change = NULL;
  group->user_data = NULL;
  group->cva_on_change = NULL;
  group->cva_on_change_user_data = NULL;
  group->cva_on_touched = NULL;
  group->cva_on_touched_user_data = NULL;
  group->is_disabled = 0;

  if (out_cva) {
    out_cva->write_value = radio_group_cva_write_value;
    out_cva->register_on_change = radio_group_cva_register_on_change;
    out_cva->register_on_touched = radio_group_cva_register_on_touched;
    out_cva->set_disabled_state = radio_group_cva_set_disabled_state;
  }

  *out_group = group;
  return UI_ERROR_NONE;
}

/*
 * \brief Destroys a radio group manager.
 * Note: This does not destroy the individual ui_toggle_base components.
 *
 * \param group The radio group manager to destroy.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_radio_group_base_destroy(struct ui_radio_group_base *group) {
  size_t i;
  if (!group)
    return UI_ERROR_NONE;

  /* Unhook callbacks to prevent dangling pointers */
  for (i = 0; i < group->count; ++i) {
    (void)ui_toggle_base_set_on_change(group->toggles[i], NULL, NULL);
  }

  if (group->toggles) {
    C_MULTIPLATFORM_FREE(group->toggles);
  }
  C_MULTIPLATFORM_FREE(group);
  return UI_ERROR_NONE;
}

/*
 * \brief Adds a toggle (radio button) to the group.
 *
 * \param group The radio group manager.
 * \param toggle The toggle component to add.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_radio_group_base_add_toggle(struct ui_radio_group_base *group,
                                          struct ui_toggle_base *toggle) {
  size_t i;

  if (!group || !toggle)
    return UI_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < group->count; ++i) {
    if (group->toggles[i] == toggle)
      return UI_ERROR_NONE; /* Already added */
  }

  if (group->count >= group->capacity) {
    size_t new_cap = group->capacity * 2;
    struct ui_toggle_base **new_arr;
    if (new_cap == 0) {
      new_cap = 4;
    }
    new_arr = (struct ui_toggle_base **)C_MULTIPLATFORM_REALLOC(
        group->toggles, (size_t)new_cap * sizeof(struct ui_toggle_base *));
    if (!new_arr)
      return UI_ERROR_OUT_OF_MEMORY;
    group->toggles = new_arr;
    group->capacity = new_cap;
  }

  group->toggles[group->count++] = toggle;
  (void)ui_toggle_base_set_on_change(toggle, on_child_toggle_change, group);

  {
    int is_checked = 0;
    (void)ui_toggle_base_is_checked(toggle, &is_checked);
    if (is_checked) {
      (void)on_child_toggle_change(toggle, 1, group);
    }
  }

  return UI_ERROR_NONE;
}

/*
 * \brief Removes a toggle from the group.
 *
 * \param group The radio group manager.
 * \param toggle The toggle component to remove.
 * \return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND.
 */
ui_error_t ui_radio_group_base_remove_toggle(struct ui_radio_group_base *group,
                                             struct ui_toggle_base *toggle) {
  size_t i;
  int found = -1;

  if (!group || !toggle)
    return UI_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < group->count; ++i) {
    if (group->toggles[i] == toggle) {
      found = (int)i;
      break;
    }
  }

  if (found < 0)
    return UI_ERROR_NOT_FOUND;

  (void)ui_toggle_base_set_on_change(toggle, NULL, NULL);

  if (group->active_toggle == toggle) {
    group->active_toggle = NULL;
  }

  for (i = (size_t)found; i < group->count - 1; ++i) {
    group->toggles[i] = group->toggles[i + 1];
  }
  group->count--;

  return UI_ERROR_NONE;
}

/*
 * \brief Manually sets the active (checked) toggle in the group.
 * Unchecks all other toggles in this group.
 *
 * \param group The radio group manager.
 * \param toggle The toggle to set as active. If NULL, unchecks all.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_radio_group_base_set_active(struct ui_radio_group_base *group,
                                          struct ui_toggle_base *toggle) {
  size_t i;
  int valid_toggle = 0;

  if (!group)
    return UI_ERROR_INVALID_ARGUMENT;

  if (toggle) {
    for (i = 0; i < group->count; ++i) {
      if (group->toggles[i] == toggle) {
        valid_toggle = 1;
        break;
      }
    }
    if (!valid_toggle)
      return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < group->count; ++i) {
    if (group->toggles[i] == toggle) {
      (void)ui_toggle_base_set_checked(group->toggles[i], 1);
    } else {
      int is_checked = 0;
      (void)ui_toggle_base_is_checked(group->toggles[i], &is_checked);
      if (is_checked) {
        (void)ui_toggle_base_set_checked(group->toggles[i], 0);
      }
    }
  }

  group->active_toggle = toggle;
  return UI_ERROR_NONE;
}

/*
 * \brief Retrieves the currently active (checked) toggle in the group.
 *
 * \param group The radio group manager.
 * \param out_toggle Pointer to receive the active toggle, or NULL if none.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_radio_group_base_get_active(const struct ui_radio_group_base *group,
                               struct ui_toggle_base **out_toggle) {
  if (!group || !out_toggle)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_toggle = group->active_toggle;
  return UI_ERROR_NONE;
}

/*
 * \brief Sets the change handler for the radio group.
 *
 * \param group The radio group manager.
 * \param on_change The callback to invoke when the active radio changes.
 * \param user_data Opaque user data passed to the callback.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_radio_group_base_set_on_change(struct ui_radio_group_base *group,
                                  ui_radio_group_on_change_t on_change,
                                  void *user_data) {
  if (!group)
    return UI_ERROR_INVALID_ARGUMENT;
  group->on_change = on_change;
  group->user_data = user_data;
  return UI_ERROR_NONE;
}

/*
 * \brief Processes an input event for keyboard routing (Arrow keys) to cycle
 * selection. Typically, this is called when the group container or an active
 * radio receives key events.
 *
 * \param group The radio group manager.
 * \param event The input event.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_radio_group_base_process_event(struct ui_radio_group_base *group,
                                             const struct ui_event *event) {
  int active_idx = -1;
  size_t i;
  int next_idx;

  if (!group || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  if (group->is_disabled)
    return UI_ERROR_NONE;

  (void)trigger_cva_touched(group);

  if (group->count == 0)
    return UI_ERROR_NONE;

  if (event->type == UI_EVENT_KEY_DOWN) {
    int kc = event->event_data.keyboard.key_code;

    if (kc == UI_KEY_UP || kc == UI_KEY_LEFT || kc == UI_KEY_DOWN ||
        kc == UI_KEY_RIGHT) {
      for (i = 0; i < group->count; ++i) {
        if (group->toggles[i] == group->active_toggle) {
          active_idx = (int)i;
          break;
        }
      }

      if (active_idx < 0) {
        active_idx = 0; /* Default to first if none active */
      }

      if (kc == UI_KEY_UP || kc == UI_KEY_LEFT) {
        next_idx = active_idx - 1;
        if (next_idx < 0)
          next_idx = (int)group->count - 1;
      } else {
        next_idx = active_idx + 1;
        if (next_idx >= (int)group->count)
          next_idx = 0;
      }

      (void)ui_toggle_base_set_checked(group->toggles[next_idx], 1);
      (void)on_child_toggle_change(group->toggles[next_idx], 1, group);
    }
  }

  return UI_ERROR_NONE;
}
