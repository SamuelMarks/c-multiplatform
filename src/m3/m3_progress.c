#include "m3/m3_progress.h"

#include <math.h>
#include <string.h>

#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
#define M3_PROGRESS_TEST_FAIL_NONE 0u
#define M3_PROGRESS_TEST_FAIL_COLOR_SET 1u
#define M3_PROGRESS_TEST_FAIL_COLOR_WITH_ALPHA 2u
#define M3_PROGRESS_TEST_FAIL_LINEAR_CORNER_RANGE 3u
#define M3_PROGRESS_TEST_FAIL_LINEAR_FILL_CORNER_RANGE                         \
  4u /* GCOVR_EXCL_LINE                                                        \
      */
#define M3_PROGRESS_TEST_FAIL_LINEAR_FILL_CORNER_CLAMP 5u
#define M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_COUNT_NEGATIVE 6u
#define M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_CLAMP_MIN 7u
#define M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_CLAMP_MAX 8u
#define M3_PROGRESS_TEST_FAIL_SLIDER_FRACTION_NEGATIVE 9u
#define M3_PROGRESS_TEST_FAIL_SLIDER_FRACTION_POSITIVE                         \
  10u /* GCOVR_EXCL_LINE                                                       \
       */
#define M3_PROGRESS_TEST_FAIL_SLIDER_FROM_X_FRACTION_HIGH 11u
#define M3_PROGRESS_TEST_FAIL_SLIDER_CORNER_RANGE 12u
#define M3_PROGRESS_TEST_FAIL_LINEAR_RESOLVE_COLORS 13u /* GCOVR_EXCL_LINE */
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
#endif /* GCOVR_EXCL_LINE */

#define M3_PROGRESS_PI 3.14159265358979323846f /* GCOVR_EXCL_LINE */

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
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
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

static int
m3_progress_color_with_alpha(const CMPColor *base, CMPScalar alpha,
                             CMPColor *out_color) { /* GCOVR_EXCL_LINE */
  int rc;

  if (base == NULL || out_color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  g_m3_progress_test_color_alpha_calls += 1u;
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_COLOR_WITH_ALPHA)) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE;
  }
  if (g_m3_progress_test_color_alpha_fail_after > 0u &&
      g_m3_progress_test_color_alpha_calls >=
          g_m3_progress_test_color_alpha_fail_after) {
    return CMP_ERR_RANGE;
  }
#endif /* GCOVR_EXCL_LINE */
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

static int
m3_linear_progress_resolve_colors(const M3LinearProgress *progress,
                                  CMPColor *out_track, /* GCOVR_EXCL_LINE */
                                  CMPColor *out_indicator) {
  int rc;

  if (progress == NULL || out_track == NULL || out_indicator == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_LINEAR_RESOLVE_COLORS)) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_IO;
  }
#endif /* GCOVR_EXCL_LINE */

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

static int
m3_linear_progress_widget_measure(void *widget, CMPMeasureSpec width,
                                  CMPMeasureSpec height, /* GCOVR_EXCL_LINE */
                                  CMPSize *out_size) {
  M3LinearProgress *progress; /* GCOVR_EXCL_LINE */
  int rc;                     /* GCOVR_EXCL_LINE */

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
  M3LinearProgress *progress; /* GCOVR_EXCL_LINE */
  int rc;                     /* GCOVR_EXCL_LINE */

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
  CMPRect bounds; /* GCOVR_EXCL_LINE */
  CMPRect fill;   /* GCOVR_EXCL_LINE */
  CMPColor track_color;
  CMPColor indicator_color; /* GCOVR_EXCL_LINE */
  CMPScalar corner;         /* GCOVR_EXCL_LINE */
  CMPScalar fill_corner;
  CMPScalar fill_width; /* GCOVR_EXCL_LINE */
  int rc;               /* GCOVR_EXCL_LINE */

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
          M3_PROGRESS_TEST_FAIL_LINEAR_CORNER_RANGE)) { /* GCOVR_EXCL_LINE */
    corner = -1.0f;
  }
