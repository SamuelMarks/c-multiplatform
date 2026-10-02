/**
 * @file cupertino_split_view.c
 * @brief Cupertino Navigation Split View implementation conforming to Apple
 * HIG.
 */

/* clang-format off */
#include "cupertino/cupertino_split_view.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_split_view_mock_base_create_fail = 0;
int g_cupertino_split_view_mock_base_destroy_fail = 0;
int g_cupertino_split_view_mock_base_set_orient_fail = 0;
int g_cupertino_split_view_mock_base_set_bounds_fail = 0;
int g_cupertino_split_view_mock_base_set_pos_fail = 0;
int g_cupertino_split_view_mock_base_forward_fail = 0;

static ui_error_t
mock_ui_split_pane_base_create(struct ui_split_pane_base **out_base) {
  if (g_cupertino_split_view_mock_base_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  return ui_split_pane_base_create(out_base);
}
#undef ui_split_pane_base_create
/** @cond */
#define ui_split_pane_base_create mock_ui_split_pane_base_create
/** @endcond */

static ui_error_t
mock_ui_split_pane_base_destroy(struct ui_split_pane_base *base) {
  if (g_cupertino_split_view_mock_base_destroy_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_split_pane_base_destroy(base);
}
#undef ui_split_pane_base_destroy
/** @cond */
#define ui_split_pane_base_destroy mock_ui_split_pane_base_destroy
/** @endcond */

static ui_error_t
mock_ui_split_pane_base_set_orientation(struct ui_split_pane_base *base,
                                        enum ui_split_pane_orientation orient) {
  if (g_cupertino_split_view_mock_base_set_orient_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_split_pane_base_set_orientation(base, orient);
}
#undef ui_split_pane_base_set_orientation
/** @cond */
#define ui_split_pane_base_set_orientation                                     \
  mock_ui_split_pane_base_set_orientation
/** @endcond */

static ui_error_t
mock_ui_split_pane_base_set_bounds(struct ui_split_pane_base *base, int min_pos,
                                   int max_pos) {
  if (g_cupertino_split_view_mock_base_set_bounds_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_split_pane_base_set_bounds(base, min_pos, max_pos);
}
#undef ui_split_pane_base_set_bounds
/** @cond */
#define ui_split_pane_base_set_bounds mock_ui_split_pane_base_set_bounds
/** @endcond */

static ui_error_t
mock_ui_split_pane_base_set_position(struct ui_split_pane_base *base,
                                     int position) {
  if (g_cupertino_split_view_mock_base_set_pos_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_split_pane_base_set_position(base, position);
}
#undef ui_split_pane_base_set_position
/** @cond */
#define ui_split_pane_base_set_position mock_ui_split_pane_base_set_position
/** @endcond */

static ui_error_t
mock_ui_split_pane_base_process_event(struct ui_split_pane_base *base,
                                      const struct ui_event *event) {
  if (g_cupertino_split_view_mock_base_forward_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_split_pane_base_process_event(base, event);
}
#undef ui_split_pane_base_process_event
/** @cond */
#define ui_split_pane_base_process_event mock_ui_split_pane_base_process_event
/** @endcond */

int g_cupertino_split_view_mock_base_get_pos_fail = 0;
static ui_error_t
mock_ui_split_pane_base_get_position(struct ui_split_pane_base *base,
                                     int *out_pos) {
  if (g_cupertino_split_view_mock_base_get_pos_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_split_pane_base_get_position(base, out_pos);
}
#undef ui_split_pane_base_get_position
/** @cond */
#define ui_split_pane_base_get_position mock_ui_split_pane_base_get_position
/** @endcond */

#endif /* UI_TEST_MOCK_ALLOC */

static void
cupertino_split_view_update_layout(struct cupertino_split_view *view) {
  if (view->is_collapsed) {
    view->show_sidebar = 0;
    view->show_content = 0;
    view->show_detail = 1;
    return;
  }

  switch (view->display_mode) {
  case CUPERTINO_SPLIT_VIEW_SECONDARY_ONLY:
    view->show_sidebar = 0;
    view->show_content = 0;
    view->show_detail = 1;
    break;

  case CUPERTINO_SPLIT_VIEW_ONE_BESIDE_SECONDARY:
    if (view->style == CUPERTINO_SPLIT_VIEW_THREE_COLUMN) {
      view->show_sidebar = 0;
      view->show_content = 1;
      view->show_detail = 1;
    } else {
      view->show_sidebar = 1;
      view->show_content = 0;
      view->show_detail = 1;
    }
    break;

  case CUPERTINO_SPLIT_VIEW_TWO_BESIDE_SECONDARY:
    if (view->style == CUPERTINO_SPLIT_VIEW_THREE_COLUMN) {
      view->show_sidebar = 1;
      view->show_content = 1;
      view->show_detail = 1;
    } else {
      view->show_sidebar = 1;
      view->show_content = 0;
      view->show_detail = 1;
    }
    break;

  case CUPERTINO_SPLIT_VIEW_AUTOMATIC:
  default:
    if (view->style == CUPERTINO_SPLIT_VIEW_THREE_COLUMN) {
      if (view->viewport_width >= 1100.0f) {
        view->show_sidebar = 1;
        view->show_content = 1;
        view->show_detail = 1;
      } else {
        view->show_sidebar = 0;
        view->show_content = 1;
        view->show_detail = 1;
      }
    } else {
      view->show_sidebar = 1;
      view->show_content = 0;
      view->show_detail = 1;
    }
    break;
  }
}

ui_error_t
cupertino_split_view_create(struct ui_engine *engine,
                            const struct cupertino_split_view_descriptor *desc,
                            struct cupertino_split_view **out_view) {
  struct cupertino_split_view *view;
  ui_error_t rc;
  ui_error_t rc_cleanup;

  if (!engine || !desc || !out_view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if ((int)desc->style < 0 || (int)desc->style > 1) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if ((int)desc->display_mode < 0 || (int)desc->display_mode > 3) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  view = (struct cupertino_split_view *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_split_view));
  if (!view) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(view, 0, sizeof(*view));
  view->style = desc->style;
  view->display_mode = desc->display_mode;
  view->viewport_width =
      (desc->viewport_width >= 0.0f) ? desc->viewport_width : 1024.0f;
  view->viewport_height =
      (desc->viewport_height >= 0.0f) ? desc->viewport_height : 768.0f;
  view->is_collapsed =
      (view->viewport_width > 0.0f &&
       view->viewport_width < CUPERTINO_SPLIT_VIEW_COMPACT_BREAKPOINT)
          ? 1
          : 0;

  if (desc->sidebar_width >= CUPERTINO_SPLIT_VIEW_MIN_SIDEBAR_WIDTH &&
      desc->sidebar_width <= CUPERTINO_SPLIT_VIEW_MAX_SIDEBAR_WIDTH) {
    view->sidebar_width = desc->sidebar_width;
  } else {
    view->sidebar_width = CUPERTINO_SPLIT_VIEW_DEFAULT_SIDEBAR_WIDTH;
  }

  if (desc->content_width >= CUPERTINO_SPLIT_VIEW_MIN_CONTENT_WIDTH &&
      desc->content_width <= CUPERTINO_SPLIT_VIEW_MAX_CONTENT_WIDTH) {
    view->content_width = desc->content_width;
  } else {
    view->content_width = CUPERTINO_SPLIT_VIEW_DEFAULT_CONTENT_WIDTH;
  }

  cupertino_split_view_update_layout(view);

  rc = ui_split_pane_base_create(&view->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(view);
    return rc;
  }

  rc = ui_split_pane_base_set_orientation(view->base,
                                          UI_SPLIT_PANE_ORIENTATION_HORIZONTAL);
  if (rc != UI_ERROR_NONE) {
    rc_cleanup = ui_split_pane_base_destroy(view->base);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* Log or preserve error */
    }
    C_MULTIPLATFORM_FREE(view);
    return rc;
  }

  rc = ui_split_pane_base_set_bounds(
      view->base, (int)CUPERTINO_SPLIT_VIEW_MIN_SIDEBAR_WIDTH,
      (int)CUPERTINO_SPLIT_VIEW_MAX_SIDEBAR_WIDTH);
  if (rc != UI_ERROR_NONE) {
    rc_cleanup = ui_split_pane_base_destroy(view->base);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* Log or preserve error */
    }
    C_MULTIPLATFORM_FREE(view);
    return rc;
  }

  rc = ui_split_pane_base_set_position(view->base, (int)view->sidebar_width);
  if (rc != UI_ERROR_NONE) {
    rc_cleanup = ui_split_pane_base_destroy(view->base);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* Log or preserve error */
    }
    C_MULTIPLATFORM_FREE(view);
    return rc;
  }

  *out_view = view;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_split_view_destroy(struct cupertino_split_view *view) {
  ui_error_t rc;

  if (!view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (view->base) {
    rc = ui_split_pane_base_destroy(view->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    view->base = NULL;
  }

  C_MULTIPLATFORM_FREE(view);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_split_view_set_style(struct cupertino_split_view *view,
                               enum cupertino_split_view_style style) {
  if (!view || ((int)style < 0 || (int)style > 1)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  view->style = style;
  cupertino_split_view_update_layout(view);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_split_view_get_style(const struct cupertino_split_view *view,
                               enum cupertino_split_view_style *out_style) {
  if (!view || !out_style) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_style = view->style;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_split_view_set_display_mode(
    struct cupertino_split_view *view,
    enum cupertino_split_view_display_mode mode) {
  if (!view || ((int)mode < 0 || (int)mode > 3)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  view->display_mode = mode;
  cupertino_split_view_update_layout(view);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_split_view_get_display_mode(
    const struct cupertino_split_view *view,
    enum cupertino_split_view_display_mode *out_mode) {
  if (!view || !out_mode) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_mode = view->display_mode;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_split_view_set_sidebar_width(struct cupertino_split_view *view,
                                       float width) {
  ui_error_t rc;

  if (!view || width < CUPERTINO_SPLIT_VIEW_MIN_SIDEBAR_WIDTH ||
      width > CUPERTINO_SPLIT_VIEW_MAX_SIDEBAR_WIDTH) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  view->sidebar_width = width;
  if (view->base) {
    rc = ui_split_pane_base_set_position(view->base, (int)width);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_split_view_get_sidebar_width(const struct cupertino_split_view *view,
                                       float *out_width) {
  if (!view || !out_width) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_width = view->sidebar_width;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_split_view_set_content_width(struct cupertino_split_view *view,
                                       float width) {
  if (!view || width < CUPERTINO_SPLIT_VIEW_MIN_CONTENT_WIDTH ||
      width > CUPERTINO_SPLIT_VIEW_MAX_CONTENT_WIDTH) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  view->content_width = width;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_split_view_get_content_width(const struct cupertino_split_view *view,
                                       float *out_width) {
  if (!view || !out_width) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_width = view->content_width;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_split_view_set_viewport_size(struct cupertino_split_view *view,
                                       float width, float height) {
  if (!view || width < 0.0f || height < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  view->viewport_width = width;
  view->viewport_height = height;
  if (width > 0.0f) {
    view->is_collapsed =
        (width < CUPERTINO_SPLIT_VIEW_COMPACT_BREAKPOINT) ? 1 : 0;
  }
  cupertino_split_view_update_layout(view);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_split_view_get_viewport_size(const struct cupertino_split_view *view,
                                       float *out_width, float *out_height) {
  if (!view || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_width = view->viewport_width;
  *out_height = view->viewport_height;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_split_view_set_collapsed(struct cupertino_split_view *view,
                                              int collapsed) {
  if (!view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  view->is_collapsed = collapsed ? 1 : 0;
  cupertino_split_view_update_layout(view);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_split_view_is_collapsed(const struct cupertino_split_view *view,
                                  int *out_collapsed) {
  if (!view || !out_collapsed) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_collapsed = view->is_collapsed;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_split_view_get_column_visibility(
    const struct cupertino_split_view *view, int *out_sidebar, int *out_content,
    int *out_detail) {
  if (!view || !out_sidebar || !out_content || !out_detail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_sidebar = view->show_sidebar;
  *out_content = view->show_content;
  *out_detail = view->show_detail;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_split_view_process_event(struct cupertino_split_view *view,
                                              const struct ui_event *event) {
  int pos = 0;
  ui_error_t rc;

  if (!view || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (view->base) {
    rc = ui_split_pane_base_process_event(view->base, event);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = ui_split_pane_base_get_position(view->base, &pos);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    view->sidebar_width = (float)pos;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_split_view_get_base(struct cupertino_split_view *view,
                                         struct ui_split_pane_base **out_base) {
  if (!view || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_base = view->base;
  return UI_ERROR_NONE;
}
