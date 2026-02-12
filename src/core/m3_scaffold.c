#include "m3/m3_scaffold.h"

#include <string.h>

static int m3_scaffold_validate_edges(const M3LayoutEdges *edges) {
  if (edges == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->right < 0.0f || edges->top < 0.0f ||
      edges->bottom < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_scaffold_validate_measure_spec(M3MeasureSpec spec) {
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.mode != M3_MEASURE_EXACTLY &&
      spec.mode != M3_MEASURE_AT_MOST) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_scaffold_validate_rect(const M3Rect *rect) {
  if (rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_scaffold_validate_style(const M3ScaffoldStyle *style) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_scaffold_validate_edges(&style->padding);
  if (rc != M3_OK) {
    return rc;
  }

  if (style->fab_margin_x < 0.0f || style->fab_margin_y < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->snackbar_margin_x < 0.0f || style->snackbar_margin_y < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->fab_snackbar_spacing < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->fab_slide_duration < 0.0f) {
    return M3_ERR_RANGE;
  }

  switch (style->fab_slide_easing) {
  case M3_ANIM_EASE_LINEAR:
  case M3_ANIM_EASE_IN:
  case M3_ANIM_EASE_OUT:
  case M3_ANIM_EASE_IN_OUT:
    break;
  default:
    return M3_ERR_RANGE;
  }

  return M3_OK;
}

static int m3_scaffold_compute_safe_bounds(const M3Rect *bounds,
                                           const M3LayoutEdges *safe_area,
                                           M3Rect *out_bounds) {
  M3Scalar width;
  M3Scalar height;

  if (bounds == NULL || safe_area == NULL || out_bounds == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  width = bounds->width - safe_area->left - safe_area->right;
  height = bounds->height - safe_area->top - safe_area->bottom;
  if (width < 0.0f || height < 0.0f) {
    return M3_ERR_RANGE;
  }

  out_bounds->x = bounds->x + safe_area->left;
  out_bounds->y = bounds->y + safe_area->top;
  out_bounds->width = width;
  out_bounds->height = height;
  return M3_OK;
}

static int m3_scaffold_compute_fab_target(const M3ScaffoldStyle *style,
                                          M3Scalar snackbar_height,
                                          M3Scalar *out_offset) {
  M3Scalar offset;

  if (style == NULL || out_offset == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (snackbar_height < 0.0f) {
    return M3_ERR_RANGE;
  }

  if (snackbar_height <= 0.0f) {
    *out_offset = 0.0f;
    return M3_OK;
  }

  offset = snackbar_height + style->fab_snackbar_spacing +
           style->snackbar_margin_y - style->fab_margin_y;
  if (offset < 0.0f) {
    offset = 0.0f; /* GCOVR_EXCL_LINE */
  }
  *out_offset = offset;
  return M3_OK;
}

static int m3_scaffold_widget_is_visible(const M3Widget *widget,
                                         M3Bool *out_visible) {
  if (out_visible == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (widget == NULL) {
    *out_visible = M3_FALSE;
    return M3_OK;
  }

  if (widget->flags & M3_WIDGET_FLAG_HIDDEN) {
    *out_visible = M3_FALSE;
    return M3_OK;
  }

  *out_visible = M3_TRUE;
  return M3_OK;
}

static int m3_scaffold_measure_child(M3Widget *child, M3MeasureSpec width,
                                     M3MeasureSpec height, M3Size *out_size) {
  int rc;

  if (child == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_scaffold_validate_measure_spec(width);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_scaffold_validate_measure_spec(height);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  if (child->vtable == NULL || child->vtable->measure == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  return child->vtable->measure(child->ctx, width, height, out_size);
}

static int m3_scaffold_layout_child(M3Widget *child, const M3Rect *bounds) {
  int rc;

  if (child == NULL || bounds == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_scaffold_validate_rect(bounds);
  if (rc != M3_OK) {
    return rc;
  }

  if (child->vtable == NULL || child->vtable->layout == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  return child->vtable->layout(child->ctx, *bounds);
}

static int m3_scaffold_paint_child(M3Widget *child, M3PaintContext *ctx) {
  if (child == NULL || ctx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (child->vtable == NULL || child->vtable->paint == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  return child->vtable->paint(child->ctx, ctx);
}

static int m3_scaffold_event_child(M3Widget *child, const M3InputEvent *event,
                                   M3Bool *out_handled) {
  M3Bool handled;

  if (out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_handled = M3_FALSE;

  if (child == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (child->vtable == NULL || child->vtable->event == NULL) {
    return M3_OK;
  }

  handled = M3_FALSE;
  {
    int rc = child->vtable->event(child->ctx, event, &handled);
    if (rc != M3_OK) {
      return rc;
    }
  }
  *out_handled = handled;
  return M3_OK;
}

static int m3_scaffold_event_get_position(const M3InputEvent *event,
                                          M3Bool *out_has_pos, M3Scalar *out_x,
                                          M3Scalar *out_y) {
  if (event == NULL || out_has_pos == NULL || out_x == NULL || out_y == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_has_pos = M3_FALSE;
  *out_x = 0.0f;
  *out_y = 0.0f;

  switch (event->type) {
  case M3_INPUT_POINTER_DOWN:
  case M3_INPUT_POINTER_UP:
  case M3_INPUT_POINTER_MOVE:
  case M3_INPUT_POINTER_SCROLL:
    *out_has_pos = M3_TRUE;
    *out_x = (M3Scalar)event->data.pointer.x;
    *out_y = (M3Scalar)event->data.pointer.y;
    return M3_OK;
  case M3_INPUT_GESTURE_TAP:
  case M3_INPUT_GESTURE_DOUBLE_TAP:
  case M3_INPUT_GESTURE_LONG_PRESS:
  case M3_INPUT_GESTURE_DRAG_START:
  case M3_INPUT_GESTURE_DRAG_UPDATE:
  case M3_INPUT_GESTURE_DRAG_END:
  case M3_INPUT_GESTURE_FLING:
    *out_has_pos = M3_TRUE;
    *out_x = event->data.gesture.x;
    *out_y = event->data.gesture.y;
    return M3_OK;
  default:
    return M3_OK;
  }
}

static int m3_scaffold_apply_measure_spec(M3Scalar desired, M3MeasureSpec spec,
                                          M3Scalar *out_value) {
  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (spec.mode == M3_MEASURE_EXACTLY) {
    *out_value = spec.size;
    return M3_OK;
  }
  if (spec.mode == M3_MEASURE_AT_MOST) {
    *out_value = (desired > spec.size) ? spec.size : desired;
    return M3_OK;
  }

  *out_value = desired;
  return M3_OK;
}

static int m3_scaffold_widget_measure(void *widget, M3MeasureSpec width,
                                      M3MeasureSpec height, M3Size *out_size) {
  M3Scaffold *scaffold;
  M3MeasureSpec child_width;
  M3MeasureSpec child_height;
  M3Size top_size;
  M3Size bottom_size;
  M3Size body_size;
  M3Size fab_size;
  M3Size snackbar_size;
  M3Scalar desired_width;
  M3Scalar desired_height;
  M3Scalar safe_width_needed;
  M3Scalar safe_height_needed;
  M3Scalar body_width_needed;
  M3Scalar body_height_needed;
  M3Scalar child_width_limit;
  M3Scalar child_height_limit;
  M3Scalar snack_width_needed;
  M3Scalar fab_width_needed;
  M3Scalar snack_height_needed;
  M3Scalar fab_height_needed;
  M3Bool visible;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_scaffold_validate_measure_spec(width);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_scaffold_validate_measure_spec(height);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  scaffold = (M3Scaffold *)widget;
  rc = m3_scaffold_validate_style(&scaffold->style);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_scaffold_validate_edges(&scaffold->safe_area);
  if (rc != M3_OK) {
    return rc;
  }

  child_width_limit = 0.0f;
  if (width.mode != M3_MEASURE_UNSPECIFIED) {
    child_width_limit =
        width.size - scaffold->safe_area.left - scaffold->safe_area.right;
    if (child_width_limit < 0.0f) {
      return M3_ERR_RANGE;
    }
  }

  child_height_limit = 0.0f;
  if (height.mode != M3_MEASURE_UNSPECIFIED) {
    child_height_limit =
        height.size - scaffold->safe_area.top - scaffold->safe_area.bottom;
    if (child_height_limit < 0.0f) {
      return M3_ERR_RANGE;
    }
  }

  if (width.mode == M3_MEASURE_UNSPECIFIED) {
    child_width.mode = M3_MEASURE_UNSPECIFIED;
    child_width.size = 0.0f;
  } else {
    child_width.mode = width.mode;
    child_width.size = child_width_limit;
  }

  if (height.mode == M3_MEASURE_UNSPECIFIED) {
    child_height.mode = M3_MEASURE_UNSPECIFIED;
    child_height.size = 0.0f;
  } else {
    child_height.mode = height.mode;
    child_height.size = child_height_limit;
  }

  top_size.width = 0.0f;
  top_size.height = 0.0f;
  bottom_size.width = 0.0f;
  bottom_size.height = 0.0f;
  body_size.width = 0.0f;
  body_size.height = 0.0f;
  fab_size.width = 0.0f;
  fab_size.height = 0.0f;
  snackbar_size.width = 0.0f;
  snackbar_size.height = 0.0f;

  rc = m3_scaffold_widget_is_visible(scaffold->top_bar, &visible);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == M3_TRUE) {
    rc = m3_scaffold_measure_child(scaffold->top_bar, child_width, child_height,
                                   &top_size);
    if (rc != M3_OK) {
      return rc;
    }
  }

  rc = m3_scaffold_widget_is_visible(scaffold->bottom_bar, &visible);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == M3_TRUE) {
    rc = m3_scaffold_measure_child(scaffold->bottom_bar, child_width,
                                   child_height, &bottom_size);
    if (rc != M3_OK) {
      return rc;
    }
  }

  rc = m3_scaffold_widget_is_visible(scaffold->body, &visible);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == M3_TRUE) {
    rc = m3_scaffold_measure_child(scaffold->body, child_width, child_height,
                                   &body_size);
    if (rc != M3_OK) {
      return rc;
    }
  }

  rc = m3_scaffold_widget_is_visible(scaffold->fab, &visible);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == M3_TRUE) {
    rc = m3_scaffold_measure_child(scaffold->fab, child_width, child_height,
                                   &fab_size);
    if (rc != M3_OK) {
      return rc;
    }
  }

  rc = m3_scaffold_widget_is_visible(scaffold->snackbar, &visible);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == M3_TRUE) {
    rc = m3_scaffold_measure_child(scaffold->snackbar, child_width,
                                   child_height, &snackbar_size);
    if (rc != M3_OK) {
      return rc;
    }
  }

  safe_width_needed = 0.0f;
  safe_height_needed = 0.0f;

  if (top_size.width > safe_width_needed) {
    safe_width_needed = top_size.width;
  }
  if (bottom_size.width > safe_width_needed) {
    safe_width_needed = bottom_size.width; /* GCOVR_EXCL_LINE */
  }

  body_width_needed = body_size.width;
  body_height_needed = body_size.height;
  if (body_width_needed > 0.0f || body_height_needed > 0.0f) {
    body_width_needed +=
        scaffold->style.padding.left + scaffold->style.padding.right;
    body_height_needed +=
        scaffold->style.padding.top + scaffold->style.padding.bottom;
  }
  if (body_width_needed > safe_width_needed) {
    safe_width_needed = body_width_needed;
  }

  snack_width_needed =
      snackbar_size.width + scaffold->style.snackbar_margin_x * 2.0f;
  if (snack_width_needed > safe_width_needed) {
    safe_width_needed = snack_width_needed;
  }

  fab_width_needed = fab_size.width + scaffold->style.fab_margin_x * 2.0f;
  if (fab_width_needed > safe_width_needed) {
    safe_width_needed = fab_width_needed; /* GCOVR_EXCL_LINE */
  }

  safe_height_needed =
      top_size.height + bottom_size.height + body_height_needed;

  snack_height_needed = bottom_size.height +
                        scaffold->style.snackbar_margin_y * 2.0f +
                        snackbar_size.height;
  if (top_size.height + snack_height_needed > safe_height_needed) {
    safe_height_needed = top_size.height + snack_height_needed;
  }

  fab_height_needed = bottom_size.height + scaffold->style.fab_margin_y * 2.0f +
                      fab_size.height;
  if (top_size.height + fab_height_needed > safe_height_needed) {
    safe_height_needed =
        top_size.height + fab_height_needed; /* GCOVR_EXCL_LINE */
  }

  desired_width =
      safe_width_needed + scaffold->safe_area.left + scaffold->safe_area.right;
  desired_height =
      safe_height_needed + scaffold->safe_area.top + scaffold->safe_area.bottom;

  rc = m3_scaffold_apply_measure_spec(desired_width, width, &out_size->width);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc =
      m3_scaffold_apply_measure_spec(desired_height, height, &out_size->height);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  return M3_OK;
}

static int m3_scaffold_widget_layout(void *widget, M3Rect bounds) {
  M3Scaffold *scaffold;
  M3Rect safe_bounds;
  M3MeasureSpec width_spec;
  M3MeasureSpec height_spec;
  M3Size top_size;
  M3Size bottom_size;
  M3Size fab_size;
  M3Size snackbar_size;
  M3Scalar safe_width;
  M3Scalar safe_height;
  M3Scalar available_height;
  M3Scalar body_width;
  M3Scalar body_height;
  M3Scalar snackbar_available_width;
  M3Scalar fab_available_width;
  M3Scalar fab_available_height;
  M3Scalar fab_base_x;
  M3Scalar fab_base_y;
  M3Scalar target_offset;
  M3Bool visible;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_scaffold_validate_rect(&bounds);
  if (rc != M3_OK) {
    return rc;
  }

  scaffold = (M3Scaffold *)widget;
  rc = m3_scaffold_validate_style(&scaffold->style);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_scaffold_validate_edges(&scaffold->safe_area);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  scaffold->bounds = bounds;

  rc = m3_scaffold_compute_safe_bounds(&bounds, &scaffold->safe_area,
                                       &safe_bounds);
  if (rc != M3_OK) {
    return rc;
  }

  safe_width = safe_bounds.width;
  safe_height = safe_bounds.height;

  width_spec.mode = M3_MEASURE_EXACTLY;
  width_spec.size = safe_width;
  height_spec.mode = M3_MEASURE_AT_MOST;
  height_spec.size = safe_height;

  top_size.width = 0.0f;
  top_size.height = 0.0f;
  bottom_size.width = 0.0f;
  bottom_size.height = 0.0f;
  fab_size.width = 0.0f;
  fab_size.height = 0.0f;
  snackbar_size.width = 0.0f;
  snackbar_size.height = 0.0f;

  rc = m3_scaffold_widget_is_visible(scaffold->top_bar, &visible);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == M3_TRUE) {
    rc = m3_scaffold_measure_child(scaffold->top_bar, width_spec, height_spec,
                                   &top_size);
    if (rc != M3_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    if (top_size.height > safe_height) {
      return M3_ERR_RANGE; /* GCOVR_EXCL_LINE */
    }
    scaffold->top_bar_bounds.x = safe_bounds.x;
    scaffold->top_bar_bounds.y = safe_bounds.y;
    scaffold->top_bar_bounds.width = safe_width;
    scaffold->top_bar_bounds.height = top_size.height;
    rc = m3_scaffold_layout_child(scaffold->top_bar, &scaffold->top_bar_bounds);
    if (rc != M3_OK) {
      return rc;
    }
  } else {
    scaffold->top_bar_bounds.x = safe_bounds.x;
    scaffold->top_bar_bounds.y = safe_bounds.y;
    scaffold->top_bar_bounds.width = safe_width;
    scaffold->top_bar_bounds.height = 0.0f;
  }

  rc = m3_scaffold_widget_is_visible(scaffold->bottom_bar, &visible);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == M3_TRUE) {
    rc = m3_scaffold_measure_child(scaffold->bottom_bar, width_spec,
                                   height_spec, &bottom_size);
    if (rc != M3_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    if (bottom_size.height > safe_height) {
      return M3_ERR_RANGE; /* GCOVR_EXCL_LINE */
    }
    scaffold->bottom_bar_bounds.x = safe_bounds.x;
    scaffold->bottom_bar_bounds.width = safe_width;
    scaffold->bottom_bar_bounds.height = bottom_size.height;
    scaffold->bottom_bar_bounds.y =
        safe_bounds.y + safe_height - bottom_size.height;
    rc = m3_scaffold_layout_child(scaffold->bottom_bar,
                                  &scaffold->bottom_bar_bounds);
    if (rc != M3_OK) {
      return rc;
    }
  } else {
    scaffold->bottom_bar_bounds.x = safe_bounds.x;
    scaffold->bottom_bar_bounds.width = safe_width;
    scaffold->bottom_bar_bounds.height = 0.0f;
    scaffold->bottom_bar_bounds.y = safe_bounds.y + safe_height;
  }

  available_height = safe_height - scaffold->top_bar_bounds.height -
                     scaffold->bottom_bar_bounds.height;
  if (available_height < 0.0f) {
    return M3_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  body_width =
      safe_width - scaffold->style.padding.left - scaffold->style.padding.right;
  body_height = available_height - scaffold->style.padding.top -
                scaffold->style.padding.bottom;
  if (body_width < 0.0f || body_height < 0.0f) {
    return M3_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  scaffold->body_bounds.x = safe_bounds.x + scaffold->style.padding.left;
  scaffold->body_bounds.y = safe_bounds.y + scaffold->top_bar_bounds.height +
                            scaffold->style.padding.top;
  scaffold->body_bounds.width = body_width;
  scaffold->body_bounds.height = body_height;

  rc = m3_scaffold_widget_is_visible(scaffold->body, &visible);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == M3_TRUE) {
    rc = m3_scaffold_layout_child(scaffold->body, &scaffold->body_bounds);
    if (rc != M3_OK) {
      return rc;
    }
  }

  rc = m3_scaffold_widget_is_visible(scaffold->snackbar, &visible);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == M3_TRUE) {
    snackbar_available_width =
        safe_width - scaffold->style.snackbar_margin_x * 2.0f;
    if (snackbar_available_width < 0.0f) {
      return M3_ERR_RANGE; /* GCOVR_EXCL_LINE */
    }

    width_spec.mode = M3_MEASURE_AT_MOST;
    width_spec.size = snackbar_available_width;
    height_spec.mode = M3_MEASURE_AT_MOST;
    height_spec.size = safe_height;

    rc = m3_scaffold_measure_child(scaffold->snackbar, width_spec, height_spec,
                                   &snackbar_size);
    if (rc != M3_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    if (snackbar_size.width > snackbar_available_width ||
        snackbar_size.height > safe_height) {
      return M3_ERR_RANGE; /* GCOVR_EXCL_LINE */
    }

    scaffold->snackbar_bounds.width = snackbar_size.width;
    scaffold->snackbar_bounds.height = snackbar_size.height;
    scaffold->snackbar_bounds.x =
        safe_bounds.x + scaffold->style.snackbar_margin_x +
        (snackbar_available_width - snackbar_size.width) * 0.5f;
    scaffold->snackbar_bounds.y =
        safe_bounds.y + safe_height - scaffold->bottom_bar_bounds.height -
        scaffold->style.snackbar_margin_y - snackbar_size.height;
    if (scaffold->snackbar_bounds.y < safe_bounds.y) {
      return M3_ERR_RANGE; /* GCOVR_EXCL_LINE */
    }

    rc = m3_scaffold_layout_child(scaffold->snackbar,
                                  &scaffold->snackbar_bounds);
    if (rc != M3_OK) {
      return rc;
    }
  } else {
    scaffold->snackbar_bounds.x = safe_bounds.x; /* GCOVR_EXCL_LINE */
    scaffold->snackbar_bounds.y = safe_bounds.y; /* GCOVR_EXCL_LINE */
    scaffold->snackbar_bounds.width = 0.0f;      /* GCOVR_EXCL_LINE */
    scaffold->snackbar_bounds.height = 0.0f;     /* GCOVR_EXCL_LINE */
  }

  rc = m3_scaffold_widget_is_visible(scaffold->fab, &visible);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == M3_TRUE) {
    fab_available_width = safe_width - scaffold->style.fab_margin_x * 2.0f;
    fab_available_height = safe_height - scaffold->bottom_bar_bounds.height -
                           scaffold->style.fab_margin_y * 2.0f;
    if (fab_available_width < 0.0f || fab_available_height < 0.0f) {
      return M3_ERR_RANGE; /* GCOVR_EXCL_LINE */
    }

    width_spec.mode = M3_MEASURE_AT_MOST;
    width_spec.size = fab_available_width;
    height_spec.mode = M3_MEASURE_AT_MOST;
    height_spec.size = fab_available_height;

    rc = m3_scaffold_measure_child(scaffold->fab, width_spec, height_spec,
                                   &fab_size);
    if (rc != M3_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    if (fab_size.width > fab_available_width ||
        fab_size.height > fab_available_height) {
      return M3_ERR_RANGE; /* GCOVR_EXCL_LINE */
    }

    fab_base_x = safe_bounds.x + safe_width - scaffold->style.fab_margin_x -
                 fab_size.width;
    fab_base_y = safe_bounds.y + safe_height -
                 scaffold->bottom_bar_bounds.height -
                 scaffold->style.fab_margin_y - fab_size.height;

    target_offset = 0.0f;
    if (scaffold->snackbar_bounds.height > 0.0f) {
      rc = m3_scaffold_compute_fab_target(
          &scaffold->style, scaffold->snackbar_bounds.height, &target_offset);
      if (rc != M3_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
    }

    if (target_offset != scaffold->fab_target_offset) {
      rc = m3_anim_controller_start_timing(
          &scaffold->fab_anim, scaffold->fab_offset, target_offset,
          scaffold->style.fab_slide_duration, scaffold->style.fab_slide_easing);
      if (rc != M3_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
      scaffold->fab_target_offset = target_offset;
      if (scaffold->style.fab_slide_duration == 0.0f) {
        scaffold->fab_offset = target_offset;
      }
    }

    scaffold->fab_bounds.width = fab_size.width;
    scaffold->fab_bounds.height = fab_size.height;
    scaffold->fab_bounds.x = fab_base_x;
    scaffold->fab_bounds.y = fab_base_y - scaffold->fab_offset;

    rc = m3_scaffold_layout_child(scaffold->fab, &scaffold->fab_bounds);
    if (rc != M3_OK) {
      return rc;
    }
  } else {
    scaffold->fab_bounds.x = safe_bounds.x; /* GCOVR_EXCL_LINE */
    scaffold->fab_bounds.y = safe_bounds.y; /* GCOVR_EXCL_LINE */
    scaffold->fab_bounds.width = 0.0f;      /* GCOVR_EXCL_LINE */
    scaffold->fab_bounds.height = 0.0f;     /* GCOVR_EXCL_LINE */
  }

  return M3_OK;
}

static int m3_scaffold_widget_paint(void *widget, M3PaintContext *ctx) {
  M3Scaffold *scaffold;
  M3Bool visible;
  int rc;

  if (widget == NULL || ctx == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  scaffold = (M3Scaffold *)widget;

  rc = m3_scaffold_widget_is_visible(scaffold->body, &visible);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == M3_TRUE) {
    rc = m3_scaffold_paint_child(scaffold->body, ctx);
    if (rc != M3_OK) {
      return rc;
    }
  }

  rc = m3_scaffold_widget_is_visible(scaffold->top_bar, &visible);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == M3_TRUE) {
    rc = m3_scaffold_paint_child(scaffold->top_bar, ctx);
    if (rc != M3_OK) {
      return rc;
    }
  }

  rc = m3_scaffold_widget_is_visible(scaffold->bottom_bar, &visible);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == M3_TRUE) {
    rc = m3_scaffold_paint_child(scaffold->bottom_bar, ctx);
    if (rc != M3_OK) {
      return rc;
    }
  }

  rc = m3_scaffold_widget_is_visible(scaffold->snackbar, &visible);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == M3_TRUE) {
    rc = m3_scaffold_paint_child(scaffold->snackbar, ctx);
    if (rc != M3_OK) {
      return rc;
    }
  }

  rc = m3_scaffold_widget_is_visible(scaffold->fab, &visible);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == M3_TRUE) {
    rc = m3_scaffold_paint_child(scaffold->fab, ctx);
    if (rc != M3_OK) {
      return rc;
    }
  }

  return M3_OK;
}

static int m3_scaffold_event_dispatch_to_child(M3Widget *child,
                                               const M3InputEvent *event,
                                               M3Bool *out_handled) {
  M3Bool visible;
  int rc;

  rc = m3_scaffold_widget_is_visible(child, &visible);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible != M3_TRUE) {
    *out_handled = M3_FALSE;
    return M3_OK;
  }

  return m3_scaffold_event_child(child, event, out_handled);
}

static int m3_scaffold_event_hit_test(const M3Rect *bounds, M3Scalar x,
                                      M3Scalar y, M3Bool *out_hit) {
  if (bounds == NULL || out_hit == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return m3_rect_contains_point(bounds, x, y, out_hit);
}

static int m3_scaffold_child_hit(M3Widget *child, const M3Rect *bounds,
                                 M3Scalar x, M3Scalar y, M3Bool *out_hit) {
  M3Bool visible;
  int rc;

  if (out_hit == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_hit = M3_FALSE;

  rc = m3_scaffold_widget_is_visible(child, &visible);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible != M3_TRUE) {
    return M3_OK;
  }

  return m3_scaffold_event_hit_test(bounds, x, y, out_hit);
}

static int m3_scaffold_widget_event(void *widget, const M3InputEvent *event,
                                    M3Bool *out_handled) {
  M3Scaffold *scaffold;
  M3Bool has_pos;
  M3Bool hit;
  M3Scalar x;
  M3Scalar y;
  int rc;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_handled = M3_FALSE;
  scaffold = (M3Scaffold *)widget;

  if (scaffold->widget.flags & M3_WIDGET_FLAG_HIDDEN) {
    return M3_OK;
  }
  if (scaffold->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    return M3_OK;
  }

  rc = m3_scaffold_event_get_position(event, &has_pos, &x, &y);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  if (has_pos == M3_FALSE) {
    if (scaffold->body != NULL) {
      return m3_scaffold_event_dispatch_to_child(scaffold->body, event,
                                                 out_handled);
    }
    return M3_OK;
  }

  rc = m3_scaffold_child_hit(scaffold->fab, &scaffold->fab_bounds, x, y, &hit);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (hit == M3_TRUE) {
    return m3_scaffold_event_child(scaffold->fab, event, out_handled);
  }

  rc = m3_scaffold_child_hit(scaffold->snackbar, &scaffold->snackbar_bounds, x,
                             y, &hit);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (hit == M3_TRUE) {
    return m3_scaffold_event_child(scaffold->snackbar, event, out_handled);
  }

  rc = m3_scaffold_child_hit(scaffold->bottom_bar, &scaffold->bottom_bar_bounds,
                             x, y, &hit);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (hit == M3_TRUE) {
    return m3_scaffold_event_child(scaffold->bottom_bar, event, out_handled);
  }

  rc = m3_scaffold_child_hit(scaffold->top_bar, &scaffold->top_bar_bounds, x, y,
                             &hit);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (hit == M3_TRUE) {
    return m3_scaffold_event_child(scaffold->top_bar, event, out_handled);
  }

  rc =
      m3_scaffold_child_hit(scaffold->body, &scaffold->body_bounds, x, y, &hit);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (hit == M3_TRUE) {
    return m3_scaffold_event_child(scaffold->body, event, out_handled);
  }

  return M3_OK; /* GCOVR_EXCL_LINE */
}

static int m3_scaffold_widget_get_semantics(void *widget,
                                            M3Semantics *out_semantics) {
  M3Scaffold *scaffold;

  if (widget == NULL || out_semantics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  scaffold = (M3Scaffold *)widget;
  out_semantics->role = M3_SEMANTIC_NONE;
  out_semantics->flags = 0u;
  if (scaffold->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
  }
  if (scaffold->widget.flags & M3_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return M3_OK;
}

static int m3_scaffold_widget_destroy(void *widget) {
  M3Scaffold *scaffold;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  scaffold = (M3Scaffold *)widget;
  scaffold->widget.ctx = NULL;
  scaffold->widget.vtable = NULL;
  scaffold->widget.handle.id = 0u;
  scaffold->widget.handle.generation = 0u;
  scaffold->widget.flags = 0u;
  memset(&scaffold->style, 0, sizeof(scaffold->style));
  memset(&scaffold->safe_area, 0, sizeof(scaffold->safe_area));
  scaffold->body = NULL;
  scaffold->top_bar = NULL;
  scaffold->bottom_bar = NULL;
  scaffold->fab = NULL;
  scaffold->snackbar = NULL;
  memset(&scaffold->bounds, 0, sizeof(scaffold->bounds));
  memset(&scaffold->body_bounds, 0, sizeof(scaffold->body_bounds));
  memset(&scaffold->top_bar_bounds, 0, sizeof(scaffold->top_bar_bounds));
  memset(&scaffold->bottom_bar_bounds, 0, sizeof(scaffold->bottom_bar_bounds));
  memset(&scaffold->fab_bounds, 0, sizeof(scaffold->fab_bounds));
  memset(&scaffold->snackbar_bounds, 0, sizeof(scaffold->snackbar_bounds));
  scaffold->fab_anim.mode = M3_ANIM_MODE_NONE;
  scaffold->fab_anim.running = M3_FALSE;
  scaffold->fab_anim.value = 0.0f;
  memset(&scaffold->fab_anim.timing, 0, sizeof(scaffold->fab_anim.timing));
  memset(&scaffold->fab_anim.spring, 0, sizeof(scaffold->fab_anim.spring));
  scaffold->fab_offset = 0.0f;
  scaffold->fab_target_offset = 0.0f;
  return M3_OK;
}

static const M3WidgetVTable g_m3_scaffold_widget_vtable = {
    m3_scaffold_widget_measure,       m3_scaffold_widget_layout,
    m3_scaffold_widget_paint,         m3_scaffold_widget_event,
    m3_scaffold_widget_get_semantics, m3_scaffold_widget_destroy};

int M3_CALL m3_scaffold_style_init(M3ScaffoldStyle *style) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_layout_edges_set(&style->padding, M3_SCAFFOLD_DEFAULT_PADDING_X,
                           M3_SCAFFOLD_DEFAULT_PADDING_Y,
                           M3_SCAFFOLD_DEFAULT_PADDING_X,
                           M3_SCAFFOLD_DEFAULT_PADDING_Y);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  style->fab_margin_x = M3_SCAFFOLD_DEFAULT_FAB_MARGIN;
  style->fab_margin_y = M3_SCAFFOLD_DEFAULT_FAB_MARGIN;
  style->snackbar_margin_x = M3_SCAFFOLD_DEFAULT_SNACKBAR_MARGIN_X;
  style->snackbar_margin_y = M3_SCAFFOLD_DEFAULT_SNACKBAR_MARGIN_Y;
  style->fab_snackbar_spacing = M3_SCAFFOLD_DEFAULT_FAB_SNACKBAR_SPACING;
  style->fab_slide_duration = M3_SCAFFOLD_DEFAULT_FAB_SLIDE_DURATION;
  style->fab_slide_easing = M3_SCAFFOLD_DEFAULT_FAB_SLIDE_EASING;
  return M3_OK;
}

int M3_CALL m3_scaffold_init(M3Scaffold *scaffold, const M3ScaffoldStyle *style,
                             M3Widget *body, M3Widget *top_bar,
                             M3Widget *bottom_bar, M3Widget *fab,
                             M3Widget *snackbar) {
  int rc;

  if (scaffold == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_scaffold_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }

  memset(scaffold, 0, sizeof(*scaffold));
  scaffold->style = *style;
  scaffold->safe_area.left = 0.0f;
  scaffold->safe_area.top = 0.0f;
  scaffold->safe_area.right = 0.0f;
  scaffold->safe_area.bottom = 0.0f;
  scaffold->body = body;
  scaffold->top_bar = top_bar;
  scaffold->bottom_bar = bottom_bar;
  scaffold->fab = fab;
  scaffold->snackbar = snackbar;
  scaffold->fab_offset = 0.0f;
  scaffold->fab_target_offset = 0.0f;

  rc = m3_anim_controller_init(&scaffold->fab_anim);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  scaffold->widget.ctx = scaffold;
  scaffold->widget.vtable = &g_m3_scaffold_widget_vtable;
  scaffold->widget.handle.id = 0u;
  scaffold->widget.handle.generation = 0u;
  scaffold->widget.flags = 0u;
  return M3_OK;
}

int M3_CALL m3_scaffold_set_style(M3Scaffold *scaffold,
                                  const M3ScaffoldStyle *style) {
  int rc;

  if (scaffold == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_scaffold_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }

  scaffold->style = *style;
  return M3_OK;
}

int M3_CALL m3_scaffold_set_body(M3Scaffold *scaffold, M3Widget *body) {
  if (scaffold == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  scaffold->body = body;
  return M3_OK;
}

int M3_CALL m3_scaffold_set_top_bar(M3Scaffold *scaffold, M3Widget *top_bar) {
  if (scaffold == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  scaffold->top_bar = top_bar;
  return M3_OK;
}

int M3_CALL m3_scaffold_set_bottom_bar(M3Scaffold *scaffold,
                                       M3Widget *bottom_bar) {
  if (scaffold == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  scaffold->bottom_bar = bottom_bar;
  return M3_OK;
}

int M3_CALL m3_scaffold_set_fab(M3Scaffold *scaffold, M3Widget *fab) {
  if (scaffold == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  scaffold->fab = fab;
  return M3_OK;
}

int M3_CALL m3_scaffold_set_snackbar(M3Scaffold *scaffold, M3Widget *snackbar) {
  if (scaffold == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  scaffold->snackbar = snackbar;
  return M3_OK;
}

int M3_CALL m3_scaffold_set_safe_area(M3Scaffold *scaffold,
                                      const M3LayoutEdges *safe_area) {
  int rc;

  if (scaffold == NULL || safe_area == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_scaffold_validate_edges(safe_area);
  if (rc != M3_OK) {
    return rc;
  }

  scaffold->safe_area = *safe_area;
  return M3_OK;
}

int M3_CALL m3_scaffold_get_safe_area(const M3Scaffold *scaffold,
                                      M3LayoutEdges *out_safe_area) {
  if (scaffold == NULL || out_safe_area == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_safe_area = scaffold->safe_area;
  return M3_OK;
}

int M3_CALL m3_scaffold_step(M3Scaffold *scaffold, M3Scalar dt,
                             M3Bool *out_changed) {
  M3Bool finished;
  M3Scalar value;
  M3Scalar prev_offset;
  M3Scalar delta;
  int rc;

  if (scaffold == NULL || out_changed == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (dt < 0.0f) {
    return M3_ERR_RANGE;
  }

  *out_changed = M3_FALSE;

  if (scaffold->fab_anim.mode == M3_ANIM_MODE_NONE) {
    return M3_OK; /* GCOVR_EXCL_LINE */
  }

  prev_offset = scaffold->fab_offset;
  rc = m3_anim_controller_step(&scaffold->fab_anim, dt, &value, &finished);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  scaffold->fab_offset = value;
  delta = value - prev_offset;
  if (delta != 0.0f) {
    scaffold->fab_bounds.y -= delta;
    if (scaffold->fab != NULL) {
      rc = m3_scaffold_layout_child(scaffold->fab, &scaffold->fab_bounds);
      if (rc != M3_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
    }
    *out_changed = M3_TRUE;
  }

  M3_UNUSED(finished);
  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_scaffold_test_validate_edges(const M3LayoutEdges *edges) {
  return m3_scaffold_validate_edges(edges);
}

int M3_CALL m3_scaffold_test_validate_style(const M3ScaffoldStyle *style) {
  return m3_scaffold_validate_style(style);
}

int M3_CALL m3_scaffold_test_validate_measure_spec(M3MeasureSpec spec) {
  return m3_scaffold_validate_measure_spec(spec);
}

int M3_CALL m3_scaffold_test_validate_rect(const M3Rect *rect) {
  return m3_scaffold_validate_rect(rect);
}

int M3_CALL m3_scaffold_test_compute_safe_bounds(const M3Rect *bounds,
                                                 const M3LayoutEdges *safe_area,
                                                 M3Rect *out_bounds) {
  return m3_scaffold_compute_safe_bounds(bounds, safe_area, out_bounds);
}

int M3_CALL m3_scaffold_test_compute_fab_target(const M3ScaffoldStyle *style,
                                                M3Scalar snackbar_height,
                                                M3Scalar *out_offset) {
  return m3_scaffold_compute_fab_target(style, snackbar_height, out_offset);
}

int M3_CALL m3_scaffold_test_widget_is_visible(const M3Widget *widget,
                                               M3Bool *out_visible) {
  return m3_scaffold_widget_is_visible(widget, out_visible);
}

int M3_CALL m3_scaffold_test_measure_child(M3Widget *child, M3MeasureSpec width,
                                           M3MeasureSpec height,
                                           M3Size *out_size) {
  return m3_scaffold_measure_child(child, width, height, out_size);
}

int M3_CALL m3_scaffold_test_layout_child(M3Widget *child,
                                          const M3Rect *bounds) {
  return m3_scaffold_layout_child(child, bounds);
}

int M3_CALL m3_scaffold_test_paint_child(M3Widget *child, M3PaintContext *ctx) {
  return m3_scaffold_paint_child(child, ctx);
}

int M3_CALL m3_scaffold_test_event_child(M3Widget *child,
                                         const M3InputEvent *event,
                                         M3Bool *out_handled) {
  return m3_scaffold_event_child(child, event, out_handled);
}

int M3_CALL m3_scaffold_test_event_get_position(const M3InputEvent *event,
                                                M3Bool *out_has_pos,
                                                M3Scalar *out_x,
                                                M3Scalar *out_y) {
  return m3_scaffold_event_get_position(event, out_has_pos, out_x, out_y);
}

int M3_CALL m3_scaffold_test_child_hit(M3Widget *child, const M3Rect *bounds,
                                       M3Scalar x, M3Scalar y,
                                       M3Bool *out_hit) {
  return m3_scaffold_child_hit(child, bounds, x, y, out_hit);
}

int M3_CALL m3_scaffold_test_apply_measure_spec(M3Scalar desired,
                                                M3MeasureSpec spec,
                                                M3Scalar *out_value) {
  return m3_scaffold_apply_measure_spec(desired, spec, out_value);
}
#endif