#endif /* GCOVR_EXCL_LINE */
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
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
    if (m3_progress_test_fail_point_match(
            M3_PROGRESS_TEST_FAIL_LINEAR_FILL_CORNER_CLAMP)) { /* GCOVR_EXCL_LINE
                                                                */
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
            M3_PROGRESS_TEST_FAIL_LINEAR_FILL_CORNER_RANGE)) { /* GCOVR_EXCL_LINE
                                                                */
      fill_corner = -1.0f;
    }
#endif /* GCOVR_EXCL_LINE */
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

static int m3_linear_progress_widget_event(
    void *widget, const CMPInputEvent *event, /* GCOVR_EXCL_LINE */
    CMPBool *out_handled) {
  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  CMP_UNUSED(widget);
  CMP_UNUSED(event);
  *out_handled = CMP_FALSE;
  return CMP_OK;
}

static int m3_linear_progress_widget_get_semantics(
    void *widget, CMPSemantics *out_semantics) { /* GCOVR_EXCL_LINE */
  M3LinearProgress *progress;                    /* GCOVR_EXCL_LINE */

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  progress = (M3LinearProgress *)widget;
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
  M3LinearProgress *progress; /* GCOVR_EXCL_LINE */

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

static const CMPWidgetVTable g_m3_linear_progress_widget_vtable =
    {/* GCOVR_EXCL_LINE */
     m3_linear_progress_widget_measure,
     m3_linear_progress_widget_layout,
     m3_linear_progress_widget_paint, /* GCOVR_EXCL_LINE */
     m3_linear_progress_widget_event, /* GCOVR_EXCL_LINE */
     m3_linear_progress_widget_get_semantics,
     m3_linear_progress_widget_destroy};

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

int CMP_CALL m3_linear_progress_init(
    M3LinearProgress *progress,
    const M3LinearProgressStyle *style, /* GCOVR_EXCL_LINE */
    CMPScalar value) {                  /* GCOVR_EXCL_LINE */
  int rc;                               /* GCOVR_EXCL_LINE */

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

int CMP_CALL m3_linear_progress_set_value(
    M3LinearProgress *progress, CMPScalar value) { /* GCOVR_EXCL_LINE */
  int rc;                                          /* GCOVR_EXCL_LINE */

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

int CMP_CALL
m3_linear_progress_get_value(const M3LinearProgress *progress,
                             CMPScalar *out_value) { /* GCOVR_EXCL_LINE */
  if (progress == NULL || out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_value = progress->value;
  return CMP_OK;
}

int CMP_CALL m3_linear_progress_set_style(
    M3LinearProgress *progress,           /* GCOVR_EXCL_LINE */
    const M3LinearProgressStyle *style) { /* GCOVR_EXCL_LINE */
  int rc;                                 /* GCOVR_EXCL_LINE */

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

int CMP_CALL m3_linear_progress_set_label(
    M3LinearProgress *progress, const char *utf8_label, /* GCOVR_EXCL_LINE */
    cmp_usize utf8_len) {                               /* GCOVR_EXCL_LINE */
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

static int /* GCOVR_EXCL_LINE */
m3_circular_progress_validate_style(const M3CircularProgressStyle *style) {
  int rc; /* GCOVR_EXCL_LINE */

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

static int /* GCOVR_EXCL_LINE */
m3_circular_progress_resolve_colors(
    const M3CircularProgress *progress,
    CMPColor *out_track,       /* GCOVR_EXCL_LINE */
    CMPColor *out_indicator) { /* GCOVR_EXCL_LINE */
  int rc;                      /* GCOVR_EXCL_LINE */

  if (progress == NULL || out_track == NULL || out_indicator == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_CIRCULAR_RESOLVE_COLORS)) { /* GCOVR_EXCL_LINE
                                                             */
    return CMP_ERR_IO;
  }
#endif /* GCOVR_EXCL_LINE */

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

static int m3_circular_progress_draw_arc(
    CMPGfx *gfx, CMPScalar cx, CMPScalar cy, CMPScalar radius,
    CMPScalar start_angle, CMPScalar end_angle,
    CMPColor color,                          /* GCOVR_EXCL_LINE */
    CMPScalar thickness, cmp_u32 segments) { /* GCOVR_EXCL_LINE */
  CMPScalar full;
  CMPScalar step;  /* GCOVR_EXCL_LINE */
  CMPScalar angle; /* GCOVR_EXCL_LINE */
  CMPScalar next;
  CMPScalar x0; /* GCOVR_EXCL_LINE */
  CMPScalar y0; /* GCOVR_EXCL_LINE */
  CMPScalar x1;
  CMPScalar y1; /* GCOVR_EXCL_LINE */
  cmp_u32 i;    /* GCOVR_EXCL_LINE */
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

static int
m3_circular_progress_widget_measure(void *widget,
                                    CMPMeasureSpec width,  /* GCOVR_EXCL_LINE */
                                    CMPMeasureSpec height, /* GCOVR_EXCL_LINE */
                                    CMPSize *out_size) {   /* GCOVR_EXCL_LINE */
  M3CircularProgress *progress;                            /* GCOVR_EXCL_LINE */
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
  int rc; /* GCOVR_EXCL_LINE */

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

static int
m3_circular_progress_widget_paint(void *widget,
                                  CMPPaintContext *ctx) { /* GCOVR_EXCL_LINE */
  M3CircularProgress *progress;
  CMPColor track_color;     /* GCOVR_EXCL_LINE */
  CMPColor indicator_color; /* GCOVR_EXCL_LINE */
  CMPRect bounds;           /* GCOVR_EXCL_LINE */
  CMPScalar min_side;
  CMPScalar radius; /* GCOVR_EXCL_LINE */
  CMPScalar cx;
  CMPScalar cy;        /* GCOVR_EXCL_LINE */
  CMPScalar end_angle; /* GCOVR_EXCL_LINE */
  int rc;              /* GCOVR_EXCL_LINE */

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

  rc = m3_circular_progress_resolve_colors(
      progress, &track_color, &indicator_color); /* GCOVR_EXCL_LINE */
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

static int m3_circular_progress_widget_event(
    void *widget, const CMPInputEvent *event, /* GCOVR_EXCL_LINE */
    CMPBool *out_handled) {                   /* GCOVR_EXCL_LINE */
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
  M3CircularProgress *progress; /* GCOVR_EXCL_LINE */

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  progress = (M3CircularProgress *)widget;
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
  M3CircularProgress *progress; /* GCOVR_EXCL_LINE */

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

static const CMPWidgetVTable
    g_m3_circular_progress_widget_vtable = /* GCOVR_EXCL_LINE */
    {m3_circular_progress_widget_measure,
     m3_circular_progress_widget_layout,        /* GCOVR_EXCL_LINE */
     m3_circular_progress_widget_paint,         /* GCOVR_EXCL_LINE */
     m3_circular_progress_widget_event,         /* GCOVR_EXCL_LINE */
     m3_circular_progress_widget_get_semantics, /* GCOVR_EXCL_LINE */
     m3_circular_progress_widget_destroy};      /* GCOVR_EXCL_LINE */

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

int CMP_CALL
m3_circular_progress_init(                              /* GCOVR_EXCL_LINE */
                          M3CircularProgress *progress, /* GCOVR_EXCL_LINE */
                          const M3CircularProgressStyle
                              *style,        /* GCOVR_EXCL_LINE */
                          CMPScalar value) { /* GCOVR_EXCL_LINE */
  int rc;                                    /* GCOVR_EXCL_LINE */

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

int CMP_CALL m3_circular_progress_set_value(
    M3CircularProgress *progress, CMPScalar value) { /* GCOVR_EXCL_LINE */
  int rc;                                            /* GCOVR_EXCL_LINE */

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

int CMP_CALL /* GCOVR_EXCL_LINE */
m3_circular_progress_get_value(const M3CircularProgress *progress,
                               CMPScalar *out_value) {
  if (progress == NULL || out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_value = progress->value;
  return CMP_OK;
}

int CMP_CALL m3_circular_progress_set_style(/* GCOVR_EXCL_LINE */
                                            M3CircularProgress *progress,
                                            const M3CircularProgressStyle
                                                *style) { /* GCOVR_EXCL_LINE */
  int rc;                                                 /* GCOVR_EXCL_LINE */

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

int CMP_CALL m3_circular_progress_set_label(
    M3CircularProgress *progress, const char *utf8_label, /* GCOVR_EXCL_LINE */
    cmp_usize utf8_len) {                                 /* GCOVR_EXCL_LINE */
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
  int rc; /* GCOVR_EXCL_LINE */

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
                                CMPScalar max_value,
                                CMPScalar step,         /* GCOVR_EXCL_LINE */
                                CMPScalar *out_value) { /* GCOVR_EXCL_LINE */
  CMPScalar clamped;                                    /* GCOVR_EXCL_LINE */
  CMPScalar range;                                      /* GCOVR_EXCL_LINE */
  CMPScalar count;                                      /* GCOVR_EXCL_LINE */
  cmp_i32 steps;                                        /* GCOVR_EXCL_LINE */
  CMPBool force_negative;                               /* GCOVR_EXCL_LINE */

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
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_COUNT_NEGATIVE)) { /* GCOVR_EXCL_LINE
                                                                */
    force_negative = CMP_TRUE;
  }
#endif /* GCOVR_EXCL_LINE */
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
          M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_CLAMP_MIN)) { /* GCOVR_EXCL_LINE */
    clamped = min_value - 1.0f;
  }
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_CLAMP_MAX)) { /* GCOVR_EXCL_LINE */
    clamped = max_value + 1.0f;
  }
#endif /* GCOVR_EXCL_LINE */
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
          M3_PROGRESS_TEST_FAIL_SLIDER_FRACTION_NEGATIVE)) { /* GCOVR_EXCL_LINE
                                                              */
    *out_fraction = -1.0f;
  }
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_FRACTION_POSITIVE)) {
    *out_fraction = 2.0f;
  }
#endif /* GCOVR_EXCL_LINE */
  if (*out_fraction < 0.0f) {
    *out_fraction = 0.0f;
  }
  if (*out_fraction > 1.0f) {
    *out_fraction = 1.0f;
  }
  return CMP_OK;
}

static int m3_slider_value_from_x(const M3Slider *slider, CMPScalar x,
                                  CMPScalar *out_value) { /* GCOVR_EXCL_LINE */
  CMPScalar range;                                        /* GCOVR_EXCL_LINE */
  CMPScalar fraction;
  int rc; /* GCOVR_EXCL_LINE */

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
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_FROM_X_FRACTION_HIGH)) { /* GCOVR_EXCL_LINE
                                                                 */
    fraction = 2.0f;
  }
#endif /* GCOVR_EXCL_LINE */
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
                                    CMPColor *out_active,
                                    CMPColor *out_thumb) { /* GCOVR_EXCL_LINE */
  CMPColor track;
  CMPColor active;
  CMPColor thumb;
  int rc;

  if (slider == NULL || out_track == NULL || out_active == NULL ||
      out_thumb == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_RESOLVE_COLORS)) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_IO;
  }
#endif /* GCOVR_EXCL_LINE */

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
                                  CMPBool notify) { /* GCOVR_EXCL_LINE */
  CMPScalar prev_value;                             /* GCOVR_EXCL_LINE */
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
                                    CMPMeasureSpec height,
                                    CMPSize *out_size) { /* GCOVR_EXCL_LINE */
  M3Slider *slider;                                      /* GCOVR_EXCL_LINE */
  CMPScalar desired_height;                              /* GCOVR_EXCL_LINE */
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
  M3Slider *slider; /* GCOVR_EXCL_LINE */
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
  M3Slider *slider;     /* GCOVR_EXCL_LINE */
  CMPRect bounds;       /* GCOVR_EXCL_LINE */
  CMPRect track;        /* GCOVR_EXCL_LINE */
  CMPRect active;       /* GCOVR_EXCL_LINE */
  CMPRect thumb;        /* GCOVR_EXCL_LINE */
  CMPColor track_color; /* GCOVR_EXCL_LINE */
  CMPColor active_color = {0};
  CMPColor thumb_color = {0};
  CMPScalar fraction;   /* GCOVR_EXCL_LINE */
  CMPScalar corner;     /* GCOVR_EXCL_LINE */
  CMPScalar thumb_size; /* GCOVR_EXCL_LINE */
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
                                &thumb_color); /* GCOVR_EXCL_LINE */
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
  corner = slider->style.track_corner_radius;
#ifdef CMP_TESTING
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_CORNER_CLAMP)) {
    corner = track.height;
  }
