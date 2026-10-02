/**
 * @file cupertino_activity_view.c
 * @brief Cupertino Activity View (Share Sheet) implementation conforming to
 * Apple HIG.
 */

/* clang-format off */
#include "cupertino/cupertino_activity_view.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

ui_error_t cupertino_activity_view_create(
    struct ui_engine *engine,
    const struct cupertino_activity_view_descriptor *desc,
    struct cupertino_activity_view **out_view) {
  struct cupertino_activity_view *view;

  if (!engine || !desc || !out_view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  view = (struct cupertino_activity_view *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_activity_view));
  if (!view) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(view, 0, sizeof(*view));
  view->is_dark = desc->is_dark ? 1 : 0;
  view->last_activated_index = -1;

  if (desc->share_text) {
#if defined(_MSC_VER)
    strncpy_s(view->share_text, sizeof(view->share_text), desc->share_text,
              _TRUNCATE);
#else
    strncpy(view->share_text, desc->share_text, sizeof(view->share_text) - 1);
    view->share_text[sizeof(view->share_text) - 1] = '\0';
#endif
  }

  if (desc->share_url) {
#if defined(_MSC_VER)
    strncpy_s(view->share_url, sizeof(view->share_url), desc->share_url,
              _TRUNCATE);
#else
    strncpy(view->share_url, desc->share_url, sizeof(view->share_url) - 1);
    view->share_url[sizeof(view->share_url) - 1] = '\0';
#endif
  }

  *out_view = view;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_activity_view_destroy(struct cupertino_activity_view *view) {
  if (!view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(view);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_activity_view_add_item(struct cupertino_activity_view *view,
                                 const struct cupertino_activity_item *item) {
  struct cupertino_activity_item *dst;

  if (!view || !item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (view->item_count >= CUPERTINO_ACTIVITY_VIEW_MAX_ITEMS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  dst = &view->items[view->item_count];
  memset(dst, 0, sizeof(*dst));

#if defined(_MSC_VER)
  strncpy_s(dst->title, sizeof(dst->title), item->title, _TRUNCATE);
  strncpy_s(dst->symbol_name, sizeof(dst->symbol_name), item->symbol_name,
            _TRUNCATE);
#else
  strncpy(dst->title, item->title, sizeof(dst->title) - 1);
  dst->title[sizeof(dst->title) - 1] = '\0';
  strncpy(dst->symbol_name, item->symbol_name, sizeof(dst->symbol_name) - 1);
  dst->symbol_name[sizeof(dst->symbol_name) - 1] = '\0';
#endif

  dst->category = item->category;
  dst->is_disabled = item->is_disabled ? 1 : 0;

  view->item_count++;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_activity_view_get_item_count(
    const struct cupertino_activity_view *view, size_t *out_count) {
  if (!view || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_count = view->item_count;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_activity_view_get_item(const struct cupertino_activity_view *view,
                                 size_t index,
                                 struct cupertino_activity_item *out_item) {
  if (!view || !out_item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (index >= view->item_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  *out_item = view->items[index];
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_activity_view_present(struct cupertino_activity_view *view) {
  if (!view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  view->is_open = 1;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_activity_view_dismiss(struct cupertino_activity_view *view) {
  if (!view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  view->is_open = 0;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_activity_view_is_open(const struct cupertino_activity_view *view,
                                int *out_is_open) {
  if (!view || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_open = view->is_open;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_activity_view_perform_activity(struct cupertino_activity_view *view,
                                         size_t index) {
  ui_error_t rc;

  if (!view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (index >= view->item_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  if (view->items[index].is_disabled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  view->last_activated_index = (int)index;

  rc = cupertino_activity_view_dismiss(view);
  return rc;
}

ui_error_t cupertino_activity_view_get_last_activated_index(
    const struct cupertino_activity_view *view, int *out_index) {
  if (!view || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_index = view->last_activated_index;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_activity_view_get_share_content(
    const struct cupertino_activity_view *view, const char **out_text,
    const char **out_url) {
  if (!view || !out_text || !out_url) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_text = view->share_text;
  *out_url = view->share_url;
  return UI_ERROR_NONE;
}
