#include "m3/m3_progress.h"

#include <math.h>
#include <string.h>

#ifdef CMP_TESTING
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

static cmp_u32 g_m3_progress_test_fail_point = M3_PROGRESS_TEST_FAIL_NONE;
static cmp_u32 g_m3_progress_test_color_set_calls = 0u;
static cmp_u32 g_m3_progress_test_color_set_fail_after = 0u;
static cmp_u32 g_m3_progress_test_color_alpha_calls = 0u;
static cmp_u32 g_m3_progress_test_color_alpha_fail_after = 0u;

static CMPBool m3_progress_test_fail_point_match(cmp_u32 point) {
  if (g_m3_progress_test_fail_point != point) {
    return CMP_FALSE;
  }
  g_m3_progress_test_fail_point = M3_PROGRESS_TEST_FAIL_NONE;
  return CMP_TRUE;
}
#endif

#define M3_PROGRESS_PI 3.14159265358979323846f

static int m3_progress_validate_color(const CMPColor *color) {
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

static int m3_progress_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
                                 CMPScalar b, CMPScalar a) {
  if (color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  g_m3_progress_test_color_set_calls += 1u;
  if (m3_progress_test_fail_point_match(M3_PROGRESS_TEST_FAIL_COLOR_SET)) {
    return CMP_ERR_RANGE;
  }
  if (g_m3_progress_test_color_set_fail_after > 0u &&
      g_m3_progress_test_color_set_calls >=
          g_m3_progress_test_color_set_fail_after) {
    return CMP_ERR_RANGE;
  }
#endif
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
  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
  return CMP_OK;
}

static int m3_progress_color_with_alpha(const CMPColor *base, CMPScalar alpha,
                                        CMPColor *out_color) {
  int rc;

  if (base == NULL || out_color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  g_m3_progress_test_color_alpha_calls += 1u;
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_COLOR_WITH_ALPHA)) {
    return CMP_ERR_RANGE;
  }
  if (g_m3_progress_test_color_alpha_fail_after > 0u &&
      g_m3_progress_test_color_alpha_calls >=
          g_m3_progress_test_color_alpha_fail_after) {
    return CMP_ERR_RANGE;
  }
#endif
  if (!(alpha >= 0.0f && alpha <= 1.0f)) {
    return CMP_ERR_RANGE;
  }

  rc = m3_progress_validate_color(base);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_color = *base;
  out_color->a = out_color->a * alpha;
  return CMP_OK;
}

