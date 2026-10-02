/**
 * @file cupertino_quick_look.c
 * @brief Quick Look Document Preview Sheet (QLPreviewController)
 * implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_quick_look.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

ui_error_t
cupertino_quick_look_create(struct ui_engine *engine,
                            const struct cupertino_quick_look_descriptor *desc,
                            struct cupertino_quick_look **out_ql) {
  struct cupertino_quick_look *ql;

  if (!engine || !desc || !out_ql) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ql = (struct cupertino_quick_look *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_quick_look));
  if (!ql) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(ql, 0, sizeof(*ql));
  ql->is_open = 1;
  ql->is_markup_enabled = desc->is_markup_enabled;
  ql->is_share_enabled = desc->is_share_enabled;
  ql->active_tool = CUPERTINO_QL_MARKUP_NONE;
  ql->is_markup_open = 0;
  ql->current_zoom = (desc->initial_zoom > 0.0f) ? desc->initial_zoom : 1.0f;
  ql->min_zoom = 0.5f;
  ql->max_zoom = 5.0f;
  ql->item_count = 0;
  ql->current_index = 0;
  ql->share_invoked_count = 0;
  ql->done_invoked_count = 0;

  *out_ql = ql;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_quick_look_destroy(struct cupertino_quick_look *ql) {
  if (!ql) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(ql);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_quick_look_add_item(struct cupertino_quick_look *ql, const char *url,
                              const char *title,
                              enum cupertino_ql_preview_item_type item_type,
                              size_t page_count, float duration_sec) {
  struct cupertino_ql_preview_item *item;

  if (!ql || !url || !title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (ql->item_count >= CUPERTINO_QL_MAX_ITEMS) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  item = &ql->items[ql->item_count];
  memset(item, 0, sizeof(*item));

#if defined(_MSC_VER)
  strncpy_s(item->url, sizeof(item->url), url, sizeof(item->url) - 1);
  strncpy_s(item->title, sizeof(item->title), title, sizeof(item->title) - 1);
#else
  strncpy(item->url, url, sizeof(item->url) - 1);
  item->url[sizeof(item->url) - 1] = '\0';
  strncpy(item->title, title, sizeof(item->title) - 1);
  item->title[sizeof(item->title) - 1] = '\0';
#endif

  item->item_type = item_type;
  item->page_count = page_count;
  item->duration_sec = duration_sec;

  ql->item_count++;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_quick_look_set_current_index(struct cupertino_quick_look *ql,
                                       size_t index) {
  if (!ql) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (ql->item_count == 0 || index >= ql->item_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ql->current_index = index;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_quick_look_get_current_item(
    const struct cupertino_quick_look *ql,
    const struct cupertino_ql_preview_item **out_item) {
  if (!ql || !out_item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (ql->item_count == 0 || ql->current_index >= ql->item_count) {
    return UI_ERROR_NOT_FOUND;
  }

  *out_item = &ql->items[ql->current_index];
  return UI_ERROR_NONE;
}

ui_error_t cupertino_quick_look_set_open(struct cupertino_quick_look *ql,
                                         int is_open) {
  if (!ql) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ql->is_open = is_open ? 1 : 0;
  if (!ql->is_open) {
    ql->is_markup_open = 0;
    ql->active_tool = CUPERTINO_QL_MARKUP_NONE;
  }
  return UI_ERROR_NONE;
}

ui_error_t cupertino_quick_look_is_open(const struct cupertino_quick_look *ql,
                                        int *out_is_open) {
  if (!ql || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_open = ql->is_open;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_quick_look_set_markup_open(struct cupertino_quick_look *ql,
                                                int is_open) {
  if (!ql) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (is_open && !ql->is_markup_enabled) {
    return UI_ERROR_UNSUPPORTED;
  }

  ql->is_markup_open = is_open ? 1 : 0;
  if (!ql->is_markup_open) {
    ql->active_tool = CUPERTINO_QL_MARKUP_NONE;
  } else if (ql->active_tool == CUPERTINO_QL_MARKUP_NONE) {
    ql->active_tool = CUPERTINO_QL_MARKUP_PEN;
  }
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_quick_look_set_markup_tool(struct cupertino_quick_look *ql,
                                     enum cupertino_ql_markup_tool tool) {
  if (!ql) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tool != CUPERTINO_QL_MARKUP_NONE && !ql->is_markup_enabled) {
    return UI_ERROR_UNSUPPORTED;
  }

  ql->active_tool = tool;
  if (tool != CUPERTINO_QL_MARKUP_NONE) {
    ql->is_markup_open = 1;
  }
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_quick_look_get_markup_tool(const struct cupertino_quick_look *ql,
                                     enum cupertino_ql_markup_tool *out_tool) {
  if (!ql || !out_tool) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_tool = ql->active_tool;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_quick_look_apply_pinch_zoom(struct cupertino_quick_look *ql,
                                      float zoom_delta,
                                      float *out_clamped_zoom) {
  float new_zoom;

  if (!ql || !out_clamped_zoom || zoom_delta <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  new_zoom = ql->current_zoom * zoom_delta;
  if (new_zoom < ql->min_zoom) {
    new_zoom = ql->min_zoom;
  } else if (new_zoom > ql->max_zoom) {
    new_zoom = ql->max_zoom;
  }

  ql->current_zoom = new_zoom;
  *out_clamped_zoom = new_zoom;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_quick_look_trigger_share(struct cupertino_quick_look *ql) {
  if (!ql) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!ql->is_share_enabled) {
    return UI_ERROR_UNSUPPORTED;
  }

  ql->share_invoked_count++;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_quick_look_trigger_done(struct cupertino_quick_look *ql) {
  if (!ql) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ql->done_invoked_count++;
  ql->is_open = 0;
  ql->is_markup_open = 0;
  ql->active_tool = CUPERTINO_QL_MARKUP_NONE;
  return UI_ERROR_NONE;
}
