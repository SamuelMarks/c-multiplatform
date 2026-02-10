#include "m3/m3_progress.h"
#include "test_utils.h"

#include <string.h>

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

typedef struct TestProgressBackend {
  int draw_rect_calls;
  int draw_line_calls;
  int fail_draw_rect;
  int fail_draw_rect_after;
  int fail_draw_line;
  M3Rect last_rect;
  M3Color last_rect_color;
  M3Scalar last_corner;
  M3Scalar last_line_x0;
  M3Scalar last_line_y0;
  M3Scalar last_line_x1;
  M3Scalar last_line_y1;
  M3Color last_line_color;
  M3Scalar last_line_thickness;
} TestProgressBackend;

typedef struct SliderCounter {
  int calls;
  M3Scalar last_value;
  int fail;
} SliderCounter;

static int test_backend_init(TestProgressBackend *backend) {
  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(backend, 0, sizeof(*backend));
  backend->fail_draw_rect = M3_OK;
  backend->fail_draw_rect_after = 0;
  backend->fail_draw_line = M3_OK;
  return M3_OK;
}

static int test_gfx_draw_rect(void *gfx, const M3Rect *rect, M3Color color,
                              M3Scalar corner_radius) {
  TestProgressBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestProgressBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_rect_color = color;
  backend->last_corner = corner_radius;
  if (backend->fail_draw_rect != M3_OK) {
    return backend->fail_draw_rect;
  }
  if (backend->fail_draw_rect_after > 0 &&
      backend->draw_rect_calls >= backend->fail_draw_rect_after) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_gfx_draw_line(void *gfx, M3Scalar x0, M3Scalar y0, M3Scalar x1,
                              M3Scalar y1, M3Color color, M3Scalar thickness) {
  TestProgressBackend *backend;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestProgressBackend *)gfx;
  backend->draw_line_calls += 1;
  backend->last_line_x0 = x0;
  backend->last_line_y0 = y0;
  backend->last_line_x1 = x1;
  backend->last_line_y1 = y1;
  backend->last_line_color = color;
  backend->last_line_thickness = thickness;
  if (backend->fail_draw_line != M3_OK) {
    return backend->fail_draw_line;
  }
  return M3_OK;
}

static const M3GfxVTable g_test_vtable = {NULL,
                                          NULL,
                                          NULL,
                                          test_gfx_draw_rect,
                                          test_gfx_draw_line,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL};

static const M3GfxVTable g_test_vtable_no_rect = {
    NULL, NULL, NULL, NULL, test_gfx_draw_line, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL};

static const M3GfxVTable g_test_vtable_no_line = {
    NULL, NULL, NULL, test_gfx_draw_rect, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL};

