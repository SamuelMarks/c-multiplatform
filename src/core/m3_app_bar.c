#include "m3/m3_app_bar.h"

#include <string.h>

#ifdef M3_TESTING
#define M3_APP_BAR_TEST_FAIL_NONE 0u
#define M3_APP_BAR_TEST_FAIL_SHADOW_INIT 1u
#define M3_APP_BAR_TEST_FAIL_TEXT_STYLE_INIT 2u
#define M3_APP_BAR_TEST_FAIL_COLOR_SET 3u
#define M3_APP_BAR_TEST_FAIL_SCROLL_INIT 4u

static m3_u32 g_m3_app_bar_test_fail_point = M3_APP_BAR_TEST_FAIL_NONE;
static m3_u32 g_m3_app_bar_test_color_fail_after = 0u;
static m3_u32 g_m3_app_bar_test_match_calls = 0u;
static m3_u32 g_m3_app_bar_test_match_fail_after = 0u;
static m3_u32 g_m3_app_bar_test_color_error_after = 0u;
static M3Bool g_m3_app_bar_test_force_collapse_error = M3_FALSE;

int M3_CALL m3_app_bar_test_set_fail_point(m3_u32 fail_point) {
  g_m3_app_bar_test_fail_point = fail_point;
  return M3_OK;
}

int M3_CALL m3_app_bar_test_set_color_fail_after(m3_u32 call_count) {
  g_m3_app_bar_test_color_fail_after = call_count;
  return M3_OK;
}

int M3_CALL m3_app_bar_test_clear_fail_points(void) {
  g_m3_app_bar_test_fail_point = M3_APP_BAR_TEST_FAIL_NONE;
  g_m3_app_bar_test_color_fail_after = 0u;
  g_m3_app_bar_test_match_calls = 0u;
  g_m3_app_bar_test_match_fail_after = 0u;
  g_m3_app_bar_test_color_error_after = 0u;
  g_m3_app_bar_test_force_collapse_error = M3_FALSE;
  return M3_OK;
}

int M3_CALL m3_app_bar_test_set_match_fail_after(m3_u32 call_count) {
  g_m3_app_bar_test_match_calls = 0u;
  g_m3_app_bar_test_match_fail_after = call_count;
  return M3_OK;
}

int M3_CALL m3_app_bar_test_set_color_error_after(m3_u32 call_count) {
  g_m3_app_bar_test_color_error_after = call_count;
  return M3_OK;
}

int M3_CALL m3_app_bar_test_set_collapse_fail(M3Bool enable) {
  g_m3_app_bar_test_force_collapse_error = enable ? M3_TRUE : M3_FALSE;
  return M3_OK;
}