#endif
  if (corner > track.height * 0.5f) {
    corner = track.height * 0.5f;
  }
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (m3_progress_test_fail_point_match(
          M3_PROGRESS_TEST_FAIL_SLIDER_CORNER_RANGE)) { /* GCOVR_EXCL_LINE */
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
  thumb.width = thumb_size;
  thumb.height = thumb_size;
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
                                  CMPBool *out_handled) { /* GCOVR_EXCL_LINE */
  M3Slider *slider;                                       /* GCOVR_EXCL_LINE */
  CMPScalar next_value;                                   /* GCOVR_EXCL_LINE */
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
  case CMP_INPUT_POINTER_DOWN: /* GCOVR_EXCL_LINE */
    if (slider->pressed == CMP_TRUE) {
      return CMP_ERR_STATE;
    }
    rc = m3_slider_value_from_x(slider, (CMPScalar)event->data.pointer.x,
                                &next_value); /* GCOVR_EXCL_LINE */
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
  case CMP_INPUT_POINTER_MOVE: /* GCOVR_EXCL_LINE */
    if (slider->pressed == CMP_FALSE) {
      return CMP_OK;
    }
    rc = m3_slider_value_from_x(slider, (CMPScalar)event->data.pointer.x,
                                &next_value); /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) {
      return rc;
    }
    rc = m3_slider_update_value(slider, next_value, CMP_TRUE);
    if (rc != CMP_OK) {
      return rc;
    }
    *out_handled = CMP_TRUE;
    return CMP_OK;
  case CMP_INPUT_POINTER_UP: /* GCOVR_EXCL_LINE */
    if (slider->pressed == CMP_FALSE) {
      return CMP_OK;
    }
    slider->pressed = CMP_FALSE;
    *out_handled = CMP_TRUE;
    return CMP_OK;
  default: /* GCOVR_EXCL_LINE */
    return CMP_OK;
  }
}

