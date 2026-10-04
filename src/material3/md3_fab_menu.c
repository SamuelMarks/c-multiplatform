/**
 * @file md3_fab_menu.c
 * @brief Implementation of Material 3 FAB Menu / Speed Dial component.
 */

/* clang-format off */
#include "material3/md3_fab_menu.h"
#include "ui_internal_mem.h"
#include "ui_component.h"
#include <string.h>
/* clang-format on */

struct md3_fab_menu_action {
  int id;
  struct md3_fab *fab;
  char *label;
  struct md3_fab_menu_action *next;
};

struct md3_fab_menu {
  struct ui_speed_dial_base base;
  struct ui_component *component;
  enum md3_fab_menu_direction direction;
  struct md3_fab *primary_fab;
  int is_expanded;
  struct md3_fab_menu_action *actions;
};

ui_error_t md3_fab_menu_create(struct ui_engine *engine,
                               enum md3_fab_menu_direction direction,
                               struct md3_fab_menu **out_menu) {
  struct md3_fab_menu *menu;
  ui_error_t rc;

  if (!engine || !out_menu || direction >= MD3_FAB_MENU_DIRECTION_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  menu = (struct md3_fab_menu *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_fab_menu));
  if (!menu) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(menu, 0, sizeof(struct md3_fab_menu));

  rc = ui_component_create(&menu->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(menu);
    return rc;
  }

  rc = ui_speed_dial_base_init(&menu->base, menu->component);
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(menu->component);
    C_MULTIPLATFORM_FREE(menu);
    return rc;
  }

  menu->direction = direction;
  *out_menu = menu;
  return UI_ERROR_NONE;
}

ui_error_t md3_fab_menu_destroy(struct md3_fab_menu *menu) {
  ui_error_t rc;
  ui_error_t final_rc = UI_ERROR_NONE;
  struct md3_fab_menu_action *curr;
  struct md3_fab_menu_action *next;

  if (!menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_speed_dial_base_cleanup(&menu->base);
  if (rc != UI_ERROR_NONE) {
    final_rc = rc;
  }

  curr = menu->actions;
  while (curr) {
    next = curr->next;
    if (curr->label) {
      C_MULTIPLATFORM_FREE(curr->label);
    }
    C_MULTIPLATFORM_FREE(curr);
    curr = next;
  }

  if (menu->component) {
    rc = ui_component_destroy(menu->component);
    if (rc != UI_ERROR_NONE) {
      final_rc = rc;
    }
  }

  C_MULTIPLATFORM_FREE(menu);
  return final_rc;
}

ui_error_t md3_fab_menu_set_primary_fab(struct md3_fab_menu *menu,
                                        struct md3_fab *fab) {
  if (!menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  menu->primary_fab = fab;
  return UI_ERROR_NONE;
}

ui_error_t md3_fab_menu_add_action(struct md3_fab_menu *menu, int id,
                                   struct md3_fab *fab, const char *label) {
  struct md3_fab_menu_action *action;
  struct ui_fab_base *fab_base;
  ui_error_t rc;
  size_t label_len;

  if (!menu || !fab) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = md3_fab_get_base(fab, &fab_base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_speed_dial_base_add_action(&menu->base, id, fab_base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  action = (struct md3_fab_menu_action *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_fab_menu_action));
  if (!action) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(action, 0, sizeof(struct md3_fab_menu_action));

  action->id = id;
  action->fab = fab;

  if (label) {
    label_len = strlen(label);
    action->label = (char *)C_MULTIPLATFORM_MALLOC(label_len + 1);
    if (!action->label) {
      C_MULTIPLATFORM_FREE(action);
      return UI_ERROR_OUT_OF_MEMORY;
    }
#if defined(_MSC_VER)
    strcpy_s(action->label, label_len + 1, label);
#else
    strcpy(action->label, label);
#endif
  }

  action->next = menu->actions;
  menu->actions = action;

  return UI_ERROR_NONE;
}

ui_error_t md3_fab_menu_get_base(struct md3_fab_menu *menu,
                                 struct ui_speed_dial_base **out_base) {
  if (!menu || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = &menu->base;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets menu expansion state explicitly.
 */
ui_error_t md3_fab_menu_set_expanded(struct md3_fab_menu *menu, int expanded) {
  if (!menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  menu->is_expanded = expanded;
  return UI_ERROR_NONE;
}

/**
 * @brief Toggles menu expansion state with spring transitions.
 */
ui_error_t md3_fab_menu_toggle(struct md3_fab_menu *menu) {
  if (!menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return md3_fab_menu_set_expanded(menu, !menu->is_expanded);
}
