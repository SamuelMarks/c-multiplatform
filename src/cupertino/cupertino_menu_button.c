/**
 * @file cupertino_menu_button.c
 * @brief Cupertino Pop-up and Pull-down Menus implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_menu_button.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_menu_button_mock_button_create_fail = 0;
int g_cupertino_menu_button_mock_button_destroy_fail = 0;
int g_cupertino_menu_button_mock_set_text_fail = 0;
int g_cupertino_menu_button_mock_get_base_fail = 0;

static ui_error_t mock_menu_button_get_base(struct cupertino_button *btn,
                                            struct ui_button_base **out_base) {
  if (g_cupertino_menu_button_mock_get_base_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return cupertino_button_get_base(btn, out_base);
}
#undef cupertino_button_get_base
/** @cond */
#define cupertino_button_get_base mock_menu_button_get_base
/** @endcond */

static ui_error_t
mock_menu_button_create(struct ui_engine *engine,
                        const struct cupertino_button_descriptor *desc,
                        struct cupertino_button **out_button) {
  if (g_cupertino_menu_button_mock_button_create_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return cupertino_button_create(engine, desc, out_button);
}
#undef cupertino_button_create
/** @cond */
#define cupertino_button_create mock_menu_button_create
/** @endcond */

static ui_error_t mock_menu_button_destroy(struct cupertino_button *btn) {
  if (g_cupertino_menu_button_mock_button_destroy_fail) {
    (cupertino_button_destroy)(btn);
    return UI_ERROR_UNKNOWN;
  }
  return (cupertino_button_destroy)(btn);
}
#undef cupertino_button_destroy
/** @cond */
#define cupertino_button_destroy mock_menu_button_destroy
/** @endcond */

static ui_error_t mock_menu_button_base_set_text(struct ui_button_base *b,
                                                 const char *t) {
  if (g_cupertino_menu_button_mock_set_text_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_button_base_set_text(b, t);
}
#undef ui_button_base_set_text
/** @cond */
#define ui_button_base_set_text mock_menu_button_base_set_text
/** @endcond */
#endif

ui_error_t cupertino_menu_button_create(
    struct ui_engine *engine,
    const struct cupertino_menu_button_descriptor *desc,
    struct cupertino_menu_button **out_menu) {
  struct cupertino_menu_button *menu;
  struct cupertino_button_descriptor btn_desc;
  ui_error_t rc;

  if (!engine || !desc || !out_menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  menu = (struct cupertino_menu_button *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_menu_button));
  if (!menu) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(menu, 0, sizeof(*menu));
  menu->mode = desc->mode;
  menu->is_open = 0;
  menu->selected_index = -1;
  menu->item_count = 0;

#if defined(_MSC_VER)
  strncpy_s(menu->default_title, sizeof(menu->default_title),
            desc->initial_title, sizeof(menu->default_title) - 1);
  strncpy_s(menu->current_title, sizeof(menu->current_title),
            desc->initial_title, sizeof(menu->current_title) - 1);
#else
  strncpy(menu->default_title, desc->initial_title,
          sizeof(menu->default_title) - 1);
  menu->default_title[sizeof(menu->default_title) - 1] = '\0';
  strncpy(menu->current_title, desc->initial_title,
          sizeof(menu->current_title) - 1);
  menu->current_title[sizeof(menu->current_title) - 1] = '\0';
#endif

  memset(&btn_desc, 0, sizeof(btn_desc));
  btn_desc.style = desc->style;
  btn_desc.size = CUPERTINO_BUTTON_SIZE_MEDIUM;
  btn_desc.is_disabled = desc->is_disabled;
  btn_desc.text = menu->current_title;

  rc = cupertino_button_create(engine, &btn_desc, &menu->button);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(menu);
    return rc;
  }

  *out_menu = menu;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_menu_button_destroy(struct cupertino_menu_button *menu) {
  ui_error_t rc;

  if (!menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (menu->button) {
    rc = cupertino_button_destroy(menu->button);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  C_MULTIPLATFORM_FREE(menu);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_menu_button_add_item(struct cupertino_menu_button *menu,
                               const struct cupertino_menu_button_item *item) {
  struct cupertino_menu_button_item *target;

  if (!menu || !item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (menu->item_count >= CUPERTINO_MENU_BUTTON_MAX_ITEMS) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  target = &menu->items[menu->item_count];
  memset(target, 0, sizeof(*target));

#if defined(_MSC_VER)
  strncpy_s(target->title, sizeof(target->title), item->title,
            sizeof(target->title) - 1);
  strncpy_s(target->icon_symbol, sizeof(target->icon_symbol), item->icon_symbol,
            sizeof(target->icon_symbol) - 1);
#else
  strncpy(target->title, item->title, sizeof(target->title) - 1);
  target->title[sizeof(target->title) - 1] = '\0';
  strncpy(target->icon_symbol, item->icon_symbol,
          sizeof(target->icon_symbol) - 1);
  target->icon_symbol[sizeof(target->icon_symbol) - 1] = '\0';
#endif

  target->is_destructive = item->is_destructive;
  target->is_disabled = item->is_disabled;
  target->on_select = item->on_select;
  target->user_data = item->user_data;

  menu->item_count++;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_menu_button_get_item_count(const struct cupertino_menu_button *menu,
                                     size_t *out_count) {
  if (!menu || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = menu->item_count;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_menu_button_get_item(const struct cupertino_menu_button *menu,
                               size_t index,
                               struct cupertino_menu_button_item *out_item) {
  if (!menu || !out_item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (index >= menu->item_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_item = menu->items[index];
  return UI_ERROR_NONE;
}

ui_error_t cupertino_menu_button_select_item(struct cupertino_menu_button *menu,
                                             size_t index) {
  struct cupertino_menu_button_item *item;
  struct ui_button_base *base = NULL;
  ui_error_t rc;

  if (!menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (index >= menu->item_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  item = &menu->items[index];
  if (item->is_disabled) {
    return UI_ERROR_UNSUPPORTED;
  }

  menu->selected_index = (int)index;

  if (menu->mode == CUPERTINO_MENU_BUTTON_POP_UP) {
#if defined(_MSC_VER)
    strncpy_s(menu->current_title, sizeof(menu->current_title), item->title,
              sizeof(menu->current_title) - 1);
#else
    strncpy(menu->current_title, item->title, sizeof(menu->current_title) - 1);
    menu->current_title[sizeof(menu->current_title) - 1] = '\0';
#endif
    if (menu->button) {
      rc = cupertino_button_get_base(menu->button, &base);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      if (base) {
        rc = ui_button_base_set_text(base, menu->current_title);
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
      }
    }
  }

  menu->is_open = 0;

  if (item->on_select) {
    item->on_select(item->user_data);
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_menu_button_get_selected_index(
    const struct cupertino_menu_button *menu, int *out_index) {
  if (!menu || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_index = menu->selected_index;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_menu_button_set_open(struct cupertino_menu_button *menu,
                                          int is_open) {
  if (!menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  menu->is_open = is_open ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_menu_button_is_open(const struct cupertino_menu_button *menu,
                              int *out_is_open) {
  if (!menu || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_open = menu->is_open;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_menu_button_get_current_title(
    const struct cupertino_menu_button *menu, const char **out_title) {
  if (!menu || !out_title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_title = menu->current_title;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_menu_button_get_button(struct cupertino_menu_button *menu,
                                 struct cupertino_button **out_button) {
  if (!menu || !out_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_button = menu->button;
  return UI_ERROR_NONE;
}
