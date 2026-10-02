/**
 * @file cupertino_tab_view.c
 * @brief Cupertino Tab View independent parallel navigation coordinator
 * implementation conforming to Apple HIG.
 */

/* clang-format off */
#include "cupertino/cupertino_tab_view.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

ui_error_t
cupertino_tab_view_create(struct ui_engine *engine,
                          const struct cupertino_tab_view_descriptor *desc,
                          struct cupertino_tab_view **out_view) {
  struct cupertino_tab_view *view;

  if (!engine || !desc || !out_view || !desc->root_view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  view = (struct cupertino_tab_view *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_tab_view));
  if (!view) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(view, 0, sizeof(*view));
  view->stack[0] = desc->root_view;
  view->stack_depth = 1;

  if (desc->default_title) {
#if defined(_MSC_VER)
    strncpy_s(view->titles[0], sizeof(view->titles[0]), desc->default_title,
              _TRUNCATE);
#else
    strncpy(view->titles[0], desc->default_title, sizeof(view->titles[0]) - 1);
    view->titles[0][sizeof(view->titles[0]) - 1] = '\0';
#endif
  }

  *out_view = view;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_view_destroy(struct cupertino_tab_view *view) {
  if (!view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(view);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_view_push(struct cupertino_tab_view *view,
                                   struct ui_component *component,
                                   const char *title) {
  if (!view || !component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (view->stack_depth >= CUPERTINO_TAB_VIEW_MAX_STACK_DEPTH) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  view->stack[view->stack_depth] = component;
  view->scroll_offsets[view->stack_depth] = 0.0f;

  if (title) {
#if defined(_MSC_VER)
    strncpy_s(view->titles[view->stack_depth],
              sizeof(view->titles[view->stack_depth]), title, _TRUNCATE);
#else
    strncpy(view->titles[view->stack_depth], title,
            sizeof(view->titles[view->stack_depth]) - 1);
    view->titles[view->stack_depth]
                [sizeof(view->titles[view->stack_depth]) - 1] = '\0';
#endif
  } else {
    view->titles[view->stack_depth][0] = '\0';
  }

  view->stack_depth++;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_view_pop(struct cupertino_tab_view *view,
                                  struct ui_component **out_popped) {
  if (!view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (view->stack_depth <= 1) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  view->stack_depth--;
  if (out_popped) {
    *out_popped = view->stack[view->stack_depth];
  }

  view->stack[view->stack_depth] = NULL;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_view_pop_to_root(struct cupertino_tab_view *view) {
  size_t i;

  if (!view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 1; i < view->stack_depth; i++) {
    view->stack[i] = NULL;
  }

  view->stack_depth = 1;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_tab_view_get_stack_depth(const struct cupertino_tab_view *view,
                                   size_t *out_depth) {
  if (!view || !out_depth) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_depth = view->stack_depth;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_tab_view_get_current_view(const struct cupertino_tab_view *view,
                                    struct ui_component **out_component) {
  if (!view || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = view->stack[view->stack_depth - 1];
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_tab_view_get_current_title(const struct cupertino_tab_view *view,
                                     const char **out_title) {
  if (!view || !out_title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_title = view->titles[view->stack_depth - 1];
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_view_set_scroll_offset(struct cupertino_tab_view *view,
                                                float offset) {
  if (!view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  view->scroll_offsets[view->stack_depth - 1] = offset;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_tab_view_get_scroll_offset(const struct cupertino_tab_view *view,
                                     float *out_offset) {
  if (!view || !out_offset) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_offset = view->scroll_offsets[view->stack_depth - 1];
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_view_handle_tab_tap(struct cupertino_tab_view *view,
                                             int *out_did_scroll_to_top) {
  if (!view || !out_did_scroll_to_top) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (view->stack_depth > 1) {
    /* Pop back to root */
    view->stack_depth = 1;
    *out_did_scroll_to_top = 0;
  } else {
    /* Already at root: scroll to top */
    view->scroll_offsets[0] = 0.0f;
    *out_did_scroll_to_top = 1;
  }

  return UI_ERROR_NONE;
}
