#include "m3/m3_progress.h"

#include <math.h>
#include <string.h>

#ifdef M3_TESTING
#define M3_PROGRESS_TEST_FAIL_NONE 0u
#define M3_PROGRESS_TEST_FAIL_COLOR_SET 1u
#define M3_PROGRESS_TEST_FAIL_COLOR_WITH_ALPHA 2u
#define M3_PROGRESS_TEST_FAIL_LINEAR_CORNER_RANGE 3u
#define M3_PROGRESS_TEST_FAIL_LINEAR_FILL_CORNER_RANGE 4u
#define M3_PROGRESS_TEST_FAIL_LINEAR_FILL_CORNER_CLAMP 5u
#define M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_COUNT_NEGATIVE 6u
#define M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_CLAMP_MIN 7u
#define M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_CLAMP_MAX 8u
#define M3_PROGRESS_TEST_FAIL_SLIDER_FRACTION_NEGATIVE 9u
#define M3_PROGRESS_TEST_FAIL_SLIDER_FRACTION_POSITIVE 10u
#define M3_PROGRESS_TEST_FAIL_SLIDER_FROM_X_FRACTION_HIGH 11u
#define M3_PROGRESS_TEST_FAIL_SLIDER_CORNER_RANGE 12u
#define M3_PROGRESS_TEST_FAIL_LINEAR_RESOLVE_COLORS 13u
#define M3_PROGRESS_TEST_FAIL_CIRCULAR_RESOLVE_COLORS 14u
#define M3_PROGRESS_TEST_FAIL_SLIDER_RESOLVE_COLORS 15u
#define M3_PROGRESS_TEST_FAIL_SLIDER_CORNER_CLAMP 16u

static m3_u32 g_m3_progress_test_fail_point = M3_PROGRESS_TEST_FAIL_NONE;
static m3_u32 g_m3_progress_test_color_set_calls = 0u;
static m3_u32 g_m3_progress_test_color_set_fail_after = 0u;
static m3_u32 g_m3_progress_test_color_alpha_calls = 0u;
static m3_u32 g_m3_progress_test_color_alpha_fail_after = 0u;

static M3Bool m3_progress_test_fail_point_match(m3_u32 point) {
  if (g_m3_progress_test_fail_point != point) {
    return M3_FALSE;
  }
  g_m3_progress_test_fail_point = M3_PROGRESS_TEST_FAIL_NONE;
  return M3_TRUE;
}
#endif

#define M3_PROGRESS_PI 3.14159265358979323846f

static int m3_progress_validate_color(const M3Color *color) {
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

static int m3_progress_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                                 M3Scalar b, M3Scalar a) {
  if (color == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#ifdef M3_TESTING
  g_m3_progress_test_color_set_calls += 1u;
  if (m3_progress_test_fail_point_match(M3_PROGRESS_TEST_FAIL_COLOR_SET)) {
    return M3_ERR_RANGE;
  }
  if (g_m3_progress_test_color_set_fail_after > 0u &&
      g_m3_progress_test_color_set_calls >=
          g_m3_progress_test_color_set_fail_after) {
    return M3_ERR_RANGE;
  }
#endif
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
  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
  return M3_OK;
}

static int m3_progress_color_with_alpha(const M3Color *base, M3Scalar alpha,
                                        M3Color *out_color) {
  int rc;

  if (base == NULL || out_color == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#ifdef M3_TESTING
  g_m3_progress_test_color_alpha_calls += 1u;
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_COLOR_WITH_ALPHA)) {
    return M3_ERR_RANGE;
  }
  if (g_m3_progress_test_color_alpha_fail_after > 0u &&
      g_m3_progress_test_color_alpha_calls >=
          g_m3_progress_test_color_alpha_fail_after) {
    return M3_ERR_RANGE;
  }
#endif
  if (!(alpha >= 0.0f && alpha <= 1.0f)) {
    return M3_ERR_RANGE;
  }

  rc = m3_progress_validate_color(base);
  if (rc != M3_OK) {
    return rc;
  }

  *out_color = *base;
  out_color->a = out_color->a * alpha;
  return M3_OK;
}