static int m3_progress_validate_measure_spec(CMPMeasureSpec spec) {
  if (spec.mode != CMP_MEASURE_UNSPECIFIED &&
      spec.mode != CMP_MEASURE_EXACTLY && spec.mode != CMP_MEASURE_AT_MOST) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_progress_apply_measure(CMPScalar desired, CMPMeasureSpec spec,
                                     CMPScalar *out_size) {
  int rc;

  if (out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_progress_validate_measure_spec(spec);
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

static int m3_progress_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_progress_validate_value01(CMPScalar value) {
  if (!(value >= 0.0f && value <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int
m3_linear_progress_validate_style(const M3LinearProgressStyle *style) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->height <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->min_width < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->corner_radius < 0.0f ||
      style->corner_radius > style->height * 0.5f) {
    return CMP_ERR_RANGE;
  }

  rc = m3_progress_validate_color(&style->track_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->indicator_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->disabled_track_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->disabled_indicator_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_linear_progress_resolve_colors(const M3LinearProgress *progress,
                                             CMPColor *out_track,
                                             CMPColor *out_indicator) {
  int rc;

  if (progress == NULL || out_track == NULL || out_indicator == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_LINEAR_RESOLVE_COLORS)) {
    return CMP_ERR_IO;
  }
#endif

  if (progress->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    *out_track = progress->style.disabled_track_color;
    *out_indicator = progress->style.disabled_indicator_color;
  } else {
    *out_track = progress->style.track_color;
    *out_indicator = progress->style.indicator_color;
  }

  rc = m3_progress_validate_color(out_track);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(out_indicator);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

static int m3_linear_progress_widget_measure(void *widget, CMPMeasureSpec width,
                                             CMPMeasureSpec height,
                                             CMPSize *out_size) {
  M3LinearProgress *progress;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  progress = (M3LinearProgress *)widget;
  rc = m3_linear_progress_validate_style(&progress->style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_progress_apply_measure(progress->style.min_width, width,
                                 &out_size->width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_apply_measure(progress->style.height, height,
                                 &out_size->height);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

static int m3_linear_progress_widget_layout(void *widget, CMPRect bounds) {
  M3LinearProgress *progress;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_progress_validate_rect(&bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  progress = (M3LinearProgress *)widget;
  progress->bounds = bounds;
  return CMP_OK;
}

static int m3_linear_progress_widget_paint(void *widget, CMPPaintContext *ctx) {
  M3LinearProgress *progress;
  CMPRect bounds;
  CMPRect fill;
  CMPColor track_color;
  CMPColor indicator_color;
  CMPScalar corner;
  CMPScalar fill_corner;
  CMPScalar fill_width;
  int rc;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable->draw_rect == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  progress = (M3LinearProgress *)widget;

  rc = m3_linear_progress_validate_style(&progress->style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_progress_validate_rect(&progress->bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_progress_validate_value01(progress->value);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_linear_progress_resolve_colors(progress, &track_color,
                                         &indicator_color);
  if (rc != CMP_OK) {
    return rc;
  }

  bounds = progress->bounds;
  corner = progress->style.corner_radius;
#ifdef CMP_TESTING
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_LINEAR_CORNER_RANGE)) {
    corner = -1.0f;
  }
#endif
  if (corner > bounds.height * 0.5f) {
    corner = bounds.height * 0.5f;
  }
  if (corner < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (track_color.a > 0.0f) {
    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &bounds, track_color,
                                     corner);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  fill_width = bounds.width * progress->value;
  if (fill_width > 0.0f && indicator_color.a > 0.0f) {
    fill = bounds;
    fill.width = fill_width;
    fill_corner = corner;
#ifdef CMP_TESTING
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
#ifdef CMP_TESTING
    if (m3_progress_test_fail_point_match(
            M3_PROGRESS_TEST_FAIL_LINEAR_FILL_CORNER_RANGE)) {
      fill_corner = -1.0f;
    }
#endif
    if (fill_corner < 0.0f) {
      return CMP_ERR_RANGE;
    }
    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &fill, indicator_color,
                                     fill_corner);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  return CMP_OK;
}

static int m3_linear_progress_widget_event(void *widget,
                                           const CMPInputEvent *event,
                                           CMPBool *out_handled) {
  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  CMP_UNUSED(widget);
  CMP_UNUSED(event);
  *out_handled = CMP_FALSE;
  return CMP_OK;
}

static int
m3_linear_progress_widget_get_semantics(void *widget,
                                        CMPSemantics *out_semantics) {
  M3LinearProgress *progress;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_semantics, 0, sizeof(*out_semantics));

  progress = (M3LinearProgress *)widget;
  memset(out_semantics, 0, sizeof(*out_semantics));
  out_semantics->role = CMP_SEMANTIC_NONE;
  out_semantics->flags = 0;
  if (progress->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED;
  }
  out_semantics->utf8_label = progress->utf8_label;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_linear_progress_widget_destroy(void *widget) {
  M3LinearProgress *progress;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
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
  return CMP_OK;
}

static const CMPWidgetVTable g_m3_linear_progress_widget_vtable = {
    m3_linear_progress_widget_measure,       m3_linear_progress_widget_layout,
    m3_linear_progress_widget_paint,         m3_linear_progress_widget_event,
    m3_linear_progress_widget_get_semantics, m3_linear_progress_widget_destroy};

int CMP_CALL m3_linear_progress_style_init(M3LinearProgressStyle *style) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->height = M3_LINEAR_PROGRESS_DEFAULT_HEIGHT;
  style->min_width = M3_LINEAR_PROGRESS_DEFAULT_MIN_WIDTH;
  style->corner_radius = M3_LINEAR_PROGRESS_DEFAULT_CORNER_RADIUS;

  rc = m3_progress_color_set(&style->track_color, 0.82f, 0.82f, 0.82f, 1.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc =
      m3_progress_color_set(&style->indicator_color, 0.26f, 0.52f, 0.96f, 1.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_color_with_alpha(&style->track_color, 0.38f,
                                    &style->disabled_track_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_color_with_alpha(&style->indicator_color, 0.38f,
                                    &style->disabled_indicator_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL m3_linear_progress_init(M3LinearProgress *progress,
                                     const M3LinearProgressStyle *style,
                                     CMPScalar value) {
  int rc;

  if (progress == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_linear_progress_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_validate_value01(value);
  if (rc != CMP_OK) {
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
  return CMP_OK;
}

int CMP_CALL m3_linear_progress_set_value(M3LinearProgress *progress,
                                          CMPScalar value) {
  int rc;

  if (progress == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_progress_validate_value01(value);
  if (rc != CMP_OK) {
    return rc;
  }

  progress->value = value;
  return CMP_OK;
}

int CMP_CALL m3_linear_progress_get_value(const M3LinearProgress *progress,
                                          CMPScalar *out_value) {
  if (progress == NULL || out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_value = progress->value;
  return CMP_OK;
}

int CMP_CALL m3_linear_progress_set_style(M3LinearProgress *progress,
                                          const M3LinearProgressStyle *style) {
  int rc;

  if (progress == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_linear_progress_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  progress->style = *style;
  return CMP_OK;
}

int CMP_CALL m3_linear_progress_set_label(M3LinearProgress *progress,
                                          const char *utf8_label,
                                          cmp_usize utf8_len) {
  if (progress == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8_label == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  progress->utf8_label = utf8_label;
  progress->utf8_len = utf8_len;
  return CMP_OK;
}

static int
m3_circular_progress_validate_style(const M3CircularProgressStyle *style) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->diameter <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->thickness <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->thickness > style->diameter * 0.5f) {
    return CMP_ERR_RANGE;
  }
  if (style->segments < 3u) {
    return CMP_ERR_RANGE;
  }

  rc = m3_progress_validate_color(&style->track_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->indicator_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->disabled_track_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->disabled_indicator_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int
m3_circular_progress_resolve_colors(const M3CircularProgress *progress,
                                    CMPColor *out_track,
                                    CMPColor *out_indicator) {
  int rc;

  if (progress == NULL || out_track == NULL || out_indicator == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_CIRCULAR_RESOLVE_COLORS)) {
    return CMP_ERR_IO;
  }
#endif

  if (progress->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    *out_track = progress->style.disabled_track_color;
    *out_indicator = progress->style.disabled_indicator_color;
  } else {
    *out_track = progress->style.track_color;
    *out_indicator = progress->style.indicator_color;
  }

  rc = m3_progress_validate_color(out_track);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(out_indicator);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

static int m3_circular_progress_draw_arc(CMPGfx *gfx, CMPScalar cx,
                                         CMPScalar cy, CMPScalar radius,
                                         CMPScalar start_angle,
                                         CMPScalar end_angle, CMPColor color,
                                         CMPScalar thickness,
                                         cmp_u32 segments) {
  CMPScalar full;
  CMPScalar step;
  CMPScalar angle;
  CMPScalar next;
  CMPScalar x0;
  CMPScalar y0;
  CMPScalar x1;
  CMPScalar y1;
  cmp_u32 i;
  int rc;

  if (gfx == NULL || gfx->vtable == NULL || gfx->vtable->draw_line == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }
  if (segments < 1u) {
    return CMP_ERR_RANGE;
  }
  if (thickness <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (end_angle <= start_angle) {
    return CMP_OK;
  }

  full = M3_PROGRESS_PI * 2.0f;
  step = full / (CMPScalar)segments;
  angle = start_angle;
  for (i = 0u; i < segments && angle < end_angle; ++i) {
    next = angle + step;
    if (next > end_angle) {
      next = end_angle;
    }
    x0 = cx + (CMPScalar)(cos(angle) * radius);
    y0 = cy + (CMPScalar)(sin(angle) * radius);
    x1 = cx + (CMPScalar)(cos(next) * radius);
    y1 = cy + (CMPScalar)(sin(next) * radius);
    rc = gfx->vtable->draw_line(gfx->ctx, x0, y0, x1, y1, color, thickness);
    if (rc != CMP_OK) {
      return rc;
    }
    angle = next;
  }
  return CMP_OK;
}

static int m3_circular_progress_widget_measure(void *widget,
                                               CMPMeasureSpec width,
                                               CMPMeasureSpec height,
                                               CMPSize *out_size) {
  M3CircularProgress *progress;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  progress = (M3CircularProgress *)widget;
  rc = m3_circular_progress_validate_style(&progress->style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_progress_apply_measure(progress->style.diameter, width,
                                 &out_size->width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_apply_measure(progress->style.diameter, height,
                                 &out_size->height);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

static int m3_circular_progress_widget_layout(void *widget, CMPRect bounds) {
  M3CircularProgress *progress;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_progress_validate_rect(&bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  progress = (M3CircularProgress *)widget;
  progress->bounds = bounds;
  return CMP_OK;
}

static int m3_circular_progress_widget_paint(void *widget,
                                             CMPPaintContext *ctx) {
  M3CircularProgress *progress;
  CMPColor track_color;
  CMPColor indicator_color;
  CMPRect bounds;
  CMPScalar min_side;
  CMPScalar radius;
  CMPScalar cx;
  CMPScalar cy;
  CMPScalar end_angle;
  int rc;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable->draw_line == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  progress = (M3CircularProgress *)widget;

  rc = m3_circular_progress_validate_style(&progress->style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_progress_validate_rect(&progress->bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_progress_validate_value01(progress->value);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_circular_progress_resolve_colors(progress, &track_color,
                                           &indicator_color);
  if (rc != CMP_OK) {
    return rc;
  }

  bounds = progress->bounds;
  min_side = bounds.width;
  if (bounds.height < min_side) {
    min_side = bounds.height;
  }

  if (min_side <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  radius = (min_side - progress->style.thickness) * 0.5f;
  if (radius <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  cx = bounds.x + bounds.width * 0.5f;
  cy = bounds.y + bounds.height * 0.5f;

  if (track_color.a > 0.0f) {
    end_angle = progress->style.start_angle + (M3_PROGRESS_PI * 2.0f);
    rc = m3_circular_progress_draw_arc(
        ctx->gfx, cx, cy, radius, progress->style.start_angle, end_angle,
        track_color, progress->style.thickness, progress->style.segments);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  if (progress->value > 0.0f && indicator_color.a > 0.0f) {
    end_angle =
        progress->style.start_angle + (M3_PROGRESS_PI * 2.0f * progress->value);
    rc = m3_circular_progress_draw_arc(
        ctx->gfx, cx, cy, radius, progress->style.start_angle, end_angle,
        indicator_color, progress->style.thickness, progress->style.segments);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  return CMP_OK;
}

static int m3_circular_progress_widget_event(void *widget,
                                             const CMPInputEvent *event,
                                             CMPBool *out_handled) {
  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  CMP_UNUSED(widget);
  CMP_UNUSED(event);
  *out_handled = CMP_FALSE;
  return CMP_OK;
}

static int
m3_circular_progress_widget_get_semantics(void *widget,
                                          CMPSemantics *out_semantics) {
  M3CircularProgress *progress;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_semantics, 0, sizeof(*out_semantics));

  progress = (M3CircularProgress *)widget;
  memset(out_semantics, 0, sizeof(*out_semantics));
  out_semantics->role = CMP_SEMANTIC_NONE;
  out_semantics->flags = 0;
  if (progress->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED;
  }
  out_semantics->utf8_label = progress->utf8_label;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_circular_progress_widget_destroy(void *widget) {
  M3CircularProgress *progress;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
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
  return CMP_OK;
}

static const CMPWidgetVTable g_m3_circular_progress_widget_vtable = {
    m3_circular_progress_widget_measure,
    m3_circular_progress_widget_layout,
    m3_circular_progress_widget_paint,
    m3_circular_progress_widget_event,
    m3_circular_progress_widget_get_semantics,
    m3_circular_progress_widget_destroy};

int CMP_CALL m3_circular_progress_style_init(M3CircularProgressStyle *style) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->diameter = M3_CIRCULAR_PROGRESS_DEFAULT_DIAMETER;
  style->thickness = M3_CIRCULAR_PROGRESS_DEFAULT_THICKNESS;
  style->start_angle = M3_CIRCULAR_PROGRESS_DEFAULT_START_ANGLE;
  style->segments = M3_CIRCULAR_PROGRESS_DEFAULT_SEGMENTS;

  rc = m3_progress_color_set(&style->track_color, 0.82f, 0.82f, 0.82f, 1.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc =
      m3_progress_color_set(&style->indicator_color, 0.26f, 0.52f, 0.96f, 1.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_color_with_alpha(&style->track_color, 0.38f,
                                    &style->disabled_track_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_color_with_alpha(&style->indicator_color, 0.38f,
                                    &style->disabled_indicator_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL m3_circular_progress_init(M3CircularProgress *progress,
                                       const M3CircularProgressStyle *style,
                                       CMPScalar value) {
  int rc;

  if (progress == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_circular_progress_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_validate_value01(value);
  if (rc != CMP_OK) {
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
  return CMP_OK;
}

int CMP_CALL m3_circular_progress_set_value(M3CircularProgress *progress,
                                            CMPScalar value) {
  int rc;

  if (progress == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_progress_validate_value01(value);
  if (rc != CMP_OK) {
    return rc;
  }

  progress->value = value;
  return CMP_OK;
}

int CMP_CALL m3_circular_progress_get_value(const M3CircularProgress *progress,
                                            CMPScalar *out_value) {
  if (progress == NULL || out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_value = progress->value;
  return CMP_OK;
}

int CMP_CALL m3_circular_progress_set_style(
    M3CircularProgress *progress, const M3CircularProgressStyle *style) {
  int rc;

  if (progress == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_circular_progress_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  progress->style = *style;
  return CMP_OK;
}

int CMP_CALL m3_circular_progress_set_label(M3CircularProgress *progress,
                                            const char *utf8_label,
                                            cmp_usize utf8_len) {
  if (progress == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8_label == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  progress->utf8_label = utf8_label;
  progress->utf8_len = utf8_len;
  return CMP_OK;
}

static int m3_slider_validate_style(const M3SliderStyle *style) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->track_length <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->track_height <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->track_corner_radius < 0.0f ||
      style->track_corner_radius > style->track_height * 0.5f) {
    return CMP_ERR_RANGE;
  }
  if (style->thumb_radius <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  rc = m3_progress_validate_color(&style->track_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->active_track_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->thumb_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->disabled_track_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->disabled_active_track_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&style->disabled_thumb_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_slider_validate_range(CMPScalar min_value, CMPScalar max_value) {
  if (!(max_value > min_value)) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_slider_validate_step(CMPScalar step) {
  if (step < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_slider_snap_value(CMPScalar value, CMPScalar min_value,
                                CMPScalar max_value, CMPScalar step,
                                CMPScalar *out_value) {
  CMPScalar clamped;
  CMPScalar range;
  CMPScalar count;
  cmp_i32 steps;
  CMPBool force_negative;

  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (step < 0.0f) {
    return CMP_ERR_RANGE;
  }

  range = max_value - min_value;
  if (range <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (step > 0.0f && step > range) {
    return CMP_ERR_RANGE;
  }

  clamped = value;
  if (clamped < min_value) {
    clamped = min_value;
  }
  if (clamped > max_value) {
    clamped = max_value;
  }

  force_negative = CMP_FALSE;
#ifdef CMP_TESTING
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_COUNT_NEGATIVE)) {
    force_negative = CMP_TRUE;
  }
#endif
  if (step <= 0.0f) {
    if (force_negative) {
      return CMP_ERR_RANGE;
    }
    *out_value = clamped;
    return CMP_OK;
  }

  count = (clamped - min_value) / step;
  if (force_negative) {
    count = -1.0f;
  }
  if (count < 0.0f) {
    return CMP_ERR_RANGE;
  }
  steps = (cmp_i32)(count + 0.5f);
  clamped = min_value + ((CMPScalar)steps) * step;
#ifdef CMP_TESTING
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
  return CMP_OK;
}

static int m3_slider_value_to_fraction(const M3Slider *slider,
                                       CMPScalar *out_fraction) {
  CMPScalar range;

  if (slider == NULL || out_fraction == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  range = slider->max_value - slider->min_value;
  if (range <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (slider->value < slider->min_value || slider->value > slider->max_value) {
    return CMP_ERR_RANGE;
  }

  *out_fraction = (slider->value - slider->min_value) / range;
#ifdef CMP_TESTING
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
  return CMP_OK;
}

static int m3_slider_value_from_x(const M3Slider *slider, CMPScalar x,
                                  CMPScalar *out_value) {
  CMPScalar range;
  CMPScalar fraction;
  int rc;

  if (slider == NULL || out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  range = slider->max_value - slider->min_value;
  if (range <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (slider->bounds.width <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  fraction = (x - slider->bounds.x) / slider->bounds.width;
#ifdef CMP_TESTING
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
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_slider_resolve_colors(const M3Slider *slider, CMPColor *out_track,
                                    CMPColor *out_active, CMPColor *out_thumb) {
  CMPColor track;
  CMPColor active;
  CMPColor thumb;
  int rc;

  if (slider == NULL || out_track == NULL || out_active == NULL ||
      out_thumb == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_RESOLVE_COLORS)) {
    return CMP_ERR_IO;
  }
#endif

  if (slider->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    track = slider->style.disabled_track_color;
    active = slider->style.disabled_active_track_color;
    thumb = slider->style.disabled_thumb_color;
  } else {
    track = slider->style.track_color;
    active = slider->style.active_track_color;
    thumb = slider->style.thumb_color;
  }

  rc = m3_progress_validate_color(&track);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&active);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_validate_color(&thumb);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_track = track;
  *out_active = active;
  *out_thumb = thumb;
  return CMP_OK;
}

static int m3_slider_update_value(M3Slider *slider, CMPScalar next_value,
                                  CMPBool notify) {
  CMPScalar prev_value;
  int rc = CMP_OK;

  if (slider == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  prev_value = slider->value;
  slider->value = next_value;
  if (notify == CMP_TRUE && slider->on_change != NULL) {
    rc = slider->on_change(slider->on_change_ctx, slider, next_value);
    if (rc != CMP_OK) {
      slider->value = prev_value;
      return rc;
    }
  }
  return CMP_OK;
}

static int m3_slider_widget_measure(void *widget, CMPMeasureSpec width,
                                    CMPMeasureSpec height, CMPSize *out_size) {
  M3Slider *slider;
  CMPScalar desired_height;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  slider = (M3Slider *)widget;
  rc = m3_slider_validate_style(&slider->style);
  if (rc != CMP_OK) {
    return rc;
  }

  desired_height = slider->style.track_height;
  if (desired_height < slider->style.thumb_radius * 2.0f) {
    desired_height = slider->style.thumb_radius * 2.0f;
  }

  rc = m3_progress_apply_measure(slider->style.track_length, width,
                                 &out_size->width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_apply_measure(desired_height, height, &out_size->height);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

static int m3_slider_widget_layout(void *widget, CMPRect bounds) {
  M3Slider *slider;
  int rc = CMP_OK;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_progress_validate_rect(&bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  slider = (M3Slider *)widget;
  slider->bounds = bounds;
  return CMP_OK;
}

static int m3_slider_widget_paint(void *widget, CMPPaintContext *ctx) {
  M3Slider *slider;
  CMPRect bounds;
  CMPRect track;
  CMPRect active;
  CMPRect thumb;
  CMPColor track_color;
  CMPColor active_color = {0};
  CMPColor thumb_color = {0};
  CMPScalar fraction;
  CMPScalar corner;
  CMPScalar thumb_size;
  CMPScalar current_track_height = 0.0f;
  CMPScalar current_corner;
  CMPScalar current_thumb_w;
  CMPScalar current_thumb_h;
  int rc = CMP_OK;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable->draw_rect == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  slider = (M3Slider *)widget;

  rc = m3_slider_validate_style(&slider->style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_progress_validate_rect(&slider->bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_slider_validate_range(slider->min_value, slider->max_value);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_slider_validate_step(slider->step);
  if (rc != CMP_OK) {
    return rc;
  }
  if (slider->step > 0.0f &&
      slider->step > (slider->max_value - slider->min_value)) {
    return CMP_ERR_RANGE;
  }

  rc = m3_slider_value_to_fraction(slider, &fraction);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_slider_resolve_colors(slider, &track_color, &active_color,
                                &thumb_color);
  if (rc != CMP_OK) {
    return rc;
  }

  bounds = slider->bounds;
  if (bounds.width <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  track.height = slider->style.track_height;
  track.width = bounds.width;
  track.x = bounds.x;
  track.y = bounds.y + (bounds.height - track.height) * 0.5f;
  current_corner = slider->style.track_corner_radius;
  if (slider->style.is_media_slider && slider->pressed) {
    current_corner = current_track_height * 0.5f;
  }
  corner = current_corner;
#ifdef CMP_TESTING
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_CORNER_CLAMP)) {
    corner = track.height;
  }
#endif
  if (corner > track.height * 0.5f) {
    corner = track.height * 0.5f;
  }
#ifdef CMP_TESTING
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_CORNER_RANGE)) {
    corner = -1.0f;
  }
#endif
  if (corner < 0.0f) {
    return CMP_ERR_RANGE;
  }

  rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &track, track_color, corner);
  if (rc != CMP_OK) {
    return rc;
  }

  active = track;
  active.width = bounds.width * fraction;
  if (active.width > 0.0f) {
    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &active, active_color,
                                     corner);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  thumb_size = slider->style.thumb_radius * 2.0f;
  if (slider->style.is_media_slider) {
    current_thumb_w = slider->style.thumb_width;
    current_thumb_h = current_track_height;
  } else {
    current_thumb_w = thumb_size;
    current_thumb_h = thumb_size;
  }
  thumb.width = current_thumb_w;
  thumb.height = current_thumb_h;
  thumb.x = bounds.x + bounds.width * fraction - slider->style.thumb_radius;
  thumb.y = bounds.y + (bounds.height - thumb_size) * 0.5f;
  rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &thumb, thumb_color,
                                   slider->style.thumb_radius);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_slider_widget_event(void *widget, const CMPInputEvent *event,
                                  CMPBool *out_handled) {
  M3Slider *slider;
  CMPScalar next_value;
  int rc = CMP_OK;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;

  slider = (M3Slider *)widget;

  if (slider->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    return CMP_OK;
  }

  switch (event->type) {
  case CMP_INPUT_POINTER_DOWN:
    if (slider->pressed == CMP_TRUE) {
      return CMP_ERR_STATE;
    }
    rc = m3_slider_value_from_x(
        slider, (CMPScalar)(CMPScalar)event->data.pointer.x, &next_value);
    if (rc != CMP_OK) {
      return rc;
    }
    rc = m3_slider_update_value(slider, next_value, CMP_TRUE);
    if (rc != CMP_OK) {
      return rc;
    }
    slider->pressed = CMP_TRUE;
    *out_handled = CMP_TRUE;
    return CMP_OK;
  case CMP_INPUT_POINTER_MOVE:
    if (slider->pressed == CMP_FALSE) {
      return CMP_OK;
    }
    rc = m3_slider_value_from_x(
        slider, (CMPScalar)(CMPScalar)event->data.pointer.x, &next_value);
    if (rc != CMP_OK) {
      return rc;
    }
    rc = m3_slider_update_value(slider, next_value, CMP_TRUE);
    if (rc != CMP_OK) {
      return rc;
    }
    *out_handled = CMP_TRUE;
    return CMP_OK;
  case CMP_INPUT_POINTER_UP:
    if (slider->pressed == CMP_FALSE) {
      return CMP_OK;
    }
    slider->pressed = CMP_FALSE;
    *out_handled = CMP_TRUE;
    return CMP_OK;
  default:
    return CMP_OK;
  }
}

static int m3_slider_widget_get_semantics(void *widget,
                                          CMPSemantics *out_semantics) {
  M3Slider *slider = NULL;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_semantics, 0, sizeof(*out_semantics));

  slider = (M3Slider *)widget;
  memset(out_semantics, 0, sizeof(*out_semantics));
  out_semantics->role = CMP_SEMANTIC_SLIDER;
  out_semantics->flags = 0;
  if (slider->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED;
  }
  if (slider->widget.flags & CMP_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = slider->utf8_label;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  out_semantics->range_min = slider->min_value;
  out_semantics->range_max = slider->max_value;
  out_semantics->range_value = slider->value;
  return CMP_OK;
}

static int m3_slider_widget_destroy(void *widget) {
  M3Slider *slider;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
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
  slider->pressed = CMP_FALSE;
  slider->on_change = NULL;
  slider->on_change_ctx = NULL;
  slider->widget.ctx = NULL;
  slider->widget.vtable = NULL;
  slider->widget.handle.id = 0u;
  slider->widget.handle.generation = 0u;
  slider->widget.flags = 0u;
  return CMP_OK;
}

static const CMPWidgetVTable g_m3_slider_widget_vtable = {
    m3_slider_widget_measure,       m3_slider_widget_layout,
    m3_slider_widget_paint,         m3_slider_widget_event,
    m3_slider_widget_get_semantics, m3_slider_widget_destroy};

int CMP_CALL m3_slider_style_init(M3SliderStyle *style) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->track_length = M3_SLIDER_DEFAULT_LENGTH;
  style->track_height = M3_SLIDER_DEFAULT_TRACK_HEIGHT;
  style->track_corner_radius = M3_SLIDER_DEFAULT_TRACK_CORNER_RADIUS;
  style->thumb_radius = M3_SLIDER_DEFAULT_THUMB_RADIUS;

  rc = m3_progress_color_set(&style->track_color, 0.82f, 0.82f, 0.82f, 1.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_color_set(&style->active_track_color, 0.26f, 0.52f, 0.96f,
                             1.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_color_set(&style->thumb_color, 0.26f, 0.52f, 0.96f, 1.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_color_with_alpha(&style->track_color, 0.38f,
                                    &style->disabled_track_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_color_with_alpha(&style->active_track_color, 0.38f,
                                    &style->disabled_active_track_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_progress_color_with_alpha(&style->thumb_color, 0.38f,
                                    &style->disabled_thumb_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL m3_slider_init(M3Slider *slider, const M3SliderStyle *style,
                            CMPScalar min_value, CMPScalar max_value,
                            CMPScalar value) {
  CMPScalar snapped;
  int rc;

  if (slider == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_slider_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_slider_validate_range(min_value, max_value);
  if (rc != CMP_OK) {
    return rc;
  }
  if (value < min_value || value > max_value) {
    return CMP_ERR_RANGE;
  }

  rc = m3_slider_snap_value(value, min_value, max_value, M3_SLIDER_DEFAULT_STEP,
                            &snapped);
  if (rc != CMP_OK) {
    return rc;
  }

  memset(slider, 0, sizeof(*slider));
  slider->style = *style;
  slider->min_value = min_value;
  slider->max_value = max_value;
  slider->value = snapped;
  slider->step = M3_SLIDER_DEFAULT_STEP;
  slider->pressed = CMP_FALSE;
  slider->widget.ctx = slider;
  slider->widget.vtable = &g_m3_slider_widget_vtable;
  slider->widget.handle.id = 0u;
  slider->widget.handle.generation = 0u;
  slider->widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;
  return CMP_OK;
}

int CMP_CALL m3_slider_set_value(M3Slider *slider, CMPScalar value) {
  CMPScalar snapped;
  int rc;

  if (slider == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_slider_validate_range(slider->min_value, slider->max_value);
  if (rc != CMP_OK) {
    return rc;
  }
  if (value < slider->min_value || value > slider->max_value) {
    return CMP_ERR_RANGE;
  }

  rc = m3_slider_snap_value(value, slider->min_value, slider->max_value,
                            slider->step, &snapped);
  if (rc != CMP_OK) {
    return rc;
  }

  slider->value = snapped;
  return CMP_OK;
}

int CMP_CALL m3_slider_get_value(const M3Slider *slider, CMPScalar *out_value) {
  if (slider == NULL || out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_value = slider->value;
  return CMP_OK;
}

int CMP_CALL m3_slider_set_range(M3Slider *slider, CMPScalar min_value,
                                 CMPScalar max_value) {
  CMPScalar snapped;
  int rc;

  if (slider == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_slider_validate_range(min_value, max_value);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_slider_validate_step(slider->step);
  if (rc != CMP_OK) {
    return rc;
  }
  if (slider->step > 0.0f && slider->step > (max_value - min_value)) {
    return CMP_ERR_RANGE;
  }

  slider->min_value = min_value;
  slider->max_value = max_value;
  rc = m3_slider_snap_value(slider->value, slider->min_value, slider->max_value,
                            slider->step, &snapped);
  if (rc != CMP_OK) {
    return rc;
  }
  slider->value = snapped;
  return CMP_OK;
}

int CMP_CALL m3_slider_set_step(M3Slider *slider, CMPScalar step) {
  CMPScalar snapped;
  int rc;

  if (slider == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_slider_validate_step(step);
  if (rc != CMP_OK) {
    return rc;
  }
  if (step > 0.0f && step > (slider->max_value - slider->min_value)) {
    return CMP_ERR_RANGE;
  }

  slider->step = step;
  rc = m3_slider_snap_value(slider->value, slider->min_value, slider->max_value,
                            slider->step, &snapped);
  if (rc != CMP_OK) {
    return rc;
  }
  slider->value = snapped;
  return CMP_OK;
}

int CMP_CALL m3_slider_set_style(M3Slider *slider, const M3SliderStyle *style) {
  int rc;

  if (slider == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_slider_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  slider->style = *style;
  return CMP_OK;
}

int CMP_CALL m3_slider_set_label(M3Slider *slider, const char *utf8_label,
                                 cmp_usize utf8_len) {
  if (slider == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8_label == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  slider->utf8_label = utf8_label;
  slider->utf8_len = utf8_len;
  return CMP_OK;
}

int CMP_CALL m3_slider_set_on_change(M3Slider *slider,
                                     CMPSliderOnChange on_change, void *ctx) {
  if (slider == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  slider->on_change = on_change;
  slider->on_change_ctx = ctx;
  return CMP_OK;
}

static int m3_range_slider_widget_measure(void *widget, CMPMeasureSpec width,
                                          CMPMeasureSpec height,
                                          CMPSize *out_size) {
  M3RangeSlider *slider = (M3RangeSlider *)widget;
  int rc;

  if (slider == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_slider_validate_style(&slider->style);
  if (rc != CMP_OK) {
    return rc;
  }

  out_size->width = slider->style.track_length;
  out_size->height = slider->style.thumb_radius * 2.0f;

  if (width.mode == CMP_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == CMP_MEASURE_AT_MOST &&
             out_size->width > width.size) {
    out_size->width = width.size;
  }
  if (height.mode == CMP_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == CMP_MEASURE_AT_MOST &&
             out_size->height > height.size) {
    out_size->height = height.size;
  }

  return CMP_OK;
}

static int m3_range_slider_widget_layout(void *widget, CMPRect bounds) {
  M3RangeSlider *slider = (M3RangeSlider *)widget;
  if (slider == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  slider->bounds = bounds;
  return CMP_OK;
}

static int m3_range_slider_widget_paint(void *widget, CMPPaintContext *ctx) {
  M3RangeSlider *slider = (M3RangeSlider *)widget;
  CMPColor track_color, active_color, thumb_color;
  CMPRect inactive_track, active_track;
  CMPScalar range, fraction_start, fraction_end;
  CMPScalar track_y;
  int rc;

  if (slider == NULL || ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx == NULL || ctx->gfx->vtable == NULL ||
      ctx->gfx->vtable->draw_rect == NULL) {
    return CMP_OK;
  }

  rc = m3_slider_validate_style(&slider->style);
  if (rc != CMP_OK)
    return rc;
  rc = m3_slider_validate_range(slider->min_value, slider->max_value);
  if (rc != CMP_OK)
    return rc;

  if (slider->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    track_color = slider->style.disabled_track_color;
    active_color = slider->style.disabled_active_track_color;
    thumb_color = slider->style.disabled_thumb_color;
  } else {
    track_color = slider->style.track_color;
    active_color = slider->style.active_track_color;
    thumb_color = slider->style.thumb_color;
  }

  range = slider->max_value - slider->min_value;
  fraction_start = (slider->start_value - slider->min_value) / range;
  fraction_end = (slider->end_value - slider->min_value) / range;

  track_y = slider->bounds.y +
            (slider->bounds.height - slider->style.track_height) * 0.5f;

  inactive_track.x = slider->bounds.x + slider->style.thumb_radius;
  inactive_track.y = track_y;
  inactive_track.width =
      slider->bounds.width - slider->style.thumb_radius * 2.0f;
  inactive_track.height = slider->style.track_height;

  ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &inactive_track, track_color,
                              slider->style.track_corner_radius);

  active_track.x = inactive_track.x + inactive_track.width * fraction_start;
  active_track.y = track_y;
  active_track.width = inactive_track.width * (fraction_end - fraction_start);
  active_track.height = slider->style.track_height;

  ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &active_track, active_color,
                              slider->style.track_corner_radius);

  {
    CMPRect thumb_rect;
    thumb_rect.width = slider->style.thumb_radius * 2.0f;
    thumb_rect.height = slider->style.thumb_radius * 2.0f;

    thumb_rect.x = active_track.x - slider->style.thumb_radius;
    thumb_rect.y =
        slider->bounds.y + (slider->bounds.height - thumb_rect.height) * 0.5f;
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &thumb_rect, thumb_color,
                                slider->style.thumb_radius);

    thumb_rect.x =
        active_track.x + active_track.width - slider->style.thumb_radius;
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &thumb_rect, thumb_color,
                                slider->style.thumb_radius);
  }

  return CMP_OK;
}

static int m3_range_slider_widget_event(void *widget,
                                        const CMPInputEvent *event,
                                        CMPBool *out_handled) {
  M3RangeSlider *slider = (M3RangeSlider *)widget;
  CMPScalar range, fraction, new_val;
  CMPScalar track_w, dx;

  if (slider == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;

  if (slider->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    return CMP_OK;
  }

  range = slider->max_value - slider->min_value;
  if (range <= 0.0f)
    return CMP_OK;

  track_w = slider->bounds.width - slider->style.thumb_radius * 2.0f;
  if (track_w <= 0.0f)
    return CMP_OK;

  if (event->type == CMP_INPUT_POINTER_DOWN) {
    if ((CMPScalar)event->data.pointer.x >= slider->bounds.x &&
        (CMPScalar)event->data.pointer.x <=
            slider->bounds.x + slider->bounds.width &&
        (CMPScalar)event->data.pointer.y >= slider->bounds.y &&
        (CMPScalar)event->data.pointer.y <=
            slider->bounds.y + slider->bounds.height) {

      *out_handled = CMP_TRUE;
      dx = (CMPScalar)event->data.pointer.x -
           (slider->bounds.x + slider->style.thumb_radius);
      fraction = dx / track_w;
      if (fraction < 0.0f)
        fraction = 0.0f;
      if (fraction > 1.0f)
        fraction = 1.0f;

      m3_slider_snap_value(slider->min_value + fraction * range,
                           slider->min_value, slider->max_value, slider->step,
                           &new_val);

      {
        CMPScalar d_start = new_val - slider->start_value;
        CMPScalar d_end;
        if (d_start < 0.0f)
          d_start = -d_start;
        d_end = new_val - slider->end_value;
        if (d_end < 0.0f)
          d_end = -d_end;

        if (d_start <= d_end) {
          slider->is_dragging_start = CMP_TRUE;
          slider->start_value = new_val;
        } else {
          slider->is_dragging_end = CMP_TRUE;
          slider->end_value = new_val;
        }
      }

      if (slider->on_change) {
        slider->on_change(slider->on_change_ctx, slider, slider->start_value,
                          slider->end_value);
      }
    }
  } else if (event->type == CMP_INPUT_POINTER_MOVE) {
    if (slider->is_dragging_start || slider->is_dragging_end) {
      *out_handled = CMP_TRUE;
      dx = (CMPScalar)event->data.pointer.x -
           (slider->bounds.x + slider->style.thumb_radius);
      fraction = dx / track_w;
      if (fraction < 0.0f)
        fraction = 0.0f;
      if (fraction > 1.0f)
        fraction = 1.0f;

      m3_slider_snap_value(slider->min_value + fraction * range,
                           slider->min_value, slider->max_value, slider->step,
                           &new_val);

      if (slider->is_dragging_start) {
        slider->start_value = new_val;
        if (slider->start_value > slider->end_value) {
          slider->start_value = slider->end_value;
        }
      } else {
        slider->end_value = new_val;
        if (slider->end_value < slider->start_value) {
          slider->end_value = slider->start_value;
        }
      }

      if (slider->on_change) {
        slider->on_change(slider->on_change_ctx, slider, slider->start_value,
                          slider->end_value);
      }
    }
  } else if (event->type == CMP_INPUT_POINTER_UP) {
    if (slider->is_dragging_start || slider->is_dragging_end) {
      slider->is_dragging_start = CMP_FALSE;
      slider->is_dragging_end = CMP_FALSE;
      *out_handled = CMP_TRUE;
    }
  }

  return CMP_OK;
}

static int m3_range_slider_widget_get_semantics(void *widget,
                                                CMPSemantics *out_semantics) {
  M3RangeSlider *slider = (M3RangeSlider *)widget;
  if (slider == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_semantics, 0, sizeof(*out_semantics));
  out_semantics->role = CMP_SEMANTIC_SLIDER;
  out_semantics->flags = 0;
  if (slider->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED;
  }
  if (slider->widget.flags & CMP_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = slider->utf8_label;
  out_semantics->utf8_hint = "Range Slider";
  out_semantics->utf8_value = NULL;
  out_semantics->range_min = slider->min_value;
  out_semantics->range_max = slider->max_value;
  out_semantics->range_value =
      slider->start_value; /* or end, start is fine for range representation */
  return CMP_OK;
}

static int m3_range_slider_widget_destroy(void *widget) {
  M3RangeSlider *slider = (M3RangeSlider *)widget;
  if (slider == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static const CMPWidgetVTable g_m3_range_slider_widget_vtable = {
    m3_range_slider_widget_measure,       m3_range_slider_widget_layout,
    m3_range_slider_widget_paint,         m3_range_slider_widget_event,
    m3_range_slider_widget_get_semantics, m3_range_slider_widget_destroy};

CMP_API int CMP_CALL m3_range_slider_init(
    M3RangeSlider *slider, const M3SliderStyle *style, CMPScalar min_value,
    CMPScalar max_value, CMPScalar start_value, CMPScalar end_value) {
  int rc;

  if (slider == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_slider_validate_style(style);
  if (rc != CMP_OK)
    return rc;
  rc = m3_slider_validate_range(min_value, max_value);
  if (rc != CMP_OK)
    return rc;
  if (start_value < min_value || start_value > max_value)
    return CMP_ERR_RANGE;
  if (end_value < min_value || end_value > max_value)
    return CMP_ERR_RANGE;
  if (start_value > end_value)
    return CMP_ERR_RANGE;

  memset(slider, 0, sizeof(*slider));
  slider->widget.vtable = &g_m3_range_slider_widget_vtable;
  slider->widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;
  slider->style = *style;
  slider->min_value = min_value;
  slider->max_value = max_value;
  slider->start_value = start_value;
  slider->end_value = end_value;
  slider->step = M3_SLIDER_DEFAULT_STEP;

  return CMP_OK;
}

CMP_API int CMP_CALL m3_range_slider_set_values(M3RangeSlider *slider,
                                                CMPScalar start_value,
                                                CMPScalar end_value) {
  int rc;

  if (slider == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_slider_validate_range(slider->min_value, slider->max_value);
  if (rc != CMP_OK)
    return rc;

  if (start_value < slider->min_value || start_value > slider->max_value)
    return CMP_ERR_RANGE;
  if (end_value < slider->min_value || end_value > slider->max_value)
    return CMP_ERR_RANGE;

  if (start_value > end_value) {
    CMPScalar temp = start_value;
    start_value = end_value;
    end_value = temp;
  }

  m3_slider_snap_value(start_value, slider->min_value, slider->max_value,
                       slider->step, &slider->start_value);
  m3_slider_snap_value(end_value, slider->min_value, slider->max_value,
                       slider->step, &slider->end_value);

  return CMP_OK;
}

CMP_API int CMP_CALL m3_range_slider_get_values(const M3RangeSlider *slider,
                                                CMPScalar *out_start_value,
                                                CMPScalar *out_end_value) {
  if (slider == NULL || out_start_value == NULL || out_end_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_start_value = slider->start_value;
  *out_end_value = slider->end_value;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_range_slider_set_range(M3RangeSlider *slider,
                                               CMPScalar min_value,
                                               CMPScalar max_value) {
  int rc;

  if (slider == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_slider_validate_range(min_value, max_value);
  if (rc != CMP_OK)
    return rc;

  slider->min_value = min_value;
  slider->max_value = max_value;

  m3_slider_snap_value(slider->start_value, slider->min_value,
                       slider->max_value, slider->step, &slider->start_value);
  m3_slider_snap_value(slider->end_value, slider->min_value, slider->max_value,
                       slider->step, &slider->end_value);

  if (slider->start_value > slider->end_value) {
    slider->start_value = slider->end_value;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL m3_range_slider_set_step(M3RangeSlider *slider,
                                              CMPScalar step) {
  int rc;

  if (slider == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_slider_validate_step(step);
  if (rc != CMP_OK)
    return rc;

  slider->step = step;

  m3_slider_snap_value(slider->start_value, slider->min_value,
                       slider->max_value, slider->step, &slider->start_value);
  m3_slider_snap_value(slider->end_value, slider->min_value, slider->max_value,
                       slider->step, &slider->end_value);

  if (slider->start_value > slider->end_value) {
    slider->start_value = slider->end_value;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL m3_range_slider_set_style(M3RangeSlider *slider,
                                               const M3SliderStyle *style) {
  int rc;

  if (slider == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_slider_validate_style(style);
  if (rc != CMP_OK)
    return rc;

  slider->style = *style;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_range_slider_set_label(M3RangeSlider *slider,
                                               const char *utf8_label) {
  if (slider == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  slider->utf8_label = utf8_label;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_range_slider_set_on_change(
    M3RangeSlider *slider, CMPRangeSliderOnChange on_change, void *ctx) {
  if (slider == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  slider->on_change = on_change;
  slider->on_change_ctx = ctx;
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL m3_progress_test_set_fail_point(cmp_u32 fail_point) {
  g_m3_progress_test_fail_point = fail_point;
  return CMP_OK;
}

int CMP_CALL m3_progress_test_clear_fail_points(void) {
  g_m3_progress_test_fail_point = M3_PROGRESS_TEST_FAIL_NONE;
  g_m3_progress_test_color_set_calls = 0u;
  g_m3_progress_test_color_set_fail_after = 0u;
  g_m3_progress_test_color_alpha_calls = 0u;
  g_m3_progress_test_color_alpha_fail_after = 0u;
  return CMP_OK;
}

int CMP_CALL m3_progress_test_set_color_set_fail_after(cmp_u32 call_count) {
  g_m3_progress_test_color_set_calls = 0u;
  g_m3_progress_test_color_set_fail_after = call_count;
  return CMP_OK;
}

int CMP_CALL m3_progress_test_set_color_alpha_fail_after(cmp_u32 call_count) {
  g_m3_progress_test_color_alpha_calls = 0u;
  g_m3_progress_test_color_alpha_fail_after = call_count;
  return CMP_OK;
}

int CMP_CALL m3_progress_test_validate_color(const CMPColor *color) {
  return m3_progress_validate_color(color);
}

int CMP_CALL m3_progress_test_color_set(CMPColor *color, CMPScalar r,
                                        CMPScalar g, CMPScalar b, CMPScalar a) {
  return m3_progress_color_set(color, r, g, b, a);
}

int CMP_CALL m3_progress_test_color_with_alpha(const CMPColor *base,
                                               CMPScalar alpha,
                                               CMPColor *out_color) {
  return m3_progress_color_with_alpha(base, alpha, out_color);
}

int CMP_CALL m3_progress_test_validate_measure_spec(CMPMeasureSpec spec) {
  return m3_progress_validate_measure_spec(spec);
}

int CMP_CALL m3_progress_test_apply_measure(CMPScalar desired,
                                            CMPMeasureSpec spec,
                                            CMPScalar *out_size) {
  return m3_progress_apply_measure(desired, spec, out_size);
}

int CMP_CALL m3_progress_test_validate_rect(const CMPRect *rect) {
  return m3_progress_validate_rect(rect);
}

int CMP_CALL m3_progress_test_validate_value01(CMPScalar value) {
  return m3_progress_validate_value01(value);
}

int CMP_CALL
m3_progress_test_linear_validate_style(const M3LinearProgressStyle *style) {
  return m3_linear_progress_validate_style(style);
}

int CMP_CALL m3_progress_test_linear_resolve_colors(
    const M3LinearProgress *progress, CMPColor *out_track,
    CMPColor *out_indicator) {
  return m3_linear_progress_resolve_colors(progress, out_track, out_indicator);
}

int CMP_CALL
m3_progress_test_circular_validate_style(const M3CircularProgressStyle *style) {
  return m3_circular_progress_validate_style(style);
}

int CMP_CALL m3_progress_test_circular_resolve_colors(
    const M3CircularProgress *progress, CMPColor *out_track,
    CMPColor *out_indicator) {
  return m3_circular_progress_resolve_colors(progress, out_track,
                                             out_indicator);
}

int CMP_CALL m3_progress_test_circular_draw_arc(
    CMPGfx *gfx, CMPScalar cx, CMPScalar cy, CMPScalar radius,
    CMPScalar start_angle, CMPScalar end_angle, CMPColor color,
    CMPScalar thickness, cmp_u32 segments) {
  return m3_circular_progress_draw_arc(gfx, cx, cy, radius, start_angle,
                                       end_angle, color, thickness, segments);
}

int CMP_CALL
m3_progress_test_slider_validate_style(const M3SliderStyle *style) {
  return m3_slider_validate_style(style);
}

int CMP_CALL m3_progress_test_slider_value_to_fraction(
    const M3Slider *slider, CMPScalar *out_fraction) {
  return m3_slider_value_to_fraction(slider, out_fraction);
}

int CMP_CALL m3_progress_test_slider_value_from_x(const M3Slider *slider,
                                                  CMPScalar x,
                                                  CMPScalar *out_value) {
  return m3_slider_value_from_x(slider, x, out_value);
}

int CMP_CALL m3_progress_test_slider_snap_value(CMPScalar value,
                                                CMPScalar min_value,
                                                CMPScalar max_value,
                                                CMPScalar step,
                                                CMPScalar *out_value) {
  return m3_slider_snap_value(value, min_value, max_value, step, out_value);
}

int CMP_CALL m3_progress_test_slider_resolve_colors(const M3Slider *slider,
                                                    CMPColor *out_track,
                                                    CMPColor *out_active,
                                                    CMPColor *out_thumb) {
  return m3_slider_resolve_colors(slider, out_track, out_active, out_thumb);
}

int CMP_CALL m3_progress_test_slider_update_value(M3Slider *slider,
                                                  CMPScalar next_value,
                                                  CMPBool notify) {
  return m3_slider_update_value(slider, next_value, notify);
}
#endif

int CMP_CALL m3_slider_style_init_media(M3SliderStyle *style) {
  int rc;
  if (style == NULL)
    return CMP_ERR_INVALID_ARGUMENT;
  memset(style, 0, sizeof(*style));
  style->track_length = M3_SLIDER_DEFAULT_LENGTH;
  style->track_height = 8.0f;
  style->active_track_height = 16.0f;
  style->track_corner_radius = 4.0f;
  style->thumb_radius = 8.0f;
  style->thumb_width = 4.0f;
  style->is_media_slider = CMP_TRUE;
  rc = m3_progress_color_set(&style->track_color, 0.82f, 0.82f, 0.82f, 0.5f);
  if (rc != CMP_OK)
    return rc;
  rc = m3_progress_color_set(&style->active_track_color, 0.26f, 0.52f, 0.96f,
                             1.0f);
  if (rc != CMP_OK)
    return rc;
  rc = m3_progress_color_set(&style->thumb_color, 1.0f, 1.0f, 1.0f, 1.0f);
  if (rc != CMP_OK)
    return rc;
  return CMP_OK;
}
