#include "m3/m3_selection.h"

#include <string.h>

#ifdef CMP_TESTING
#define M3_SELECTION_TEST_FAIL_NONE 0u
#define M3_SELECTION_TEST_FAIL_CHECKBOX_RESOLVE_COLORS 1u
#define M3_SELECTION_TEST_FAIL_SWITCH_RESOLVE_COLORS 2u
#define M3_SELECTION_TEST_FAIL_RADIO_RESOLVE_COLORS 3u
#define M3_SELECTION_TEST_FAIL_CHECKBOX_CHECK_THICKNESS_NEGATIVE 4u
#define M3_SELECTION_TEST_FAIL_RADIO_DOT_RADIUS_NEGATIVE 5u
#define M3_SELECTION_TEST_FAIL_RADIO_INNER_CORNER_NEGATIVE 6u

static cmp_u32 g_m3_selection_test_fail_point = M3_SELECTION_TEST_FAIL_NONE;

static CMPBool m3_selection_test_consume_fail(cmp_u32 point) {
  if (g_m3_selection_test_fail_point != point) {
    return CMP_FALSE;
  }
  g_m3_selection_test_fail_point = M3_SELECTION_TEST_FAIL_NONE;
  return CMP_TRUE;
}

int CMP_CALL m3_selection_test_set_fail_point(cmp_u32 point) {
  g_m3_selection_test_fail_point = point;
  return CMP_OK;
}

int CMP_CALL m3_selection_test_clear_fail_points(void) {
  g_m3_selection_test_fail_point = M3_SELECTION_TEST_FAIL_NONE;
  return CMP_OK;
}
#endif /* GCOVR_EXCL_LINE */