static int m3_slider_widget_get_semantics(void *widget,
                                          CMPSemantics *out_semantics) {
  M3Slider *slider = NULL;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  slider = (M3Slider *)widget;
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
  return CMP_OK;
}

static int m3_slider_widget_destroy(void *widget) {
  M3Slider *slider; /* GCOVR_EXCL_LINE */

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

static const CMPWidgetVTable g_m3_slider_widget_vtable =
    {/* GCOVR_EXCL_LINE */
     m3_slider_widget_measure,
     m3_slider_widget_layout,
     m3_slider_widget_paint,
     m3_slider_widget_event, /* GCOVR_EXCL_LINE */
     m3_slider_widget_get_semantics,
     m3_slider_widget_destroy};

int CMP_CALL m3_slider_style_init(M3SliderStyle *style) {
  int rc; /* GCOVR_EXCL_LINE */

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
                             1.0f); /* GCOVR_EXCL_LINE */
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
                            CMPScalar min_value, /* GCOVR_EXCL_LINE */
                            CMPScalar max_value, /* GCOVR_EXCL_LINE */
                            CMPScalar value) {   /* GCOVR_EXCL_LINE */
  CMPScalar snapped;
  int rc; /* GCOVR_EXCL_LINE */

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
                            &snapped); /* GCOVR_EXCL_LINE */
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
  CMPScalar snapped; /* GCOVR_EXCL_LINE */
  int rc;            /* GCOVR_EXCL_LINE */

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
  CMPScalar snapped; /* GCOVR_EXCL_LINE */
  int rc;            /* GCOVR_EXCL_LINE */

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
  CMPScalar snapped; /* GCOVR_EXCL_LINE */
  int rc;            /* GCOVR_EXCL_LINE */

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
  int rc; /* GCOVR_EXCL_LINE */

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
                                 cmp_usize utf8_len) { /* GCOVR_EXCL_LINE */
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
                                     CMPSliderOnChange on_change,
                                     void *ctx) { /* GCOVR_EXCL_LINE */
  if (slider == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  slider->on_change = on_change;
  slider->on_change_ctx = ctx;
  return CMP_OK;
}

#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
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

int CMP_CALL m3_progress_test_color_with_alpha(
    const CMPColor *base, CMPScalar alpha, /* GCOVR_EXCL_LINE */
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

int CMP_CALL /* GCOVR_EXCL_LINE */
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
