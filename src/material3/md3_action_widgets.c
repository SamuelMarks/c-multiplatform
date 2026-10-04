/**
 * @file md3_action_widgets.c
 * @brief Material 3 Action Widgets (fab_menu, button_group, toggle_button,
 * toolbar) implementation.
 */

/* clang-format off */
#include "material3/md3_action_widgets.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

/* ========================================================================= */
/* Button Group Implementation                                               */
/* ========================================================================= */

/**
 * @brief Creates a Material 3 Button Group.
 */
ui_error_t
md3_button_group_create(struct ui_engine *engine,
                        enum md3_button_group_orientation orientation,
                        struct md3_button_group **out_group) {
  struct md3_button_group *group;
  ui_error_t rc;

  if (!engine || !out_group ||
      (unsigned)orientation >= (unsigned)MD3_BUTTON_GROUP_ORIENTATION_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  group = (struct md3_button_group *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_button_group));
  if (!group) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(group, 0, sizeof(struct md3_button_group));
  group->orientation = orientation;
  group->selection_mode = MD3_BUTTON_GROUP_SELECTION_NONE;
  group->focused_index = 0;

  rc = ui_button_group_base_create(&group->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(group);
    return rc;
  }

  rc = ui_button_group_base_set_orientation(
      group->base, (orientation == MD3_BUTTON_GROUP_VERTICAL) ? 1 : 0);

  *out_group = group;
  return rc;
}

/**
 * @brief Destroys a Material 3 button group.
 */