static int m3_selection_validate_bool(CMPBool value) {
  if (value != CMP_FALSE && value != CMP_TRUE) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static int m3_selection_validate_color(CMPColor color) {
  if (!(color.r >= 0.0f && color.r <= 1.0f && color.g >= 0.0f &&
        color.g <= 1.0f && color.b >= 0.0f && color.b <= 1.0f &&
        color.a >= 0.0f && color.a <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_selection_validate_colors(M3SelectionColors colors) {
  if (m3_selection_validate_color(colors.fill) != CMP_OK ||
      m3_selection_validate_color(colors.border) != CMP_OK ||
      m3_selection_validate_color(colors.mark) != CMP_OK) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_selection_validate_switch_colors(M3SwitchColors colors) {
  if (m3_selection_validate_color(colors.track) != CMP_OK ||
      m3_selection_validate_color(colors.thumb) != CMP_OK) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_selection_validate_measure_spec(CMPMeasureSpec spec) {
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.mode != CMP_MEASURE_EXACTLY &&
      spec.mode != CMP_MEASURE_AT_MOST) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int
m3_selection_apply_measure(CMPScalar desired, CMPMeasureSpec spec,
                           CMPScalar *out_size) { /* GCOVR_EXCL_LINE */
  int rc = CMP_OK;

  rc = m3_selection_validate_measure_spec(spec);
  if (rc != CMP_OK) {
    return rc;
  }

  if (spec.mode == CMP_MEASURE_EXACTLY) {
    *out_size = spec.size;
  } else if (spec.mode == CMP_MEASURE_AT_MOST) {
    *out_size = desired;
    if (*out_size > spec.size) {
      *out_size = spec.size;
    }
  } else {
    *out_size = desired;
  }
  return CMP_OK;
}

static int m3_selection_validate_rect(CMPRect rect) {
  if (rect.width < 0.0f || rect.height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_checkbox_validate_style(const M3CheckboxStyle *style) {
  if (style->size <= 0.0f || style->corner_radius < 0.0f ||
      style->border_width < 0.0f || style->check_thickness <= 0.0f ||
      style->border_width > style->size * 0.5f ||
      style->corner_radius > style->size * 0.5f ||
      style->check_thickness > style->size) {
    return CMP_ERR_RANGE;
  }
  if (m3_selection_validate_colors(style->unchecked) != CMP_OK ||
      m3_selection_validate_colors(style->checked) != CMP_OK ||
      m3_selection_validate_colors(style->disabled_unchecked) != CMP_OK ||
      m3_selection_validate_colors(style->disabled_checked) != CMP_OK) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_switch_validate_style(const M3SwitchStyle *style) {
  CMPScalar thumb_size = 0.0f;

  if (style->track_width <= 0.0f || style->track_height <= 0.0f ||
      style->thumb_inset < 0.0f || style->track_width < style->track_height ||
      style->thumb_inset > style->track_height * 0.5f) {
    return CMP_ERR_RANGE;
  }

  thumb_size = style->track_height - style->thumb_inset * 2.0f;
  if (thumb_size <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (m3_selection_validate_switch_colors(style->off) != CMP_OK ||
      m3_selection_validate_switch_colors(style->on) != CMP_OK ||
      m3_selection_validate_switch_colors(style->disabled_off) != CMP_OK ||
      m3_selection_validate_switch_colors(style->disabled_on) != CMP_OK) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_radio_validate_style(const M3RadioStyle *style) {
  CMPScalar max_dot; /* GCOVR_EXCL_LINE */

  if (style->size <= 0.0f || style->border_width < 0.0f ||
      style->dot_radius < 0.0f || style->border_width > style->size * 0.5f) {
    return CMP_ERR_RANGE;
  }

  max_dot = style->size * 0.5f;
  if (style->dot_radius > max_dot) {
    return CMP_ERR_RANGE;
  }

  if (m3_selection_validate_colors(style->unchecked) != CMP_OK ||
      m3_selection_validate_colors(style->checked) != CMP_OK ||
      m3_selection_validate_colors(style->disabled_unchecked) != CMP_OK ||
      m3_selection_validate_colors(style->disabled_checked) != CMP_OK) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_checkbox_resolve_colors(
    const M3Checkbox *checkbox,      /* GCOVR_EXCL_LINE */
    M3SelectionColors *out_colors) { /* GCOVR_EXCL_LINE */
#ifdef CMP_TESTING
  if (m3_selection_test_consume_fail(
          M3_SELECTION_TEST_FAIL_CHECKBOX_RESOLVE_COLORS)) { /* GCOVR_EXCL_LINE
                                                              */
    return CMP_ERR_UNKNOWN;
  }
#endif
  if (checkbox->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    if (checkbox->checked == CMP_TRUE) {
      *out_colors = checkbox->style.disabled_checked;
    } else {
      *out_colors = checkbox->style.disabled_unchecked;
    }
  } else {
    if (checkbox->checked == CMP_TRUE) {
      *out_colors = checkbox->style.checked;
    } else {
      *out_colors = checkbox->style.unchecked;
    }
  }
  return CMP_OK;
}

static int
m3_switch_resolve_colors(const M3Switch *widget,
                         M3SwitchColors *out_colors) { /* GCOVR_EXCL_LINE */
#ifdef CMP_TESTING                                      /* GCOVR_EXCL_LINE */
  if (m3_selection_test_consume_fail(
          M3_SELECTION_TEST_FAIL_SWITCH_RESOLVE_COLORS)) {
    return CMP_ERR_UNKNOWN;
  }
#endif /* GCOVR_EXCL_LINE */
  if (widget->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    if (widget->on == CMP_TRUE) {
      *out_colors = widget->style.disabled_on;
    } else {
      *out_colors = widget->style.disabled_off;
    }
  } else {
    if (widget->on == CMP_TRUE) {
      *out_colors = widget->style.on;
    } else {
      *out_colors = widget->style.off;
    }
  }
  return CMP_OK;
}

static int /* GCOVR_EXCL_LINE */
cmp_radio_resolve_colors(const M3Radio *radio,
                        M3SelectionColors *out_colors) {
#ifdef CMP_TESTING
  if (m3_selection_test_consume_fail(
          M3_SELECTION_TEST_FAIL_RADIO_RESOLVE_COLORS)) {
    return CMP_ERR_UNKNOWN;
  }
#endif /* GCOVR_EXCL_LINE */
  if (radio->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    if (radio->selected == CMP_TRUE) {
      *out_colors = radio->style.disabled_checked;
    } else {
      *out_colors = radio->style.disabled_unchecked;
    }
  } else {
    if (radio->selected == CMP_TRUE) {
      *out_colors = radio->style.checked;
    } else {
      *out_colors = radio->style.unchecked;
    }
  }
  return CMP_OK;
}

static int m3_checkbox_widget_measure(void *widget, CMPMeasureSpec width,
                                      CMPMeasureSpec height, CMPSize *out_size) {
  M3Checkbox *checkbox = NULL;
  int rc = CMP_OK;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  checkbox = (M3Checkbox *)widget;
  rc = m3_checkbox_validate_style(&checkbox->style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc =
      m3_selection_apply_measure(checkbox->style.size, width, &out_size->width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_selection_apply_measure(checkbox->style.size, height,
                                  &out_size->height);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

static int m3_checkbox_widget_layout(void *widget, CMPRect bounds) {
  M3Checkbox *checkbox = NULL;
  int rc = CMP_OK;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_selection_validate_rect(bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  checkbox = (M3Checkbox *)widget;
  checkbox->bounds = bounds;
  return CMP_OK;
}

static int m3_checkbox_widget_paint(void *widget, CMPPaintContext *ctx) {
  M3Checkbox *checkbox = NULL;
  M3SelectionColors colors = {0};
  CMPRect bounds;
  CMPRect inner = {0};
  CMPScalar border;
  CMPScalar corner;
  CMPScalar inner_corner;
  CMPScalar check_thickness;
  CMPScalar x0;
  CMPScalar y0;
  CMPScalar x1;
  CMPScalar y1;
  CMPScalar x2;
  CMPScalar y2;
  int rc; /* GCOVR_EXCL_LINE */

  if (widget == NULL || ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx == NULL || ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable->draw_rect == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  checkbox = (M3Checkbox *)widget;

  rc = m3_checkbox_validate_style(&checkbox->style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_selection_validate_rect(checkbox->bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_checkbox_resolve_colors(checkbox, &colors);
  if (rc != CMP_OK) {
    return rc;
  }

  bounds = checkbox->bounds;
  border = checkbox->style.border_width;
  corner = checkbox->style.corner_radius;
  inner_corner = corner;
  inner = bounds;

  if (border > 0.0f) {
    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &bounds, colors.border,
                                     corner);
    if (rc != CMP_OK) {
      return rc;
    }

    inner.x += border;
    inner.y += border;
    inner.width -= border * 2.0f;
    inner.height -= border * 2.0f;
    if (inner.width < 0.0f || inner.height < 0.0f) {
      return CMP_ERR_RANGE;
    }

    inner_corner = corner - border;
    if (inner_corner < 0.0f) {
      inner_corner = 0.0f;
    }
  }

  rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &inner, colors.fill,
                                   inner_corner);
  if (rc != CMP_OK) {
    return rc;
  }

  if (checkbox->checked == CMP_TRUE) {
    if (ctx->gfx->vtable->draw_line == NULL) {
      return CMP_ERR_UNSUPPORTED;
    }
    check_thickness = checkbox->style.check_thickness;
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
    if (m3_selection_test_consume_fail(
            M3_SELECTION_TEST_FAIL_CHECKBOX_CHECK_THICKNESS_NEGATIVE)) {
      check_thickness = -1.0f;
    }
#endif
    if (check_thickness <= 0.0f) {
      return CMP_ERR_RANGE;
    }

    x0 = bounds.x + bounds.width * 0.25f;
    y0 = bounds.y + bounds.height * 0.55f;
    x1 = bounds.x + bounds.width * 0.45f;
    y1 = bounds.y + bounds.height * 0.72f;
    x2 = bounds.x + bounds.width * 0.75f;
    y2 = bounds.y + bounds.height * 0.32f;

    rc = ctx->gfx->vtable->draw_line(ctx->gfx->ctx, x0, y0, x1, y1, colors.mark,
                                     check_thickness);
    if (rc != CMP_OK) {
      return rc;
    }
    rc = ctx->gfx->vtable->draw_line(ctx->gfx->ctx, x1, y1, x2, y2, colors.mark,
                                     check_thickness);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  return CMP_OK;
}

static int m3_checkbox_widget_event(void *widget, const CMPInputEvent *event,
                                    CMPBool *out_handled) {
  M3Checkbox *checkbox;
  CMPBool prev_checked;
  CMPBool next_checked;
  int rc;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;
  checkbox = (M3Checkbox *)widget;

  if (checkbox->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    return CMP_OK;
  }

  rc = m3_selection_validate_bool(checkbox->checked);
  if (rc != CMP_OK) {
    return rc;
  }

  switch (event->type) {
  case CMP_INPUT_POINTER_DOWN:
    if (checkbox->pressed == CMP_TRUE) {
      return CMP_ERR_STATE;
    }
    checkbox->pressed = CMP_TRUE;
    *out_handled = CMP_TRUE;
    return CMP_OK;
  case CMP_INPUT_POINTER_UP:
    if (checkbox->pressed == CMP_FALSE) {
      return CMP_OK;
    }
    checkbox->pressed = CMP_FALSE;
    prev_checked = checkbox->checked;
    next_checked = (prev_checked == CMP_TRUE) ? CMP_FALSE : CMP_TRUE;
    checkbox->checked = next_checked;
    if (checkbox->on_change != NULL) {
      rc = checkbox->on_change(checkbox->on_change_ctx, checkbox, next_checked);
      if (rc != CMP_OK) {
        checkbox->checked = prev_checked;
        return rc;
      }
    }
    *out_handled = CMP_TRUE;
    return CMP_OK;
  default:
    return CMP_OK;
  }
}

static int m3_checkbox_widget_get_semantics(void *widget,
                                            CMPSemantics *out_semantics) {
  M3Checkbox *checkbox;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  checkbox = (M3Checkbox *)widget;
  out_semantics->role = CMP_SEMANTIC_CHECKBOX;
  out_semantics->flags = 0;
  if (checkbox->checked == CMP_TRUE) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_SELECTED;
  }
  if (checkbox->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED;
  }
  if (checkbox->widget.flags & CMP_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = checkbox->utf8_label;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_checkbox_widget_destroy(void *widget) {
  M3Checkbox *checkbox;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  checkbox = (M3Checkbox *)widget;
  memset(&checkbox->style, 0, sizeof(checkbox->style));
  checkbox->bounds.x = 0.0f;
  checkbox->bounds.y = 0.0f;
  checkbox->bounds.width = 0.0f;
  checkbox->bounds.height = 0.0f;
  checkbox->utf8_label = NULL;
  checkbox->utf8_len = 0;
  checkbox->checked = CMP_FALSE;
  checkbox->pressed = CMP_FALSE;
  checkbox->on_change = NULL;
  checkbox->on_change_ctx = NULL;
  checkbox->widget.ctx = NULL;
  checkbox->widget.vtable = NULL;
  checkbox->widget.handle.id = 0u;
  checkbox->widget.handle.generation = 0u;
  checkbox->widget.flags = 0u;
  return CMP_OK;
}

static const CMPWidgetVTable g_m3_checkbox_widget_vtable = {
    m3_checkbox_widget_measure,       m3_checkbox_widget_layout,
    m3_checkbox_widget_paint,         m3_checkbox_widget_event,
    m3_checkbox_widget_get_semantics, m3_checkbox_widget_destroy};

static int m3_switch_widget_measure(void *widget, CMPMeasureSpec width,
                                    CMPMeasureSpec height, CMPSize *out_size) {
  M3Switch *sw;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  sw = (M3Switch *)widget;
  rc = m3_switch_validate_style(&sw->style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_selection_apply_measure(sw->style.track_width, width,
                                  &out_size->width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_selection_apply_measure(sw->style.track_height, height,
                                  &out_size->height);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

static int m3_switch_widget_layout(void *widget, CMPRect bounds) {
  M3Switch *sw = NULL;
  int rc = CMP_OK;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_selection_validate_rect(bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  sw = (M3Switch *)widget;
  sw->bounds = bounds;
  return CMP_OK;
}

static int m3_switch_widget_paint(void *widget, CMPPaintContext *ctx) {
  M3Switch *sw = NULL;
  M3SwitchColors colors;
  CMPRect bounds = {0};
  CMPRect thumb;
  CMPScalar corner = 0.0f;
  CMPScalar thumb_size = 0.0f;
  int rc = CMP_OK;

  if (widget == NULL || ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx == NULL || ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable->draw_rect == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  sw = (M3Switch *)widget;

  rc = m3_switch_validate_style(&sw->style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_selection_validate_rect(sw->bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_switch_resolve_colors(sw, &colors);
  if (rc != CMP_OK) {
    return rc;
  }

  bounds = sw->bounds;
  corner = bounds.height * 0.5f;
  rc =
      ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &bounds, colors.track, corner);
  if (rc != CMP_OK) {
    return rc;
  }

  thumb_size = bounds.height - sw->style.thumb_inset * 2.0f;
  if (thumb_size <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  thumb.width = thumb_size;
  thumb.height = thumb_size;
  thumb.y = bounds.y + (bounds.height - thumb_size) * 0.5f;
  if (sw->on == CMP_TRUE) {
    thumb.x = bounds.x + bounds.width - sw->style.thumb_inset - thumb_size;
  } else {
    thumb.x = bounds.x + sw->style.thumb_inset;
  }
  corner = thumb_size * 0.5f;
  rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &thumb, colors.thumb, corner);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_switch_widget_event(void *widget, const CMPInputEvent *event,
                                  CMPBool *out_handled) { /* GCOVR_EXCL_LINE */
  M3Switch *sw = NULL;
  CMPBool prev_on = CMP_FALSE;
  CMPBool next_on = CMP_FALSE;
  int rc = CMP_OK;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;
  sw = (M3Switch *)widget;

  if (sw->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    return CMP_OK;
  }

  rc = m3_selection_validate_bool(sw->on);
  if (rc != CMP_OK) {
    return rc;
  }

  switch (event->type) {
  case CMP_INPUT_POINTER_DOWN:
    if (sw->pressed == CMP_TRUE) {
      return CMP_ERR_STATE;
    }
    sw->pressed = CMP_TRUE;
    *out_handled = CMP_TRUE;
    return CMP_OK;
  case CMP_INPUT_POINTER_UP:
    if (sw->pressed == CMP_FALSE) {
      return CMP_OK;
    }
    sw->pressed = CMP_FALSE;
    prev_on = sw->on;
    next_on = (prev_on == CMP_TRUE) ? CMP_FALSE : CMP_TRUE;
    sw->on = next_on;
    if (sw->on_change != NULL) {
      rc = sw->on_change(sw->on_change_ctx, sw, next_on);
      if (rc != CMP_OK) {
        sw->on = prev_on;
        return rc;
      }
    }
    *out_handled = CMP_TRUE;
    return CMP_OK;
  default:
    return CMP_OK;
  }
}

static int m3_switch_widget_get_semantics(void *widget,
                                          CMPSemantics *out_semantics) {
  M3Switch *sw;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  sw = (M3Switch *)widget;
  out_semantics->role = CMP_SEMANTIC_SWITCH;
  out_semantics->flags = 0;
  if (sw->on == CMP_TRUE) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_SELECTED;
  }
  if (sw->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED;
  }
  if (sw->widget.flags & CMP_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = sw->utf8_label;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_switch_widget_destroy(void *widget) {
  M3Switch *sw = NULL;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  sw = (M3Switch *)widget;
  memset(&sw->style, 0, sizeof(sw->style));
  sw->bounds.x = 0.0f;
  sw->bounds.y = 0.0f;
  sw->bounds.width = 0.0f;
  sw->bounds.height = 0.0f;
  sw->utf8_label = NULL;
  sw->utf8_len = 0;
  sw->on = CMP_FALSE;
  sw->pressed = CMP_FALSE;
  sw->on_change = NULL;
  sw->on_change_ctx = NULL;
  sw->widget.ctx = NULL;
  sw->widget.vtable = NULL;
  sw->widget.handle.id = 0u;
  sw->widget.handle.generation = 0u;
  sw->widget.flags = 0u;
  return CMP_OK;
}

static const CMPWidgetVTable g_m3_switch_widget_vtable =
    {/* GCOVR_EXCL_LINE */
     m3_switch_widget_measure,
     m3_switch_widget_layout,
     m3_switch_widget_paint,
     m3_switch_widget_event,
     m3_switch_widget_get_semantics,
     m3_switch_widget_destroy}; /* GCOVR_EXCL_LINE */

static int m3_radio_widget_measure(void *widget, CMPMeasureSpec width,
                                   CMPMeasureSpec height, CMPSize *out_size) {
  M3Radio *radio;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  radio = (M3Radio *)widget;
  rc = m3_radio_validate_style(&radio->style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_selection_apply_measure(radio->style.size, width, &out_size->width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_selection_apply_measure(radio->style.size, height, &out_size->height);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

static int m3_radio_widget_layout(void *widget, CMPRect bounds) {
  M3Radio *radio;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_selection_validate_rect(bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  radio = (M3Radio *)widget;
  radio->bounds = bounds;
  return CMP_OK;
}

static int m3_radio_widget_paint(void *widget, CMPPaintContext *ctx) {
  M3Radio *radio = NULL;
  M3SelectionColors colors;
  CMPRect bounds;
  CMPRect inner;
  CMPRect dot;
  CMPScalar border = 0.0f;
  CMPScalar corner = 0.0f;
  CMPScalar inner_corner = 0.0f;
  CMPScalar size = 0.0f;
  CMPScalar dot_radius = 0.0f;
  int rc = CMP_OK;

  if (widget == NULL || ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx == NULL || ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable->draw_rect == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  radio = (M3Radio *)widget;

  rc = m3_radio_validate_style(&radio->style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_selection_validate_rect(radio->bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_radio_resolve_colors(radio, &colors);
  if (rc != CMP_OK) {
    return rc;
  }

  bounds = radio->bounds;
  size = bounds.width;
  bounds.y += (bounds.height - size) * 0.5f;
  bounds.height = size;
  border = radio->style.border_width;
  corner = bounds.height * 0.5f;
  inner = bounds;
  inner_corner = corner;

  if (border > 0.0f) {
    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &bounds, colors.border,
                                     corner);
    if (rc != CMP_OK) {
      return rc;
    }

    inner.x += border;
    inner.y += border;
    inner.width -= border * 2.0f;
    inner.height -= border * 2.0f;
    if (inner.width < 0.0f || inner.height < 0.0f) {
      return CMP_ERR_RANGE;
    }

    inner_corner = corner - border;
#ifdef CMP_TESTING
    if (m3_selection_test_consume_fail(
            M3_SELECTION_TEST_FAIL_RADIO_INNER_CORNER_NEGATIVE)) {
      inner_corner = -1.0f;
    }
#endif
    if (inner_corner < 0.0f) {
      inner_corner = 0.0f;
    }
  }

  rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &inner, colors.fill,
                                   inner_corner);
  if (rc != CMP_OK) {
    return rc;
  }

  if (radio->selected == CMP_TRUE) {
    dot_radius = radio->style.dot_radius;
#ifdef CMP_TESTING
    if (m3_selection_test_consume_fail(
            M3_SELECTION_TEST_FAIL_RADIO_DOT_RADIUS_NEGATIVE)) {
      dot_radius = -1.0f;
    }
#endif
    if (dot_radius < 0.0f) {
      return CMP_ERR_RANGE;
    }
    if (dot_radius > 0.0f) {
      dot.width = dot_radius * 2.0f;
      dot.height = dot_radius * 2.0f;
      dot.x = bounds.x + (bounds.width - dot.width) * 0.5f;
      dot.y = bounds.y + (bounds.height - dot.height) * 0.5f;
      rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &dot, colors.mark,
                                       dot_radius);
      if (rc != CMP_OK) {
        return rc;
      }
    }
  }

  return CMP_OK;
}

static int m3_radio_widget_event(void *widget, const CMPInputEvent *event,
                                 CMPBool *out_handled) { /* GCOVR_EXCL_LINE */
  M3Radio *radio = NULL;
  int rc = CMP_OK;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;
  radio = (M3Radio *)widget;

  if (radio->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    return CMP_OK;
  }

  rc = m3_selection_validate_bool(radio->selected);
  if (rc != CMP_OK) {
    return rc;
  }

  switch (event->type) {
  case CMP_INPUT_POINTER_DOWN: /* GCOVR_EXCL_LINE */
    if (radio->pressed == CMP_TRUE) {
      return CMP_ERR_STATE;
    }
    radio->pressed = CMP_TRUE;
    *out_handled = CMP_TRUE;
    return CMP_OK;
  case CMP_INPUT_POINTER_UP:
    if (radio->pressed == CMP_FALSE) {
      return CMP_OK;
    }
    radio->pressed = CMP_FALSE;
    if (radio->selected == CMP_FALSE) {
      radio->selected = CMP_TRUE;
      if (radio->on_change != NULL) {
        rc = radio->on_change(radio->on_change_ctx, radio, CMP_TRUE);
        if (rc != CMP_OK) {
          radio->selected = CMP_FALSE;
          return rc;
        }
      }
    }
    *out_handled = CMP_TRUE;
    return CMP_OK;
  default: /* GCOVR_EXCL_LINE */
    return CMP_OK;
  }
}

static int m3_radio_widget_get_semantics(
    void *widget, CMPSemantics *out_semantics) { /* GCOVR_EXCL_LINE */
  M3Radio *radio = NULL;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  radio = (M3Radio *)widget;
  out_semantics->role = CMP_SEMANTIC_RADIO;
  out_semantics->flags = 0;
  if (radio->selected == CMP_TRUE) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_SELECTED;
  }
  if (radio->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED;
  }
  if (radio->widget.flags & CMP_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = radio->utf8_label;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_radio_widget_destroy(void *widget) {
  M3Radio *radio = NULL;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  radio = (M3Radio *)widget;
  memset(&radio->style, 0, sizeof(radio->style));
  radio->bounds.x = 0.0f;
  radio->bounds.y = 0.0f;
  radio->bounds.width = 0.0f;
  radio->bounds.height = 0.0f;
  radio->utf8_label = NULL;
  radio->utf8_len = 0;
  radio->selected = CMP_FALSE;
  radio->pressed = CMP_FALSE;
  radio->on_change = NULL;
  radio->on_change_ctx = NULL;
  radio->widget.ctx = NULL;
  radio->widget.vtable = NULL;
  radio->widget.handle.id = 0u;
  radio->widget.handle.generation = 0u;
  radio->widget.flags = 0u;
  return CMP_OK;
}

static const CMPWidgetVTable g_m3_radio_widget_vtable = { /* GCOVR_EXCL_LINE */
    m3_radio_widget_measure,       m3_radio_widget_layout, /* GCOVR_EXCL_LINE */
    m3_radio_widget_paint,         m3_radio_widget_event,
    m3_radio_widget_get_semantics, m3_radio_widget_destroy};

int CMP_CALL m3_checkbox_style_init(M3CheckboxStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->size = M3_CHECKBOX_DEFAULT_SIZE;
  style->corner_radius = M3_CHECKBOX_DEFAULT_CORNER_RADIUS;
  style->border_width = M3_CHECKBOX_DEFAULT_BORDER_WIDTH;
  style->check_thickness = M3_CHECKBOX_DEFAULT_CHECK_THICKNESS;

  style->unchecked.fill.r = 1.0f;
  style->unchecked.fill.g = 1.0f;
  style->unchecked.fill.b = 1.0f;
  style->unchecked.fill.a = 0.0f;
  style->unchecked.border.r = 0.55f;
  style->unchecked.border.g = 0.55f;
  style->unchecked.border.b = 0.55f;
  style->unchecked.border.a = 1.0f;
  style->unchecked.mark.r = 0.0f;
  style->unchecked.mark.g = 0.0f;
  style->unchecked.mark.b = 0.0f;
  style->unchecked.mark.a = 0.0f;

  style->checked.fill.r = 0.26f;
  style->checked.fill.g = 0.52f;
  style->checked.fill.b = 0.96f;
  style->checked.fill.a = 1.0f;
  style->checked.border = style->checked.fill;
  style->checked.mark.r = 1.0f;
  style->checked.mark.g = 1.0f;
  style->checked.mark.b = 1.0f;
  style->checked.mark.a = 1.0f;

  style->disabled_unchecked.fill = style->unchecked.fill;
  style->disabled_unchecked.border.r = 0.55f;
  style->disabled_unchecked.border.g = 0.55f;
  style->disabled_unchecked.border.b = 0.55f;
  style->disabled_unchecked.border.a = 0.38f;
  style->disabled_unchecked.mark = style->unchecked.mark;

  style->disabled_checked.fill = style->checked.fill;
  style->disabled_checked.fill.a = 0.38f;
  style->disabled_checked.border = style->disabled_checked.fill;
  style->disabled_checked.mark.r = 1.0f;
  style->disabled_checked.mark.g = 1.0f;
  style->disabled_checked.mark.b = 1.0f;
  style->disabled_checked.mark.a = 0.38f;

  return m3_checkbox_validate_style(style);
}

int CMP_CALL m3_checkbox_init(M3Checkbox *checkbox, const M3CheckboxStyle *style,
                             CMPBool checked) {
  int rc = CMP_OK;

  if (checkbox == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_selection_validate_bool(checked);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_checkbox_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  memset(checkbox, 0, sizeof(*checkbox));
  checkbox->style = *style;
  checkbox->checked = checked;
  checkbox->pressed = CMP_FALSE;
  checkbox->widget.ctx = checkbox;
  checkbox->widget.vtable = &g_m3_checkbox_widget_vtable;
  checkbox->widget.handle.id = 0u;
  checkbox->widget.handle.generation = 0u;
  checkbox->widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;
  return CMP_OK;
}

int CMP_CALL m3_checkbox_set_checked(M3Checkbox *checkbox, CMPBool checked) {
  int rc;

  if (checkbox == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_selection_validate_bool(checked);
  if (rc != CMP_OK) {
    return rc;
  }

  checkbox->checked = checked;
  return CMP_OK;
}

int CMP_CALL m3_checkbox_get_checked(const M3Checkbox *checkbox,
                                    CMPBool *out_checked) {
  if (checkbox == NULL || out_checked == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_checked = checkbox->checked;
  return CMP_OK;
}

int CMP_CALL m3_checkbox_set_style(M3Checkbox *checkbox,
                                  const M3CheckboxStyle *style) {
  int rc;

  if (checkbox == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_checkbox_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  checkbox->style = *style;
  return CMP_OK;
}

int CMP_CALL m3_checkbox_set_label(M3Checkbox *checkbox, const char *utf8_label,
                                  cmp_usize utf8_len) {
  if (checkbox == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8_label == NULL && utf8_len != 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  checkbox->utf8_label = utf8_label;
  checkbox->utf8_len = utf8_len;
  return CMP_OK;
}

int CMP_CALL m3_checkbox_set_on_change(M3Checkbox *checkbox,
                                      CMPCheckboxOnChange on_change, /* GCOVR_EXCL_LINE */
                                      void *ctx) {
  if (checkbox == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  checkbox->on_change = on_change;
  checkbox->on_change_ctx = ctx;
  return CMP_OK;
}

int CMP_CALL m3_switch_style_init(M3SwitchStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->track_width = M3_SWITCH_DEFAULT_TRACK_WIDTH;
  style->track_height = M3_SWITCH_DEFAULT_TRACK_HEIGHT;
  style->thumb_inset = M3_SWITCH_DEFAULT_THUMB_INSET;

  style->off.track.r = 0.75f;
  style->off.track.g = 0.75f;
  style->off.track.b = 0.75f;
  style->off.track.a = 1.0f;
  style->off.thumb.r = 1.0f;
  style->off.thumb.g = 1.0f;
  style->off.thumb.b = 1.0f;
  style->off.thumb.a = 1.0f;

  style->on.track.r = 0.26f;
  style->on.track.g = 0.52f;
  style->on.track.b = 0.96f;
  style->on.track.a = 1.0f;
  style->on.thumb.r = 1.0f;
  style->on.thumb.g = 1.0f;
  style->on.thumb.b = 1.0f;
  style->on.thumb.a = 1.0f;

  style->disabled_off.track = style->off.track;
  style->disabled_off.track.a = 0.38f;
  style->disabled_off.thumb = style->off.thumb;
  style->disabled_off.thumb.a = 0.38f;

  style->disabled_on.track = style->on.track;
  style->disabled_on.track.a = 0.38f;
  style->disabled_on.thumb = style->on.thumb;
  style->disabled_on.thumb.a = 0.38f;

  return m3_switch_validate_style(style);
}

int CMP_CALL m3_switch_init(M3Switch *widget, const M3SwitchStyle *style,
                           CMPBool on) {
  int rc;

  if (widget == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_selection_validate_bool(on);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_switch_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  memset(widget, 0, sizeof(*widget));
  widget->style = *style;
  widget->on = on;
  widget->pressed = CMP_FALSE;
  widget->widget.ctx = widget;
  widget->widget.vtable = &g_m3_switch_widget_vtable;
  widget->widget.handle.id = 0u;
  widget->widget.handle.generation = 0u;
  widget->widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;
  return CMP_OK;
}

int CMP_CALL m3_switch_set_on(M3Switch *widget, CMPBool on) {
  int rc; /* GCOVR_EXCL_LINE */

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_selection_validate_bool(on);
  if (rc != CMP_OK) {
    return rc;
  }

  widget->on = on;
  return CMP_OK;
}

int CMP_CALL m3_switch_get_on(const M3Switch *widget, CMPBool *out_on) {
  if (widget == NULL || out_on == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_on = widget->on;
  return CMP_OK;
}

int CMP_CALL m3_switch_set_style(M3Switch *widget, const M3SwitchStyle *style) {
  int rc;

  if (widget == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_switch_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  widget->style = *style;
  return CMP_OK;
}

int CMP_CALL m3_switch_set_label(M3Switch *widget, const char *utf8_label,
                                cmp_usize utf8_len) {
  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8_label == NULL && utf8_len != 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  widget->utf8_label = utf8_label;
  widget->utf8_len = utf8_len;
  return CMP_OK;
}

int CMP_CALL m3_switch_set_on_change(M3Switch *widget,
                                    CMPSwitchOnChange on_change,
                                    void *ctx) { /* GCOVR_EXCL_LINE */
  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  widget->on_change = on_change;
  widget->on_change_ctx = ctx;
  return CMP_OK;
}

int CMP_CALL m3_radio_style_init(M3RadioStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->size = M3_RADIO_DEFAULT_SIZE;
  style->border_width = M3_RADIO_DEFAULT_BORDER_WIDTH;
  style->dot_radius = M3_RADIO_DEFAULT_DOT_RADIUS;

  style->unchecked.fill.r = 1.0f;
  style->unchecked.fill.g = 1.0f;
  style->unchecked.fill.b = 1.0f;
  style->unchecked.fill.a = 0.0f;
  style->unchecked.border.r = 0.55f;
  style->unchecked.border.g = 0.55f;
  style->unchecked.border.b = 0.55f;
  style->unchecked.border.a = 1.0f;
  style->unchecked.mark.r = 0.0f;
  style->unchecked.mark.g = 0.0f;
  style->unchecked.mark.b = 0.0f;
  style->unchecked.mark.a = 0.0f;

  style->checked.fill.r = 0.26f;
  style->checked.fill.g = 0.52f;
  style->checked.fill.b = 0.96f;
  style->checked.fill.a = 1.0f;
  style->checked.border = style->checked.fill;
  style->checked.mark.r = 1.0f;
  style->checked.mark.g = 1.0f;
  style->checked.mark.b = 1.0f;
  style->checked.mark.a = 1.0f;

  style->disabled_unchecked.fill = style->unchecked.fill;
  style->disabled_unchecked.border.r = 0.55f;
  style->disabled_unchecked.border.g = 0.55f;
  style->disabled_unchecked.border.b = 0.55f;
  style->disabled_unchecked.border.a = 0.38f;
  style->disabled_unchecked.mark = style->unchecked.mark;

  style->disabled_checked.fill = style->checked.fill;
  style->disabled_checked.fill.a = 0.38f;
  style->disabled_checked.border = style->disabled_checked.fill;
  style->disabled_checked.mark.r = 1.0f;
  style->disabled_checked.mark.g = 1.0f;
  style->disabled_checked.mark.b = 1.0f;
  style->disabled_checked.mark.a = 0.38f;

  return m3_radio_validate_style(style);
}

int CMP_CALL m3_radio_init(M3Radio *radio, const M3RadioStyle *style,
                          CMPBool selected) {
  int rc;

  if (radio == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_selection_validate_bool(selected);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_radio_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  memset(radio, 0, sizeof(*radio));
  radio->style = *style;
  radio->selected = selected;
  radio->pressed = CMP_FALSE;
  radio->widget.ctx = radio;
  radio->widget.vtable = &g_m3_radio_widget_vtable;
  radio->widget.handle.id = 0u;
  radio->widget.handle.generation = 0u;
  radio->widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;
  return CMP_OK;
}

int CMP_CALL m3_radio_set_selected(M3Radio *radio, CMPBool selected) {
  int rc;

  if (radio == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_selection_validate_bool(selected);
  if (rc != CMP_OK) {
    return rc;
  }

  radio->selected = selected;
  return CMP_OK;
}

int CMP_CALL m3_radio_get_selected(const M3Radio *radio, CMPBool *out_selected) {
  if (radio == NULL || out_selected == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_selected = radio->selected;
  return CMP_OK;
}

int CMP_CALL m3_radio_set_style(M3Radio *radio, const M3RadioStyle *style) {
  int rc = CMP_OK;

  if (radio == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_radio_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  radio->style = *style;
  return CMP_OK;
}

int CMP_CALL m3_radio_set_label(M3Radio *radio, const char *utf8_label,
                               cmp_usize utf8_len) {
  if (radio == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8_label == NULL && utf8_len != 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  radio->utf8_label = utf8_label;
  radio->utf8_len = utf8_len;
  return CMP_OK;
}

int CMP_CALL m3_radio_set_on_change(M3Radio *radio, CMPRadioOnChange on_change,
                                   void *ctx) { /* GCOVR_EXCL_LINE */
  if (radio == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  radio->on_change = on_change;
  radio->on_change_ctx = ctx;
  return CMP_OK;
}
