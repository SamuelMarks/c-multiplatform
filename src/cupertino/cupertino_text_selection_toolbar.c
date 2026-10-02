/**
 * @file cupertino_text_selection_toolbar.c
 * @brief Cupertino iOS Text Selection Toolbar and Grabber Handles
 * implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_text_selection_toolbar.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

ui_error_t cupertino_text_selection_toolbar_create(
    struct ui_engine *engine,
    const struct cupertino_text_selection_toolbar_descriptor *desc,
    struct cupertino_text_selection_toolbar **out_toolbar) {
  struct cupertino_text_selection_toolbar *tb;
  int i;
  int mask;

  if (!engine || !desc || !out_toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tb = (struct cupertino_text_selection_toolbar *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_text_selection_toolbar));
  if (!tb) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(tb, 0, sizeof(*tb));
  tb->is_visible = 0;
  tb->current_page = 0;
  tb->arrow_on_bottom = 1;

  mask = desc->allowed_actions_mask;
  for (i = 0; i < (int)CUPERTINO_TEXT_ACTION_COUNT; i++) {
    if (mask == 0 || (mask & (1 << i))) {
      tb->actions[tb->action_count++] = (enum cupertino_text_action)i;
    }
  }

  tb->total_pages = (tb->action_count + CUPERTINO_TEXT_TOOLBAR_PAGE_SIZE - 1) /
                    CUPERTINO_TEXT_TOOLBAR_PAGE_SIZE;
  if (tb->total_pages == 0) {
    tb->total_pages = 1;
  }

  *out_toolbar = tb;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_selection_toolbar_destroy(
    struct cupertino_text_selection_toolbar *toolbar) {
  if (!toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(toolbar);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_selection_toolbar_show(
    struct cupertino_text_selection_toolbar *toolbar, float anchor_x,
    float anchor_y, float selection_w, float selection_h) {
  int items_on_page;

  if (!toolbar || selection_w < 0.0f || selection_h < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toolbar->anchor_x = anchor_x;
  toolbar->anchor_y = anchor_y;
  toolbar->selection_w = selection_w;
  toolbar->selection_h = selection_h;
  toolbar->is_visible = 1;

  items_on_page = (toolbar->action_count < CUPERTINO_TEXT_TOOLBAR_PAGE_SIZE)
                      ? toolbar->action_count
                      : CUPERTINO_TEXT_TOOLBAR_PAGE_SIZE;
  if (items_on_page == 0) {
    items_on_page = 1;
  }

  toolbar->bubble_w = (float)items_on_page * CUPERTINO_TEXT_TOOLBAR_ITEM_WIDTH;
  toolbar->bubble_h =
      CUPERTINO_TEXT_TOOLBAR_HEIGHT + CUPERTINO_TEXT_TOOLBAR_ARROW_HEIGHT;

  if (anchor_y - toolbar->bubble_h > 10.0f) {
    toolbar->bubble_y = anchor_y - toolbar->bubble_h - 6.0f;
    toolbar->arrow_on_bottom = 1;
  } else {
    toolbar->bubble_y = anchor_y + selection_h + 6.0f;
    toolbar->arrow_on_bottom = 0;
  }

  toolbar->bubble_x =
      anchor_x + (selection_w * 0.5f) - (toolbar->bubble_w * 0.5f);
  if (toolbar->bubble_x < 10.0f) {
    toolbar->bubble_x = 10.0f;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_selection_toolbar_hide(
    struct cupertino_text_selection_toolbar *toolbar) {
  if (!toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toolbar->is_visible = 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_selection_toolbar_is_visible(
    const struct cupertino_text_selection_toolbar *toolbar, int *out_visible) {
  if (!toolbar || !out_visible) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_visible = toolbar->is_visible;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_selection_toolbar_get_page(
    const struct cupertino_text_selection_toolbar *toolbar, int *out_page,
    int *out_total_pages) {
  if (!toolbar || !out_page || !out_total_pages) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_page = toolbar->current_page;
  *out_total_pages = toolbar->total_pages;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_selection_toolbar_paginate(
    struct cupertino_text_selection_toolbar *toolbar, int forward) {
  if (!toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (forward) {
    if (toolbar->current_page < toolbar->total_pages - 1) {
      toolbar->current_page++;
    }
  } else {
    if (toolbar->current_page > 0) {
      toolbar->current_page--;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_selection_toolbar_trigger_action(
    struct cupertino_text_selection_toolbar *toolbar,
    enum cupertino_text_action action, int *out_handled) {
  int i;

  if (!toolbar || !out_handled || (int)action < 0 ||
      (int)action >= (int)CUPERTINO_TEXT_ACTION_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_handled = 0;
  for (i = 0; i < toolbar->action_count; i++) {
    if (toolbar->actions[i] == action) {
      *out_handled = 1;
      break;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_selection_toolbar_get_bounds(
    const struct cupertino_text_selection_toolbar *toolbar, float *out_x,
    float *out_y, float *out_w, float *out_h) {
  if (!toolbar || !out_x || !out_y || !out_w || !out_h) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_x = toolbar->bubble_x;
  *out_y = toolbar->bubble_y;
  *out_w = toolbar->bubble_w;
  *out_h = toolbar->bubble_h;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_selection_toolbar_get_handle_bounds(
    const struct cupertino_text_selection_toolbar *toolbar, int is_start_handle,
    float *out_x, float *out_y, float *out_w, float *out_h) {
  if (!toolbar || !out_x || !out_y || !out_w || !out_h) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_w = CUPERTINO_TEXT_HANDLE_RADIUS * 2.0f;
  *out_h = CUPERTINO_TEXT_HANDLE_BAR_HEIGHT;

  if (is_start_handle) {
    *out_x = toolbar->anchor_x - CUPERTINO_TEXT_HANDLE_RADIUS;
    *out_y = toolbar->anchor_y;
  } else {
    *out_x =
        toolbar->anchor_x + toolbar->selection_w - CUPERTINO_TEXT_HANDLE_RADIUS;
    *out_y = toolbar->anchor_y;
  }

  return UI_ERROR_NONE;
}
