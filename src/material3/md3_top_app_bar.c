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

#ifdef UI_TEST_MOCK_ALLOC
/** @brief Mock failure flag for md3_top_app_bar testing. */
int g_md3_top_app_bar_mock_fail = 0;
/** @brief Mock destroy failure flag for md3_top_app_bar testing. */
int g_md3_top_app_bar_destroy_mock_fail = 0;

/**
 * @brief Mock implementation of ui_top_app_bar_base_create for failure testing.
 * @param arena The arena instance.
 * @param cfg The config pointer.
 * @param out_base Output pointer for the base.
 * @return UI_ERROR_UNKNOWN on injected mock failure, or result of
 * ui_top_app_bar_base_create.
 */
static ui_error_t
mock_md3_top_app_bar_base_create(struct ui_arena *arena,
                                 const struct ui_top_app_bar_config *cfg,
                                 struct ui_top_app_bar_base **out_base) {
  if (g_md3_top_app_bar_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_top_app_bar_base_create(arena, cfg, out_base);
}
#undef ui_top_app_bar_base_create
/** @cond */
#define ui_top_app_bar_base_create mock_md3_top_app_bar_base_create
/** @endcond */

/**
 * @brief Mock implementation of ui_top_app_bar_base_destroy for failure
 * testing.
 * @param base The base instance.
 * @return UI_ERROR_UNKNOWN on injected mock failure, or result of
 * ui_top_app_bar_base_destroy.
 */
static ui_error_t
mock_md3_top_app_bar_base_destroy(struct ui_top_app_bar_base *base) {
  if (g_md3_top_app_bar_destroy_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_top_app_bar_base_destroy(base);
}
#undef ui_top_app_bar_base_destroy
/** @cond */
#define ui_top_app_bar_base_destroy mock_md3_top_app_bar_base_destroy
/** @endcond */

/**
 * @brief Mock implementation of ui_arena_destroy for failure testing.
 * @param arena The arena instance.
 * @return UI_ERROR_UNKNOWN on injected mock failure, or result of
 * ui_arena_destroy.
 */
static ui_error_t mock_md3_top_app_bar_arena_destroy(struct ui_arena *arena) {
  if (g_md3_top_app_bar_destroy_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_arena_destroy(arena);
}
#undef ui_arena_destroy
/** @cond */
#define ui_arena_destroy mock_md3_top_app_bar_arena_destroy
/** @endcond */

/**
 * @brief Mock implementation of ui_arena_alloc for failure testing.
 * @param arena The arena instance.
 * @param size Allocation size.
 * @param align Alignment.
 * @param out_ptr Output pointer.
 * @return UI_ERROR_UNKNOWN on injected mock failure, or result of
 * ui_arena_alloc.
 */
static ui_error_t mock_md3_top_app_bar_arena_alloc(struct ui_arena *arena,
                                                   size_t size, size_t align,
                                                   void **out_ptr) {
  if (g_md3_top_app_bar_mock_fail == 2) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  if (g_md3_top_app_bar_mock_fail == 3) {
    /* Let first call succeed, fail subsequent call */
    g_md3_top_app_bar_mock_fail = 2;
    return ui_arena_alloc(arena, size, align, out_ptr);
  }
  return ui_arena_alloc(arena, size, align, out_ptr);
}
#undef ui_arena_alloc
/** @cond */
#define ui_arena_alloc mock_md3_top_app_bar_arena_alloc
/** @endcond */
#endif

/**
 * @struct md3_top_app_bar
 * @brief Internal implementation structure for the Material 3 Top App Bar.
 */
struct md3_top_app_bar {
  struct ui_top_app_bar_base *base; /**< Underlying CDK top app bar base. */
  struct ui_arena *arena; /**< Memory arena for string and item storage. */
  enum md3_top_app_bar_variant variant; /**< Top app bar visual variant. */
  enum md3_top_app_bar_scroll_behavior
      scroll_behavior; /**< Dynamic scroll behavior. */
  char *title;         /**< Title string. */
  char *subtitle;      /**< Subtitle string. */
  struct md3_icon_button
      *navigation_icon; /**< Leading navigation icon button. */
  struct md3_icon_button *
      *action_items;            /**< Trailing action icon buttons array. */
  size_t num_action_items;      /**< Current count of action buttons. */
  size_t action_items_capacity; /**< Capacity of action buttons array. */
};

/**
 * @brief Duplicates a string using the provided arena allocator.
 * @param arena Memory arena to allocate from.
 * @param src Source string to duplicate (can be NULL).
 * @param out_str Pointer receiving duplicated string.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
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

#ifdef UI_TEST_MOCK_ALLOC
/**
 * @brief Internal test helper to exercise duplicate_string parameter checks.
 * @param arena Memory arena.
 * @param src Source string.
 * @param out_str Output pointer.
 * @return Error code.
 */
ui_error_t md3_top_app_bar_test_duplicate_string(struct ui_arena *arena,
                                                 const char *src,
                                                 char **out_str) {
  return duplicate_string(arena, src, out_str);
}
#endif

ui_error_t md3_top_app_bar_create(struct ui_engine *engine,
                                  const struct md3_top_app_bar_config *config,
                                  struct md3_top_app_bar **out_bar) {
  struct md3_top_app_bar *bar;
  struct ui_top_app_bar_config base_config;
  ui_error_t rc;
  ui_error_t rc_cleanup;

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
    goto cleanup;
  }

  rc = duplicate_string(bar->arena, config->subtitle, &bar->subtitle);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
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
    goto cleanup;
  }

  *out_bar = bar;
  return UI_ERROR_NONE;

cleanup:
  rc_cleanup = ui_arena_destroy(bar->arena);
  if (rc_cleanup != UI_ERROR_NONE) {
    rc = rc_cleanup;
  }
  C_MULTIPLATFORM_FREE(bar);
  return rc;
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
