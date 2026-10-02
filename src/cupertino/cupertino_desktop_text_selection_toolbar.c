/**
 * @file cupertino_desktop_text_selection_toolbar.c
 * @brief macOS Desktop Text Selection Context Menu Toolbar implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_desktop_text_selection_toolbar.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_desktop_text_mock_add_item_fail_at = -1;
int g_cupertino_desktop_text_mock_recompute_fail = 0;
int g_cupertino_desktop_text_mock_test_helpers = 0;
#endif

static ui_error_t cupertino_desktop_add_item(
    struct cupertino_desktop_text_selection_toolbar *toolbar, const char *title,
    const char *shortcut, int action_id, int has_separator, int is_disabled) {
  size_t idx;

#ifdef UI_TEST_MOCK_ALLOC
  if (toolbar != NULL && (int)toolbar->item_count ==
                             g_cupertino_desktop_text_mock_add_item_fail_at) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#endif

  if (!toolbar || !title || !shortcut) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (toolbar->item_count >= CUPERTINO_DESKTOP_TEXT_TOOLBAR_MAX_ITEMS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  idx = toolbar->item_count;
#if defined(_MSC_VER)
  strncpy_s(toolbar->items[idx].title, sizeof(toolbar->items[idx].title), title,
            _TRUNCATE);
  strncpy_s(toolbar->items[idx].shortcut, sizeof(toolbar->items[idx].shortcut),
            shortcut, _TRUNCATE);
#else
  strncpy(toolbar->items[idx].title, title,
          sizeof(toolbar->items[idx].title) - 1);
  toolbar->items[idx].title[sizeof(toolbar->items[idx].title) - 1] = '\0';
  strncpy(toolbar->items[idx].shortcut, shortcut,
          sizeof(toolbar->items[idx].shortcut) - 1);
  toolbar->items[idx].shortcut[sizeof(toolbar->items[idx].shortcut) - 1] = '\0';
#endif

  toolbar->items[idx].action_id = action_id;
  toolbar->items[idx].has_separator_below = has_separator;
  toolbar->items[idx].is_disabled = is_disabled;
  toolbar->item_count++;
  return UI_ERROR_NONE;
}
static ui_error_t cupertino_desktop_recompute_bounds(
    struct cupertino_desktop_text_selection_toolbar *toolbar) {
  size_t i;
  float total_h = 10.0f; /* 5pt top and bottom padding */

#ifdef UI_TEST_MOCK_ALLOC
  if (g_cupertino_desktop_text_mock_recompute_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#endif

  if (!toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toolbar->width = CUPERTINO_DESKTOP_TEXT_TOOLBAR_WIDTH;
  for (i = 0; i < toolbar->item_count; i++) {
    total_h += CUPERTINO_DESKTOP_TEXT_TOOLBAR_ROW_HEIGHT;
    if (toolbar->items[i].has_separator_below) {
      total_h += 8.0f; /* Separator height + spacing */
    }
  }

  toolbar->height = total_h;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_desktop_text_selection_toolbar_create(
    struct ui_engine *engine,
    const struct cupertino_desktop_text_selection_toolbar_descriptor *desc,
    struct cupertino_desktop_text_selection_toolbar **out_toolbar) {
  struct cupertino_desktop_text_selection_toolbar *tb;
  ui_error_t rc;

  if (!engine || !desc || !out_toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tb =
      (struct cupertino_desktop_text_selection_toolbar *)C_MULTIPLATFORM_MALLOC(
          sizeof(struct cupertino_desktop_text_selection_toolbar));
  if (!tb) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(tb, 0, sizeof(*tb));
  tb->is_visible = 0;
  tb->hovered_index = -1;
  tb->item_count = 0;

  rc = cupertino_desktop_add_item(tb, "Cut", "Cmd+X", 0, 0,
                                  desc->can_cut ? 0 : 1);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }
  rc = cupertino_desktop_add_item(tb, "Copy", "Cmd+C", 1, 0,
                                  desc->can_copy ? 0 : 1);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }
  rc = cupertino_desktop_add_item(tb, "Paste", "Cmd+V", 2, 1,
                                  desc->can_paste ? 0 : 1);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }
  rc = cupertino_desktop_add_item(tb, "Select All", "Cmd+A", 3, 1,
                                  desc->can_select_all ? 0 : 1);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }
  rc = cupertino_desktop_add_item(tb, "Look Up", "Cmd+Ctrl+D", 4, 0, 0);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }
  rc = cupertino_desktop_add_item(tb, "Share...", "", 5, 0, 0);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  rc = cupertino_desktop_recompute_bounds(tb);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

