/**
 * @file cupertino_context_menu.c
 * @brief Cupertino Context Menu implementation conforming to Apple HIG.
 */

/* clang-format off */
#include "cupertino/cupertino_context_menu.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_context_menu_mock_create_fail = 0;
int g_cupertino_context_menu_mock_destroy_fail = 0;
int g_cupertino_context_menu_mock_open_fail = 0;
int g_cupertino_context_menu_mock_close_fail = 0;

static ui_error_t
mock_context_menu_base_create(struct ui_context_menu_base **out_base) {
  if (g_cupertino_context_menu_mock_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  return ui_context_menu_base_create(out_base);
}
#undef ui_context_menu_base_create
/** @cond */
#define ui_context_menu_base_create mock_context_menu_base_create
/** @endcond */

static ui_error_t
mock_context_menu_base_destroy(struct ui_context_menu_base *base) {
  if (g_cupertino_context_menu_mock_destroy_fail) {
    (ui_context_menu_base_destroy)(base);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_context_menu_base_destroy)(base);
}
#undef ui_context_menu_base_destroy
/** @cond */
#define ui_context_menu_base_destroy mock_context_menu_base_destroy
/** @endcond */

static ui_error_t mock_context_menu_base_open_at(
    struct ui_context_menu_base *base, struct ui_overlay_director *director,
    int x, int y, int menu_w, int menu_h, int viewport_w, int viewport_h) {
  if (g_cupertino_context_menu_mock_open_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_context_menu_base_open_at(base, director, x, y, menu_w, menu_h,
                                      viewport_w, viewport_h);
}
#undef ui_context_menu_base_open_at
/** @cond */
#define ui_context_menu_base_open_at mock_context_menu_base_open_at
/** @endcond */
#endif

ui_error_t cupertino_context_menu_create(
    struct ui_engine *engine,
    const struct cupertino_context_menu_descriptor *desc,
    struct cupertino_context_menu **out_menu) {
  struct cupertino_context_menu *menu;
  ui_error_t rc;

  if (!engine || !desc || !out_menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  menu = (struct cupertino_context_menu *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_context_menu));
  if (!menu) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(menu, 0, sizeof(*menu));
  menu->is_dark = desc->is_dark ? 1 : 0;
  menu->last_selected_index = -1;

  if (desc->title) {
#if defined(_MSC_VER)
    strncpy_s(menu->title, sizeof(menu->title), desc->title, _TRUNCATE);
#else
    strncpy(menu->title, desc->title, sizeof(menu->title) - 1);
    menu->title[sizeof(menu->title) - 1] = '\0';
#endif
  }

  rc = ui_context_menu_base_create(&menu->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(menu);
    return rc;
  }

  *out_menu = menu;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_context_menu_destroy(struct cupertino_context_menu *menu) {
  ui_error_t rc;

  if (!menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (menu->base) {
    rc = ui_context_menu_base_destroy(menu->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    menu->base = NULL;
  }

  C_MULTIPLATFORM_FREE(menu);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_context_menu_add_action(
    struct cupertino_context_menu *menu,
    const struct cupertino_context_menu_action *action) {
  struct cupertino_context_menu_action *dst;

  if (!menu || !action) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (menu->action_count >= CUPERTINO_CONTEXT_MENU_MAX_ACTIONS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  dst = &menu->actions[menu->action_count];
  memset(dst, 0, sizeof(*dst));

#if defined(_MSC_VER)
  strncpy_s(dst->title, sizeof(dst->title), action->title, _TRUNCATE);
  strncpy_s(dst->symbol_name, sizeof(dst->symbol_name), action->symbol_name,
            _TRUNCATE);
#else
  strncpy(dst->title, action->title, sizeof(dst->title) - 1);
  dst->title[sizeof(dst->title) - 1] = '\0';
  strncpy(dst->symbol_name, action->symbol_name, sizeof(dst->symbol_name) - 1);
  dst->symbol_name[sizeof(dst->symbol_name) - 1] = '\0';
#endif

  dst->is_destructive = action->is_destructive ? 1 : 0;
  dst->is_disabled = action->is_disabled ? 1 : 0;
  dst->is_checked = action->is_checked ? 1 : 0;

  menu->action_count++;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_context_menu_get_action_count(
    const struct cupertino_context_menu *menu, size_t *out_count) {
  if (!menu || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_count = menu->action_count;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_context_menu_get_action(
    const struct cupertino_context_menu *menu, size_t index,
    struct cupertino_context_menu_action *out_action) {
  if (!menu || !out_action) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (index >= menu->action_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  *out_action = menu->actions[index];
  return UI_ERROR_NONE;
}

ui_error_t cupertino_context_menu_open_at(struct cupertino_context_menu *menu,
                                          struct ui_overlay_director *director,
                                          float x, float y, float viewport_w,
                                          float viewport_h) {
  int menu_w;
  int menu_h;
  ui_error_t rc;

  if (!menu || viewport_w <= 0.0f || viewport_h <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  menu->target_x = x;
  menu->target_y = y;
  menu->is_open = 1;

  if (menu->base && director) {
    menu_w = (int)CUPERTINO_CONTEXT_MENU_DEFAULT_WIDTH;
    menu_h = (int)(menu->action_count * CUPERTINO_CONTEXT_MENU_ITEM_HEIGHT);
    if (menu_h < 44) {
      menu_h = 44;
    }

    rc = ui_context_menu_base_open_at(menu->base, director, (int)x, (int)y,
                                      menu_w, menu_h, (int)viewport_w,
                                      (int)viewport_h);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_context_menu_close(struct cupertino_context_menu *menu) {
  if (!menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#ifdef UI_TEST_MOCK_ALLOC
  if (g_cupertino_context_menu_mock_close_fail) {
    return UI_ERROR_UNKNOWN;
  }
#endif
  menu->is_open = 0;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_context_menu_is_open(const struct cupertino_context_menu *menu,
                               int *out_is_open) {
  if (!menu || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_open = menu->is_open;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_context_menu_select_action(struct cupertino_context_menu *menu,
                                     size_t index) {
  ui_error_t rc;

  if (!menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (index >= menu->action_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  if (menu->actions[index].is_disabled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  menu->last_selected_index = (int)index;

  rc = cupertino_context_menu_close(menu);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_context_menu_get_last_selected_index(
    const struct cupertino_context_menu *menu, int *out_index) {
  if (!menu || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_index = menu->last_selected_index;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_context_menu_get_base(struct cupertino_context_menu *menu,
                                struct ui_context_menu_base **out_base) {
  if (!menu || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_base = menu->base;
  return UI_ERROR_NONE;
}