static int m3_near(M3Scalar a, M3Scalar b, M3Scalar tol) {
  M3Scalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

static int init_pointer_event(M3InputEvent *event, m3_u32 type, m3_i32 x,
                              m3_i32 y) {
  if (event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(event, 0, sizeof(*event));
  event->type = type;
  event->data.pointer.x = x;
  event->data.pointer.y = y;
  return M3_OK;
}

static int test_slider_on_change(void *ctx, struct M3Slider *slider,
                                 M3Scalar value) {
  SliderCounter *counter;

  if (ctx == NULL || slider == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  counter = (SliderCounter *)ctx;
  counter->calls += 1;
  counter->last_value = value;
  if (counter->fail) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_progress_helpers(void) {
  TestProgressBackend backend;
  M3Gfx gfx;
  M3Color color;
  M3Color out_color;
  M3MeasureSpec spec;
  M3Scalar size;
  M3Rect rect;
  M3LinearProgressStyle linear_style;
  M3LinearProgress linear;
  M3CircularProgressStyle circular_style;
  M3CircularProgress circular;
  M3SliderStyle slider_style;
  M3Slider slider;
  M3Scalar fraction;
  M3Scalar value;
  volatile const M3Color *null_color;
  volatile const M3Rect *null_rect;
  volatile M3Scalar invalid_scalar;

  null_color = NULL;
  null_rect = NULL;
  M3_TEST_EXPECT(m3_progress_test_validate_color((const M3Color *)null_color),
                 M3_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  M3_TEST_EXPECT(m3_progress_test_validate_color(&color), M3_ERR_RANGE);
  color.r = 0.0f;
  invalid_scalar = 1.2f;
  color.g = (M3Scalar)invalid_scalar;
  M3_TEST_EXPECT(m3_progress_test_validate_color(&color), M3_ERR_RANGE);
  color.g = 0.0f;
  invalid_scalar = 1.2f;
  color.b = (M3Scalar)invalid_scalar;
  M3_TEST_EXPECT(m3_progress_test_validate_color(&color), M3_ERR_RANGE);
  color.b = 0.0f;
  invalid_scalar = 1.2f;
  color.a = (M3Scalar)invalid_scalar;
  M3_TEST_EXPECT(m3_progress_test_validate_color(&color), M3_ERR_RANGE);
  color.a = 1.0f;
  M3_TEST_OK(m3_progress_test_validate_color(&color));

  M3_TEST_EXPECT(
      m3_progress_test_color_set((M3Color *)null_color, 0.0f, 0.0f, 0.0f, 0.0f),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_progress_test_color_set(&color, -0.1f, 0.0f, 0.0f, 0.0f),
                 M3_ERR_RANGE);
  invalid_scalar = 1.2f;
  M3_TEST_EXPECT(m3_progress_test_color_set(
                     &color, 0.0f, (M3Scalar)invalid_scalar, 0.0f, 0.0f),
                 M3_ERR_RANGE);
  invalid_scalar = 1.2f;
  M3_TEST_EXPECT(m3_progress_test_color_set(&color, 0.0f, 0.0f,
                                            (M3Scalar)invalid_scalar, 0.0f),
                 M3_ERR_RANGE);
  invalid_scalar = 1.2f;
  M3_TEST_EXPECT(m3_progress_test_color_set(&color, 0.0f, 0.0f, 0.0f,
                                            (M3Scalar)invalid_scalar),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_color_set(&color, 0.1f, 0.2f, 0.3f, 0.4f));

  M3_TEST_EXPECT(m3_progress_test_color_with_alpha((const M3Color *)null_color,
                                                   0.5f, &out_color),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_progress_test_color_with_alpha(&color, 0.5f, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_progress_test_color_with_alpha(&color, -0.1f, &out_color),
                 M3_ERR_RANGE);
  color.r = 2.0f;
  M3_TEST_EXPECT(m3_progress_test_color_with_alpha(&color, 0.5f, &out_color),
                 M3_ERR_RANGE);
  color.r = 0.1f;
  M3_TEST_OK(m3_progress_test_color_with_alpha(&color, 0.5f, &out_color));
  M3_TEST_ASSERT(m3_near(out_color.a, 0.2f, 0.001f));

  spec.mode = 99u;
  spec.size = 0.0f;
  M3_TEST_EXPECT(m3_progress_test_validate_measure_spec(spec),
                 M3_ERR_INVALID_ARGUMENT);
  spec.mode = M3_MEASURE_AT_MOST;
  spec.size = -1.0f;
  M3_TEST_EXPECT(m3_progress_test_validate_measure_spec(spec), M3_ERR_RANGE);
  spec.mode = M3_MEASURE_UNSPECIFIED;
  spec.size = -1.0f;
  M3_TEST_OK(m3_progress_test_validate_measure_spec(spec));
  M3_TEST_EXPECT(m3_progress_test_apply_measure(1.0f, spec, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  spec.mode = M3_MEASURE_EXACTLY;
  spec.size = 5.0f;
  M3_TEST_OK(m3_progress_test_apply_measure(1.0f, spec, &size));
  M3_TEST_ASSERT(m3_near(size, 5.0f, 0.001f));
  spec.mode = M3_MEASURE_AT_MOST;
  spec.size = 3.0f;
  M3_TEST_OK(m3_progress_test_apply_measure(5.0f, spec, &size));
  M3_TEST_ASSERT(m3_near(size, 3.0f, 0.001f));

  M3_TEST_EXPECT(m3_progress_test_validate_rect((const M3Rect *)null_rect),
                 M3_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 1.0f;
  M3_TEST_EXPECT(m3_progress_test_validate_rect(&rect), M3_ERR_RANGE);
  rect.width = 1.0f;
  rect.height = -1.0f;
  M3_TEST_EXPECT(m3_progress_test_validate_rect(&rect), M3_ERR_RANGE);
  rect.height = 1.0f;
  M3_TEST_OK(m3_progress_test_validate_rect(&rect));

  M3_TEST_EXPECT(m3_progress_test_validate_value01(-0.1f), M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_progress_test_validate_value01(1.1f), M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_validate_value01(0.5f));

  M3_TEST_OK(m3_progress_test_set_fail_point(M3_PROGRESS_TEST_FAIL_COLOR_SET));
  M3_TEST_EXPECT(m3_linear_progress_style_init(&linear_style), M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(
      m3_progress_test_set_fail_point(M3_PROGRESS_TEST_FAIL_COLOR_WITH_ALPHA));
  M3_TEST_EXPECT(m3_linear_progress_style_init(&linear_style), M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(m3_progress_test_set_color_set_fail_after(2u));
  M3_TEST_EXPECT(m3_linear_progress_style_init(&linear_style), M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(m3_progress_test_set_color_alpha_fail_after(2u));
  M3_TEST_EXPECT(m3_linear_progress_style_init(&linear_style), M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());

  M3_TEST_OK(m3_linear_progress_style_init(&linear_style));
  M3_TEST_EXPECT(m3_progress_test_linear_validate_style(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  linear_style.height = 0.0f;
  M3_TEST_EXPECT(m3_progress_test_linear_validate_style(&linear_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_linear_progress_style_init(&linear_style));
  linear_style.min_width = -1.0f;
  M3_TEST_EXPECT(m3_progress_test_linear_validate_style(&linear_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_linear_progress_style_init(&linear_style));
  linear_style.corner_radius = linear_style.height;
  M3_TEST_EXPECT(m3_progress_test_linear_validate_style(&linear_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_linear_progress_style_init(&linear_style));
  linear_style.track_color.r = -1.0f;
  M3_TEST_EXPECT(m3_progress_test_linear_validate_style(&linear_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_linear_progress_style_init(&linear_style));
  linear_style.disabled_track_color.r = -1.0f;
  M3_TEST_EXPECT(m3_progress_test_linear_validate_style(&linear_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_linear_progress_style_init(&linear_style));
  linear_style.disabled_indicator_color.r = -1.0f;
  M3_TEST_EXPECT(m3_progress_test_linear_validate_style(&linear_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_linear_progress_style_init(&linear_style));
  M3_TEST_OK(m3_progress_test_linear_validate_style(&linear_style));

  M3_TEST_OK(m3_linear_progress_init(&linear, &linear_style, 0.5f));
  M3_TEST_EXPECT(
      m3_progress_test_linear_resolve_colors(NULL, &color, &out_color),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_progress_test_linear_resolve_colors(&linear, NULL, &out_color),
      M3_ERR_INVALID_ARGUMENT);
  linear.widget.flags = M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(
      m3_progress_test_linear_resolve_colors(&linear, &color, &out_color));
  linear.widget.flags = 0u;
  linear.style.track_color.r = -1.0f;
  M3_TEST_EXPECT(
      m3_progress_test_linear_resolve_colors(&linear, &color, &out_color),
      M3_ERR_RANGE);
  linear.style = linear_style;
  linear.style.indicator_color.r = -1.0f;
  M3_TEST_EXPECT(
      m3_progress_test_linear_resolve_colors(&linear, &color, &out_color),
      M3_ERR_RANGE);
  linear.style = linear_style;

  M3_TEST_OK(m3_progress_test_set_fail_point(M3_PROGRESS_TEST_FAIL_COLOR_SET));
  M3_TEST_EXPECT(m3_circular_progress_style_init(&circular_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(
      m3_progress_test_set_fail_point(M3_PROGRESS_TEST_FAIL_COLOR_WITH_ALPHA));
  M3_TEST_EXPECT(m3_circular_progress_style_init(&circular_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(m3_progress_test_set_color_set_fail_after(2u));
  M3_TEST_EXPECT(m3_circular_progress_style_init(&circular_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(m3_progress_test_set_color_alpha_fail_after(2u));
  M3_TEST_EXPECT(m3_circular_progress_style_init(&circular_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());

  M3_TEST_OK(m3_circular_progress_style_init(&circular_style));
  M3_TEST_EXPECT(m3_progress_test_circular_validate_style(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  circular_style.diameter = 0.0f;
  M3_TEST_EXPECT(m3_progress_test_circular_validate_style(&circular_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_circular_progress_style_init(&circular_style));
  circular_style.thickness = 0.0f;
  M3_TEST_EXPECT(m3_progress_test_circular_validate_style(&circular_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_circular_progress_style_init(&circular_style));
  circular_style.thickness = circular_style.diameter;
  M3_TEST_EXPECT(m3_progress_test_circular_validate_style(&circular_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_circular_progress_style_init(&circular_style));
  circular_style.segments = 2u;
  M3_TEST_EXPECT(m3_progress_test_circular_validate_style(&circular_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_circular_progress_style_init(&circular_style));
  circular_style.track_color.a = 2.0f;
  M3_TEST_EXPECT(m3_progress_test_circular_validate_style(&circular_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_circular_progress_style_init(&circular_style));
  circular_style.indicator_color.a = 2.0f;
  M3_TEST_EXPECT(m3_progress_test_circular_validate_style(&circular_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_circular_progress_style_init(&circular_style));
  circular_style.disabled_track_color.a = 2.0f;
  M3_TEST_EXPECT(m3_progress_test_circular_validate_style(&circular_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_circular_progress_style_init(&circular_style));
  circular_style.disabled_indicator_color.a = 2.0f;
  M3_TEST_EXPECT(m3_progress_test_circular_validate_style(&circular_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_circular_progress_style_init(&circular_style));
  M3_TEST_OK(m3_progress_test_circular_validate_style(&circular_style));

  M3_TEST_OK(m3_circular_progress_init(&circular, &circular_style, 0.25f));
  M3_TEST_EXPECT(
      m3_progress_test_circular_resolve_colors(NULL, &color, &out_color),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_progress_test_circular_resolve_colors(&circular, NULL, &out_color),
      M3_ERR_INVALID_ARGUMENT);
  circular.widget.flags = M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(
      m3_progress_test_circular_resolve_colors(&circular, &color, &out_color));
  circular.widget.flags = 0u;
  circular.style.track_color.r = -1.0f;
  M3_TEST_EXPECT(
      m3_progress_test_circular_resolve_colors(&circular, &color, &out_color),
      M3_ERR_RANGE);
  circular.style = circular_style;
  circular.style.indicator_color.r = -1.0f;
  M3_TEST_EXPECT(
      m3_progress_test_circular_resolve_colors(&circular, &color, &out_color),
      M3_ERR_RANGE);
  circular.style = circular_style;

  M3_TEST_OK(test_backend_init(&backend));
  gfx.ctx = &backend;
  gfx.vtable = NULL;
  gfx.text_vtable = NULL;
  M3_TEST_EXPECT(m3_progress_test_circular_draw_arc(
                     NULL, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, color, 1.0f, 8u),
                 M3_ERR_UNSUPPORTED);
  M3_TEST_EXPECT(m3_progress_test_circular_draw_arc(
                     &gfx, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, color, 1.0f, 8u),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable_no_line;
  M3_TEST_EXPECT(m3_progress_test_circular_draw_arc(
                     &gfx, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, color, 1.0f, 8u),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable;
  M3_TEST_EXPECT(m3_progress_test_circular_draw_arc(
                     &gfx, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, color, 0.0f, 8u),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_progress_test_circular_draw_arc(
                     &gfx, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, color, 1.0f, 0u),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_circular_draw_arc(&gfx, 0.0f, 0.0f, 1.0f, 1.0f,
                                                0.0f, color, 1.0f, 8u));
  backend.fail_draw_line = M3_ERR_IO;
  M3_TEST_EXPECT(m3_progress_test_circular_draw_arc(
                     &gfx, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, color, 1.0f, 8u),
                 M3_ERR_IO);

  M3_TEST_OK(m3_slider_style_init(&slider_style));
  M3_TEST_EXPECT(m3_progress_test_slider_validate_style(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_progress_test_set_fail_point(M3_PROGRESS_TEST_FAIL_COLOR_SET));
  M3_TEST_EXPECT(m3_slider_style_init(&slider_style), M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(
      m3_progress_test_set_fail_point(M3_PROGRESS_TEST_FAIL_COLOR_WITH_ALPHA));
  M3_TEST_EXPECT(m3_slider_style_init(&slider_style), M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(m3_progress_test_set_color_set_fail_after(2u));
  M3_TEST_EXPECT(m3_slider_style_init(&slider_style), M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(m3_progress_test_set_color_set_fail_after(3u));
  M3_TEST_EXPECT(m3_slider_style_init(&slider_style), M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(m3_progress_test_set_color_alpha_fail_after(2u));
  M3_TEST_EXPECT(m3_slider_style_init(&slider_style), M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(m3_progress_test_set_color_alpha_fail_after(3u));
  M3_TEST_EXPECT(m3_slider_style_init(&slider_style), M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  slider_style.track_length = 0.0f;
  M3_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_slider_style_init(&slider_style));
  slider_style.track_height = 0.0f;
  M3_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_slider_style_init(&slider_style));
  slider_style.track_corner_radius = slider_style.track_height;
  M3_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_slider_style_init(&slider_style));
  slider_style.thumb_radius = 0.0f;
  M3_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_slider_style_init(&slider_style));
  slider_style.track_color.a = 2.0f;
  M3_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_slider_style_init(&slider_style));
  slider_style.active_track_color.a = 2.0f;
  M3_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_slider_style_init(&slider_style));
  slider_style.thumb_color.a = 2.0f;
  M3_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_slider_style_init(&slider_style));
  slider_style.disabled_track_color.a = 2.0f;
  M3_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_slider_style_init(&slider_style));
  slider_style.disabled_active_track_color.a = 2.0f;
  M3_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_slider_style_init(&slider_style));
  slider_style.disabled_thumb_color.a = 2.0f;
  M3_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_slider_style_init(&slider_style));
  M3_TEST_OK(m3_progress_test_slider_validate_style(&slider_style));

  memset(&slider, 0, sizeof(slider));
  slider.min_value = 1.0f;
  slider.max_value = 1.0f;
  slider.value = 1.0f;
  M3_TEST_EXPECT(m3_progress_test_slider_value_to_fraction(NULL, &fraction),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_progress_test_slider_value_to_fraction(&slider, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_progress_test_slider_value_to_fraction(&slider, &fraction),
                 M3_ERR_RANGE);
  slider.min_value = 0.0f;
  slider.max_value = 1.0f;
  slider.value = 2.0f;
  M3_TEST_EXPECT(m3_progress_test_slider_value_to_fraction(&slider, &fraction),
                 M3_ERR_RANGE);
  slider.value = 0.5f;
  M3_TEST_OK(m3_progress_test_slider_value_to_fraction(&slider, &fraction));
  M3_TEST_ASSERT(m3_near(fraction, 0.5f, 0.001f));
  M3_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_FRACTION_NEGATIVE));
  M3_TEST_OK(m3_progress_test_slider_value_to_fraction(&slider, &fraction));
  M3_TEST_ASSERT(m3_near(fraction, 0.0f, 0.001f));
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_FRACTION_POSITIVE));
  M3_TEST_OK(m3_progress_test_slider_value_to_fraction(&slider, &fraction));
  M3_TEST_ASSERT(m3_near(fraction, 1.0f, 0.001f));
  M3_TEST_OK(m3_progress_test_clear_fail_points());

  slider.bounds.x = 0.0f;
  slider.bounds.y = 0.0f;
  slider.bounds.width = 0.0f;
  slider.bounds.height = 10.0f;
  M3_TEST_EXPECT(m3_progress_test_slider_value_from_x(NULL, 1.0f, &value),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_progress_test_slider_value_from_x(&slider, 1.0f, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_progress_test_slider_value_from_x(&slider, 1.0f, &value),
                 M3_ERR_RANGE);
  slider.bounds.width = 10.0f;
  slider.max_value = 0.0f;
  M3_TEST_EXPECT(m3_progress_test_slider_value_from_x(&slider, 1.0f, &value),
                 M3_ERR_RANGE);
  slider.max_value = 1.0f;
  slider.step = -1.0f;
  M3_TEST_EXPECT(m3_progress_test_slider_value_from_x(&slider, 1.0f, &value),
                 M3_ERR_RANGE);
  slider.step = 0.0f;
  M3_TEST_OK(m3_progress_test_slider_value_from_x(&slider, -5.0f, &value));
  M3_TEST_ASSERT(m3_near(value, 0.0f, 0.001f));
  M3_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_FROM_X_FRACTION_HIGH));
  M3_TEST_OK(m3_progress_test_slider_value_from_x(&slider, 5.0f, &value));
  M3_TEST_OK(m3_progress_test_clear_fail_points());

  M3_TEST_EXPECT(
      m3_progress_test_slider_snap_value(0.0f, 0.0f, 1.0f, 0.0f, NULL),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_progress_test_slider_snap_value(0.0f, 0.0f, 1.0f, -1.0f, &value),
      M3_ERR_RANGE);
  M3_TEST_EXPECT(
      m3_progress_test_slider_snap_value(0.0f, 1.0f, 0.0f, 0.0f, &value),
      M3_ERR_RANGE);
  M3_TEST_EXPECT(
      m3_progress_test_slider_snap_value(0.0f, 0.0f, 1.0f, 2.0f, &value),
      M3_ERR_RANGE);
  M3_TEST_OK(
      m3_progress_test_slider_snap_value(-1.0f, 0.0f, 1.0f, 0.0f, &value));
  M3_TEST_ASSERT(m3_near(value, 0.0f, 0.001f));
  M3_TEST_OK(
      m3_progress_test_slider_snap_value(2.0f, 0.0f, 1.0f, 0.0f, &value));
  M3_TEST_ASSERT(m3_near(value, 1.0f, 0.001f));
  M3_TEST_OK(
      m3_progress_test_slider_snap_value(0.6f, 0.0f, 1.0f, 0.25f, &value));
  M3_TEST_ASSERT(m3_near(value, 0.5f, 0.001f));
  M3_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_COUNT_NEGATIVE));
  M3_TEST_EXPECT(
      m3_progress_test_slider_snap_value(0.6f, 0.0f, 1.0f, 0.25f, &value),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_CLAMP_MIN));
  M3_TEST_OK(
      m3_progress_test_slider_snap_value(0.6f, 0.0f, 1.0f, 0.25f, &value));
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_CLAMP_MAX));
  M3_TEST_OK(
      m3_progress_test_slider_snap_value(0.6f, 0.0f, 1.0f, 0.25f, &value));
  M3_TEST_OK(m3_progress_test_clear_fail_points());

  M3_TEST_OK(m3_slider_init(&slider, &slider_style, 0.0f, 1.0f, 0.5f));
  M3_TEST_EXPECT(m3_progress_test_slider_resolve_colors(NULL, &color,
                                                        &out_color, &out_color),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_progress_test_slider_resolve_colors(&slider, NULL,
                                                        &out_color, &out_color),
                 M3_ERR_INVALID_ARGUMENT);
  slider.widget.flags = M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(m3_progress_test_slider_resolve_colors(&slider, &color, &out_color,
                                                    &out_color));
  slider.widget.flags = 0u;
  slider.style.track_color.r = -1.0f;
  M3_TEST_EXPECT(m3_progress_test_slider_resolve_colors(&slider, &color,
                                                        &out_color, &out_color),
                 M3_ERR_RANGE);
  slider.style = slider_style;
  slider.style.active_track_color.r = -1.0f;
  M3_TEST_EXPECT(m3_progress_test_slider_resolve_colors(&slider, &color,
                                                        &out_color, &out_color),
                 M3_ERR_RANGE);
  slider.style = slider_style;
  slider.style.thumb_color.r = -1.0f;
  M3_TEST_EXPECT(m3_progress_test_slider_resolve_colors(&slider, &color,
                                                        &out_color, &out_color),
                 M3_ERR_RANGE);
  slider.style = slider_style;

  M3_TEST_EXPECT(m3_progress_test_slider_update_value(NULL, 0.5f, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);

  return M3_OK;
}

int main(void) {
  TestProgressBackend backend;
  M3Gfx gfx;
  M3PaintContext paint_ctx;
  M3LinearProgressStyle linear_style;
  M3LinearProgressStyle bad_linear_style;
  M3LinearProgress linear;
  M3CircularProgressStyle circular_style;
  M3CircularProgressStyle bad_circular_style;
  M3CircularProgress circular;
  M3SliderStyle slider_style;
  M3SliderStyle bad_slider_style;
  M3Slider slider;
  M3MeasureSpec width_spec;
  M3MeasureSpec height_spec;
  M3Size size;
  M3Rect bounds;
  M3Semantics semantics;
  M3Bool handled;
  M3Scalar value;
  SliderCounter counter;
  M3InputEvent event;

  M3_TEST_OK(test_progress_helpers());

  M3_TEST_OK(test_backend_init(&backend));
  gfx.ctx = &backend;
  gfx.vtable = &g_test_vtable;
  gfx.text_vtable = NULL;

  paint_ctx.gfx = &gfx;
  paint_ctx.dpi_scale = 1.0f;
  paint_ctx.clip.x = 0.0f;
  paint_ctx.clip.y = 0.0f;
  paint_ctx.clip.width = 200.0f;
  paint_ctx.clip.height = 200.0f;

  M3_TEST_EXPECT(m3_linear_progress_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_linear_progress_style_init(&linear_style));
  M3_TEST_ASSERT(
      m3_near(linear_style.height, M3_LINEAR_PROGRESS_DEFAULT_HEIGHT, 0.001f));

  M3_TEST_EXPECT(m3_linear_progress_init(NULL, &linear_style, 0.5f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_linear_progress_init(&linear, NULL, 0.5f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_linear_progress_init(&linear, &linear_style, -0.1f),
                 M3_ERR_RANGE);

  bad_linear_style = linear_style;
  bad_linear_style.height = 0.0f;
  M3_TEST_EXPECT(m3_linear_progress_init(&linear, &bad_linear_style, 0.5f),
                 M3_ERR_RANGE);

  M3_TEST_OK(m3_linear_progress_init(&linear, &linear_style, 0.25f));
  M3_TEST_ASSERT(linear.widget.ctx == &linear);
  M3_TEST_ASSERT(linear.widget.vtable != NULL);

  M3_TEST_EXPECT(m3_linear_progress_set_value(NULL, 0.5f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_linear_progress_set_value(&linear, 1.5f), M3_ERR_RANGE);
  M3_TEST_OK(m3_linear_progress_set_value(&linear, 0.75f));
  M3_TEST_OK(m3_linear_progress_get_value(&linear, &value));
  M3_TEST_ASSERT(m3_near(value, 0.75f, 0.001f));
  M3_TEST_EXPECT(m3_linear_progress_get_value(NULL, &value),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_linear_progress_get_value(&linear, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_linear_progress_set_label(NULL, "A", 1),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_linear_progress_set_label(&linear, NULL, 1),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_linear_progress_set_label(&linear, "Loading", 7));

  M3_TEST_EXPECT(m3_linear_progress_set_style(NULL, &linear_style),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_linear_progress_set_style(&linear, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  bad_linear_style = linear_style;
  bad_linear_style.track_color.r = -1.0f;
  M3_TEST_EXPECT(m3_linear_progress_set_style(&linear, &bad_linear_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_linear_progress_set_style(&linear, &linear_style));

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(linear.widget.vtable->measure(linear.widget.ctx, width_spec,
                                               height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      linear.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(linear.widget.vtable->measure(linear.widget.ctx, width_spec,
                                               height_spec, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  M3_TEST_EXPECT(linear.widget.vtable->measure(linear.widget.ctx, width_spec,
                                               height_spec, &size),
                 M3_ERR_RANGE);

  bad_linear_style = linear_style;
  bad_linear_style.min_width = -1.0f;
  linear.style = bad_linear_style;
  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(linear.widget.vtable->measure(linear.widget.ctx, width_spec,
                                               height_spec, &size),
                 M3_ERR_RANGE);
  linear.style = linear_style;

  width_spec.mode = M3_MEASURE_EXACTLY;
  width_spec.size = 10.0f;
  height_spec.mode = M3_MEASURE_AT_MOST;
  height_spec.size = -1.0f;
  M3_TEST_EXPECT(linear.widget.vtable->measure(linear.widget.ctx, width_spec,
                                               height_spec, &size),
                 M3_ERR_RANGE);

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_OK(linear.widget.vtable->measure(linear.widget.ctx, width_spec,
                                           height_spec, &size));
  M3_TEST_ASSERT(m3_near(size.width, linear_style.min_width, 0.001f));
  M3_TEST_ASSERT(m3_near(size.height, linear_style.height, 0.001f));

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = 10.0f;
  height_spec.mode = M3_MEASURE_EXACTLY;
  height_spec.size = 12.0f;
  M3_TEST_OK(linear.widget.vtable->measure(linear.widget.ctx, width_spec,
                                           height_spec, &size));
  M3_TEST_ASSERT(m3_near(size.width, 10.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(size.height, 12.0f, 0.001f));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  M3_TEST_EXPECT(linear.widget.vtable->layout(linear.widget.ctx, bounds),
                 M3_ERR_RANGE);

  bounds.width = 120.0f;
  bounds.height = 8.0f;
  M3_TEST_OK(linear.widget.vtable->layout(linear.widget.ctx, bounds));

  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_EXPECT(linear.widget.vtable->paint(NULL, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  gfx.vtable = NULL;
  M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_vtable_no_rect;
  M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable;

  linear.value = 2.0f;
  M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  linear.value = 0.5f;

  linear.bounds.width = -1.0f;
  M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  linear.bounds = bounds;

  bad_linear_style = linear_style;
  bad_linear_style.height = 0.0f;
  linear.style = bad_linear_style;
  M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  linear.style = linear_style;

  linear.style.indicator_color.r = -1.0f;
  M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  linear.style = linear_style;
  M3_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_LINEAR_RESOLVE_COLORS));
  M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_LINEAR_CORNER_RANGE));
  M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_LINEAR_FILL_CORNER_CLAMP));
  M3_TEST_OK(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx));
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_LINEAR_FILL_CORNER_RANGE));
  M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());

  M3_TEST_OK(test_backend_init(&backend));
  backend.fail_draw_rect = M3_ERR_IO;
  M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                 M3_ERR_IO);

  M3_TEST_OK(test_backend_init(&backend));
  linear.style = linear_style;
  linear.style.track_color.a = 0.0f;
  M3_TEST_OK(m3_linear_progress_set_style(&linear, &linear.style));
  linear.value = 0.5f;
  backend.fail_draw_rect = M3_ERR_IO;
  M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend.fail_draw_rect = M3_OK;
  linear.style = linear_style;
  M3_TEST_OK(m3_linear_progress_set_style(&linear, &linear.style));

  M3_TEST_OK(test_backend_init(&backend));
  linear.value = 0.0f;
  M3_TEST_OK(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(backend.draw_rect_calls == 1);

  M3_TEST_OK(test_backend_init(&backend));
  linear.value = 0.5f;
  M3_TEST_OK(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(backend.draw_rect_calls == 2);

  linear.style = linear_style;
  linear.style.track_color.a = 0.0f;
  M3_TEST_OK(m3_linear_progress_set_style(&linear, &linear.style));
  M3_TEST_OK(test_backend_init(&backend));
  linear.value = 0.5f;
  M3_TEST_OK(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx));

  linear.style = linear_style;
  linear.style.indicator_color.a = 0.0f;
  M3_TEST_OK(m3_linear_progress_set_style(&linear, &linear.style));
  M3_TEST_OK(test_backend_init(&backend));
  linear.value = 0.5f;
  M3_TEST_OK(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx));

  linear.style = linear_style;
  linear.style.height = 8.0f;
  linear.style.corner_radius = 4.0f;
  M3_TEST_OK(m3_linear_progress_set_style(&linear, &linear.style));
  bounds.width = 2.0f;
  bounds.height = 4.0f;
  M3_TEST_OK(linear.widget.vtable->layout(linear.widget.ctx, bounds));
  M3_TEST_OK(test_backend_init(&backend));
  linear.value = 1.0f;
  M3_TEST_OK(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx));

  M3_TEST_EXPECT(linear.widget.vtable->layout(NULL, bounds),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(linear.widget.vtable->event(NULL, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(linear.widget.vtable->event(linear.widget.ctx, NULL, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(linear.widget.vtable->event(linear.widget.ctx, &event, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(linear.widget.vtable->get_semantics(NULL, &semantics),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(linear.widget.vtable->get_semantics(linear.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(linear.widget.vtable->destroy(NULL), M3_ERR_INVALID_ARGUMENT);

  linear.widget.flags = M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(
      linear.widget.vtable->get_semantics(linear.widget.ctx, &semantics));
  M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_NONE);
  M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_DISABLED) != 0u);
  linear.widget.flags = 0u;

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
  M3_TEST_OK(linear.widget.vtable->event(linear.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(linear.widget.vtable->destroy(linear.widget.ctx));
  M3_TEST_ASSERT(linear.widget.vtable == NULL);

  M3_TEST_EXPECT(m3_circular_progress_style_init(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_circular_progress_style_init(&circular_style));
  M3_TEST_ASSERT(circular_style.segments ==
                 M3_CIRCULAR_PROGRESS_DEFAULT_SEGMENTS);

  M3_TEST_EXPECT(m3_circular_progress_init(NULL, &circular_style, 0.5f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_circular_progress_init(&circular, NULL, 0.5f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_circular_progress_init(&circular, &circular_style, 1.5f),
                 M3_ERR_RANGE);

  bad_circular_style = circular_style;
  bad_circular_style.segments = 2u;
  M3_TEST_EXPECT(
      m3_circular_progress_init(&circular, &bad_circular_style, 0.5f),
      M3_ERR_RANGE);

  M3_TEST_OK(m3_circular_progress_init(&circular, &circular_style, 0.25f));
  M3_TEST_ASSERT(circular.widget.ctx == &circular);
  M3_TEST_ASSERT(circular.widget.vtable != NULL);

  M3_TEST_EXPECT(m3_circular_progress_set_value(NULL, 0.5f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_circular_progress_set_value(&circular, -0.5f),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_circular_progress_set_value(&circular, 0.75f));
  M3_TEST_OK(m3_circular_progress_get_value(&circular, &value));
  M3_TEST_ASSERT(m3_near(value, 0.75f, 0.001f));

  M3_TEST_EXPECT(m3_circular_progress_get_value(NULL, &value),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_circular_progress_get_value(&circular, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_circular_progress_set_label(NULL, "B", 1),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_circular_progress_set_label(&circular, NULL, 1),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_circular_progress_set_label(&circular, "Sync", 4));

  bad_circular_style = circular_style;
  bad_circular_style.track_color.a = 2.0f;
  M3_TEST_EXPECT(m3_circular_progress_set_style(&circular, &bad_circular_style),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_circular_progress_set_style(NULL, &circular_style),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_circular_progress_set_style(&circular, &circular_style));

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(circular.widget.vtable->measure(
                     circular.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      circular.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(circular.widget.vtable->measure(circular.widget.ctx,
                                                 width_spec, height_spec, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  M3_TEST_EXPECT(circular.widget.vtable->measure(
                     circular.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_RANGE);

  bad_circular_style = circular_style;
  bad_circular_style.diameter = -1.0f;
  circular.style = bad_circular_style;
  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(circular.widget.vtable->measure(
                     circular.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_RANGE);
  circular.style = circular_style;

  width_spec.mode = M3_MEASURE_EXACTLY;
  width_spec.size = 40.0f;
  height_spec.mode = M3_MEASURE_AT_MOST;
  height_spec.size = -1.0f;
  M3_TEST_EXPECT(circular.widget.vtable->measure(
                     circular.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_RANGE);

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_OK(circular.widget.vtable->measure(circular.widget.ctx, width_spec,
                                             height_spec, &size));
  M3_TEST_ASSERT(m3_near(size.width, circular_style.diameter, 0.001f));
  M3_TEST_ASSERT(m3_near(size.height, circular_style.diameter, 0.001f));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  M3_TEST_EXPECT(circular.widget.vtable->layout(circular.widget.ctx, bounds),
                 M3_ERR_RANGE);

  bounds.width = 60.0f;
  bounds.height = 60.0f;
  M3_TEST_OK(circular.widget.vtable->layout(circular.widget.ctx, bounds));

  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_EXPECT(circular.widget.vtable->paint(NULL, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(circular.widget.vtable->paint(circular.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  gfx.vtable = &g_test_vtable_no_line;
  M3_TEST_EXPECT(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable;

  circular.value = 2.0f;
  M3_TEST_EXPECT(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  circular.value = 0.25f;

  circular.bounds.width = -1.0f;
  M3_TEST_EXPECT(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  circular.bounds = bounds;

  bad_circular_style = circular_style;
  bad_circular_style.thickness = bad_circular_style.diameter;
  circular.style = bad_circular_style;
  M3_TEST_EXPECT(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  circular.style = circular_style;
  M3_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_CIRCULAR_RESOLVE_COLORS));
  M3_TEST_EXPECT(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  M3_TEST_OK(m3_progress_test_clear_fail_points());

  M3_TEST_OK(test_backend_init(&backend));
  backend.fail_draw_line = M3_ERR_IO;
  M3_TEST_EXPECT(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
                 M3_ERR_IO);

  M3_TEST_OK(test_backend_init(&backend));
  circular.style = circular_style;
  circular.style.track_color.a = 0.0f;
  M3_TEST_OK(m3_circular_progress_set_style(&circular, &circular.style));
  backend.fail_draw_line = M3_ERR_IO;
  M3_TEST_EXPECT(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend.fail_draw_line = M3_OK;

  gfx.vtable = NULL;
  M3_TEST_EXPECT(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_vtable;

  bounds.width = 60.0f;
  bounds.height = 40.0f;
  M3_TEST_OK(circular.widget.vtable->layout(circular.widget.ctx, bounds));
  circular.style = circular_style;
  circular.style.track_color.a = 0.0f;
  M3_TEST_OK(m3_circular_progress_set_style(&circular, &circular.style));
  M3_TEST_OK(test_backend_init(&backend));
  circular.value = 0.5f;
  M3_TEST_OK(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx));

  bounds.width = 0.0f;
  bounds.height = 10.0f;
  M3_TEST_OK(circular.widget.vtable->layout(circular.widget.ctx, bounds));
  M3_TEST_EXPECT(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  bounds.width = 4.0f;
  bounds.height = 4.0f;
  M3_TEST_OK(circular.widget.vtable->layout(circular.widget.ctx, bounds));
  M3_TEST_EXPECT(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);

  bounds.width = 60.0f;
  bounds.height = 40.0f;
  M3_TEST_OK(circular.widget.vtable->layout(circular.widget.ctx, bounds));

  M3_TEST_EXPECT(circular.widget.vtable->event(NULL, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      circular.widget.vtable->event(circular.widget.ctx, NULL, &handled),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      circular.widget.vtable->event(circular.widget.ctx, &event, NULL),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(circular.widget.vtable->get_semantics(NULL, &semantics),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      circular.widget.vtable->get_semantics(circular.widget.ctx, NULL),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(circular.widget.vtable->layout(NULL, bounds),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(circular.widget.vtable->destroy(NULL),
                 M3_ERR_INVALID_ARGUMENT);

  circular.style = circular_style;
  M3_TEST_OK(m3_circular_progress_set_style(&circular, &circular.style));
  M3_TEST_OK(test_backend_init(&backend));
  circular.value = 0.0f;
  M3_TEST_OK(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(backend.draw_line_calls == (int)circular.style.segments);

  M3_TEST_OK(test_backend_init(&backend));
  circular.value = 0.5f;
  M3_TEST_OK(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(backend.draw_line_calls > (int)circular.style.segments);

  circular.widget.flags = M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(
      circular.widget.vtable->get_semantics(circular.widget.ctx, &semantics));
  M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_DISABLED) != 0u);
  circular.widget.flags = 0u;

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
  M3_TEST_OK(
      circular.widget.vtable->event(circular.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(circular.widget.vtable->destroy(circular.widget.ctx));
  M3_TEST_ASSERT(circular.widget.vtable == NULL);

  M3_TEST_EXPECT(m3_slider_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_slider_style_init(&slider_style));
  M3_TEST_ASSERT(
      m3_near(slider_style.track_length, M3_SLIDER_DEFAULT_LENGTH, 0.001f));

  M3_TEST_EXPECT(m3_slider_init(NULL, &slider_style, 0.0f, 1.0f, 0.5f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_slider_init(&slider, NULL, 0.0f, 1.0f, 0.5f),
                 M3_ERR_INVALID_ARGUMENT);

  bad_slider_style = slider_style;
  bad_slider_style.thumb_radius = 0.0f;
  M3_TEST_EXPECT(m3_slider_init(&slider, &bad_slider_style, 0.0f, 1.0f, 0.5f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_slider_init(&slider, &slider_style, 1.0f, 0.0f, 0.5f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_slider_init(&slider, &slider_style, 0.0f, 1.0f, 2.0f),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_COUNT_NEGATIVE));
  M3_TEST_EXPECT(m3_slider_init(&slider, &slider_style, 0.0f, 1.0f, 0.5f),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());

  M3_TEST_OK(m3_slider_init(&slider, &slider_style, 0.0f, 1.0f, 0.25f));
  M3_TEST_ASSERT(slider.widget.ctx == &slider);
  M3_TEST_ASSERT(slider.widget.vtable != NULL);
  M3_TEST_ASSERT((slider.widget.flags & M3_WIDGET_FLAG_FOCUSABLE) != 0u);

  M3_TEST_EXPECT(m3_slider_set_value(NULL, 0.5f), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_slider_set_value(&slider, 2.0f), M3_ERR_RANGE);
  slider.min_value = 1.0f;
  slider.max_value = 1.0f;
  M3_TEST_EXPECT(m3_slider_set_value(&slider, 1.0f), M3_ERR_RANGE);
  slider.min_value = 0.0f;
  slider.max_value = 1.0f;
  M3_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_COUNT_NEGATIVE));
  M3_TEST_EXPECT(m3_slider_set_value(&slider, 0.5f), M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_EXPECT(m3_slider_get_value(NULL, &value), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_slider_get_value(&slider, NULL), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_slider_set_step(&slider, 0.25f));
  M3_TEST_OK(m3_slider_set_value(&slider, 0.4f));
  M3_TEST_OK(m3_slider_get_value(&slider, &value));
  M3_TEST_ASSERT(m3_near(value, 0.5f, 0.001f));

  M3_TEST_EXPECT(m3_slider_set_step(NULL, 0.1f), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_slider_set_step(&slider, -1.0f), M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_slider_set_step(&slider, 2.0f), M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_COUNT_NEGATIVE));
  M3_TEST_EXPECT(m3_slider_set_step(&slider, 0.25f), M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());

  M3_TEST_EXPECT(m3_slider_set_range(NULL, 0.0f, 1.0f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_slider_set_range(&slider, 1.0f, 0.0f), M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_slider_set_range(&slider, 0.0f, 0.1f), M3_ERR_RANGE);
  slider.step = -1.0f;
  M3_TEST_EXPECT(m3_slider_set_range(&slider, 0.0f, 1.0f), M3_ERR_RANGE);
  slider.step = 0.0f;
  M3_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_COUNT_NEGATIVE));
  M3_TEST_EXPECT(m3_slider_set_range(&slider, 0.0f, 1.0f), M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(m3_slider_set_step(&slider, 0.0f));
  M3_TEST_OK(m3_slider_set_range(&slider, 0.0f, 1.0f));

  M3_TEST_EXPECT(m3_slider_set_style(NULL, &slider_style),
                 M3_ERR_INVALID_ARGUMENT);
  bad_slider_style = slider_style;
  bad_slider_style.track_color.a = 2.0f;
  M3_TEST_EXPECT(m3_slider_set_style(&slider, &bad_slider_style), M3_ERR_RANGE);
  M3_TEST_OK(m3_slider_set_style(&slider, &slider_style));

  M3_TEST_EXPECT(m3_slider_set_label(NULL, "S", 1), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_slider_set_label(&slider, NULL, 1),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_slider_set_label(&slider, "Volume", 6));

  M3_TEST_EXPECT(m3_slider_set_on_change(NULL, test_slider_on_change, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  counter.calls = 0;
  counter.last_value = 0.0f;
  counter.fail = 0;
  M3_TEST_OK(m3_slider_set_on_change(&slider, test_slider_on_change, &counter));

  M3_TEST_EXPECT(slider.widget.vtable->event(NULL, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(slider.widget.vtable->event(slider.widget.ctx, NULL, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(slider.widget.vtable->event(slider.widget.ctx, &event, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(slider.widget.vtable->measure(slider.widget.ctx, width_spec,
                                               height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      slider.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(slider.widget.vtable->measure(slider.widget.ctx, width_spec,
                                               height_spec, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  M3_TEST_EXPECT(slider.widget.vtable->measure(slider.widget.ctx, width_spec,
                                               height_spec, &size),
                 M3_ERR_RANGE);

  bad_slider_style = slider_style;
  bad_slider_style.track_height = 0.0f;
  slider.style = bad_slider_style;
  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(slider.widget.vtable->measure(slider.widget.ctx, width_spec,
                                               height_spec, &size),
                 M3_ERR_RANGE);
  slider.style = slider_style;

  width_spec.mode = M3_MEASURE_EXACTLY;
  width_spec.size = 80.0f;
  height_spec.mode = M3_MEASURE_AT_MOST;
  height_spec.size = -1.0f;
  M3_TEST_EXPECT(slider.widget.vtable->measure(slider.widget.ctx, width_spec,
                                               height_spec, &size),
                 M3_ERR_RANGE);

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_OK(slider.widget.vtable->measure(slider.widget.ctx, width_spec,
                                           height_spec, &size));
  M3_TEST_ASSERT(m3_near(size.width, slider_style.track_length, 0.001f));
  M3_TEST_ASSERT(
      m3_near(size.height, slider_style.thumb_radius * 2.0f, 0.001f));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 20.0f;
  M3_TEST_EXPECT(slider.widget.vtable->layout(slider.widget.ctx, bounds),
                 M3_ERR_RANGE);

  bounds.width = 100.0f;
  bounds.height = 20.0f;
  M3_TEST_OK(slider.widget.vtable->layout(slider.widget.ctx, bounds));

  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_EXPECT(slider.widget.vtable->paint(NULL, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  gfx.vtable = NULL;
  M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_vtable_no_rect;
  M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable;

  slider.bounds.width = 0.0f;
  M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  slider.bounds = bounds;

  bad_slider_style = slider_style;
  bad_slider_style.track_length = 0.0f;
  slider.style = bad_slider_style;
  M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  slider.style = slider_style;

  slider.bounds.width = -1.0f;
  M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  slider.bounds = bounds;

  slider.step = -1.0f;
  M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  slider.step = 0.0f;

  slider.max_value = slider.min_value;
  M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  slider.max_value = 1.0f;

  slider.value = 2.0f;
  M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  slider.value = 0.5f;

  M3_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_RESOLVE_COLORS));
  M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  M3_TEST_OK(m3_progress_test_clear_fail_points());

  M3_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_CORNER_CLAMP));
  M3_TEST_OK(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx));
  M3_TEST_OK(m3_progress_test_clear_fail_points());
  M3_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_CORNER_RANGE));
  M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_progress_test_clear_fail_points());

  M3_TEST_OK(test_backend_init(&backend));
  backend.fail_draw_rect_after = 2;
  slider.value = 0.5f;
  M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend.fail_draw_rect_after = 0;

  M3_TEST_OK(test_backend_init(&backend));
  backend.fail_draw_rect_after = 3;
  slider.value = 0.5f;
  M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend.fail_draw_rect_after = 0;

  M3_TEST_OK(test_backend_init(&backend));
  backend.fail_draw_rect = M3_ERR_IO;
  M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                 M3_ERR_IO);

  M3_TEST_OK(test_backend_init(&backend));
  slider.value = 0.5f;
  M3_TEST_OK(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(backend.draw_rect_calls == 3);

  slider.step = 2.0f;
  M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  slider.step = 0.0f;

  slider.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(
      slider.widget.vtable->get_semantics(slider.widget.ctx, &semantics));
  M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_SLIDER);
  M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_DISABLED) != 0u);
  slider.widget.flags = M3_WIDGET_FLAG_FOCUSABLE;

  M3_TEST_EXPECT(slider.widget.vtable->get_semantics(NULL, &semantics),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(slider.widget.vtable->get_semantics(slider.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(slider.widget.vtable->layout(NULL, bounds),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(slider.widget.vtable->destroy(NULL), M3_ERR_INVALID_ARGUMENT);

  counter.calls = 0;
  counter.fail = 0;
  M3_TEST_OK(m3_slider_set_on_change(&slider, test_slider_on_change, &counter));

  slider.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 50, 10));
  M3_TEST_OK(slider.widget.vtable->event(slider.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  slider.widget.flags = M3_WIDGET_FLAG_FOCUSABLE;

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_MOVE, 50, 10));
  M3_TEST_OK(slider.widget.vtable->event(slider.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 50, 10));
  M3_TEST_OK(slider.widget.vtable->event(slider.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(counter.calls == 1);
  M3_TEST_ASSERT(m3_near(counter.last_value, 0.5f, 0.01f));

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_MOVE, 80, 10));
  M3_TEST_OK(slider.widget.vtable->event(slider.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(counter.calls == 2);
  M3_TEST_ASSERT(m3_near(counter.last_value, 0.8f, 0.02f));

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 80, 10));
  M3_TEST_OK(slider.widget.vtable->event(slider.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 80, 10));
  M3_TEST_OK(slider.widget.vtable->event(slider.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  slider.pressed = M3_TRUE;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
  M3_TEST_EXPECT(
      slider.widget.vtable->event(slider.widget.ctx, &event, &handled),
      M3_ERR_STATE);
  slider.pressed = M3_FALSE;

  slider.step = -1.0f;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
  M3_TEST_EXPECT(
      slider.widget.vtable->event(slider.widget.ctx, &event, &handled),
      M3_ERR_RANGE);
  slider.step = 0.0f;

  slider.pressed = M3_TRUE;
  slider.bounds.width = 0.0f;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_MOVE, 10, 10));
  M3_TEST_EXPECT(
      slider.widget.vtable->event(slider.widget.ctx, &event, &handled),
      M3_ERR_RANGE);
  slider.bounds.width = bounds.width;
  slider.pressed = M3_FALSE;

  counter.fail = 1;
  slider.value = 0.2f;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 90, 10));
  M3_TEST_EXPECT(
      slider.widget.vtable->event(slider.widget.ctx, &event, &handled),
      M3_ERR_IO);
  M3_TEST_ASSERT(m3_near(slider.value, 0.2f, 0.001f));
  M3_TEST_ASSERT(slider.pressed == M3_FALSE);

  counter.fail = 1;
  slider.pressed = M3_TRUE;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_MOVE, 90, 10));
  M3_TEST_EXPECT(
      slider.widget.vtable->event(slider.widget.ctx, &event, &handled),
      M3_ERR_IO);
  slider.pressed = M3_FALSE;
  counter.fail = 0;

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_KEY_DOWN, 0, 0));
  M3_TEST_OK(slider.widget.vtable->event(slider.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(slider.widget.vtable->destroy(slider.widget.ctx));
  M3_TEST_ASSERT(slider.widget.vtable == NULL);

  return 0;
}