ui_error_t md3_button_group_destroy(struct md3_button_group *group) {
  size_t i;
  ui_error_t rc = UI_ERROR_NONE;

  if (!group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (group->buttons) {
    for (i = 0; i < group->button_count; i++) {
      rc = md3_button_destroy(group->buttons[i]);
    }
    C_MULTIPLATFORM_FREE(group->buttons);
  }

  if (group->selected_states) {
    C_MULTIPLATFORM_FREE(group->selected_states);
  }

  C_MULTIPLATFORM_FREE(group->base);

  C_MULTIPLATFORM_FREE(group);
  return rc;
}

/**
 * @brief Adds a child button to the group.
 */
ui_error_t md3_button_group_add_button(struct md3_button_group *group,
                                       struct md3_button *button) {
  struct md3_button **new_buttons;
  int *new_states;
  size_t new_cap;
  size_t i;

  if (!group || !button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (group->button_count >= group->capacity) {
    new_cap = (group->capacity == 0) ? 4 : group->capacity * 2;
    new_buttons = (struct md3_button **)C_MULTIPLATFORM_MALLOC(
        sizeof(struct md3_button *) * new_cap);
    if (!new_buttons) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    new_states = (int *)C_MULTIPLATFORM_MALLOC(sizeof(int) * new_cap);
    if (!new_states) {
      C_MULTIPLATFORM_FREE(new_buttons);
      return UI_ERROR_OUT_OF_MEMORY;
    }

    if (group->buttons) {
      for (i = 0; i < group->button_count; i++) {
        new_buttons[i] = group->buttons[i];
        new_states[i] = group->selected_states[i];
      }
      C_MULTIPLATFORM_FREE(group->buttons);
      C_MULTIPLATFORM_FREE(group->selected_states);
    }

    group->buttons = new_buttons;
    group->selected_states = new_states;
    group->capacity = new_cap;
  }

  group->selected_states[group->button_count] = 0;
  group->buttons[group->button_count] = button;
  group->button_count++;

  return UI_ERROR_NONE;
}

/**
 * @brief Sets the selection mode of the group.
 */
ui_error_t
md3_button_group_set_selection_mode(struct md3_button_group *group,
                                    enum md3_button_group_selection_mode mode) {
  if (!group ||
      (unsigned)mode >= (unsigned)MD3_BUTTON_GROUP_SELECTION_MODE_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  group->selection_mode = mode;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets selection state for a specific button index.
 */
ui_error_t md3_button_group_set_selected(struct md3_button_group *group,
                                         size_t index, int selected) {
  size_t i;

  if (!group || index >= group->button_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (group->selection_mode == MD3_BUTTON_GROUP_SELECTION_SINGLE && selected) {
    for (i = 0; i < group->button_count; i++) {
      group->selected_states[i] = 0;
    }
  }

  group->selected_states[index] = selected ? 1 : 0;
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves selection state for a button index.
 */
ui_error_t md3_button_group_is_selected(const struct md3_button_group *group,
                                        size_t index, int *out_selected) {
  if (!group || index >= group->button_count || !out_selected) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_selected = group->selected_states[index];
  return UI_ERROR_NONE;
}

/**
 * @brief Roving tabindex navigation helper for arrow keys.
 */
ui_error_t md3_button_group_navigate(struct md3_button_group *group,
                                     int delta) {
  int new_idx;

  if (!group || group->button_count == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  new_idx = group->focused_index + delta;
  if (new_idx < 0) {
    new_idx = (int)group->button_count - 1;
  } else if ((size_t)new_idx >= group->button_count) {
    new_idx = 0;
  }
  group->focused_index = new_idx;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Toggle Button Implementation                                              */
/* ========================================================================= */

/**
 * @brief Creates a Material 3 Toggle Button.
 */
ui_error_t
md3_toggle_button_create(struct ui_engine *engine,
                         struct md3_toggle_button **out_toggle,
                         struct ui_control_value_accessor **out_cva) {
  struct md3_toggle_button *toggle;
  ui_error_t rc;

  if (!engine || !out_toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toggle = (struct md3_toggle_button *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_toggle_button));
  if (!toggle) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(toggle, 0, sizeof(struct md3_toggle_button));
  rc = ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &toggle->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(toggle);
    return rc;
  }

  if (out_cva) {
    *out_cva = (struct ui_control_value_accessor *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_control_value_accessor));
    if (!*out_cva) {
      ui_toggle_base_destroy(toggle->base);
      C_MULTIPLATFORM_FREE(toggle);
      return UI_ERROR_OUT_OF_MEMORY;
    }
    rc = ui_toggle_base_get_cva(toggle->base, *out_cva);
  }

  *out_toggle = toggle;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a Material 3 toggle button.
 */
ui_error_t md3_toggle_button_destroy(struct md3_toggle_button *toggle) {
  ui_error_t rc;

  if (!toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_toggle_base_destroy(toggle->base);

  C_MULTIPLATFORM_FREE(toggle);
  return rc;
}

/**
 * @brief Sets selected/pressed state on toggle button.
 */
ui_error_t md3_toggle_button_set_selected(struct md3_toggle_button *toggle,
                                          int selected) {
  if (!toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  toggle->is_selected = selected ? 1 : 0;
  return ui_toggle_base_set_checked(toggle->base, toggle->is_selected);
}

/**
 * @brief Retrieves selected state from toggle button.
 */
ui_error_t md3_toggle_button_is_selected(const struct md3_toggle_button *toggle,
                                         int *out_selected) {
  if (!toggle || !out_selected) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_toggle_base_is_checked(toggle->base, out_selected);
}

/**
 * @brief Sets text label on toggle button.
 */
ui_error_t md3_toggle_button_set_text(struct md3_toggle_button *toggle,
                                      const char *text) {
  if (!toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!text) {
    toggle->text[0] = '\0';
  } else {
#if defined(_MSC_VER)
    strncpy_s(toggle->text, sizeof(toggle->text), text, _TRUNCATE);
#else
    strncpy(toggle->text, text, sizeof(toggle->text) - 1);
    toggle->text[sizeof(toggle->text) - 1] = '\0';
#endif
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Sets icon token on toggle button.
 */
ui_error_t md3_toggle_button_set_icon(struct md3_toggle_button *toggle,
                                      const char *icon_name) {
  if (!toggle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!icon_name) {
    toggle->icon_name[0] = '\0';
  } else {
#if defined(_MSC_VER)
    strncpy_s(toggle->icon_name, sizeof(toggle->icon_name), icon_name,
              _TRUNCATE);
#else
    strncpy(toggle->icon_name, icon_name, sizeof(toggle->icon_name) - 1);
    toggle->icon_name[sizeof(toggle->icon_name) - 1] = '\0';
#endif
  }
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Toolbar Implementation                                                    */
/* ========================================================================= */

/**
 * @brief Creates a Material 3 Toolbar.
 */
ui_error_t md3_toolbar_create(struct ui_engine *engine,
                              struct md3_toolbar **out_toolbar) {
  struct md3_toolbar *tb;
  ui_error_t rc;

  if (!engine || !out_toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tb = (struct md3_toolbar *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_toolbar));
  if (!tb) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(tb, 0, sizeof(struct md3_toolbar));
  tb->variant = MD3_TOOLBAR_STANDARD;
  tb->focused_index = 0;

  rc = ui_toolbar_base_create(&tb->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  *out_toolbar = tb;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a Material 3 toolbar.
 */
ui_error_t md3_toolbar_destroy(struct md3_toolbar *toolbar) {
  struct md3_toolbar_item *curr;
  struct md3_toolbar_item *next;
  ui_error_t rc;

  if (!toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = toolbar->items;
  while (curr) {
    next = curr->next;
    C_MULTIPLATFORM_FREE(curr);
    curr = next;
  }

  rc = ui_toolbar_base_destroy(toolbar->base);

  C_MULTIPLATFORM_FREE(toolbar);
  return rc;
}

/**
 * @brief Sets toolbar density variant (Standard 48dp or Dense 40dp).
 */
ui_error_t md3_toolbar_set_variant(struct md3_toolbar *toolbar,
                                   enum md3_toolbar_variant variant) {
  if (!toolbar || (unsigned)variant >= (unsigned)MD3_TOOLBAR_VARIANT_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  toolbar->variant = variant;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets toolbar title.
 */
ui_error_t md3_toolbar_set_title(struct md3_toolbar *toolbar,
                                 const char *title) {
  if (!toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_toolbar_base_set_title(toolbar->base, title);
}

/**
 * @brief Adds an action item to toolbar.
 */
ui_error_t md3_toolbar_add_action(struct md3_toolbar *toolbar, int id,
                                  const char *title, float width) {
  struct md3_toolbar_item *item;
  struct md3_toolbar_item **tail;

  if (!toolbar || width <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  item = (struct md3_toolbar_item *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_toolbar_item));
  if (!item) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(item, 0, sizeof(struct md3_toolbar_item));
  item->id = id;
  item->width = width;
  item->is_overflow = 0;

  if (title) {
#if defined(_MSC_VER)
    strncpy_s(item->title, sizeof(item->title), title, _TRUNCATE);
#else
    strncpy(item->title, title, sizeof(item->title) - 1);
    item->title[sizeof(item->title) - 1] = '\0';
#endif
  }

  tail = &toolbar->items;
  while (*tail) {
    tail = &((*tail)->next);
  }
  *tail = item;
  toolbar->item_count++;

  return UI_ERROR_NONE;
}

/**
 * @brief Performs automatic responsive overflow calculation given available
 * width.
 */
ui_error_t md3_toolbar_calculate_overflow(struct md3_toolbar *toolbar,
                                          float available_width) {
  struct md3_toolbar_item *curr;
  float accumulated_w;
  float overflow_btn_width;

  if (!toolbar || available_width <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toolbar->available_width = available_width;
  toolbar->visible_count = 0;
  toolbar->overflow_count = 0;
  accumulated_w = 0.0f;
  overflow_btn_width = 48.0f; /* 48dp reserved for overflow button */

  curr = toolbar->items;
  while (curr) {
    if (accumulated_w + curr->width <= (available_width - overflow_btn_width)) {
      curr->is_overflow = 0;
      accumulated_w += curr->width;
      toolbar->visible_count++;
    } else {
      curr->is_overflow = 1;
      toolbar->overflow_count++;
    }
    curr = curr->next;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Roving tabindex navigation helper for toolbar actions.
 */
ui_error_t md3_toolbar_navigate(struct md3_toolbar *toolbar, int delta) {
  int new_idx;

  if (!toolbar || toolbar->item_count == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  new_idx = toolbar->focused_index + delta;
  if (new_idx < 0) {
    new_idx = (int)toolbar->item_count - 1;
  } else if ((size_t)new_idx >= toolbar->item_count) {
    new_idx = 0;
  }
  toolbar->focused_index = new_idx;
  return UI_ERROR_NONE;
}