static int m3_progress_validate_measure_spec(M3MeasureSpec spec) {
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.mode != M3_MEASURE_EXACTLY &&
      spec.mode != M3_MEASURE_AT_MOST) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_progress_apply_measure(M3Scalar desired, M3MeasureSpec spec,
                                     M3Scalar *out_size) {
  int rc;

  if (out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_progress_validate_measure_spec(spec);
  if (rc != M3_OK) {
    return rc;
  }

  if (spec.mode == M3_MEASURE_EXACTLY) {
    *out_size = spec.size;
  } else if (spec.mode == M3_MEASURE_AT_MOST) {
    *out_size = desired;
    if (*out_size > spec.size) {
      *out_size = spec.size;
    }
  } else {
    *out_size = desired;
  }
  return M3_OK;
}

static int m3_progress_validate_rect(const M3Rect *rect) {
  if (rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_progress_validate_value01(M3Scalar value) {
  if (!(value >= 0.0f && value <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int
m3_linear_progress_validate_style(const M3LinearProgressStyle *style) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (style->height <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->min_width < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->corner_radius < 0.0f ||
      style->corner_radius > style->height * 0.5f) {
    return M3_ERR_RANGE;
  }

  rc = m3_progress_validate_color(&style->track_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->indicator_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->disabled_track_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->disabled_indicator_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_linear_progress_resolve_colors(const M3LinearProgress *progress,
                                             M3Color *out_track,
                                             M3Color *out_indicator) {
  int rc;

  if (progress == NULL || out_track == NULL || out_indicator == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#ifdef M3_TESTING
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_LINEAR_RESOLVE_COLORS)) {
    return M3_ERR_IO;
  }
#endif

  if (progress->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    *out_track = progress->style.disabled_track_color;
    *out_indicator = progress->style.disabled_indicator_color;
  } else {
    *out_track = progress->style.track_color;
    *out_indicator = progress->style.indicator_color;
  }

  rc = m3_progress_validate_color(out_track);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(out_indicator);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

static int m3_linear_progress_widget_measure(void *widget, M3MeasureSpec width,
                                             M3MeasureSpec height,
                                             M3Size *out_size) {
  M3LinearProgress *progress;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  progress = (M3LinearProgress *)widget;
  rc = m3_linear_progress_validate_style(&progress->style);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_progress_apply_measure(progress->style.min_width, width,
                                 &out_size->width);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_apply_measure(progress->style.height, height,
                                 &out_size->height);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

static int m3_linear_progress_widget_layout(void *widget, M3Rect bounds) {
  M3LinearProgress *progress;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_progress_validate_rect(&bounds);
  if (rc != M3_OK) {
    return rc;
  }

  progress = (M3LinearProgress *)widget;
  progress->bounds = bounds;
  return M3_OK;
}

static int m3_linear_progress_widget_paint(void *widget, M3PaintContext *ctx) {
  M3LinearProgress *progress;
  M3Rect bounds;
  M3Rect fill;
  M3Color track_color;
  M3Color indicator_color;
  M3Scalar corner;
  M3Scalar fill_corner;
  M3Scalar fill_width;
  int rc;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable->draw_rect == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  progress = (M3LinearProgress *)widget;

  rc = m3_linear_progress_validate_style(&progress->style);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_progress_validate_rect(&progress->bounds);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_progress_validate_value01(progress->value);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_linear_progress_resolve_colors(progress, &track_color,
                                         &indicator_color);
  if (rc != M3_OK) {
    return rc;
  }

  bounds = progress->bounds;
  corner = progress->style.corner_radius;
#ifdef M3_TESTING
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_LINEAR_CORNER_RANGE)) {
    corner = -1.0f;
  }
#endif
  if (corner > bounds.height * 0.5f) {
    corner = bounds.height * 0.5f;
  }
  if (corner < 0.0f) {
    return M3_ERR_RANGE;
  }

  if (track_color.a > 0.0f) {
    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &bounds, track_color,
                                     corner);
    if (rc != M3_OK) {
      return rc;
    }
  }

  fill_width = bounds.width * progress->value;
  if (fill_width > 0.0f && indicator_color.a > 0.0f) {
    fill = bounds;
    fill.width = fill_width;
    fill_corner = corner;
#ifdef M3_TESTING
    if (m3_progress_test_fail_point_match(
            M3_PROGRESS_TEST_FAIL_LINEAR_FILL_CORNER_CLAMP)) {
      fill_corner = fill.height;
    }
#endif
    if (fill_corner > fill.height * 0.5f) {
      fill_corner = fill.height * 0.5f;
    }
    if (fill_corner > fill.width * 0.5f) {
      fill_corner = fill.width * 0.5f;
    }
#ifdef M3_TESTING
    if (m3_progress_test_fail_point_match(
            M3_PROGRESS_TEST_FAIL_LINEAR_FILL_CORNER_RANGE)) {
      fill_corner = -1.0f;
    }
#endif
    if (fill_corner < 0.0f) {
      return M3_ERR_RANGE;
    }
    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &fill, indicator_color,
                                     fill_corner);
    if (rc != M3_OK) {
      return rc;
    }
  }

  return M3_OK;
}

static int m3_linear_progress_widget_event(void *widget,
                                           const M3InputEvent *event,
                                           M3Bool *out_handled) {
  if (widget == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  M3_UNUSED(widget);
  M3_UNUSED(event);
  *out_handled = M3_FALSE;
  return M3_OK;
}

static int m3_linear_progress_widget_get_semantics(void *widget,
                                                   M3Semantics *out_semantics) {
  M3LinearProgress *progress;

  if (widget == NULL || out_semantics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  progress = (M3LinearProgress *)widget;
  out_semantics->role = M3_SEMANTIC_NONE;
  out_semantics->flags = 0;
  if (progress->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
  }
  out_semantics->utf8_label = progress->utf8_label;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return M3_OK;
}

static int m3_linear_progress_widget_destroy(void *widget) {
  M3LinearProgress *progress;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  progress = (M3LinearProgress *)widget;
  memset(&progress->style, 0, sizeof(progress->style));
  progress->bounds.x = 0.0f;
  progress->bounds.y = 0.0f;
  progress->bounds.width = 0.0f;
  progress->bounds.height = 0.0f;
  progress->value = 0.0f;
  progress->utf8_label = NULL;
  progress->utf8_len = 0;
  progress->widget.ctx = NULL;
  progress->widget.vtable = NULL;
  progress->widget.handle.id = 0u;
  progress->widget.handle.generation = 0u;
  progress->widget.flags = 0u;
  return M3_OK;
}

static const M3WidgetVTable g_m3_linear_progress_widget_vtable = {
    m3_linear_progress_widget_measure,       m3_linear_progress_widget_layout,
    m3_linear_progress_widget_paint,         m3_linear_progress_widget_event,
    m3_linear_progress_widget_get_semantics, m3_linear_progress_widget_destroy};

int M3_CALL m3_linear_progress_style_init(M3LinearProgressStyle *style) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->height = M3_LINEAR_PROGRESS_DEFAULT_HEIGHT;
  style->min_width = M3_LINEAR_PROGRESS_DEFAULT_MIN_WIDTH;
  style->corner_radius = M3_LINEAR_PROGRESS_DEFAULT_CORNER_RADIUS;

  rc = m3_progress_color_set(&style->track_color, 0.82f, 0.82f, 0.82f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc =
      m3_progress_color_set(&style->indicator_color, 0.26f, 0.52f, 0.96f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_color_with_alpha(&style->track_color, 0.38f,
                                    &style->disabled_track_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_color_with_alpha(&style->indicator_color, 0.38f,
                                    &style->disabled_indicator_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_linear_progress_init(M3LinearProgress *progress,
                                    const M3LinearProgressStyle *style,
                                    M3Scalar value) {
  int rc;

  if (progress == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_linear_progress_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_validate_value01(value);
  if (rc != M3_OK) {
    return rc;
  }

  memset(progress, 0, sizeof(*progress));
  progress->style = *style;
  progress->value = value;
  progress->widget.ctx = progress;
  progress->widget.vtable = &g_m3_linear_progress_widget_vtable;
  progress->widget.handle.id = 0u;
  progress->widget.handle.generation = 0u;
  progress->widget.flags = 0u;
  return M3_OK;
}

int M3_CALL m3_linear_progress_set_value(M3LinearProgress *progress,
                                         M3Scalar value) {
  int rc;

  if (progress == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_progress_validate_value01(value);
  if (rc != M3_OK) {
    return rc;
  }

  progress->value = value;
  return M3_OK;
}

int M3_CALL m3_linear_progress_get_value(const M3LinearProgress *progress,
                                         M3Scalar *out_value) {
  if (progress == NULL || out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_value = progress->value;
  return M3_OK;
}

int M3_CALL m3_linear_progress_set_style(M3LinearProgress *progress,
                                         const M3LinearProgressStyle *style) {
  int rc;

  if (progress == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_linear_progress_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }

  progress->style = *style;
  return M3_OK;
}

int M3_CALL m3_linear_progress_set_label(M3LinearProgress *progress,
                                         const char *utf8_label,
                                         m3_usize utf8_len) {
  if (progress == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8_label == NULL && utf8_len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  progress->utf8_label = utf8_label;
  progress->utf8_len = utf8_len;
  return M3_OK;
}

static int
m3_circular_progress_validate_style(const M3CircularProgressStyle *style) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (style->diameter <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->thickness <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->thickness > style->diameter * 0.5f) {
    return M3_ERR_RANGE;
  }
  if (style->segments < 3u) {
    return M3_ERR_RANGE;
  }

  rc = m3_progress_validate_color(&style->track_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->indicator_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->disabled_track_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->disabled_indicator_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int
m3_circular_progress_resolve_colors(const M3CircularProgress *progress,
                                    M3Color *out_track,
                                    M3Color *out_indicator) {
  int rc;

  if (progress == NULL || out_track == NULL || out_indicator == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#ifdef M3_TESTING
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_CIRCULAR_RESOLVE_COLORS)) {
    return M3_ERR_IO;
  }
#endif

  if (progress->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    *out_track = progress->style.disabled_track_color;
    *out_indicator = progress->style.disabled_indicator_color;
  } else {
    *out_track = progress->style.track_color;
    *out_indicator = progress->style.indicator_color;
  }

  rc = m3_progress_validate_color(out_track);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(out_indicator);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

static int m3_circular_progress_draw_arc(M3Gfx *gfx, M3Scalar cx, M3Scalar cy,
                                         M3Scalar radius, M3Scalar start_angle,
                                         M3Scalar end_angle, M3Color color,
                                         M3Scalar thickness, m3_u32 segments) {
  M3Scalar full;
  M3Scalar step;
  M3Scalar angle;
  M3Scalar next;
  M3Scalar x0;
  M3Scalar y0;
  M3Scalar x1;
  M3Scalar y1;
  m3_u32 i;
  int rc;

  if (gfx == NULL || gfx->vtable == NULL || gfx->vtable->draw_line == NULL) {
    return M3_ERR_UNSUPPORTED;
  }
  if (segments < 1u) {
    return M3_ERR_RANGE;
  }
  if (thickness <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (end_angle <= start_angle) {
    return M3_OK;
  }

  full = M3_PROGRESS_PI * 2.0f;
  step = full / (M3Scalar)segments;
  angle = start_angle;
  for (i = 0u; i < segments && angle < end_angle; ++i) {
    next = angle + step;
    if (next > end_angle) {
      next = end_angle;
    }
    x0 = cx + (M3Scalar)(cos(angle) * radius);
    y0 = cy + (M3Scalar)(sin(angle) * radius);
    x1 = cx + (M3Scalar)(cos(next) * radius);
    y1 = cy + (M3Scalar)(sin(next) * radius);
    rc = gfx->vtable->draw_line(gfx->ctx, x0, y0, x1, y1, color, thickness);
    if (rc != M3_OK) {
      return rc;
    }
    angle = next;
  }
  return M3_OK;
}

static int m3_circular_progress_widget_measure(void *widget,
                                               M3MeasureSpec width,
                                               M3MeasureSpec height,
                                               M3Size *out_size) {
  M3CircularProgress *progress;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  progress = (M3CircularProgress *)widget;
  rc = m3_circular_progress_validate_style(&progress->style);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_progress_apply_measure(progress->style.diameter, width,
                                 &out_size->width);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_apply_measure(progress->style.diameter, height,
                                 &out_size->height);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

static int m3_circular_progress_widget_layout(void *widget, M3Rect bounds) {
  M3CircularProgress *progress;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_progress_validate_rect(&bounds);
  if (rc != M3_OK) {
    return rc;
  }

  progress = (M3CircularProgress *)widget;
  progress->bounds = bounds;
  return M3_OK;
}

static int m3_circular_progress_widget_paint(void *widget,
                                             M3PaintContext *ctx) {
  M3CircularProgress *progress;
  M3Color track_color;
  M3Color indicator_color;
  M3Rect bounds;
  M3Scalar min_side;
  M3Scalar radius;
  M3Scalar cx;
  M3Scalar cy;
  M3Scalar end_angle;
  int rc;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable->draw_line == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  progress = (M3CircularProgress *)widget;

  rc = m3_circular_progress_validate_style(&progress->style);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_progress_validate_rect(&progress->bounds);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_progress_validate_value01(progress->value);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_circular_progress_resolve_colors(progress, &track_color,
                                           &indicator_color);
  if (rc != M3_OK) {
    return rc;
  }

  bounds = progress->bounds;
  min_side = bounds.width;
  if (bounds.height < min_side) {
    min_side = bounds.height;
  }

  if (min_side <= 0.0f) {
    return M3_ERR_RANGE;
  }

  radius = (min_side - progress->style.thickness) * 0.5f;
  if (radius <= 0.0f) {
    return M3_ERR_RANGE;
  }

  cx = bounds.x + bounds.width * 0.5f;
  cy = bounds.y + bounds.height * 0.5f;

  if (track_color.a > 0.0f) {
    end_angle = progress->style.start_angle + (M3_PROGRESS_PI * 2.0f);
    rc = m3_circular_progress_draw_arc(
        ctx->gfx, cx, cy, radius, progress->style.start_angle, end_angle,
        track_color, progress->style.thickness, progress->style.segments);
    if (rc != M3_OK) {
      return rc;
    }
  }

  if (progress->value > 0.0f && indicator_color.a > 0.0f) {
    end_angle =
        progress->style.start_angle + (M3_PROGRESS_PI * 2.0f * progress->value);
    rc = m3_circular_progress_draw_arc(
        ctx->gfx, cx, cy, radius, progress->style.start_angle, end_angle,
        indicator_color, progress->style.thickness, progress->style.segments);
    if (rc != M3_OK) {
      return rc;
    }
  }

  return M3_OK;
}

static int m3_circular_progress_widget_event(void *widget,
                                             const M3InputEvent *event,
                                             M3Bool *out_handled) {
  if (widget == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  M3_UNUSED(widget);
  M3_UNUSED(event);
  *out_handled = M3_FALSE;
  return M3_OK;
}

static int
m3_circular_progress_widget_get_semantics(void *widget,
                                          M3Semantics *out_semantics) {
  M3CircularProgress *progress;

  if (widget == NULL || out_semantics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  progress = (M3CircularProgress *)widget;
  out_semantics->role = M3_SEMANTIC_NONE;
  out_semantics->flags = 0;
  if (progress->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
  }
  out_semantics->utf8_label = progress->utf8_label;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return M3_OK;
}

static int m3_circular_progress_widget_destroy(void *widget) {
  M3CircularProgress *progress;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  progress = (M3CircularProgress *)widget;
  memset(&progress->style, 0, sizeof(progress->style));
  progress->bounds.x = 0.0f;
  progress->bounds.y = 0.0f;
  progress->bounds.width = 0.0f;
  progress->bounds.height = 0.0f;
  progress->value = 0.0f;
  progress->utf8_label = NULL;
  progress->utf8_len = 0;
  progress->widget.ctx = NULL;
  progress->widget.vtable = NULL;
  progress->widget.handle.id = 0u;
  progress->widget.handle.generation = 0u;
  progress->widget.flags = 0u;
  return M3_OK;
}

static const M3WidgetVTable g_m3_circular_progress_widget_vtable = {
    m3_circular_progress_widget_measure,
    m3_circular_progress_widget_layout,
    m3_circular_progress_widget_paint,
    m3_circular_progress_widget_event,
    m3_circular_progress_widget_get_semantics,
    m3_circular_progress_widget_destroy};

int M3_CALL m3_circular_progress_style_init(M3CircularProgressStyle *style) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->diameter = M3_CIRCULAR_PROGRESS_DEFAULT_DIAMETER;
  style->thickness = M3_CIRCULAR_PROGRESS_DEFAULT_THICKNESS;
  style->start_angle = M3_CIRCULAR_PROGRESS_DEFAULT_START_ANGLE;
  style->segments = M3_CIRCULAR_PROGRESS_DEFAULT_SEGMENTS;

  rc = m3_progress_color_set(&style->track_color, 0.82f, 0.82f, 0.82f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc =
      m3_progress_color_set(&style->indicator_color, 0.26f, 0.52f, 0.96f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_color_with_alpha(&style->track_color, 0.38f,
                                    &style->disabled_track_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_color_with_alpha(&style->indicator_color, 0.38f,
                                    &style->disabled_indicator_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_circular_progress_init(M3CircularProgress *progress,
                                      const M3CircularProgressStyle *style,
                                      M3Scalar value) {
  int rc;

  if (progress == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_circular_progress_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_validate_value01(value);
  if (rc != M3_OK) {
    return rc;
  }

  memset(progress, 0, sizeof(*progress));
  progress->style = *style;
  progress->value = value;
  progress->widget.ctx = progress;
  progress->widget.vtable = &g_m3_circular_progress_widget_vtable;
  progress->widget.handle.id = 0u;
  progress->widget.handle.generation = 0u;
  progress->widget.flags = 0u;
  return M3_OK;
}

int M3_CALL m3_circular_progress_set_value(M3CircularProgress *progress,
                                           M3Scalar value) {
  int rc;

  if (progress == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_progress_validate_value01(value);
  if (rc != M3_OK) {
    return rc;
  }

  progress->value = value;
  return M3_OK;
}

int M3_CALL m3_circular_progress_get_value(const M3CircularProgress *progress,
                                           M3Scalar *out_value) {
  if (progress == NULL || out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_value = progress->value;
  return M3_OK;
}

int M3_CALL m3_circular_progress_set_style(
    M3CircularProgress *progress, const M3CircularProgressStyle *style) {
  int rc;

  if (progress == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_circular_progress_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }

  progress->style = *style;
  return M3_OK;
}

int M3_CALL m3_circular_progress_set_label(M3CircularProgress *progress,
                                           const char *utf8_label,
                                           m3_usize utf8_len) {
  if (progress == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8_label == NULL && utf8_len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  progress->utf8_label = utf8_label;
  progress->utf8_len = utf8_len;
  return M3_OK;
}

static int m3_slider_validate_style(const M3SliderStyle *style) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (style->track_length <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->track_height <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->track_corner_radius < 0.0f ||
      style->track_corner_radius > style->track_height * 0.5f) {
    return M3_ERR_RANGE;
  }
  if (style->thumb_radius <= 0.0f) {
    return M3_ERR_RANGE;
  }

  rc = m3_progress_validate_color(&style->track_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->active_track_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->thumb_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->disabled_track_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->disabled_active_track_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->disabled_thumb_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_slider_validate_range(M3Scalar min_value, M3Scalar max_value) {
  if (!(max_value > min_value)) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_slider_validate_step(M3Scalar step) {
  if (step < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_slider_snap_value(M3Scalar value, M3Scalar min_value,
                                M3Scalar max_value, M3Scalar step,
                                M3Scalar *out_value) {
  M3Scalar clamped;
  M3Scalar range;
  M3Scalar count;
  m3_i32 steps;
  M3Bool force_negative;

  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (step < 0.0f) {
    return M3_ERR_RANGE;
  }

  range = max_value - min_value;
  if (range <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (step > 0.0f && step > range) {
    return M3_ERR_RANGE;
  }

  clamped = value;
  if (clamped < min_value) {
    clamped = min_value;
  }
  if (clamped > max_value) {
    clamped = max_value;
  }

  force_negative = M3_FALSE;
#ifdef M3_TESTING
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_COUNT_NEGATIVE)) {
    force_negative = M3_TRUE;
  }
#endif
  if (step <= 0.0f) {
    if (force_negative) {
      return M3_ERR_RANGE;
    }
    *out_value = clamped;
    return M3_OK;
  }

  count = (clamped - min_value) / step;
  if (force_negative) {
    count = -1.0f;
  }
  if (count < 0.0f) { return M3_ERR_RANGE; }
  steps = (m3_i32)(count + 0.5f);
  clamped = min_value + ((M3Scalar)steps) * step;
#ifdef M3_TESTING
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_CLAMP_MIN)) {
    clamped = min_value - 1.0f;
  }
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_CLAMP_MAX)) {
    clamped = max_value + 1.0f;
  }
#endif
  if (clamped < min_value) {
    clamped = min_value;
  }
  if (clamped > max_value) {
    clamped = max_value;
  }
  *out_value = clamped;
  return M3_OK;
}

static int m3_slider_value_to_fraction(const M3Slider *slider,
                                       M3Scalar *out_fraction) {
  M3Scalar range;

  if (slider == NULL || out_fraction == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  range = slider->max_value - slider->min_value;
  if (range <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (slider->value < slider->min_value || slider->value > slider->max_value) {
    return M3_ERR_RANGE;
  }

  *out_fraction = (slider->value - slider->min_value) / range;
#ifdef M3_TESTING
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_FRACTION_NEGATIVE)) {
    *out_fraction = -1.0f;
  }
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_FRACTION_POSITIVE)) {
    *out_fraction = 2.0f;
  }
#endif
  if (*out_fraction < 0.0f) {
    *out_fraction = 0.0f;
  }
  if (*out_fraction > 1.0f) {
    *out_fraction = 1.0f;
  }
  return M3_OK;
}

static int m3_slider_value_from_x(const M3Slider *slider, M3Scalar x,
                                  M3Scalar *out_value) {
  M3Scalar range;
  M3Scalar fraction;
  int rc;

  if (slider == NULL || out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  range = slider->max_value - slider->min_value;
  if (range <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (slider->bounds.width <= 0.0f) {
    return M3_ERR_RANGE;
  }

  fraction = (x - slider->bounds.x) / slider->bounds.width;
#ifdef M3_TESTING
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_FROM_X_FRACTION_HIGH)) {
    fraction = 2.0f;
  }
#endif
  if (fraction < 0.0f) {
    fraction = 0.0f;
  }
  if (fraction > 1.0f) {
    fraction = 1.0f;
  }

  rc = m3_slider_snap_value(slider->min_value + fraction * range,
                            slider->min_value, slider->max_value, slider->step,
                            out_value);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_slider_resolve_colors(const M3Slider *slider, M3Color *out_track,
                                    M3Color *out_active, M3Color *out_thumb) {
  M3Color track;
  M3Color active;
  M3Color thumb;
  int rc;

  if (slider == NULL || out_track == NULL || out_active == NULL ||
      out_thumb == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#ifdef M3_TESTING
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_RESOLVE_COLORS)) {
    return M3_ERR_IO;
  }
#endif

  if (slider->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    track = slider->style.disabled_track_color;
    active = slider->style.disabled_active_track_color;
    thumb = slider->style.disabled_thumb_color;
  } else {
    track = slider->style.track_color;
    active = slider->style.active_track_color;
    thumb = slider->style.thumb_color;
  }

  rc = m3_progress_validate_color(&track);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&active);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&thumb);
  if (rc != M3_OK) {
    return rc;
  }

  *out_track = track;
  *out_active = active;
  *out_thumb = thumb;
  return M3_OK;
}

static int m3_slider_update_value(M3Slider *slider, M3Scalar next_value,
                                  M3Bool notify) {
  M3Scalar prev_value;
  int rc;

  if (slider == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  prev_value = slider->value;
  slider->value = next_value;
  if (notify == M3_TRUE && slider->on_change != NULL) {
    rc = slider->on_change(slider->on_change_ctx, slider, next_value);
    if (rc != M3_OK) {
      slider->value = prev_value;
      return rc;
    }
  }
  return M3_OK;
}

static int m3_slider_widget_measure(void *widget, M3MeasureSpec width,
                                    M3MeasureSpec height, M3Size *out_size) {
  M3Slider *slider;
  M3Scalar desired_height;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  slider = (M3Slider *)widget;
  rc = m3_slider_validate_style(&slider->style);
  if (rc != M3_OK) {
    return rc;
  }

  desired_height = slider->style.track_height;
  if (desired_height < slider->style.thumb_radius * 2.0f) {
    desired_height = slider->style.thumb_radius * 2.0f;
  }

  rc = m3_progress_apply_measure(slider->style.track_length, width,
                                 &out_size->width);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_apply_measure(desired_height, height, &out_size->height);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

static int m3_slider_widget_layout(void *widget, M3Rect bounds) {
  M3Slider *slider;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_progress_validate_rect(&bounds);
  if (rc != M3_OK) {
    return rc;
  }

  slider = (M3Slider *)widget;
  slider->bounds = bounds;
  return M3_OK;
}

static int m3_slider_widget_paint(void *widget, M3PaintContext *ctx) {
  M3Slider *slider;
  M3Rect bounds;
  M3Rect track;
  M3Rect active;
  M3Rect thumb;
  M3Color track_color;
  M3Color active_color;
  M3Color thumb_color;
  M3Scalar fraction;
  M3Scalar corner;
  M3Scalar thumb_size;
  int rc;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable->draw_rect == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  slider = (M3Slider *)widget;

  rc = m3_slider_validate_style(&slider->style);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_progress_validate_rect(&slider->bounds);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_slider_validate_range(slider->min_value, slider->max_value);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_slider_validate_step(slider->step);
  if (rc != M3_OK) {
    return rc;
  }
  if (slider->step > 0.0f &&
      slider->step > (slider->max_value - slider->min_value)) {
    return M3_ERR_RANGE;
  }

  rc = m3_slider_value_to_fraction(slider, &fraction);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_slider_resolve_colors(slider, &track_color, &active_color,
                                &thumb_color);
  if (rc != M3_OK) {
    return rc;
  }

  bounds = slider->bounds;
  if (bounds.width <= 0.0f) {
    return M3_ERR_RANGE;
  }

  track.height = slider->style.track_height;
  track.width = bounds.width;
  track.x = bounds.x;
  track.y = bounds.y + (bounds.height - track.height) * 0.5f;
  corner = slider->style.track_corner_radius;
#ifdef M3_TESTING
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_CORNER_CLAMP)) {
    corner = track.height;
  }
#endif
  if (corner > track.height * 0.5f) {
    corner = track.height * 0.5f;
  }
#ifdef M3_TESTING
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_CORNER_RANGE)) {
    corner = -1.0f;
  }
#endif
  if (corner < 0.0f) {
    return M3_ERR_RANGE;
  }

  rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &track, track_color, corner);
  if (rc != M3_OK) {
    return rc;
  }

  active = track;
  active.width = bounds.width * fraction;
  if (active.width > 0.0f) {
    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &active, active_color,
                                     corner);
    if (rc != M3_OK) {
      return rc;
    }
  }

  thumb_size = slider->style.thumb_radius * 2.0f;
  thumb.width = thumb_size;
  thumb.height = thumb_size;
  thumb.x = bounds.x + bounds.width * fraction - slider->style.thumb_radius;
  thumb.y = bounds.y + (bounds.height - thumb_size) * 0.5f;
  rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &thumb, thumb_color,
                                   slider->style.thumb_radius);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_slider_widget_event(void *widget, const M3InputEvent *event,
                                  M3Bool *out_handled) {
  M3Slider *slider;
  M3Scalar next_value;
  int rc;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_handled = M3_FALSE;

  slider = (M3Slider *)widget;

  if (slider->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    return M3_OK;
  }

  switch (event->type) {
  case M3_INPUT_POINTER_DOWN:
    if (slider->pressed == M3_TRUE) {
      return M3_ERR_STATE;
    }
    rc = m3_slider_value_from_x(slider, (M3Scalar)event->data.pointer.x,
                                &next_value);
    if (rc != M3_OK) {
      return rc;
    }
    rc = m3_slider_update_value(slider, next_value, M3_TRUE);
    if (rc != M3_OK) {
      return rc;
    }
    slider->pressed = M3_TRUE;
    *out_handled = M3_TRUE;
    return M3_OK;
  case M3_INPUT_POINTER_MOVE:
    if (slider->pressed == M3_FALSE) {
      return M3_OK;
    }
    rc = m3_slider_value_from_x(slider, (M3Scalar)event->data.pointer.x,
                                &next_value);
    if (rc != M3_OK) {
      return rc;
    }
    rc = m3_slider_update_value(slider, next_value, M3_TRUE);
    if (rc != M3_OK) {
      return rc;
    }
    *out_handled = M3_TRUE;
    return M3_OK;
  case M3_INPUT_POINTER_UP:
    if (slider->pressed == M3_FALSE) {
      return M3_OK;
    }
    slider->pressed = M3_FALSE;
    *out_handled = M3_TRUE;
    return M3_OK;
  default:
    return M3_OK;
  }
}

static int m3_slider_widget_get_semantics(void *widget,
                                          M3Semantics *out_semantics) {
  M3Slider *slider;

  if (widget == NULL || out_semantics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  slider = (M3Slider *)widget;
  out_semantics->role = M3_SEMANTIC_SLIDER;
  out_semantics->flags = 0;
  if (slider->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
  }
  if (slider->widget.flags & M3_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = slider->utf8_label;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return M3_OK;
}

static int m3_slider_widget_destroy(void *widget) {
  M3Slider *slider;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  slider = (M3Slider *)widget;
  memset(&slider->style, 0, sizeof(slider->style));
  slider->bounds.x = 0.0f;
  slider->bounds.y = 0.0f;
  slider->bounds.width = 0.0f;
  slider->bounds.height = 0.0f;
  slider->min_value = 0.0f;
  slider->max_value = 0.0f;
  slider->value = 0.0f;
  slider->step = 0.0f;
  slider->utf8_label = NULL;
  slider->utf8_len = 0;
  slider->pressed = M3_FALSE;
  slider->on_change = NULL;
  slider->on_change_ctx = NULL;
  slider->widget.ctx = NULL;
  slider->widget.vtable = NULL;
  slider->widget.handle.id = 0u;
  slider->widget.handle.generation = 0u;
  slider->widget.flags = 0u;
  return M3_OK;
}

static const M3WidgetVTable g_m3_slider_widget_vtable = {
    m3_slider_widget_measure,       m3_slider_widget_layout,
    m3_slider_widget_paint,         m3_slider_widget_event,
    m3_slider_widget_get_semantics, m3_slider_widget_destroy};

int M3_CALL m3_slider_style_init(M3SliderStyle *style) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->track_length = M3_SLIDER_DEFAULT_LENGTH;
  style->track_height = M3_SLIDER_DEFAULT_TRACK_HEIGHT;
  style->track_corner_radius = M3_SLIDER_DEFAULT_TRACK_CORNER_RADIUS;
  style->thumb_radius = M3_SLIDER_DEFAULT_THUMB_RADIUS;

  rc = m3_progress_color_set(&style->track_color, 0.82f, 0.82f, 0.82f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_color_set(&style->active_track_color, 0.26f, 0.52f, 0.96f,
                             1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_color_set(&style->thumb_color, 0.26f, 0.52f, 0.96f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_color_with_alpha(&style->track_color, 0.38f,
                                    &style->disabled_track_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_color_with_alpha(&style->active_track_color, 0.38f,
                                    &style->disabled_active_track_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_progress_color_with_alpha(&style->thumb_color, 0.38f,
                                    &style->disabled_thumb_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_slider_init(M3Slider *slider, const M3SliderStyle *style,
                           M3Scalar min_value, M3Scalar max_value,
                           M3Scalar value) {
  M3Scalar snapped;
  int rc;

  if (slider == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_slider_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_slider_validate_range(min_value, max_value);
  if (rc != M3_OK) {
    return rc;
  }
  if (value < min_value || value > max_value) {
    return M3_ERR_RANGE;
  }

  rc = m3_slider_snap_value(value, min_value, max_value, M3_SLIDER_DEFAULT_STEP,
                            &snapped);
  if (rc != M3_OK) {
    return rc;
  }

  memset(slider, 0, sizeof(*slider));
  slider->style = *style;
  slider->min_value = min_value;
  slider->max_value = max_value;
  slider->value = snapped;
  slider->step = M3_SLIDER_DEFAULT_STEP;
  slider->pressed = M3_FALSE;
  slider->widget.ctx = slider;
  slider->widget.vtable = &g_m3_slider_widget_vtable;
  slider->widget.handle.id = 0u;
  slider->widget.handle.generation = 0u;
  slider->widget.flags = M3_WIDGET_FLAG_FOCUSABLE;
  return M3_OK;
}

int M3_CALL m3_slider_set_value(M3Slider *slider, M3Scalar value) {
  M3Scalar snapped;
  int rc;

  if (slider == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_slider_validate_range(slider->min_value, slider->max_value);
  if (rc != M3_OK) {
    return rc;
  }
  if (value < slider->min_value || value > slider->max_value) {
    return M3_ERR_RANGE;
  }

  rc = m3_slider_snap_value(value, slider->min_value, slider->max_value,
                            slider->step, &snapped);
  if (rc != M3_OK) {
    return rc;
  }

  slider->value = snapped;
  return M3_OK;
}

int M3_CALL m3_slider_get_value(const M3Slider *slider, M3Scalar *out_value) {
  if (slider == NULL || out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_value = slider->value;
  return M3_OK;
}

int M3_CALL m3_slider_set_range(M3Slider *slider, M3Scalar min_value,
                                M3Scalar max_value) {
  M3Scalar snapped;
  int rc;

  if (slider == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_slider_validate_range(min_value, max_value);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_slider_validate_step(slider->step);
  if (rc != M3_OK) {
    return rc;
  }
  if (slider->step > 0.0f && slider->step > (max_value - min_value)) {
    return M3_ERR_RANGE;
  }

  slider->min_value = min_value;
  slider->max_value = max_value;
  rc = m3_slider_snap_value(slider->value, slider->min_value, slider->max_value,
                            slider->step, &snapped);
  if (rc != M3_OK) {
    return rc;
  }
  slider->value = snapped;
  return M3_OK;
}

int M3_CALL m3_slider_set_step(M3Slider *slider, M3Scalar step) {
  M3Scalar snapped;
  int rc;

  if (slider == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_slider_validate_step(step);
  if (rc != M3_OK) {
    return rc;
  }
  if (step > 0.0f && step > (slider->max_value - slider->min_value)) {
    return M3_ERR_RANGE;
  }

  slider->step = step;
  rc = m3_slider_snap_value(slider->value, slider->min_value, slider->max_value,
                            slider->step, &snapped);
  if (rc != M3_OK) {
    return rc;
  }
  slider->value = snapped;
  return M3_OK;
}

int M3_CALL m3_slider_set_style(M3Slider *slider, const M3SliderStyle *style) {
  int rc;

  if (slider == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_slider_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }

  slider->style = *style;
  return M3_OK;
}

int M3_CALL m3_slider_set_label(M3Slider *slider, const char *utf8_label,
                                m3_usize utf8_len) {
  if (slider == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8_label == NULL && utf8_len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  slider->utf8_label = utf8_label;
  slider->utf8_len = utf8_len;
  return M3_OK;
}

int M3_CALL m3_slider_set_on_change(M3Slider *slider,
                                    M3SliderOnChange on_change, void *ctx) {
  if (slider == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  slider->on_change = on_change;
  slider->on_change_ctx = ctx;
  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_progress_test_set_fail_point(m3_u32 fail_point) {
  g_m3_progress_test_fail_point = fail_point;
  return M3_OK;
}

int M3_CALL m3_progress_test_clear_fail_points(void) {
  g_m3_progress_test_fail_point = M3_PROGRESS_TEST_FAIL_NONE;
  g_m3_progress_test_color_set_calls = 0u;
  g_m3_progress_test_color_set_fail_after = 0u;
  g_m3_progress_test_color_alpha_calls = 0u;
  g_m3_progress_test_color_alpha_fail_after = 0u;
  return M3_OK;
}

int M3_CALL m3_progress_test_set_color_set_fail_after(m3_u32 call_count) {
  g_m3_progress_test_color_set_calls = 0u;
  g_m3_progress_test_color_set_fail_after = call_count;
  return M3_OK;
}

int M3_CALL m3_progress_test_set_color_alpha_fail_after(m3_u32 call_count) {
  g_m3_progress_test_color_alpha_calls = 0u;
  g_m3_progress_test_color_alpha_fail_after = call_count;
  return M3_OK;
}

int M3_CALL m3_progress_test_validate_color(const M3Color *color) {
  return m3_progress_validate_color(color);
}

int M3_CALL m3_progress_test_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                                       M3Scalar b, M3Scalar a) {
  return m3_progress_color_set(color, r, g, b, a);
}

int M3_CALL m3_progress_test_color_with_alpha(const M3Color *base,
                                              M3Scalar alpha,
                                              M3Color *out_color) {
  return m3_progress_color_with_alpha(base, alpha, out_color);
}

int M3_CALL m3_progress_test_validate_measure_spec(M3MeasureSpec spec) {
  return m3_progress_validate_measure_spec(spec);
}

int M3_CALL m3_progress_test_apply_measure(M3Scalar desired, M3MeasureSpec spec,
                                           M3Scalar *out_size) {
  return m3_progress_apply_measure(desired, spec, out_size);
}

int M3_CALL m3_progress_test_validate_rect(const M3Rect *rect) {
  return m3_progress_validate_rect(rect);
}

int M3_CALL m3_progress_test_validate_value01(M3Scalar value) {
  return m3_progress_validate_value01(value);
}

int M3_CALL
m3_progress_test_linear_validate_style(const M3LinearProgressStyle *style) {
  return m3_linear_progress_validate_style(style);
}

int M3_CALL m3_progress_test_linear_resolve_colors(
    const M3LinearProgress *progress, M3Color *out_track,
    M3Color *out_indicator) {
  return m3_linear_progress_resolve_colors(progress, out_track, out_indicator);
}

int M3_CALL
m3_progress_test_circular_validate_style(const M3CircularProgressStyle *style) {
  return m3_circular_progress_validate_style(style);
}

int M3_CALL m3_progress_test_circular_resolve_colors(
    const M3CircularProgress *progress, M3Color *out_track,
    M3Color *out_indicator) {
  return m3_circular_progress_resolve_colors(progress, out_track,
                                             out_indicator);
}

int M3_CALL m3_progress_test_circular_draw_arc(
    M3Gfx *gfx, M3Scalar cx, M3Scalar cy, M3Scalar radius, M3Scalar start_angle,
    M3Scalar end_angle, M3Color color, M3Scalar thickness, m3_u32 segments) {
  return m3_circular_progress_draw_arc(gfx, cx, cy, radius, start_angle,
                                       end_angle, color, thickness, segments);
}

int M3_CALL m3_progress_test_slider_validate_style(const M3SliderStyle *style) {
  return m3_slider_validate_style(style);
}

int M3_CALL m3_progress_test_slider_value_to_fraction(const M3Slider *slider,
                                                      M3Scalar *out_fraction) {
  return m3_slider_value_to_fraction(slider, out_fraction);
}

int M3_CALL m3_progress_test_slider_value_from_x(const M3Slider *slider,
                                                 M3Scalar x,
                                                 M3Scalar *out_value) {
  return m3_slider_value_from_x(slider, x, out_value);
}

int M3_CALL m3_progress_test_slider_snap_value(M3Scalar value,
                                               M3Scalar min_value,
                                               M3Scalar max_value,
                                               M3Scalar step,
                                               M3Scalar *out_value) {
  return m3_slider_snap_value(value, min_value, max_value, step, out_value);
}

int M3_CALL m3_progress_test_slider_resolve_colors(const M3Slider *slider,
                                                   M3Color *out_track,
                                                   M3Color *out_active,
                                                   M3Color *out_thumb) {
  return m3_slider_resolve_colors(slider, out_track, out_active, out_thumb);
}

int M3_CALL m3_progress_test_slider_update_value(M3Slider *slider,
                                                 M3Scalar next_value,
                                                 M3Bool notify) {
  return m3_slider_update_value(slider, next_value, notify);
}
#endif