#ifdef UI_TEST_MOCK_ALLOC
  if (g_cupertino_desktop_text_mock_test_helpers == 1) {
    C_MULTIPLATFORM_FREE(tb);
    return cupertino_desktop_add_item(NULL, "T", "S", 0, 0, 0);
  }
  if (g_cupertino_desktop_text_mock_test_helpers == 2) {
    struct cupertino_desktop_text_selection_toolbar dummy_tb;
    memset(&dummy_tb, 0, sizeof(dummy_tb));
    C_MULTIPLATFORM_FREE(tb);
    return cupertino_desktop_add_item(&dummy_tb, NULL, "S", 0, 0, 0);
  }
  if (g_cupertino_desktop_text_mock_test_helpers == 3) {
    struct cupertino_desktop_text_selection_toolbar dummy_tb;
    memset(&dummy_tb, 0, sizeof(dummy_tb));
    C_MULTIPLATFORM_FREE(tb);
    return cupertino_desktop_add_item(&dummy_tb, "T", NULL, 0, 0, 0);
  }
  if (g_cupertino_desktop_text_mock_test_helpers == 4) {
    struct cupertino_desktop_text_selection_toolbar dummy_tb;
    memset(&dummy_tb, 0, sizeof(dummy_tb));
    dummy_tb.item_count = CUPERTINO_DESKTOP_TEXT_TOOLBAR_MAX_ITEMS;
    C_MULTIPLATFORM_FREE(tb);
    return cupertino_desktop_add_item(&dummy_tb, "T", "S", 0, 0, 0);
  }
  if (g_cupertino_desktop_text_mock_test_helpers == 5) {
    C_MULTIPLATFORM_FREE(tb);
    return cupertino_desktop_recompute_bounds(NULL);
  }
#endif

  *out_toolbar = tb;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_desktop_text_selection_toolbar_destroy(
    struct cupertino_desktop_text_selection_toolbar *toolbar) {
  if (!toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(toolbar);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_desktop_text_selection_toolbar_show(
    struct cupertino_desktop_text_selection_toolbar *toolbar, float x,
    float y) {
  if (!toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toolbar->x = x;
  toolbar->y = y;
  toolbar->is_visible = 1;
  toolbar->hovered_index = -1;

  return UI_ERROR_NONE;
}

ui_error_t cupertino_desktop_text_selection_toolbar_hide(
    struct cupertino_desktop_text_selection_toolbar *toolbar) {
  if (!toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toolbar->is_visible = 0;
  toolbar->hovered_index = -1;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_desktop_text_selection_toolbar_is_visible(
    const struct cupertino_desktop_text_selection_toolbar *toolbar,
    int *out_visible) {
  if (!toolbar || !out_visible) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_visible = toolbar->is_visible;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_desktop_text_selection_toolbar_set_hover_index(
    struct cupertino_desktop_text_selection_toolbar *toolbar, int index) {
  if (!toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (index < -1 || index >= (int)toolbar->item_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toolbar->hovered_index = index;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_desktop_text_selection_toolbar_get_hover_index(
    const struct cupertino_desktop_text_selection_toolbar *toolbar,
    int *out_index) {
  if (!toolbar || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_index = toolbar->hovered_index;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_desktop_text_selection_toolbar_trigger_action(
    struct cupertino_desktop_text_selection_toolbar *toolbar, int action_id,
    int *out_handled) {
  size_t i;

  if (!toolbar || !out_handled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_handled = 0;
  for (i = 0; i < toolbar->item_count; i++) {
    if (toolbar->items[i].action_id == action_id) {
      if (!toolbar->items[i].is_disabled) {
        *out_handled = 1;
        toolbar->is_visible = 0; /* Auto-dismiss on action */
        return UI_ERROR_NONE;
      }
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_desktop_text_selection_toolbar_get_item_count(
    const struct cupertino_desktop_text_selection_toolbar *toolbar,
    size_t *out_count) {
  if (!toolbar || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = toolbar->item_count;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_desktop_text_selection_toolbar_get_item_at(
    const struct cupertino_desktop_text_selection_toolbar *toolbar,
    size_t index, const char **out_title, const char **out_shortcut,
    int *out_action_id, int *out_disabled) {
  if (!toolbar || index >= toolbar->item_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (out_title) {
    *out_title = toolbar->items[index].title;
  }
  if (out_shortcut) {
    *out_shortcut = toolbar->items[index].shortcut;
  }
  if (out_action_id) {
    *out_action_id = toolbar->items[index].action_id;
  }
  if (out_disabled) {
    *out_disabled = toolbar->items[index].is_disabled;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_desktop_text_selection_toolbar_get_bounds(
    const struct cupertino_desktop_text_selection_toolbar *toolbar,
    float *out_x, float *out_y, float *out_w, float *out_h) {
  if (!toolbar || !out_x || !out_y || !out_w || !out_h) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_x = toolbar->x;
  *out_y = toolbar->y;
  *out_w = toolbar->width;
  *out_h = toolbar->height;

  return UI_ERROR_NONE;
}
