#include "m3/m3_app_bar.h"

#include <string.h>

#ifdef CMP_TESTING
#define M3_APP_BAR_TEST_FAIL_NONE 0u
#define M3_APP_BAR_TEST_FAIL_SHADOW_INIT 1u
#define M3_APP_BAR_TEST_FAIL_TEXT_STYLE_INIT 2u
#define M3_APP_BAR_TEST_FAIL_COLOR_SET 3u
#define M3_APP_BAR_TEST_FAIL_SCROLL_INIT 4u

static cmp_u32 g_m3_app_bar_test_fail_point = M3_APP_BAR_TEST_FAIL_NONE;
static cmp_u32 g_m3_app_bar_test_color_fail_after = 0u;
static cmp_u32 g_m3_app_bar_test_match_calls = 0u;
static cmp_u32 g_m3_app_bar_test_match_fail_after = 0u;
static cmp_u32 g_m3_app_bar_test_color_error_after = 0u;
static CMPBool g_m3_app_bar_test_force_collapse_error = CMP_FALSE;

int CMP_CALL m3_app_bar_test_set_fail_point(cmp_u32 fail_point) {
  g_m3_app_bar_test_fail_point = fail_point;
  return CMP_OK;
}

int CMP_CALL m3_app_bar_test_set_color_fail_after(cmp_u32 call_count) {
  g_m3_app_bar_test_color_fail_after = call_count;
  return CMP_OK;
}

