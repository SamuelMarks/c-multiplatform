/**
 * @file md3_search.c
 * @brief Material 3 Search Bar and Search View component implementation.
 */

/* clang-format off */
#include "material3/md3_search.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
/** @brief Global flag to simulate failures in mocked dependencies. */
int g_md3_search_mock_fail = 0;

/**
 * @brief Mock for ui_search_bar_base_init.
 * @param search_bar Parameter search_bar.
 * @param component Parameter component.
 * @param out_cva Parameter out_cva.
 * @return Return value.
 */
static ui_error_t
mock_search_bar_base_init(struct ui_search_bar_base *search_bar,
                          struct ui_component *component,
                          struct ui_control_value_accessor *out_cva) {
  if (g_md3_search_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_search_bar_base_init(search_bar, component, out_cva);
}
#undef ui_search_bar_base_init
/** @cond */
#define ui_search_bar_base_init mock_search_bar_base_init
/** @endcond */
#endif

ui_error_t md3_search_bar_create(struct ui_engine *engine,
                                 const char *placeholder,
                                 struct md3_search_bar **out_search_bar,
                                 struct ui_control_value_accessor **out_cva) {
  struct md3_search_bar *sb;
  ui_error_t rc;

  if (!engine || !out_search_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sb = (struct md3_search_bar *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_search_bar));
  if (!sb) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(sb, 0, sizeof(struct md3_search_bar));
  sb->elevation = 3; /* Material 3 docked search bar defaults to Level 3 */

  if (placeholder) {
#if defined(_MSC_VER)
    strncpy_s(sb->placeholder, sizeof(sb->placeholder), placeholder,
              sizeof(sb->placeholder) - 1);
#else
    strncpy(sb->placeholder, placeholder, sizeof(sb->placeholder) - 1);
    sb->placeholder[sizeof(sb->placeholder) - 1] = '\0';
#endif
  }

  rc = ui_search_bar_base_init(&sb->base, &sb->component, &sb->cva);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(sb);
    return rc;
  }

  if (out_cva) {
    *out_cva = &sb->cva;
  }

  *out_search_bar = sb;
  return rc;
}

ui_error_t md3_search_bar_destroy(struct md3_search_bar *search_bar) {
  ui_error_t rc;

  if (!search_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_search_bar_base_cleanup(&search_bar->base);
  /* component is value, don't destroy pointer */
  C_MULTIPLATFORM_FREE(search_bar);
  return rc;
}

ui_error_t md3_search_bar_set_query(struct md3_search_bar *search_bar,
                                    const char *query) {
  if (!search_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_search_bar_base_set_query(&search_bar->base, query);
}

ui_error_t md3_search_bar_get_query(const struct md3_search_bar *search_bar,
                                    const char **out_query) {
  if (!search_bar || !out_query) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_query = search_bar->base.query;
  return UI_ERROR_NONE;
}

ui_error_t md3_search_bar_set_loading(struct md3_search_bar *search_bar,
                                      int is_loading) {
  if (!search_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_search_bar_base_set_loading(&search_bar->base, is_loading);
}

ui_error_t md3_search_bar_open_view(struct md3_search_bar *search_bar,
                                    struct md3_search_view **out_view) {
  if (!search_bar || !out_view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  search_bar->view.is_open = 1;
  *out_view = &search_bar->view;
  return UI_ERROR_NONE;
}

ui_error_t md3_search_view_close(struct md3_search_view *view) {
  if (!view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  view->is_open = 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_search_view_add_history_item(struct md3_search_view *view,
                                            const char *item) {
  size_t idx;

  if (!view || !item || view->history_count >= MD3_SEARCH_MAX_HISTORY) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  idx = view->history_count++;
#if defined(_MSC_VER)
  strncpy_s(view->history[idx], sizeof(view->history[idx]), item,
            sizeof(view->history[idx]) - 1);
#else
  strncpy(view->history[idx], item, sizeof(view->history[idx]) - 1);
  view->history[idx][sizeof(view->history[idx]) - 1] = '\0';
#endif

  return UI_ERROR_NONE;
}

ui_error_t md3_search_view_get_history_count(const struct md3_search_view *view,
                                             size_t *out_count) {
  if (!view || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = view->history_count;
  return UI_ERROR_NONE;
}

ui_error_t md3_search_view_get_history_item(const struct md3_search_view *view,
                                            size_t index,
                                            const char **out_item) {
  if (!view || !out_item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (index >= view->history_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  *out_item = view->history[index];
  return UI_ERROR_NONE;
}

ui_error_t md3_search_bar_get_base(struct md3_search_bar *search_bar,
                                   struct ui_search_bar_base **out_base) {
  if (!search_bar || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = &search_bar->base;
  return UI_ERROR_NONE;
}
