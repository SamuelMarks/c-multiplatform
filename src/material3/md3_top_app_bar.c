/**
 * @file md3_top_app_bar.c
 * @brief Implementation of the Material 3 Top App Bar.
 */

/* clang-format off */
#include "material3/md3_top_app_bar.h"
#include "ui_arena.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

struct md3_top_app_bar {
  struct ui_top_app_bar_base *base;
  struct ui_arena *arena;
  enum md3_top_app_bar_variant variant;
  enum md3_top_app_bar_scroll_behavior scroll_behavior;
  char *title;
  char *subtitle;
  struct md3_icon_button *navigation_icon;
  struct md3_icon_button **action_items;
  size_t num_action_items;
  size_t action_items_capacity;
};

static ui_error_t duplicate_string(struct ui_arena *arena, const char *src,
                                   char **out_str) {
  size_t len;
  void *ptr;
  ui_error_t rc;

  if (!out_str) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!src) {
    *out_str = NULL;
    return UI_ERROR_NONE;
  }

  len = strlen(src);
  rc = ui_arena_alloc(arena, len + 1, 1, &ptr);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  *out_str = (char *)ptr;

#if defined(_MSC_VER)
  strcpy_s(*out_str, len + 1, src);
#else
  strcpy(*out_str, src);
#endif

  return UI_ERROR_NONE;
}

ui_error_t md3_top_app_bar_create(struct ui_engine *engine,
                                  const struct md3_top_app_bar_config *config,
                                  struct md3_top_app_bar **out_bar) {
  struct md3_top_app_bar *bar;
  struct ui_top_app_bar_config base_config;
  ui_error_t rc;

  if (!engine || !config || !out_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar = (struct md3_top_app_bar *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_top_app_bar));
  if (!bar) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(bar, 0, sizeof(struct md3_top_app_bar));

  bar->variant = config->variant;
  bar->scroll_behavior = config->scroll_behavior;

  rc = ui_arena_create(4096, &bar->arena);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(bar);
    return rc;
  }

  rc = duplicate_string(bar->arena, config->title, &bar->title);
  if (rc != UI_ERROR_NONE) {
    ui_arena_destroy(bar->arena);
    C_MULTIPLATFORM_FREE(bar);
    return rc;
  }

  rc = duplicate_string(bar->arena, config->subtitle, &bar->subtitle);
  if (rc != UI_ERROR_NONE) {
    ui_arena_destroy(bar->arena);
    C_MULTIPLATFORM_FREE(bar);
    return rc;
  }

  base_config.initial_state = UI_TOP_APP_BAR_STATE_EXPANDED;
  base_config.scroll_threshold = 50.0f;

  switch (bar->variant) {
  case MD3_TOP_APP_BAR_SMALL:
  case MD3_TOP_APP_BAR_CENTER_ALIGNED:
    base_config.expanded_height = 64.0f;
    base_config.collapsed_height = 64.0f;
    break;
  case MD3_TOP_APP_BAR_MEDIUM:
    base_config.expanded_height = 112.0f;
    base_config.collapsed_height = 64.0f;
    break;
  case MD3_TOP_APP_BAR_LARGE:
    base_config.expanded_height = 152.0f;
    base_config.collapsed_height = 64.0f;
    break;
  default:
    base_config.expanded_height = 64.0f;
    base_config.collapsed_height = 64.0f;
    break;
  }

  rc = ui_top_app_bar_base_create(bar->arena, &base_config, &bar->base);
  if (rc != UI_ERROR_NONE) {
    ui_arena_destroy(bar->arena);
    C_MULTIPLATFORM_FREE(bar);
    return rc;
  }

  *out_bar = bar;
  return UI_ERROR_NONE;
}

ui_error_t md3_top_app_bar_destroy(struct md3_top_app_bar *bar) {
  ui_error_t rc;
  ui_error_t final_rc = UI_ERROR_NONE;

  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (bar->base) {
    rc = ui_top_app_bar_base_destroy(bar->base);
    if (rc != UI_ERROR_NONE) {
      final_rc = rc;
    }
  }

  if (bar->arena) {
    rc = ui_arena_destroy(bar->arena);
    if (rc != UI_ERROR_NONE) {
      final_rc = rc;
    }
  }

  C_MULTIPLATFORM_FREE(bar);
  return final_rc;
}

ui_error_t md3_top_app_bar_handle_scroll(struct md3_top_app_bar *bar,
                                         float scroll_y, float delta_y) {
  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Assuming the base handles states gracefully. */
  if (bar->base) {
    return ui_top_app_bar_base_handle_scroll(bar->base, scroll_y, delta_y);
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_top_app_bar_get_base(struct md3_top_app_bar *bar,
                                    struct ui_top_app_bar_base **out_base) {
  if (!bar || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = bar->base;
  return UI_ERROR_NONE;
}
ui_error_t md3_top_app_bar_set_navigation_icon(struct md3_top_app_bar *bar,
                                               struct md3_icon_button *icon) {
  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  bar->navigation_icon = icon;
  return UI_ERROR_NONE;
}

ui_error_t md3_top_app_bar_add_action_item(struct md3_top_app_bar *bar,
                                           struct md3_icon_button *action) {
  size_t new_capacity;
  void *new_arr;
  size_t i;
  struct md3_icon_button **old_arr;

  if (!bar || !action) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (bar->num_action_items == bar->action_items_capacity) {
    new_capacity =
        bar->action_items_capacity == 0 ? 4 : bar->action_items_capacity * 2;
    if (ui_arena_alloc(bar->arena,
                       new_capacity * sizeof(struct md3_icon_button *),
                       sizeof(void *), &new_arr) != UI_ERROR_NONE) {
      return UI_ERROR_OUT_OF_MEMORY;
    }

    old_arr = bar->action_items;
    for (i = 0; i < bar->num_action_items; i++) {
      ((struct md3_icon_button **)new_arr)[i] = old_arr[i];
    }

    bar->action_items = (struct md3_icon_button **)new_arr;
    bar->action_items_capacity = new_capacity;
  }

  bar->action_items[bar->num_action_items++] = action;
  return UI_ERROR_NONE;
}