int CMP_CALL m3_app_bar_test_clear_fail_points(void) {
  g_m3_app_bar_test_fail_point = M3_APP_BAR_TEST_FAIL_NONE;
  g_m3_app_bar_test_color_fail_after = 0u;
  g_m3_app_bar_test_match_calls = 0u;
  g_m3_app_bar_test_match_fail_after = 0u;
  g_m3_app_bar_test_color_error_after = 0u;
  g_m3_app_bar_test_force_collapse_error = CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL m3_app_bar_test_set_match_fail_after(cmp_u32 call_count) {
  g_m3_app_bar_test_match_calls = 0u;
  g_m3_app_bar_test_match_fail_after = call_count;
  return CMP_OK;
}

int CMP_CALL m3_app_bar_test_set_color_error_after(cmp_u32 call_count) {
  g_m3_app_bar_test_color_error_after = call_count;
  return CMP_OK;
}

int CMP_CALL m3_app_bar_test_set_collapse_fail(CMPBool enable) {
  g_m3_app_bar_test_force_collapse_error = enable ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}

static int m3_app_bar_test_color_should_fail(CMPBool *out_fail) {
  if (out_fail == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (g_m3_app_bar_test_color_error_after > 0u) {
    g_m3_app_bar_test_color_error_after -= 1u;
    if (g_m3_app_bar_test_color_error_after == 0u) {
      return CMP_ERR_UNKNOWN;
    }
  }
  if (g_m3_app_bar_test_color_fail_after == 0u) {
    *out_fail = CMP_FALSE;
    return CMP_OK;
  }
  g_m3_app_bar_test_color_fail_after -= 1u;
  *out_fail = (g_m3_app_bar_test_color_fail_after == 0u) ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}

static int m3_app_bar_test_fail_point_match(cmp_u32 point, CMPBool *out_match) {
  if (out_match == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  g_m3_app_bar_test_match_calls += 1u;
  if (g_m3_app_bar_test_match_fail_after > 0u &&
      g_m3_app_bar_test_match_calls >= g_m3_app_bar_test_match_fail_after) {
    g_m3_app_bar_test_match_fail_after = 0u;
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (g_m3_app_bar_test_fail_point != point) {
    *out_match = CMP_FALSE;
    return CMP_OK;
  }
  g_m3_app_bar_test_fail_point = M3_APP_BAR_TEST_FAIL_NONE;
  *out_match = CMP_TRUE;
  return CMP_OK;
}
#endif

static int m3_app_bar_validate_color(const CMPColor *color) {
  if (color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!(color->r >= 0.0f && color->r <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->g >= 0.0f && color->g <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->b >= 0.0f && color->b <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->a >= 0.0f && color->a <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_app_bar_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
                                CMPScalar b, CMPScalar a) {
  int rc;
#ifdef CMP_TESTING
  CMPBool should_fail;
#endif

  if (color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!(r >= 0.0f && r <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(g >= 0.0f && g <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(b >= 0.0f && b <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(a >= 0.0f && a <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
#ifdef CMP_TESTING
  rc = m3_app_bar_test_color_should_fail(&should_fail);
  if (rc != CMP_OK) {
    return rc;
  }
  if (should_fail == CMP_TRUE) {
    return CMP_ERR_IO;
  }
#else
  rc = CMP_OK;
#endif
  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
  return rc;
}

static int m3_app_bar_validate_edges(const CMPLayoutEdges *edges) {
  if (edges == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->right < 0.0f || edges->top < 0.0f ||
      edges->bottom < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_app_bar_validate_text_style(const CMPTextStyle *style,
                                          CMPBool require_family) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (require_family == CMP_TRUE && style->utf8_family == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->size_px <= 0) {
    return CMP_ERR_RANGE;
  }
  if (style->weight < 100 || style->weight > 900) {
    return CMP_ERR_RANGE;
  }
  if (style->italic != CMP_FALSE && style->italic != CMP_TRUE) {
    return CMP_ERR_RANGE;
  }

  rc = m3_app_bar_validate_color(&style->color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_app_bar_validate_style(const M3AppBarStyle *style,
                                     CMPBool require_family) {
  CMPScalar padding_height;
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  switch (style->variant) {
  case M3_APP_BAR_VARIANT_SMALL:
  case M3_APP_BAR_VARIANT_CENTER:
  case M3_APP_BAR_VARIANT_MEDIUM:
  case M3_APP_BAR_VARIANT_LARGE:
  case M3_APP_BAR_VARIANT_BOTTOM:
    break;
  default:
    return CMP_ERR_RANGE;
  }

  if (style->collapsed_height <= 0.0f || style->expanded_height <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->expanded_height < style->collapsed_height) {
    return CMP_ERR_RANGE;
  }

  rc = m3_app_bar_validate_edges(&style->padding);
  if (rc != CMP_OK) {
    return rc;
  }

  padding_height = style->padding.top + style->padding.bottom;
  if (style->collapsed_height < padding_height ||
      style->expanded_height < padding_height) {
    return CMP_ERR_RANGE;
  }

  if (style->shadow_enabled != CMP_FALSE && style->shadow_enabled != CMP_TRUE) {
    return CMP_ERR_RANGE;
  }

  rc = m3_app_bar_validate_text_style(&style->title_style, require_family);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_app_bar_validate_color(&style->background_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_app_bar_validate_title(const char *utf8_title,
                                     cmp_usize title_len) {
  if (utf8_title == NULL && title_len != 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static int m3_app_bar_validate_backend(const CMPTextBackend *backend) {
  if (backend == NULL || backend->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static int m3_app_bar_validate_measure_spec(CMPMeasureSpec spec) {
  if (spec.mode != CMP_MEASURE_UNSPECIFIED &&
      spec.mode != CMP_MEASURE_EXACTLY && spec.mode != CMP_MEASURE_AT_MOST) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_app_bar_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_app_bar_compute_collapse_range(const M3AppBarStyle *style,
                                             CMPScalar *out_range) {
  if (style == NULL || out_range == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_m3_app_bar_test_force_collapse_error == CMP_TRUE) {
    g_m3_app_bar_test_force_collapse_error = CMP_FALSE;
    return CMP_ERR_UNKNOWN;
  }
#endif
  if (style->expanded_height < style->collapsed_height) {
    return CMP_ERR_RANGE;
  }
  *out_range = style->expanded_height - style->collapsed_height;
  return CMP_OK;
}

static int m3_app_bar_compute_current_height(const M3AppBar *bar,
                                             CMPScalar *out_height) {
  CMPScalar range;
  CMPScalar height;
  int rc;

  if (bar == NULL || out_height == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_app_bar_compute_collapse_range(&bar->style, &range);
  if (rc != CMP_OK) {
    return rc;
  }
  if (bar->collapse_offset < 0.0f || bar->collapse_offset > range) {
    return CMP_ERR_RANGE;
  }

  height = bar->style.expanded_height - bar->collapse_offset;
  if (height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  *out_height = height;
  return CMP_OK;
}

static int m3_app_bar_measure_title(const M3AppBar *bar,
                                    CMPTextMetrics *out_metrics) {
  if (bar == NULL || out_metrics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (bar->title_len == 0u) {
    out_metrics->width = 0.0f;
    out_metrics->height = 0.0f;
    out_metrics->baseline = 0.0f;
    return CMP_OK;
  }

  return cmp_text_measure_utf8(&bar->text_backend, bar->title_font,
                               bar->utf8_title, bar->title_len, out_metrics);
}

static int m3_app_bar_compute_content_bounds(const M3AppBar *bar,
                                             CMPRect *out_bounds) {
  CMPScalar height;
  CMPScalar width;
  int rc;

  if (bar == NULL || out_bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_app_bar_compute_current_height(bar, &height);
  if (rc != CMP_OK) {
    return rc;
  }

  width =
      bar->bounds.width - (bar->style.padding.left + bar->style.padding.right);
  height -= bar->style.padding.top + bar->style.padding.bottom;
  if (width < 0.0f || height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  out_bounds->x = bar->bounds.x + bar->style.padding.left;
  out_bounds->y = bar->bounds.y + bar->style.padding.top;
  out_bounds->width = width;
  out_bounds->height = height;
  return CMP_OK;
}

static int m3_app_bar_apply_scroll(M3AppBar *bar, CMPScalar delta,
                                   CMPScalar *out_consumed) {
  CMPScalar range;
  CMPScalar offset;
  CMPScalar available;
  CMPScalar consumed;
  int rc;

  if (bar == NULL || out_consumed == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_app_bar_compute_collapse_range(&bar->style, &range);
  if (rc != CMP_OK) {
    return rc;
  }

  offset = bar->collapse_offset;
  if (offset < 0.0f || offset > range) {
    return CMP_ERR_RANGE;
  }

  consumed = 0.0f;
  if (delta > 0.0f) {
    available = range - offset;
    if (delta > available) {
      delta = available;
    }
    offset += delta;
    consumed = delta;
  } else if (delta < 0.0f) {
    available = -offset;
    if (delta < available) {
      delta = available;
    }
    offset += delta;
    consumed = delta;
  }

  bar->collapse_offset = offset;
  *out_consumed = consumed;
  return CMP_OK;
}

static int m3_app_bar_compute_title_position(const M3AppBar *bar,
                                             const CMPTextMetrics *metrics,
                                             CMPScalar *out_x,
                                             CMPScalar *out_y) {
  CMPScalar content_width;
  CMPScalar collapsed_height;
  CMPScalar expanded_height;
  CMPScalar collapsed_y;
  CMPScalar expanded_y;
  CMPScalar range;
  CMPScalar t;
  int rc;

  if (bar == NULL || metrics == NULL || out_x == NULL || out_y == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  content_width =
      bar->bounds.width - (bar->style.padding.left + bar->style.padding.right);
  if (content_width < 0.0f) {
    return CMP_ERR_RANGE;
  }

  collapsed_height = bar->style.collapsed_height -
                     (bar->style.padding.top + bar->style.padding.bottom);
  expanded_height = bar->style.expanded_height -
                    (bar->style.padding.top + bar->style.padding.bottom);
  if (collapsed_height < 0.0f || expanded_height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (bar->style.variant == M3_APP_BAR_VARIANT_CENTER) {
    *out_x = bar->bounds.x + bar->style.padding.left +
             (content_width - metrics->width) * 0.5f;
  } else {
    *out_x = bar->bounds.x + bar->style.padding.left;
  }

  collapsed_y = bar->bounds.y + bar->style.padding.top +
                (collapsed_height - metrics->height) * 0.5f + metrics->baseline;
  expanded_y = bar->bounds.y + bar->style.padding.top + expanded_height -
               metrics->height + metrics->baseline;

  if (bar->style.variant == M3_APP_BAR_VARIANT_MEDIUM ||
      bar->style.variant == M3_APP_BAR_VARIANT_LARGE) {
    rc = m3_app_bar_compute_collapse_range(&bar->style, &range);
    if (rc != CMP_OK) {
      return rc;
    }
    if (range <= 0.0f) {
      *out_y = collapsed_y;
      return CMP_OK;
    }
    t = bar->collapse_offset / range;
    if (t < 0.0f) {
      t = 0.0f;
    }
    if (t > 1.0f) {
      t = 1.0f;
    }
    *out_y = expanded_y + (collapsed_y - expanded_y) * t;
    return CMP_OK;
  }

  *out_y = collapsed_y;
  return CMP_OK;
}

static int m3_app_bar_scroll_pre(void *ctx, const CMPScrollDelta *delta,
                                 CMPScrollDelta *out_consumed) {
  M3AppBar *bar;
  CMPScalar consumed_y;
  int rc;

  if (ctx == NULL || delta == NULL || out_consumed == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  bar = (M3AppBar *)ctx;
  out_consumed->x = 0.0f;
  out_consumed->y = 0.0f;

  if (delta->y <= 0.0f) {
    return CMP_OK;
  }

  rc = m3_app_bar_apply_scroll(bar, delta->y, &consumed_y);
  if (rc != CMP_OK) {
    return rc;
  }

  out_consumed->y = consumed_y;
  return CMP_OK;
}

static int m3_app_bar_scroll_post(void *ctx, const CMPScrollDelta *delta,
                                  const CMPScrollDelta *child_consumed,
                                  CMPScrollDelta *out_consumed) {
  M3AppBar *bar;
  CMPScalar consumed_y;
  int rc;

  if (ctx == NULL || delta == NULL || child_consumed == NULL ||
      out_consumed == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  CMP_UNUSED(child_consumed);
  bar = (M3AppBar *)ctx;
  out_consumed->x = 0.0f;
  out_consumed->y = 0.0f;

  if (delta->y >= 0.0f) {
    return CMP_OK;
  }

  rc = m3_app_bar_apply_scroll(bar, delta->y, &consumed_y);
  if (rc != CMP_OK) {
    return rc;
  }

  out_consumed->y = consumed_y;
  return CMP_OK;
}

static const CMPScrollParentVTable g_m3_app_bar_scroll_vtable = {
    m3_app_bar_scroll_pre, m3_app_bar_scroll_post};

static int m3_app_bar_widget_measure(void *widget, CMPMeasureSpec width,
                                     CMPMeasureSpec height, CMPSize *out_size) {
  M3AppBar *bar;
  CMPTextMetrics metrics;
  CMPScalar desired_width;
  CMPScalar desired_height;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_app_bar_validate_measure_spec(width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_app_bar_validate_measure_spec(height);
  if (rc != CMP_OK) {
    return rc;
  }

  bar = (M3AppBar *)widget;
  rc = m3_app_bar_validate_style(&bar->style, CMP_FALSE);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_app_bar_measure_title(bar, &metrics);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_app_bar_compute_current_height(bar, &desired_height);
  if (rc != CMP_OK) {
    return rc;
  }

  desired_width =
      bar->style.padding.left + bar->style.padding.right + metrics.width;

  if (width.mode == CMP_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == CMP_MEASURE_AT_MOST) {
    out_size->width = (desired_width > width.size) ? width.size : desired_width;
  } else {
    out_size->width = desired_width;
  }

  if (height.mode == CMP_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == CMP_MEASURE_AT_MOST) {
    out_size->height =
        (desired_height > height.size) ? height.size : desired_height;
  } else {
    out_size->height = desired_height;
  }

  return CMP_OK;
}

static int m3_app_bar_widget_layout(void *widget, CMPRect bounds) {
  M3AppBar *bar;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_app_bar_validate_rect(&bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  bar = (M3AppBar *)widget;
  bar->bounds = bounds;
  return CMP_OK;
}

static int m3_app_bar_widget_paint(void *widget, CMPPaintContext *ctx) {
  M3AppBar *bar;
  CMPRect rect;
  CMPTextMetrics metrics;
  CMPScalar height;
  CMPScalar text_x;
  CMPScalar text_y;
  int rc;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  bar = (M3AppBar *)widget;
  rc = m3_app_bar_validate_style(&bar->style, CMP_FALSE);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_app_bar_compute_current_height(bar, &height);
  if (rc != CMP_OK) {
    return rc;
  }

  rect = bar->bounds;
  rect.height = height;
  if (rect.width < 0.0f || rect.height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (bar->style.shadow_enabled == CMP_TRUE) {
    rc = cmp_shadow_paint(&bar->style.shadow, ctx->gfx, &rect, &ctx->clip);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  if (ctx->gfx->vtable->draw_rect == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &rect,
                                   bar->style.background_color, 0.0f);
  if (rc != CMP_OK) {
    return rc;
  }

  if (bar->title_len == 0u) {
    return CMP_OK;
  }

  if (ctx->gfx->text_vtable == NULL ||
      ctx->gfx->text_vtable->draw_text == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = m3_app_bar_measure_title(bar, &metrics);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_app_bar_compute_title_position(bar, &metrics, &text_x, &text_y);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, bar->title_font,
                                        bar->utf8_title, bar->title_len, text_x,
                                        text_y, bar->style.title_style.color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_app_bar_widget_event(void *widget, const CMPInputEvent *event,
                                   CMPBool *out_handled) {
  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;
  return CMP_OK;
}

static int m3_app_bar_widget_get_semantics(void *widget,
                                           CMPSemantics *out_semantics) {
  M3AppBar *bar;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  bar = (M3AppBar *)widget;
  out_semantics->role = CMP_SEMANTIC_NONE;
  out_semantics->flags = 0;
  if (bar->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED;
  }
  if (bar->widget.flags & CMP_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = bar->utf8_title;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_app_bar_widget_destroy(void *widget) {
  M3AppBar *bar;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  bar = (M3AppBar *)widget;
  rc = CMP_OK;
  if (bar->owns_font == CMP_TRUE &&
      (bar->title_font.id != 0u || bar->title_font.generation != 0u)) {
    rc = cmp_text_font_destroy(&bar->text_backend, bar->title_font);
  }

  bar->title_font.id = 0u;
  bar->title_font.generation = 0u;
  bar->utf8_title = NULL;
  bar->title_len = 0u;
  bar->bounds.x = 0.0f;
  bar->bounds.y = 0.0f;
  bar->bounds.width = 0.0f;
  bar->bounds.height = 0.0f;
  bar->collapse_offset = 0.0f;
  bar->text_backend.ctx = NULL;
  bar->text_backend.vtable = NULL;
  bar->widget.ctx = NULL;
  bar->widget.vtable = NULL;
  bar->scroll_parent.ctx = NULL;
  bar->scroll_parent.vtable = NULL;
  bar->owns_font = CMP_FALSE;
  memset(&bar->style, 0, sizeof(bar->style));
  return rc;
}

static const CMPWidgetVTable g_m3_app_bar_widget_vtable = {
    m3_app_bar_widget_measure,       m3_app_bar_widget_layout,
    m3_app_bar_widget_paint,         m3_app_bar_widget_event,
    m3_app_bar_widget_get_semantics, m3_app_bar_widget_destroy};

static int m3_app_bar_style_init_base(M3AppBarStyle *style, cmp_u32 variant,
                                      CMPScalar collapsed_height,
                                      CMPScalar expanded_height) {
  int rc;
#ifdef CMP_TESTING
  CMPBool matched;
#endif

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));

  rc = cmp_shadow_init(&style->shadow);
#ifdef CMP_TESTING
  if (rc == CMP_OK) {
    rc = m3_app_bar_test_fail_point_match(M3_APP_BAR_TEST_FAIL_SHADOW_INIT,
                                          &matched);
    if (rc != CMP_OK) {
      return rc;
    }
    if (matched == CMP_TRUE) {
      rc = CMP_ERR_IO;
    }
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_text_style_init(&style->title_style);
#ifdef CMP_TESTING
  if (rc == CMP_OK) {
    rc = m3_app_bar_test_fail_point_match(M3_APP_BAR_TEST_FAIL_TEXT_STYLE_INIT,
                                          &matched);
    if (rc != CMP_OK) {
      return rc;
    }
    if (matched == CMP_TRUE) {
      rc = CMP_ERR_IO;
    }
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  style->variant = variant;
  style->collapsed_height = collapsed_height;
  style->expanded_height = expanded_height;
  style->padding.left = M3_APP_BAR_DEFAULT_PADDING_X;
  style->padding.right = M3_APP_BAR_DEFAULT_PADDING_X;
  style->padding.top = M3_APP_BAR_DEFAULT_PADDING_Y;
  style->padding.bottom = M3_APP_BAR_DEFAULT_PADDING_Y;
  style->title_style.size_px = M3_APP_BAR_DEFAULT_TITLE_SIZE;
  style->title_style.weight = M3_APP_BAR_DEFAULT_TITLE_WEIGHT;
  style->title_style.color.r = 0.0f;
  style->title_style.color.g = 0.0f;
  style->title_style.color.b = 0.0f;
  style->title_style.color.a = 1.0f;
  style->shadow_enabled = CMP_FALSE;

  rc = m3_app_bar_color_set(&style->background_color, 1.0f, 1.0f, 1.0f, 1.0f);
#ifdef CMP_TESTING
  if (rc == CMP_OK) {
    rc = m3_app_bar_test_fail_point_match(M3_APP_BAR_TEST_FAIL_COLOR_SET,
                                          &matched);
    if (rc != CMP_OK) {
      return rc;
    }
    if (matched == CMP_TRUE) {
      rc = CMP_ERR_IO;
    }
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL m3_app_bar_style_init_small(M3AppBarStyle *style) {
  return m3_app_bar_style_init_base(style, M3_APP_BAR_VARIANT_SMALL,
                                    M3_APP_BAR_DEFAULT_SMALL_HEIGHT,
                                    M3_APP_BAR_DEFAULT_SMALL_HEIGHT);
}

int CMP_CALL m3_app_bar_style_init_center(M3AppBarStyle *style) {
  return m3_app_bar_style_init_base(style, M3_APP_BAR_VARIANT_CENTER,
                                    M3_APP_BAR_DEFAULT_SMALL_HEIGHT,
                                    M3_APP_BAR_DEFAULT_SMALL_HEIGHT);
}

int CMP_CALL m3_app_bar_style_init_medium(M3AppBarStyle *style) {
  return m3_app_bar_style_init_base(style, M3_APP_BAR_VARIANT_MEDIUM,
                                    M3_APP_BAR_DEFAULT_SMALL_HEIGHT,
                                    M3_APP_BAR_DEFAULT_MEDIUM_HEIGHT);
}

int CMP_CALL m3_app_bar_style_init_large(M3AppBarStyle *style) {
  return m3_app_bar_style_init_base(style, M3_APP_BAR_VARIANT_LARGE,
                                    M3_APP_BAR_DEFAULT_SMALL_HEIGHT,
                                    M3_APP_BAR_DEFAULT_LARGE_HEIGHT);
}

int CMP_CALL m3_app_bar_style_init_bottom(M3AppBarStyle *style) {
  return m3_app_bar_style_init_base(style, M3_APP_BAR_VARIANT_BOTTOM,
                                    M3_APP_BAR_DEFAULT_BOTTOM_HEIGHT,
                                    M3_APP_BAR_DEFAULT_BOTTOM_HEIGHT);
}

int CMP_CALL m3_app_bar_init(M3AppBar *bar, const CMPTextBackend *backend,
                             const M3AppBarStyle *style, const char *utf8_title,
                             cmp_usize title_len) {
  int rc;
#ifdef CMP_TESTING
  CMPBool matched;
#endif

  if (bar == NULL || backend == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_app_bar_validate_backend(backend);
  if (rc != CMP_OK) {
    return rc;
  }
  if (backend->vtable->create_font == NULL ||
      backend->vtable->destroy_font == NULL ||
      backend->vtable->measure_text == NULL ||
      backend->vtable->draw_text == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = m3_app_bar_validate_style(style, CMP_TRUE);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_app_bar_validate_title(utf8_title, title_len);
  if (rc != CMP_OK) {
    return rc;
  }

  memset(bar, 0, sizeof(*bar));
  bar->text_backend = *backend;
  bar->style = *style;
  bar->utf8_title = utf8_title;
  bar->title_len = title_len;
  bar->bounds.x = 0.0f;
  bar->bounds.y = 0.0f;
  bar->bounds.width = 0.0f;
  bar->bounds.height = 0.0f;
  bar->collapse_offset = 0.0f;

  rc = cmp_text_font_create(backend, &style->title_style, &bar->title_font);
  if (rc != CMP_OK) {
    return rc;
  }

  bar->owns_font = CMP_TRUE;
  bar->widget.ctx = bar;
  bar->widget.vtable = &g_m3_app_bar_widget_vtable;
  bar->widget.handle.id = 0u;
  bar->widget.handle.generation = 0u;
  bar->widget.flags = 0u;

  rc = cmp_scroll_parent_init(&bar->scroll_parent, bar,
                              &g_m3_app_bar_scroll_vtable);
#ifdef CMP_TESTING
  if (rc == CMP_OK) {
    rc = m3_app_bar_test_fail_point_match(M3_APP_BAR_TEST_FAIL_SCROLL_INIT,
                                          &matched);
    if (rc != CMP_OK) {
      return rc;
    }
    if (matched == CMP_TRUE) {
      rc = CMP_ERR_IO;
    }
  }
#endif
  if (rc != CMP_OK) {
    int destroy_rc;

    destroy_rc = cmp_text_font_destroy(&bar->text_backend, bar->title_font);
    if (destroy_rc != CMP_OK) {
      return destroy_rc;
    }
    bar->title_font.id = 0u;
    bar->title_font.generation = 0u;
    bar->owns_font = CMP_FALSE;
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL m3_app_bar_set_title(M3AppBar *bar, const char *utf8_title,
                                  cmp_usize title_len) {
  int rc;

  if (bar == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_app_bar_validate_title(utf8_title, title_len);
  if (rc != CMP_OK) {
    return rc;
  }

  bar->utf8_title = utf8_title;
  bar->title_len = title_len;
  return CMP_OK;
}

int CMP_CALL m3_app_bar_set_style(M3AppBar *bar, const M3AppBarStyle *style) {
  CMPHandle new_font;
  CMPScalar range;
  int rc;

  if (bar == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_app_bar_validate_style(style, CMP_TRUE);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_text_font_create(&bar->text_backend, &style->title_style, &new_font);
  if (rc != CMP_OK) {
    return rc;
  }

  if (bar->owns_font == CMP_TRUE) {
    rc = cmp_text_font_destroy(&bar->text_backend, bar->title_font);
    if (rc != CMP_OK) {
      cmp_text_font_destroy(&bar->text_backend, new_font);
      return rc;
    }
  }

  bar->style = *style;
  bar->title_font = new_font;
  bar->owns_font = CMP_TRUE;

  rc = m3_app_bar_compute_collapse_range(&bar->style, &range);
  if (rc != CMP_OK) {
    return rc;
  }
  if (bar->collapse_offset > range) {
    bar->collapse_offset = range;
  }
  if (bar->collapse_offset < 0.0f) {
    bar->collapse_offset = 0.0f;
  }

  return CMP_OK;
}

int CMP_CALL m3_app_bar_set_collapse_offset(M3AppBar *bar, CMPScalar offset) {
  CMPScalar range;
  int rc;

  if (bar == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_app_bar_compute_collapse_range(&bar->style, &range);
  if (rc != CMP_OK) {
    return rc;
  }
  if (offset < 0.0f || offset > range) {
    return CMP_ERR_RANGE;
  }

  bar->collapse_offset = offset;
  return CMP_OK;
}

int CMP_CALL m3_app_bar_get_collapse_offset(const M3AppBar *bar,
                                            CMPScalar *out_offset) {
  if (bar == NULL || out_offset == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_offset = bar->collapse_offset;
  return CMP_OK;
}

int CMP_CALL m3_app_bar_get_height(const M3AppBar *bar, CMPScalar *out_height) {
  return m3_app_bar_compute_current_height(bar, out_height);
}

int CMP_CALL m3_app_bar_get_content_bounds(const M3AppBar *bar,
                                           CMPRect *out_bounds) {
  return m3_app_bar_compute_content_bounds(bar, out_bounds);
}

#ifdef CMP_TESTING
int CMP_CALL m3_app_bar_test_validate_color(const CMPColor *color) {
  return m3_app_bar_validate_color(color);
}

int CMP_CALL m3_app_bar_test_color_set(CMPColor *color, CMPScalar r,
                                       CMPScalar g, CMPScalar b, CMPScalar a) {
  return m3_app_bar_color_set(color, r, g, b, a);
}

int CMP_CALL m3_app_bar_test_validate_edges(const CMPLayoutEdges *edges) {
  return m3_app_bar_validate_edges(edges);
}

int CMP_CALL m3_app_bar_test_validate_text_style(const CMPTextStyle *style,
                                                 CMPBool require_family) {
  return m3_app_bar_validate_text_style(style, require_family);
}

int CMP_CALL m3_app_bar_test_validate_style(const M3AppBarStyle *style,
                                            CMPBool require_family) {
  return m3_app_bar_validate_style(style, require_family);
}

int CMP_CALL m3_app_bar_test_call_color_should_fail(CMPBool *out_fail) {
  return m3_app_bar_test_color_should_fail(out_fail);
}

int CMP_CALL m3_app_bar_test_call_fail_point_match(cmp_u32 point,
                                                   CMPBool *out_match) {
  return m3_app_bar_test_fail_point_match(point, out_match);
}

int CMP_CALL m3_app_bar_test_validate_measure_spec(CMPMeasureSpec spec) {
  return m3_app_bar_validate_measure_spec(spec);
}

int CMP_CALL m3_app_bar_test_validate_rect(const CMPRect *rect) {
  return m3_app_bar_validate_rect(rect);
}

int CMP_CALL m3_app_bar_test_compute_collapse_range(const M3AppBarStyle *style,
                                                    CMPScalar *out_range) {
  return m3_app_bar_compute_collapse_range(style, out_range);
}

int CMP_CALL m3_app_bar_test_compute_current_height(const M3AppBar *bar,
                                                    CMPScalar *out_height) {
  return m3_app_bar_compute_current_height(bar, out_height);
}

int CMP_CALL m3_app_bar_test_compute_content_bounds(const M3AppBar *bar,
                                                    CMPRect *out_bounds) {
  return m3_app_bar_compute_content_bounds(bar, out_bounds);
}

int CMP_CALL m3_app_bar_test_compute_title_position(
    const M3AppBar *bar, const CMPTextMetrics *metrics, CMPScalar *out_x,
    CMPScalar *out_y) {
  return m3_app_bar_compute_title_position(bar, metrics, out_x, out_y);
}

int CMP_CALL m3_app_bar_test_measure_title(const M3AppBar *bar,
                                           CMPTextMetrics *out_metrics) {
  return m3_app_bar_measure_title(bar, out_metrics);
}

int CMP_CALL m3_app_bar_test_apply_scroll(M3AppBar *bar, CMPScalar delta,
                                          CMPScalar *out_consumed) {
  return m3_app_bar_apply_scroll(bar, delta, out_consumed);
}
#endif
