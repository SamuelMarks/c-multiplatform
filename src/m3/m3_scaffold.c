#include "m3/m3_scaffold.h"

#include <string.h>

static int m3_scaffold_validate_edges(const CMPLayoutEdges *edges) {
  if (edges == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->right < 0.0f || edges->top < 0.0f ||
      edges->bottom < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_scaffold_validate_measure_spec(CMPMeasureSpec spec) {
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.mode != CMP_MEASURE_EXACTLY &&
      spec.mode != CMP_MEASURE_AT_MOST) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_scaffold_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_scaffold_validate_style(const M3ScaffoldStyle *style) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_scaffold_validate_edges(&style->padding);
  if (rc != CMP_OK) {
    return rc;
  }

  if (style->fab_margin_x < 0.0f || style->fab_margin_y < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->snackbar_margin_x < 0.0f || style->snackbar_margin_y < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->fab_snackbar_spacing < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->fab_slide_duration < 0.0f) {
    return CMP_ERR_RANGE;
  }

  switch (style->fab_slide_easing) {
  case CMP_ANIM_EASE_LINEAR:
  case CMP_ANIM_EASE_IN:
  case CMP_ANIM_EASE_OUT:
  case CMP_ANIM_EASE_IN_OUT:
    break;
  default:
    return CMP_ERR_RANGE;
  }

  return CMP_OK;
}

static int m3_scaffold_compute_safe_bounds(const CMPRect *bounds,
                                           const CMPLayoutEdges *safe_area,
                                           CMPRect *out_bounds) {
  CMPScalar width;
  CMPScalar height;

  if (bounds == NULL || safe_area == NULL || out_bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  width = bounds->width - safe_area->left - safe_area->right;
  height = bounds->height - safe_area->top - safe_area->bottom;
  if (width < 0.0f || height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  out_bounds->x = bounds->x + safe_area->left;
  out_bounds->y = bounds->y + safe_area->top;
  out_bounds->width = width;
  out_bounds->height = height;
  return CMP_OK;
}

static int m3_scaffold_compute_fab_target(const M3ScaffoldStyle *style,
                                          CMPScalar snackbar_height,
                                          CMPScalar *out_offset) {
  CMPScalar offset;

  if (style == NULL || out_offset == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (snackbar_height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (snackbar_height <= 0.0f) {
    *out_offset = 0.0f;
    return CMP_OK;
  }

  offset = snackbar_height + style->fab_snackbar_spacing +
           style->snackbar_margin_y - style->fab_margin_y;
  if (offset < 0.0f) {
    offset = 0.0f; /* GCOVR_EXCL_LINE */
  }
  *out_offset = offset;
  return CMP_OK;
}

static int m3_scaffold_widget_is_visible(const CMPWidget *widget,
                                         CMPBool *out_visible) {
  if (out_visible == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (widget == NULL) {
    *out_visible = CMP_FALSE;
    return CMP_OK;
  }

  if (widget->flags & CMP_WIDGET_FLAG_HIDDEN) {
    *out_visible = CMP_FALSE;
    return CMP_OK;
  }

  *out_visible = CMP_TRUE;
  return CMP_OK;
}

static int m3_scaffold_measure_child(CMPWidget *child, CMPMeasureSpec width,
                                     CMPMeasureSpec height, CMPSize *out_size) {
  int rc;

  if (child == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_scaffold_validate_measure_spec(width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_scaffold_validate_measure_spec(height);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  if (child->vtable == NULL || child->vtable->measure == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  return child->vtable->measure(child->ctx, width, height, out_size);
}

static int m3_scaffold_layout_child(CMPWidget *child, const CMPRect *bounds) {
  int rc;

  if (child == NULL || bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_scaffold_validate_rect(bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  if (child->vtable == NULL || child->vtable->layout == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  return child->vtable->layout(child->ctx, *bounds);
}

static int m3_scaffold_paint_child(CMPWidget *child, CMPPaintContext *ctx) {
  if (child == NULL || ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (child->vtable == NULL || child->vtable->paint == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  return child->vtable->paint(child->ctx, ctx);
}

static int m3_scaffold_event_child(CMPWidget *child, const CMPInputEvent *event,
                                   CMPBool *out_handled) {
  CMPBool handled;

  if (out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_handled = CMP_FALSE;

  if (child == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (child->vtable == NULL || child->vtable->event == NULL) {
    return CMP_OK;
  }

  handled = CMP_FALSE;
  {
    int rc = child->vtable->event(child->ctx, event, &handled);
    if (rc != CMP_OK) {
      return rc;
    }
  }
  *out_handled = handled;
  return CMP_OK;
}

static int m3_scaffold_event_get_position(const CMPInputEvent *event,
                                          CMPBool *out_has_pos, CMPScalar *out_x,
                                          CMPScalar *out_y) {
  if (event == NULL || out_has_pos == NULL || out_x == NULL || out_y == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_has_pos = CMP_FALSE;
  *out_x = 0.0f;
  *out_y = 0.0f;

  switch (event->type) {
  case CMP_INPUT_POINTER_DOWN:
  case CMP_INPUT_POINTER_UP:
  case CMP_INPUT_POINTER_MOVE:
  case CMP_INPUT_POINTER_SCROLL:
    *out_has_pos = CMP_TRUE;
    *out_x = (CMPScalar)event->data.pointer.x;
    *out_y = (CMPScalar)event->data.pointer.y;
    return CMP_OK;
  case CMP_INPUT_GESTURE_TAP:
  case CMP_INPUT_GESTURE_DOUBLE_TAP:
  case CMP_INPUT_GESTURE_LONG_PRESS:
  case CMP_INPUT_GESTURE_DRAG_START:
  case CMP_INPUT_GESTURE_DRAG_UPDATE:
  case CMP_INPUT_GESTURE_DRAG_END:
  case CMP_INPUT_GESTURE_FLING:
    *out_has_pos = CMP_TRUE;
    *out_x = event->data.gesture.x;
    *out_y = event->data.gesture.y;
    return CMP_OK;
  default:
    return CMP_OK;
  }
}

static int m3_scaffold_apply_measure_spec(CMPScalar desired, CMPMeasureSpec spec,
                                          CMPScalar *out_value) {
  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (spec.mode == CMP_MEASURE_EXACTLY) {
    *out_value = spec.size;
    return CMP_OK;
  }
  if (spec.mode == CMP_MEASURE_AT_MOST) {
    *out_value = (desired > spec.size) ? spec.size : desired;
    return CMP_OK;
  }

  *out_value = desired;
  return CMP_OK;
}

static int m3_scaffold_widget_measure(void *widget, CMPMeasureSpec width,
                                      CMPMeasureSpec height, CMPSize *out_size) {
  M3Scaffold *scaffold;
  CMPMeasureSpec child_width;
  CMPMeasureSpec child_height;
  CMPSize top_size;
  CMPSize bottom_size;
  CMPSize body_size;
  CMPSize fab_size;
  CMPSize snackbar_size;
  CMPScalar desired_width;
  CMPScalar desired_height;
  CMPScalar safe_width_needed;
  CMPScalar safe_height_needed;
  CMPScalar body_width_needed;
  CMPScalar body_height_needed;
  CMPScalar child_width_limit;
  CMPScalar child_height_limit;
  CMPScalar snack_width_needed;
  CMPScalar fab_width_needed;
  CMPScalar snack_height_needed;
  CMPScalar fab_height_needed;
  CMPBool visible;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_scaffold_validate_measure_spec(width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_scaffold_validate_measure_spec(height);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  scaffold = (M3Scaffold *)widget;
  rc = m3_scaffold_validate_style(&scaffold->style);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_scaffold_validate_edges(&scaffold->safe_area);
  if (rc != CMP_OK) {
    return rc;
  }

  child_width_limit = 0.0f;
  if (width.mode != CMP_MEASURE_UNSPECIFIED) {
    child_width_limit =
        width.size - scaffold->safe_area.left - scaffold->safe_area.right;
    if (child_width_limit < 0.0f) {
      return CMP_ERR_RANGE;
    }
  }

  child_height_limit = 0.0f;
  if (height.mode != CMP_MEASURE_UNSPECIFIED) {
    child_height_limit =
        height.size - scaffold->safe_area.top - scaffold->safe_area.bottom;
    if (child_height_limit < 0.0f) {
      return CMP_ERR_RANGE;
    }
  }

  if (width.mode == CMP_MEASURE_UNSPECIFIED) {
    child_width.mode = CMP_MEASURE_UNSPECIFIED;
    child_width.size = 0.0f;
  } else {
    child_width.mode = width.mode;
    child_width.size = child_width_limit;
  }

  if (height.mode == CMP_MEASURE_UNSPECIFIED) {
    child_height.mode = CMP_MEASURE_UNSPECIFIED;
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
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == CMP_TRUE) {
    rc = m3_scaffold_measure_child(scaffold->top_bar, child_width, child_height,
                                   &top_size);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = m3_scaffold_widget_is_visible(scaffold->bottom_bar, &visible);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == CMP_TRUE) {
    rc = m3_scaffold_measure_child(scaffold->bottom_bar, child_width,
                                   child_height, &bottom_size);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = m3_scaffold_widget_is_visible(scaffold->body, &visible);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == CMP_TRUE) {
    rc = m3_scaffold_measure_child(scaffold->body, child_width, child_height,
                                   &body_size);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = m3_scaffold_widget_is_visible(scaffold->fab, &visible);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == CMP_TRUE) {
    rc = m3_scaffold_measure_child(scaffold->fab, child_width, child_height,
                                   &fab_size);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = m3_scaffold_widget_is_visible(scaffold->snackbar, &visible);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == CMP_TRUE) {
    rc = m3_scaffold_measure_child(scaffold->snackbar, child_width,
                                   child_height, &snackbar_size);
    if (rc != CMP_OK) {
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
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc =
      m3_scaffold_apply_measure_spec(desired_height, height, &out_size->height);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

static int m3_scaffold_widget_layout(void *widget, CMPRect bounds) {
  M3Scaffold *scaffold;
  CMPRect safe_bounds;
  CMPMeasureSpec width_spec;
  CMPMeasureSpec height_spec;
  CMPSize top_size;
  CMPSize bottom_size;
  CMPSize fab_size;
  CMPSize snackbar_size;
  CMPScalar safe_width;
  CMPScalar safe_height;
  CMPScalar available_height;
  CMPScalar body_width;
  CMPScalar body_height;
  CMPScalar snackbar_available_width;
  CMPScalar fab_available_width;
  CMPScalar fab_available_height;
  CMPScalar fab_base_x;
  CMPScalar fab_base_y;
  CMPScalar target_offset;
  CMPBool visible;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_scaffold_validate_rect(&bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  scaffold = (M3Scaffold *)widget;
  rc = m3_scaffold_validate_style(&scaffold->style);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_scaffold_validate_edges(&scaffold->safe_area);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  scaffold->bounds = bounds;

  rc = m3_scaffold_compute_safe_bounds(&bounds, &scaffold->safe_area,
                                       &safe_bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  safe_width = safe_bounds.width;
  safe_height = safe_bounds.height;

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = safe_width;
  height_spec.mode = CMP_MEASURE_AT_MOST;
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
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == CMP_TRUE) {
    rc = m3_scaffold_measure_child(scaffold->top_bar, width_spec, height_spec,
                                   &top_size);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    if (top_size.height > safe_height) {
      return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
    }
    scaffold->top_bar_bounds.x = safe_bounds.x;
    scaffold->top_bar_bounds.y = safe_bounds.y;
    scaffold->top_bar_bounds.width = safe_width;
    scaffold->top_bar_bounds.height = top_size.height;
    rc = m3_scaffold_layout_child(scaffold->top_bar, &scaffold->top_bar_bounds);
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    scaffold->top_bar_bounds.x = safe_bounds.x;
    scaffold->top_bar_bounds.y = safe_bounds.y;
    scaffold->top_bar_bounds.width = safe_width;
    scaffold->top_bar_bounds.height = 0.0f;
  }

  rc = m3_scaffold_widget_is_visible(scaffold->bottom_bar, &visible);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == CMP_TRUE) {
    rc = m3_scaffold_measure_child(scaffold->bottom_bar, width_spec,
                                   height_spec, &bottom_size);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    if (bottom_size.height > safe_height) {
      return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
    }
    scaffold->bottom_bar_bounds.x = safe_bounds.x;
    scaffold->bottom_bar_bounds.width = safe_width;
    scaffold->bottom_bar_bounds.height = bottom_size.height;
    scaffold->bottom_bar_bounds.y =
        safe_bounds.y + safe_height - bottom_size.height;
    rc = m3_scaffold_layout_child(scaffold->bottom_bar,
                                  &scaffold->bottom_bar_bounds);
    if (rc != CMP_OK) {
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
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  body_width =
      safe_width - scaffold->style.padding.left - scaffold->style.padding.right;
  body_height = available_height - scaffold->style.padding.top -
                scaffold->style.padding.bottom;
  if (body_width < 0.0f || body_height < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  scaffold->body_bounds.x = safe_bounds.x + scaffold->style.padding.left;
  scaffold->body_bounds.y = safe_bounds.y + scaffold->top_bar_bounds.height +
                            scaffold->style.padding.top;
  scaffold->body_bounds.width = body_width;
  scaffold->body_bounds.height = body_height;

  rc = m3_scaffold_widget_is_visible(scaffold->body, &visible);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == CMP_TRUE) {
    rc = m3_scaffold_layout_child(scaffold->body, &scaffold->body_bounds);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = m3_scaffold_widget_is_visible(scaffold->snackbar, &visible);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == CMP_TRUE) {
    snackbar_available_width =
        safe_width - scaffold->style.snackbar_margin_x * 2.0f;
    if (snackbar_available_width < 0.0f) {
      return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
    }

    width_spec.mode = CMP_MEASURE_AT_MOST;
    width_spec.size = snackbar_available_width;
    height_spec.mode = CMP_MEASURE_AT_MOST;
    height_spec.size = safe_height;

    rc = m3_scaffold_measure_child(scaffold->snackbar, width_spec, height_spec,
                                   &snackbar_size);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    if (snackbar_size.width > snackbar_available_width ||
        snackbar_size.height > safe_height) {
      return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
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
      return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
    }

    rc = m3_scaffold_layout_child(scaffold->snackbar,
                                  &scaffold->snackbar_bounds);
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    scaffold->snackbar_bounds.x = safe_bounds.x; /* GCOVR_EXCL_LINE */
    scaffold->snackbar_bounds.y = safe_bounds.y; /* GCOVR_EXCL_LINE */
    scaffold->snackbar_bounds.width = 0.0f;      /* GCOVR_EXCL_LINE */
    scaffold->snackbar_bounds.height = 0.0f;     /* GCOVR_EXCL_LINE */
  }

  rc = m3_scaffold_widget_is_visible(scaffold->fab, &visible);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == CMP_TRUE) {
    fab_available_width = safe_width - scaffold->style.fab_margin_x * 2.0f;
    fab_available_height = safe_height - scaffold->bottom_bar_bounds.height -
                           scaffold->style.fab_margin_y * 2.0f;
    if (fab_available_width < 0.0f || fab_available_height < 0.0f) {
      return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
    }

    width_spec.mode = CMP_MEASURE_AT_MOST;
    width_spec.size = fab_available_width;
    height_spec.mode = CMP_MEASURE_AT_MOST;
    height_spec.size = fab_available_height;

    rc = m3_scaffold_measure_child(scaffold->fab, width_spec, height_spec,
                                   &fab_size);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    if (fab_size.width > fab_available_width ||
        fab_size.height > fab_available_height) {
      return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
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
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
    }

    if (target_offset != scaffold->fab_target_offset) {
      rc = cmp_anim_controller_start_timing(
          &scaffold->fab_anim, scaffold->fab_offset, target_offset,
          scaffold->style.fab_slide_duration, scaffold->style.fab_slide_easing);
      if (rc != CMP_OK) {
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
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    scaffold->fab_bounds.x = safe_bounds.x; /* GCOVR_EXCL_LINE */
    scaffold->fab_bounds.y = safe_bounds.y; /* GCOVR_EXCL_LINE */
    scaffold->fab_bounds.width = 0.0f;      /* GCOVR_EXCL_LINE */
    scaffold->fab_bounds.height = 0.0f;     /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

static int m3_scaffold_widget_paint(void *widget, CMPPaintContext *ctx) {
  M3Scaffold *scaffold;
  CMPBool visible;
  int rc;

  if (widget == NULL || ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  scaffold = (M3Scaffold *)widget;

  rc = m3_scaffold_widget_is_visible(scaffold->body, &visible);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == CMP_TRUE) {
    rc = m3_scaffold_paint_child(scaffold->body, ctx);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = m3_scaffold_widget_is_visible(scaffold->top_bar, &visible);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == CMP_TRUE) {
    rc = m3_scaffold_paint_child(scaffold->top_bar, ctx);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = m3_scaffold_widget_is_visible(scaffold->bottom_bar, &visible);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == CMP_TRUE) {
    rc = m3_scaffold_paint_child(scaffold->bottom_bar, ctx);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = m3_scaffold_widget_is_visible(scaffold->snackbar, &visible);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == CMP_TRUE) {
    rc = m3_scaffold_paint_child(scaffold->snackbar, ctx);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = m3_scaffold_widget_is_visible(scaffold->fab, &visible);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible == CMP_TRUE) {
    rc = m3_scaffold_paint_child(scaffold->fab, ctx);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  return CMP_OK;
}

static int m3_scaffold_event_dispatch_to_child(CMPWidget *child,
                                               const CMPInputEvent *event,
                                               CMPBool *out_handled) {
  CMPBool visible;
  int rc;

  rc = m3_scaffold_widget_is_visible(child, &visible);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible != CMP_TRUE) {
    *out_handled = CMP_FALSE;
    return CMP_OK;
  }

  return m3_scaffold_event_child(child, event, out_handled);
}

static int m3_scaffold_event_hit_test(const CMPRect *bounds, CMPScalar x,
                                      CMPScalar y, CMPBool *out_hit) {
  if (bounds == NULL || out_hit == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return cmp_rect_contains_point(bounds, x, y, out_hit);
}

static int m3_scaffold_child_hit(CMPWidget *child, const CMPRect *bounds,
                                 CMPScalar x, CMPScalar y, CMPBool *out_hit) {
  CMPBool visible;
  int rc;

  if (out_hit == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_hit = CMP_FALSE;

  rc = m3_scaffold_widget_is_visible(child, &visible);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (visible != CMP_TRUE) {
    return CMP_OK;
  }

  return m3_scaffold_event_hit_test(bounds, x, y, out_hit);
}

static int m3_scaffold_widget_event(void *widget, const CMPInputEvent *event,
                                    CMPBool *out_handled) {
  M3Scaffold *scaffold;
  CMPBool has_pos;
  CMPBool hit;
  CMPScalar x;
  CMPScalar y;
  int rc;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;
  scaffold = (M3Scaffold *)widget;

  if (scaffold->widget.flags & CMP_WIDGET_FLAG_HIDDEN) {
    return CMP_OK;
  }
  if (scaffold->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    return CMP_OK;
  }

  rc = m3_scaffold_event_get_position(event, &has_pos, &x, &y);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  if (has_pos == CMP_FALSE) {
    if (scaffold->body != NULL) {
      return m3_scaffold_event_dispatch_to_child(scaffold->body, event,
                                                 out_handled);
    }
    return CMP_OK;
  }

  rc = m3_scaffold_child_hit(scaffold->fab, &scaffold->fab_bounds, x, y, &hit);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (hit == CMP_TRUE) {
    return m3_scaffold_event_child(scaffold->fab, event, out_handled);
  }

  rc = m3_scaffold_child_hit(scaffold->snackbar, &scaffold->snackbar_bounds, x,
                             y, &hit);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (hit == CMP_TRUE) {
    return m3_scaffold_event_child(scaffold->snackbar, event, out_handled);
  }

  rc = m3_scaffold_child_hit(scaffold->bottom_bar, &scaffold->bottom_bar_bounds,
                             x, y, &hit);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (hit == CMP_TRUE) {
    return m3_scaffold_event_child(scaffold->bottom_bar, event, out_handled);
  }

  rc = m3_scaffold_child_hit(scaffold->top_bar, &scaffold->top_bar_bounds, x, y,
                             &hit);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (hit == CMP_TRUE) {
    return m3_scaffold_event_child(scaffold->top_bar, event, out_handled);
  }

  rc =
      m3_scaffold_child_hit(scaffold->body, &scaffold->body_bounds, x, y, &hit);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (hit == CMP_TRUE) {
    return m3_scaffold_event_child(scaffold->body, event, out_handled);
  }

  return CMP_OK; /* GCOVR_EXCL_LINE */
}

static int m3_scaffold_widget_get_semantics(void *widget,
                                            CMPSemantics *out_semantics) {
  M3Scaffold *scaffold;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  scaffold = (M3Scaffold *)widget;
  out_semantics->role = CMP_SEMANTIC_NONE;
  out_semantics->flags = 0u;
  if (scaffold->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED;
  }
  if (scaffold->widget.flags & CMP_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_scaffold_widget_destroy(void *widget) {
  M3Scaffold *scaffold;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
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
  scaffold->fab_anim.mode = CMP_ANIM_MODE_NONE;
  scaffold->fab_anim.running = CMP_FALSE;
  scaffold->fab_anim.value = 0.0f;
  memset(&scaffold->fab_anim.timing, 0, sizeof(scaffold->fab_anim.timing));
  memset(&scaffold->fab_anim.spring, 0, sizeof(scaffold->fab_anim.spring));
  scaffold->fab_offset = 0.0f;
  scaffold->fab_target_offset = 0.0f;
  return CMP_OK;
}

static const CMPWidgetVTable g_m3_scaffold_widget_vtable = {
    m3_scaffold_widget_measure,       m3_scaffold_widget_layout,
    m3_scaffold_widget_paint,         m3_scaffold_widget_event,
    m3_scaffold_widget_get_semantics, m3_scaffold_widget_destroy};

int CMP_CALL m3_scaffold_style_init(M3ScaffoldStyle *style) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_layout_edges_set(&style->padding, M3_SCAFFOLD_DEFAULT_PADDING_X,
                           M3_SCAFFOLD_DEFAULT_PADDING_Y,
                           M3_SCAFFOLD_DEFAULT_PADDING_X,
                           M3_SCAFFOLD_DEFAULT_PADDING_Y);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  style->fab_margin_x = M3_SCAFFOLD_DEFAULT_FAB_MARGIN;
  style->fab_margin_y = M3_SCAFFOLD_DEFAULT_FAB_MARGIN;
  style->snackbar_margin_x = M3_SCAFFOLD_DEFAULT_SNACKBAR_MARGIN_X;
  style->snackbar_margin_y = M3_SCAFFOLD_DEFAULT_SNACKBAR_MARGIN_Y;
  style->fab_snackbar_spacing = M3_SCAFFOLD_DEFAULT_FAB_SNACKBAR_SPACING;
  style->fab_slide_duration = M3_SCAFFOLD_DEFAULT_FAB_SLIDE_DURATION;
  style->fab_slide_easing = M3_SCAFFOLD_DEFAULT_FAB_SLIDE_EASING;
  return CMP_OK;
}

int CMP_CALL m3_scaffold_init(M3Scaffold *scaffold, const M3ScaffoldStyle *style,
                             CMPWidget *body, CMPWidget *top_bar,
                             CMPWidget *bottom_bar, CMPWidget *fab,
                             CMPWidget *snackbar) {
  int rc;

  if (scaffold == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_scaffold_validate_style(style);
  if (rc != CMP_OK) {
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

  rc = cmp_anim_controller_init(&scaffold->fab_anim);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  scaffold->widget.ctx = scaffold;
  scaffold->widget.vtable = &g_m3_scaffold_widget_vtable;
  scaffold->widget.handle.id = 0u;
  scaffold->widget.handle.generation = 0u;
  scaffold->widget.flags = 0u;
  return CMP_OK;
}

int CMP_CALL m3_scaffold_set_style(M3Scaffold *scaffold,
                                  const M3ScaffoldStyle *style) {
  int rc;

  if (scaffold == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_scaffold_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  scaffold->style = *style;
  return CMP_OK;
}

int CMP_CALL m3_scaffold_set_body(M3Scaffold *scaffold, CMPWidget *body) {
  if (scaffold == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  scaffold->body = body;
  return CMP_OK;
}

int CMP_CALL m3_scaffold_set_top_bar(M3Scaffold *scaffold, CMPWidget *top_bar) {
  if (scaffold == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  scaffold->top_bar = top_bar;
  return CMP_OK;
}

int CMP_CALL m3_scaffold_set_bottom_bar(M3Scaffold *scaffold,
                                       CMPWidget *bottom_bar) {
  if (scaffold == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  scaffold->bottom_bar = bottom_bar;
  return CMP_OK;
}

int CMP_CALL m3_scaffold_set_fab(M3Scaffold *scaffold, CMPWidget *fab) {
  if (scaffold == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  scaffold->fab = fab;
  return CMP_OK;
}

int CMP_CALL m3_scaffold_set_snackbar(M3Scaffold *scaffold, CMPWidget *snackbar) {
  if (scaffold == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  scaffold->snackbar = snackbar;
  return CMP_OK;
}

int CMP_CALL m3_scaffold_set_safe_area(M3Scaffold *scaffold,
                                      const CMPLayoutEdges *safe_area) {
  int rc;

  if (scaffold == NULL || safe_area == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_scaffold_validate_edges(safe_area);
  if (rc != CMP_OK) {
    return rc;
  }

  scaffold->safe_area = *safe_area;
  return CMP_OK;
}

int CMP_CALL m3_scaffold_get_safe_area(const M3Scaffold *scaffold,
                                      CMPLayoutEdges *out_safe_area) {
  if (scaffold == NULL || out_safe_area == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_safe_area = scaffold->safe_area;
  return CMP_OK;
}

int CMP_CALL m3_scaffold_step(M3Scaffold *scaffold, CMPScalar dt,
                             CMPBool *out_changed) {
  CMPBool finished;
  CMPScalar value;
  CMPScalar prev_offset;
  CMPScalar delta;
  int rc;

  if (scaffold == NULL || out_changed == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (dt < 0.0f) {
    return CMP_ERR_RANGE;
  }

  *out_changed = CMP_FALSE;

  if (scaffold->fab_anim.mode == CMP_ANIM_MODE_NONE) {
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  prev_offset = scaffold->fab_offset;
  rc = cmp_anim_controller_step(&scaffold->fab_anim, dt, &value, &finished);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  scaffold->fab_offset = value;
  delta = value - prev_offset;
  if (delta != 0.0f) {
    scaffold->fab_bounds.y -= delta;
    if (scaffold->fab != NULL) {
      rc = m3_scaffold_layout_child(scaffold->fab, &scaffold->fab_bounds);
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
    }
    *out_changed = CMP_TRUE;
  }

  CMP_UNUSED(finished);
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL m3_scaffold_test_validate_edges(const CMPLayoutEdges *edges) {
  return m3_scaffold_validate_edges(edges);
}

int CMP_CALL m3_scaffold_test_validate_style(const M3ScaffoldStyle *style) {
  return m3_scaffold_validate_style(style);
}

int CMP_CALL m3_scaffold_test_validate_measure_spec(CMPMeasureSpec spec) {
  return m3_scaffold_validate_measure_spec(spec);
}

int CMP_CALL m3_scaffold_test_validate_rect(const CMPRect *rect) {
  return m3_scaffold_validate_rect(rect);
}

int CMP_CALL m3_scaffold_test_compute_safe_bounds(const CMPRect *bounds,
                                                 const CMPLayoutEdges *safe_area,
                                                 CMPRect *out_bounds) {
  return m3_scaffold_compute_safe_bounds(bounds, safe_area, out_bounds);
}

int CMP_CALL m3_scaffold_test_compute_fab_target(const M3ScaffoldStyle *style,
                                                CMPScalar snackbar_height,
                                                CMPScalar *out_offset) {
  return m3_scaffold_compute_fab_target(style, snackbar_height, out_offset);
}

int CMP_CALL m3_scaffold_test_widget_is_visible(const CMPWidget *widget,
                                               CMPBool *out_visible) {
  return m3_scaffold_widget_is_visible(widget, out_visible);
}

int CMP_CALL m3_scaffold_test_measure_child(CMPWidget *child, CMPMeasureSpec width,
                                           CMPMeasureSpec height,
                                           CMPSize *out_size) {
  return m3_scaffold_measure_child(child, width, height, out_size);
}

int CMP_CALL m3_scaffold_test_layout_child(CMPWidget *child,
                                          const CMPRect *bounds) {
  return m3_scaffold_layout_child(child, bounds);
}

int CMP_CALL m3_scaffold_test_paint_child(CMPWidget *child, CMPPaintContext *ctx) {
  return m3_scaffold_paint_child(child, ctx);
}

int CMP_CALL m3_scaffold_test_event_child(CMPWidget *child,
                                         const CMPInputEvent *event,
                                         CMPBool *out_handled) {
  return m3_scaffold_event_child(child, event, out_handled);
}

int CMP_CALL m3_scaffold_test_event_get_position(const CMPInputEvent *event,
                                                CMPBool *out_has_pos,
                                                CMPScalar *out_x,
                                                CMPScalar *out_y) {
  return m3_scaffold_event_get_position(event, out_has_pos, out_x, out_y);
}

int CMP_CALL m3_scaffold_test_child_hit(CMPWidget *child, const CMPRect *bounds,
                                       CMPScalar x, CMPScalar y,
                                       CMPBool *out_hit) {
  return m3_scaffold_child_hit(child, bounds, x, y, out_hit);
}

int CMP_CALL m3_scaffold_test_apply_measure_spec(CMPScalar desired,
                                                CMPMeasureSpec spec,
                                                CMPScalar *out_value) {
  return m3_scaffold_apply_measure_spec(desired, spec, out_value);
}
#endif
