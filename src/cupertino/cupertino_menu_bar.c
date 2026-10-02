/**
 * @file cupertino_menu_bar.c
 * @brief macOS Global Application Menu Bar engine implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_menu_bar.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

ui_error_t
cupertino_menu_bar_create(struct ui_engine *engine,
                          const struct cupertino_menu_bar_descriptor *desc,
                          struct cupertino_menu_bar **out_bar) {
  struct cupertino_menu_bar *bar;

  if (!engine || !desc || !out_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar = (struct cupertino_menu_bar *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_menu_bar));
  if (!bar) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(bar, 0, sizeof(*bar));
  bar->active_menu_index = -1;
  bar->width = 1920.0f;
  bar->height = CUPERTINO_MENU_BAR_HEIGHT;

  if (desc->app_name) {
#if defined(_MSC_VER)
    strncpy_s(bar->app_name, sizeof(bar->app_name), desc->app_name, _TRUNCATE);
#else
    strncpy(bar->app_name, desc->app_name, sizeof(bar->app_name) - 1);
    bar->app_name[sizeof(bar->app_name) - 1] = '\0';
#endif
  } else {
#if defined(_MSC_VER)
    strcpy_s(bar->app_name, sizeof(bar->app_name), "App");
#else
    strcpy(bar->app_name, "App");
#endif
  }

  /* Default macOS menu 0: Apple Menu */
#if defined(_MSC_VER)
  strcpy_s(bar->menus[0].title, sizeof(bar->menus[0].title), "Apple");
  strcpy_s(bar->menus[1].title, sizeof(bar->menus[1].title), bar->app_name);
#else
  strcpy(bar->menus[0].title, "Apple");
  strcpy(bar->menus[1].title, bar->app_name);
#endif
  bar->menu_count = 2;

  *out_bar = bar;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_menu_bar_destroy(struct cupertino_menu_bar *bar) {
  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(bar);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_menu_bar_add_menu(struct cupertino_menu_bar *bar,
                                       const char *title, int *out_menu_index) {
  int idx;

  if (!bar || !title || !out_menu_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (bar->menu_count >= CUPERTINO_MENU_BAR_MAX_MENUS) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  idx = bar->menu_count;
  memset(&bar->menus[idx], 0, sizeof(struct cupertino_menu));

#if defined(_MSC_VER)
  strncpy_s(bar->menus[idx].title, sizeof(bar->menus[idx].title), title,
            _TRUNCATE);
#else
  strncpy(bar->menus[idx].title, title, sizeof(bar->menus[idx].title) - 1);
  bar->menus[idx].title[sizeof(bar->menus[idx].title) - 1] = '\0';
#endif

  bar->menu_count++;
  *out_menu_index = idx;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_menu_bar_add_item(struct cupertino_menu_bar *bar,
                                       int menu_index, const char *title,
                                       const char *shortcut, int is_separator,
                                       int *out_item_index) {
  struct cupertino_menu *m;
  struct cupertino_menu_item *item;
  int item_idx;

  if (!bar || menu_index < 0 || menu_index >= bar->menu_count ||
      !out_item_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  m = &bar->menus[menu_index];
  if (m->item_count >= CUPERTINO_MENU_BAR_MAX_ITEMS) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  item_idx = m->item_count;
  item = &m->items[item_idx];
  memset(item, 0, sizeof(*item));

  item->is_separator = is_separator ? 1 : 0;
  item->is_enabled = 1;
  item->state = CUPERTINO_MENU_ITEM_STATE_OFF;

  if (title) {
#if defined(_MSC_VER)
    strncpy_s(item->title, sizeof(item->title), title, _TRUNCATE);
#else
    strncpy(item->title, title, sizeof(item->title) - 1);
    item->title[sizeof(item->title) - 1] = '\0';
#endif
  }

  if (shortcut) {
#if defined(_MSC_VER)
    strncpy_s(item->shortcut, sizeof(item->shortcut), shortcut, _TRUNCATE);
#else
    strncpy(item->shortcut, shortcut, sizeof(item->shortcut) - 1);
    item->shortcut[sizeof(item->shortcut) - 1] = '\0';
#endif
  }

  m->item_count++;
  *out_item_index = item_idx;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_menu_bar_set_item_enabled(struct cupertino_menu_bar *bar,
                                               int menu_index, int item_index,
                                               int is_enabled) {
  if (!bar || menu_index < 0 || menu_index >= bar->menu_count ||
      item_index < 0 || item_index >= bar->menus[menu_index].item_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar->menus[menu_index].items[item_index].is_enabled = is_enabled ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_menu_bar_set_item_state(struct cupertino_menu_bar *bar,
                                  int menu_index, int item_index,
                                  enum cupertino_menu_item_state state) {
  if (!bar || menu_index < 0 || menu_index >= bar->menu_count ||
      item_index < 0 || item_index >= bar->menus[menu_index].item_count ||
      (int)state < 0 || (int)state > 2) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar->menus[menu_index].items[item_index].state = state;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_menu_bar_open_menu(struct cupertino_menu_bar *bar,
                                        int menu_index) {
  if (!bar || menu_index < 0 || menu_index >= bar->menu_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar->active_menu_index = menu_index;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_menu_bar_close(struct cupertino_menu_bar *bar) {
  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar->active_menu_index = -1;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_menu_bar_get_active_menu(const struct cupertino_menu_bar *bar,
                                   int *out_menu_index) {
  if (!bar || !out_menu_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_menu_index = bar->active_menu_index;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_menu_bar_get_dimensions(const struct cupertino_menu_bar *bar,
                                  float *out_width, float *out_height) {
  if (!bar || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_width = bar->width;
  *out_height = bar->height;
  return UI_ERROR_NONE;
}