static int m3_app_bar_test_color_should_fail(M3Bool *out_fail) {
  if (out_fail == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (g_m3_app_bar_test_color_error_after > 0u) {
    g_m3_app_bar_test_color_error_after -= 1u;
    if (g_m3_app_bar_test_color_error_after == 0u) {
      return M3_ERR_UNKNOWN;
    }
  }
  if (g_m3_app_bar_test_color_fail_after == 0u) {
    *out_fail = M3_FALSE;
    return M3_OK;
  }
  g_m3_app_bar_test_color_fail_after -= 1u;
  *out_fail = (g_m3_app_bar_test_color_fail_after == 0u) ? M3_TRUE : M3_FALSE;
  return M3_OK;
}

static int m3_app_bar_test_fail_point_match(m3_u32 point, M3Bool *out_match) {
  if (out_match == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  g_m3_app_bar_test_match_calls += 1u;
  if (g_m3_app_bar_test_match_fail_after > 0u &&
      g_m3_app_bar_test_match_calls >= g_m3_app_bar_test_match_fail_after) {
    g_m3_app_bar_test_match_fail_after = 0u;
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (g_m3_app_bar_test_fail_point != point) {
    *out_match = M3_FALSE;
    return M3_OK;
  }
  g_m3_app_bar_test_fail_point = M3_APP_BAR_TEST_FAIL_NONE;
  *out_match = M3_TRUE;
  return M3_OK;
}
#endif

static int m3_app_bar_validate_color(const M3Color *color) {
  if (color == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!(color->r >= 0.0f && color->r <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->g >= 0.0f && color->g <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->b >= 0.0f && color->b <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->a >= 0.0f && color->a <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_app_bar_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                                M3Scalar b, M3Scalar a) {
  int rc;
#ifdef M3_TESTING
  M3Bool should_fail;
#endif

  if (color == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!(r >= 0.0f && r <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(g >= 0.0f && g <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(b >= 0.0f && b <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(a >= 0.0f && a <= 1.0f)) {
    return M3_ERR_RANGE;
  }
#ifdef M3_TESTING
  rc = m3_app_bar_test_color_should_fail(&should_fail);
  if (rc != M3_OK) {
    return rc;
  }
  if (should_fail == M3_TRUE) {
    return M3_ERR_IO;
  }
#else
  rc = M3_OK;
#endif
  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
  return rc;
}

static int m3_app_bar_validate_edges(const M3LayoutEdges *edges) {
  if (edges == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->right < 0.0f || edges->top < 0.0f ||
      edges->bottom < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_app_bar_validate_text_style(const M3TextStyle *style,
                                          M3Bool require_family) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (require_family == M3_TRUE && style->utf8_family == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (style->size_px <= 0) {
    return M3_ERR_RANGE;
  }
  if (style->weight < 100 || style->weight > 900) {
    return M3_ERR_RANGE;
  }
  if (style->italic != M3_FALSE && style->italic != M3_TRUE) {
    return M3_ERR_RANGE;
  }

  rc = m3_app_bar_validate_color(&style->color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_app_bar_validate_style(const M3AppBarStyle *style,
                                     M3Bool require_family) {
  M3Scalar padding_height;
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  switch (style->variant) {
  case M3_APP_BAR_VARIANT_SMALL:
  case M3_APP_BAR_VARIANT_CENTER:
  case M3_APP_BAR_VARIANT_MEDIUM:
  case M3_APP_BAR_VARIANT_LARGE:
    break;
  default:
    return M3_ERR_RANGE;
  }

  if (style->collapsed_height <= 0.0f || style->expanded_height <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->expanded_height < style->collapsed_height) {
    return M3_ERR_RANGE;
  }

  rc = m3_app_bar_validate_edges(&style->padding);
  if (rc != M3_OK) {
    return rc;
  }

  padding_height = style->padding.top + style->padding.bottom;
  if (style->collapsed_height < padding_height ||
      style->expanded_height < padding_height) {
    return M3_ERR_RANGE;
  }

  if (style->shadow_enabled != M3_FALSE && style->shadow_enabled != M3_TRUE) {
    return M3_ERR_RANGE;
  }

  rc = m3_app_bar_validate_text_style(&style->title_style, require_family);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_app_bar_validate_color(&style->background_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_app_bar_validate_title(const char *utf8_title,
                                     m3_usize title_len) {
  if (utf8_title == NULL && title_len != 0u) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_OK;
}

static int m3_app_bar_validate_backend(const M3TextBackend *backend) {
  if (backend == NULL || backend->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_OK;
}

static int m3_app_bar_validate_measure_spec(M3MeasureSpec spec) {
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.mode != M3_MEASURE_EXACTLY &&
      spec.mode != M3_MEASURE_AT_MOST) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_app_bar_validate_rect(const M3Rect *rect) {
  if (rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_app_bar_compute_collapse_range(const M3AppBarStyle *style,
                                             M3Scalar *out_range) {
  if (style == NULL || out_range == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#ifdef M3_TESTING
  if (g_m3_app_bar_test_force_collapse_error == M3_TRUE) {
    g_m3_app_bar_test_force_collapse_error = M3_FALSE;
    return M3_ERR_UNKNOWN;
  }
#endif
  if (style->expanded_height < style->collapsed_height) {
    return M3_ERR_RANGE;
  }
  *out_range = style->expanded_height - style->collapsed_height;
  return M3_OK;
}

static int m3_app_bar_compute_current_height(const M3AppBar *bar,
                                             M3Scalar *out_height) {
  M3Scalar range;
  M3Scalar height;
  int rc;

  if (bar == NULL || out_height == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_app_bar_compute_collapse_range(&bar->style, &range);
  if (rc != M3_OK) {
    return rc;
  }
  if (bar->collapse_offset < 0.0f || bar->collapse_offset > range) {
    return M3_ERR_RANGE;
  }

  height = bar->style.expanded_height - bar->collapse_offset;
  if (height < 0.0f) {
    return M3_ERR_RANGE;
  }

  *out_height = height;
  return M3_OK;
}

static int m3_app_bar_measure_title(const M3AppBar *bar,
                                    M3TextMetrics *out_metrics) {
  if (bar == NULL || out_metrics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (bar->title_len == 0u) {
    out_metrics->width = 0.0f;
    out_metrics->height = 0.0f;
    out_metrics->baseline = 0.0f;
    return M3_OK;
  }

  return m3_text_measure_utf8(&bar->text_backend, bar->title_font,
                              bar->utf8_title, bar->title_len, out_metrics);
}

static int m3_app_bar_compute_content_bounds(const M3AppBar *bar,
                                             M3Rect *out_bounds) {
  M3Scalar height;
  M3Scalar width;
  int rc;

  if (bar == NULL || out_bounds == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_app_bar_compute_current_height(bar, &height);
  if (rc != M3_OK) {
    return rc;
  }

  width =
      bar->bounds.width - (bar->style.padding.left + bar->style.padding.right);
  height -= bar->style.padding.top + bar->style.padding.bottom;
  if (width < 0.0f || height < 0.0f) {
    return M3_ERR_RANGE;
  }

  out_bounds->x = bar->bounds.x + bar->style.padding.left;
  out_bounds->y = bar->bounds.y + bar->style.padding.top;
  out_bounds->width = width;
  out_bounds->height = height;
  return M3_OK;
}

static int m3_app_bar_apply_scroll(M3AppBar *bar, M3Scalar delta,
                                   M3Scalar *out_consumed) {
  M3Scalar range;
  M3Scalar offset;
  M3Scalar available;
  M3Scalar consumed;
  int rc;

  if (bar == NULL || out_consumed == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_app_bar_compute_collapse_range(&bar->style, &range);
  if (rc != M3_OK) {
    return rc;
  }

  offset = bar->collapse_offset;
  if (offset < 0.0f || offset > range) {
    return M3_ERR_RANGE;
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
  return M3_OK;
}

static int m3_app_bar_compute_title_position(const M3AppBar *bar,
                                             const M3TextMetrics *metrics,
                                             M3Scalar *out_x, M3Scalar *out_y) {
  M3Scalar content_width;
  M3Scalar collapsed_height;
  M3Scalar expanded_height;
  M3Scalar collapsed_y;
  M3Scalar expanded_y;
  M3Scalar range;
  M3Scalar t;
  int rc;

  if (bar == NULL || metrics == NULL || out_x == NULL || out_y == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  content_width =
      bar->bounds.width - (bar->style.padding.left + bar->style.padding.right);
  if (content_width < 0.0f) {
    return M3_ERR_RANGE;
  }

  collapsed_height = bar->style.collapsed_height -
                     (bar->style.padding.top + bar->style.padding.bottom);
  expanded_height = bar->style.expanded_height -
                    (bar->style.padding.top + bar->style.padding.bottom);
  if (collapsed_height < 0.0f || expanded_height < 0.0f) {
    return M3_ERR_RANGE;
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
    if (rc != M3_OK) {
      return rc;
    }
    if (range <= 0.0f) {
      *out_y = collapsed_y;
      return M3_OK;
    }
    t = bar->collapse_offset / range;
    if (t < 0.0f) {
      t = 0.0f;
    }
    if (t > 1.0f) {
      t = 1.0f;
    }
    *out_y = expanded_y + (collapsed_y - expanded_y) * t;
    return M3_OK;
  }

  *out_y = collapsed_y;
  return M3_OK;
}

static int m3_app_bar_scroll_pre(void *ctx, const M3ScrollDelta *delta,
                                 M3ScrollDelta *out_consumed) {
  M3AppBar *bar;
  M3Scalar consumed_y;
  int rc;

  if (ctx == NULL || delta == NULL || out_consumed == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  bar = (M3AppBar *)ctx;
  out_consumed->x = 0.0f;
  out_consumed->y = 0.0f;

  if (delta->y <= 0.0f) {
    return M3_OK;
  }

  rc = m3_app_bar_apply_scroll(bar, delta->y, &consumed_y);
  if (rc != M3_OK) {
    return rc;
  }

  out_consumed->y = consumed_y;
  return M3_OK;
}

static int m3_app_bar_scroll_post(void *ctx, const M3ScrollDelta *delta,
                                  const M3ScrollDelta *child_consumed,
                                  M3ScrollDelta *out_consumed) {
  M3AppBar *bar;
  M3Scalar consumed_y;
  int rc;

  if (ctx == NULL || delta == NULL || child_consumed == NULL ||
      out_consumed == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  M3_UNUSED(child_consumed);
  bar = (M3AppBar *)ctx;
  out_consumed->x = 0.0f;
  out_consumed->y = 0.0f;

  if (delta->y >= 0.0f) {
    return M3_OK;
  }

  rc = m3_app_bar_apply_scroll(bar, delta->y, &consumed_y);
  if (rc != M3_OK) {
    return rc;
  }

  out_consumed->y = consumed_y;
  return M3_OK;
}

static const M3ScrollParentVTable g_m3_app_bar_scroll_vtable = {
    m3_app_bar_scroll_pre, m3_app_bar_scroll_post};

static int m3_app_bar_widget_measure(void *widget, M3MeasureSpec width,
                                     M3MeasureSpec height, M3Size *out_size) {
  M3AppBar *bar;
  M3TextMetrics metrics;
  M3Scalar desired_width;
  M3Scalar desired_height;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_app_bar_validate_measure_spec(width);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_app_bar_validate_measure_spec(height);
  if (rc != M3_OK) {
    return rc;
  }

  bar = (M3AppBar *)widget;
  rc = m3_app_bar_validate_style(&bar->style, M3_FALSE);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_app_bar_measure_title(bar, &metrics);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_app_bar_compute_current_height(bar, &desired_height);
  if (rc != M3_OK) {
    return rc;
  }

  desired_width =
      bar->style.padding.left + bar->style.padding.right + metrics.width;

  if (width.mode == M3_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == M3_MEASURE_AT_MOST) {
    out_size->width = (desired_width > width.size) ? width.size : desired_width;
  } else {
    out_size->width = desired_width;
  }

  if (height.mode == M3_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == M3_MEASURE_AT_MOST) {
    out_size->height =
        (desired_height > height.size) ? height.size : desired_height;
  } else {
    out_size->height = desired_height;
  }

  return M3_OK;
}

static int m3_app_bar_widget_layout(void *widget, M3Rect bounds) {
  M3AppBar *bar;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_app_bar_validate_rect(&bounds);
  if (rc != M3_OK) {
    return rc;
  }

  bar = (M3AppBar *)widget;
  bar->bounds = bounds;
  return M3_OK;
}

static int m3_app_bar_widget_paint(void *widget, M3PaintContext *ctx) {
  M3AppBar *bar;
  M3Rect rect;
  M3TextMetrics metrics;
  M3Scalar height;
  M3Scalar text_x;
  M3Scalar text_y;
  int rc;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  bar = (M3AppBar *)widget;
  rc = m3_app_bar_validate_style(&bar->style, M3_FALSE);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_app_bar_compute_current_height(bar, &height);
  if (rc != M3_OK) {
    return rc;
  }

  rect = bar->bounds;
  rect.height = height;
  if (rect.width < 0.0f || rect.height < 0.0f) {
    return M3_ERR_RANGE;
  }

  if (bar->style.shadow_enabled == M3_TRUE) {
    rc = m3_shadow_paint(&bar->style.shadow, ctx->gfx, &rect, &ctx->clip);
    if (rc != M3_OK) {
      return rc;
    }
  }

  if (ctx->gfx->vtable->draw_rect == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &rect,
                                   bar->style.background_color, 0.0f);
  if (rc != M3_OK) {
    return rc;
  }

  if (bar->title_len == 0u) {
    return M3_OK;
  }

  if (ctx->gfx->text_vtable == NULL ||
      ctx->gfx->text_vtable->draw_text == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  rc = m3_app_bar_measure_title(bar, &metrics);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_app_bar_compute_title_position(bar, &metrics, &text_x, &text_y);
  if (rc != M3_OK) {
    return rc;
  }

  rc = ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, bar->title_font,
                                        bar->utf8_title, bar->title_len, text_x,
                                        text_y, bar->style.title_style.color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_app_bar_widget_event(void *widget, const M3InputEvent *event,
                                   M3Bool *out_handled) {
  if (widget == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_handled = M3_FALSE;
  return M3_OK;
}

static int m3_app_bar_widget_get_semantics(void *widget,
                                           M3Semantics *out_semantics) {
  M3AppBar *bar;

  if (widget == NULL || out_semantics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  bar = (M3AppBar *)widget;
  out_semantics->role = M3_SEMANTIC_NONE;
  out_semantics->flags = 0;
  if (bar->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
  }
  if (bar->widget.flags & M3_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = bar->utf8_title;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return M3_OK;
}

static int m3_app_bar_widget_destroy(void *widget) {
  M3AppBar *bar;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  bar = (M3AppBar *)widget;
  rc = M3_OK;
  if (bar->owns_font == M3_TRUE &&
      (bar->title_font.id != 0u || bar->title_font.generation != 0u)) {
    rc = m3_text_font_destroy(&bar->text_backend, bar->title_font);
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
  bar->owns_font = M3_FALSE;
  memset(&bar->style, 0, sizeof(bar->style));
  return rc;
}

static const M3WidgetVTable g_m3_app_bar_widget_vtable = {
    m3_app_bar_widget_measure,       m3_app_bar_widget_layout,
    m3_app_bar_widget_paint,         m3_app_bar_widget_event,
    m3_app_bar_widget_get_semantics, m3_app_bar_widget_destroy};

static int m3_app_bar_style_init_base(M3AppBarStyle *style, m3_u32 variant,
                                      M3Scalar collapsed_height,
                                      M3Scalar expanded_height) {
  int rc;
#ifdef M3_TESTING
  M3Bool matched;
#endif

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));

  rc = m3_shadow_init(&style->shadow);
#ifdef M3_TESTING
  if (rc == M3_OK) {
    rc = m3_app_bar_test_fail_point_match(M3_APP_BAR_TEST_FAIL_SHADOW_INIT,
                                          &matched);
    if (rc != M3_OK) {
      return rc;
    }
    if (matched == M3_TRUE) {
      rc = M3_ERR_IO;
    }
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_text_style_init(&style->title_style);
#ifdef M3_TESTING
  if (rc == M3_OK) {
    rc = m3_app_bar_test_fail_point_match(M3_APP_BAR_TEST_FAIL_TEXT_STYLE_INIT,
                                          &matched);
    if (rc != M3_OK) {
      return rc;
    }
    if (matched == M3_TRUE) {
      rc = M3_ERR_IO;
    }
  }
#endif
  if (rc != M3_OK) {
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
  style->shadow_enabled = M3_FALSE;

  rc = m3_app_bar_color_set(&style->background_color, 1.0f, 1.0f, 1.0f, 1.0f);
#ifdef M3_TESTING
  if (rc == M3_OK) {
    rc = m3_app_bar_test_fail_point_match(M3_APP_BAR_TEST_FAIL_COLOR_SET,
                                          &matched);
    if (rc != M3_OK) {
      return rc;
    }
    if (matched == M3_TRUE) {
      rc = M3_ERR_IO;
    }
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_app_bar_style_init_small(M3AppBarStyle *style) {
  return m3_app_bar_style_init_base(style, M3_APP_BAR_VARIANT_SMALL,
                                    M3_APP_BAR_DEFAULT_SMALL_HEIGHT,
                                    M3_APP_BAR_DEFAULT_SMALL_HEIGHT);
}

int M3_CALL m3_app_bar_style_init_center(M3AppBarStyle *style) {
  return m3_app_bar_style_init_base(style, M3_APP_BAR_VARIANT_CENTER,
                                    M3_APP_BAR_DEFAULT_SMALL_HEIGHT,
                                    M3_APP_BAR_DEFAULT_SMALL_HEIGHT);
}

int M3_CALL m3_app_bar_style_init_medium(M3AppBarStyle *style) {
  return m3_app_bar_style_init_base(style, M3_APP_BAR_VARIANT_MEDIUM,
                                    M3_APP_BAR_DEFAULT_SMALL_HEIGHT,
                                    M3_APP_BAR_DEFAULT_MEDIUM_HEIGHT);
}

int M3_CALL m3_app_bar_style_init_large(M3AppBarStyle *style) {
  return m3_app_bar_style_init_base(style, M3_APP_BAR_VARIANT_LARGE,
                                    M3_APP_BAR_DEFAULT_SMALL_HEIGHT,
                                    M3_APP_BAR_DEFAULT_LARGE_HEIGHT);
}

int M3_CALL m3_app_bar_init(M3AppBar *bar, const M3TextBackend *backend,
                            const M3AppBarStyle *style, const char *utf8_title,
                            m3_usize title_len) {
  int rc;
#ifdef M3_TESTING
  M3Bool matched;
#endif

  if (bar == NULL || backend == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_app_bar_validate_backend(backend);
  if (rc != M3_OK) {
    return rc;
  }
  if (backend->vtable->create_font == NULL ||
      backend->vtable->destroy_font == NULL ||
      backend->vtable->measure_text == NULL ||
      backend->vtable->draw_text == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  rc = m3_app_bar_validate_style(style, M3_TRUE);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_app_bar_validate_title(utf8_title, title_len);
  if (rc != M3_OK) {
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

  rc = m3_text_font_create(backend, &style->title_style, &bar->title_font);
  if (rc != M3_OK) {
    return rc;
  }

  bar->owns_font = M3_TRUE;
  bar->widget.ctx = bar;
  bar->widget.vtable = &g_m3_app_bar_widget_vtable;
  bar->widget.handle.id = 0u;
  bar->widget.handle.generation = 0u;
  bar->widget.flags = 0u;

  rc = m3_scroll_parent_init(&bar->scroll_parent, bar,
                             &g_m3_app_bar_scroll_vtable);
#ifdef M3_TESTING
  if (rc == M3_OK) {
    rc = m3_app_bar_test_fail_point_match(M3_APP_BAR_TEST_FAIL_SCROLL_INIT,
                                          &matched);
    if (rc != M3_OK) {
      return rc;
    }
    if (matched == M3_TRUE) {
      rc = M3_ERR_IO;
    }
  }
#endif
  if (rc != M3_OK) {
    int destroy_rc;

    destroy_rc = m3_text_font_destroy(&bar->text_backend, bar->title_font);
    if (destroy_rc != M3_OK) {
      return destroy_rc;
    }
    bar->title_font.id = 0u;
    bar->title_font.generation = 0u;
    bar->owns_font = M3_FALSE;
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_app_bar_set_title(M3AppBar *bar, const char *utf8_title,
                                 m3_usize title_len) {
  int rc;

  if (bar == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_app_bar_validate_title(utf8_title, title_len);
  if (rc != M3_OK) {
    return rc;
  }

  bar->utf8_title = utf8_title;
  bar->title_len = title_len;
  return M3_OK;
}

int M3_CALL m3_app_bar_set_style(M3AppBar *bar, const M3AppBarStyle *style) {
  M3Handle new_font;
  M3Scalar range;
  int rc;

  if (bar == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_app_bar_validate_style(style, M3_TRUE);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_text_font_create(&bar->text_backend, &style->title_style, &new_font);
  if (rc != M3_OK) {
    return rc;
  }

  if (bar->owns_font == M3_TRUE) {
    rc = m3_text_font_destroy(&bar->text_backend, bar->title_font);
    if (rc != M3_OK) {
      m3_text_font_destroy(&bar->text_backend, new_font);
      return rc;
    }
  }

  bar->style = *style;
  bar->title_font = new_font;
  bar->owns_font = M3_TRUE;

  rc = m3_app_bar_compute_collapse_range(&bar->style, &range);
  if (rc != M3_OK) {
    return rc;
  }
  if (bar->collapse_offset > range) {
    bar->collapse_offset = range;
  }
  if (bar->collapse_offset < 0.0f) {
    bar->collapse_offset = 0.0f;
  }

  return M3_OK;
}

int M3_CALL m3_app_bar_set_collapse_offset(M3AppBar *bar, M3Scalar offset) {
  M3Scalar range;
  int rc;

  if (bar == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_app_bar_compute_collapse_range(&bar->style, &range);
  if (rc != M3_OK) {
    return rc;
  }
  if (offset < 0.0f || offset > range) {
    return M3_ERR_RANGE;
  }

  bar->collapse_offset = offset;
  return M3_OK;
}

int M3_CALL m3_app_bar_get_collapse_offset(const M3AppBar *bar,
                                           M3Scalar *out_offset) {
  if (bar == NULL || out_offset == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_offset = bar->collapse_offset;
  return M3_OK;
}

int M3_CALL m3_app_bar_get_height(const M3AppBar *bar, M3Scalar *out_height) {
  return m3_app_bar_compute_current_height(bar, out_height);
}

int M3_CALL m3_app_bar_get_content_bounds(const M3AppBar *bar,
                                          M3Rect *out_bounds) {
  return m3_app_bar_compute_content_bounds(bar, out_bounds);
}

#ifdef M3_TESTING
int M3_CALL m3_app_bar_test_validate_color(const M3Color *color) {
  return m3_app_bar_validate_color(color);
}

int M3_CALL m3_app_bar_test_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                                      M3Scalar b, M3Scalar a) {
  return m3_app_bar_color_set(color, r, g, b, a);
}

int M3_CALL m3_app_bar_test_validate_edges(const M3LayoutEdges *edges) {
  return m3_app_bar_validate_edges(edges);
}

int M3_CALL m3_app_bar_test_validate_text_style(const M3TextStyle *style,
                                                M3Bool require_family) {
  return m3_app_bar_validate_text_style(style, require_family);
}

int M3_CALL m3_app_bar_test_validate_style(const M3AppBarStyle *style,
                                           M3Bool require_family) {
  return m3_app_bar_validate_style(style, require_family);
}

int M3_CALL m3_app_bar_test_call_color_should_fail(M3Bool *out_fail) {
  return m3_app_bar_test_color_should_fail(out_fail);
}

int M3_CALL m3_app_bar_test_call_fail_point_match(m3_u32 point,
                                                  M3Bool *out_match) {
  return m3_app_bar_test_fail_point_match(point, out_match);
}

int M3_CALL m3_app_bar_test_validate_measure_spec(M3MeasureSpec spec) {
  return m3_app_bar_validate_measure_spec(spec);
}

int M3_CALL m3_app_bar_test_validate_rect(const M3Rect *rect) {
  return m3_app_bar_validate_rect(rect);
}

int M3_CALL m3_app_bar_test_compute_collapse_range(const M3AppBarStyle *style,
                                                   M3Scalar *out_range) {
  return m3_app_bar_compute_collapse_range(style, out_range);
}

int M3_CALL m3_app_bar_test_compute_current_height(const M3AppBar *bar,
                                                   M3Scalar *out_height) {
  return m3_app_bar_compute_current_height(bar, out_height);
}

int M3_CALL m3_app_bar_test_compute_content_bounds(const M3AppBar *bar,
                                                   M3Rect *out_bounds) {
  return m3_app_bar_compute_content_bounds(bar, out_bounds);
}

int M3_CALL m3_app_bar_test_compute_title_position(const M3AppBar *bar,
                                                   const M3TextMetrics *metrics,
                                                   M3Scalar *out_x,
                                                   M3Scalar *out_y) {
  return m3_app_bar_compute_title_position(bar, metrics, out_x, out_y);
}

int M3_CALL m3_app_bar_test_measure_title(const M3AppBar *bar,
                                          M3TextMetrics *out_metrics) {
  return m3_app_bar_measure_title(bar, out_metrics);
}

int M3_CALL m3_app_bar_test_apply_scroll(M3AppBar *bar, M3Scalar delta,
                                         M3Scalar *out_consumed) {
  return m3_app_bar_apply_scroll(bar, delta, out_consumed);
}
#endif
