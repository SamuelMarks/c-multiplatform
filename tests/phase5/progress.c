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
  CMPRect last_rect;
  CMPColor last_rect_color;
  CMPScalar last_corner;
  CMPScalar last_line_x0;
  CMPScalar last_line_y0;
  CMPScalar last_line_x1;
  CMPScalar last_line_y1;
  CMPColor last_line_color;
  CMPScalar last_line_thickness;
} TestProgressBackend;

typedef struct SliderCounter {
  int calls;
  CMPScalar last_value;
  int fail;
} SliderCounter;

static int test_backend_init(TestProgressBackend *backend) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(backend, 0, sizeof(*backend));
  backend->fail_draw_rect = CMP_OK;
  backend->fail_draw_rect_after = 0;
  backend->fail_draw_line = CMP_OK;
  return CMP_OK;
}

static int test_gfx_draw_rect(void *gfx, const CMPRect *rect, CMPColor color,
                              CMPScalar corner_radius) {
  TestProgressBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestProgressBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_rect_color = color;
  backend->last_corner = corner_radius;
  if (backend->fail_draw_rect != CMP_OK) {
    return backend->fail_draw_rect;
  }
  if (backend->fail_draw_rect_after > 0 &&
      backend->draw_rect_calls >= backend->fail_draw_rect_after) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_gfx_draw_line(void *gfx, CMPScalar x0, CMPScalar y0,
                              CMPScalar x1, CMPScalar y1, CMPColor color,
                              CMPScalar thickness) {
  TestProgressBackend *backend;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestProgressBackend *)gfx;
  backend->draw_line_calls += 1;
  backend->last_line_x0 = x0;
  backend->last_line_y0 = y0;
  backend->last_line_x1 = x1;
  backend->last_line_y1 = y1;
  backend->last_line_color = color;
  backend->last_line_thickness = thickness;
  if (backend->fail_draw_line != CMP_OK) {
    return backend->fail_draw_line;
  }
  return CMP_OK;
}

static const CMPGfxVTable g_test_vtable = {NULL,
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
                                           NULL,
                                           NULL};

static const CMPGfxVTable g_test_vtable_no_rect = {
    NULL, NULL, NULL, NULL, test_gfx_draw_line, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL};

static const CMPGfxVTable g_test_vtable_no_line = {
    NULL, NULL, NULL, test_gfx_draw_rect, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL};

static int cmp_near(CMPScalar a, CMPScalar b, CMPScalar tol) {
  CMPScalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

static int init_pointer_event(CMPInputEvent *event, cmp_u32 type, cmp_i32 x,
                              cmp_i32 y) {
  if (event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(event, 0, sizeof(*event));
  event->type = type;
  event->data.pointer.x = x;
  event->data.pointer.y = y;
  return CMP_OK;
}

static int test_slider_on_change(void *ctx, struct M3Slider *slider,
                                 CMPScalar value) {
  SliderCounter *counter;

  if (ctx == NULL || slider == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  counter = (SliderCounter *)ctx;
  counter->calls += 1;
  counter->last_value = value;
  if (counter->fail) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_progress_helpers(void) {
  TestProgressBackend backend;
  CMPGfx gfx;
  CMPColor color;
  CMPColor out_color;
  CMPMeasureSpec spec;
  CMPScalar size;
  CMPRect rect;
  M3LinearProgressStyle linear_style;
  M3LinearProgress linear;
  M3CircularProgressStyle circular_style;
  M3CircularProgress circular;
  M3SliderStyle slider_style;
  M3Slider slider;
  CMPScalar fraction;
  CMPScalar value;
  volatile const CMPColor *null_color;
  volatile const CMPRect *null_rect;
  volatile CMPScalar invalid_scalar;

  null_color = NULL;
  null_rect = NULL;
  CMP_TEST_EXPECT(m3_progress_test_validate_color((const CMPColor *)null_color),
                  CMP_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  CMP_TEST_EXPECT(m3_progress_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 0.0f;
  invalid_scalar = 1.2f;
  color.g = (CMPScalar)invalid_scalar;
  CMP_TEST_EXPECT(m3_progress_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = 0.0f;
  invalid_scalar = 1.2f;
  color.b = (CMPScalar)invalid_scalar;
  CMP_TEST_EXPECT(m3_progress_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = 0.0f;
  invalid_scalar = 1.2f;
  color.a = (CMPScalar)invalid_scalar;
  CMP_TEST_EXPECT(m3_progress_test_validate_color(&color), CMP_ERR_RANGE);
  color.a = 1.0f;
  CMP_TEST_OK(m3_progress_test_validate_color(&color));

  CMP_TEST_EXPECT(m3_progress_test_color_set((CMPColor *)null_color, 0.0f, 0.0f,
                                             0.0f, 0.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_progress_test_color_set(&color, -0.1f, 0.0f, 0.0f, 0.0f),
                  CMP_ERR_RANGE);
  invalid_scalar = 1.2f;
  CMP_TEST_EXPECT(m3_progress_test_color_set(
                      &color, 0.0f, (CMPScalar)invalid_scalar, 0.0f, 0.0f),
                  CMP_ERR_RANGE);
  invalid_scalar = 1.2f;
  CMP_TEST_EXPECT(m3_progress_test_color_set(&color, 0.0f, 0.0f,
                                             (CMPScalar)invalid_scalar, 0.0f),
                  CMP_ERR_RANGE);
  invalid_scalar = 1.2f;
  CMP_TEST_EXPECT(m3_progress_test_color_set(&color, 0.0f, 0.0f, 0.0f,
                                             (CMPScalar)invalid_scalar),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_color_set(&color, 0.1f, 0.2f, 0.3f, 0.4f));

  CMP_TEST_EXPECT(m3_progress_test_color_with_alpha(
                      (const CMPColor *)null_color, 0.5f, &out_color),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_progress_test_color_with_alpha(&color, 0.5f, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_progress_test_color_with_alpha(&color, -0.1f, &out_color),
                  CMP_ERR_RANGE);
  color.r = 2.0f;
  CMP_TEST_EXPECT(m3_progress_test_color_with_alpha(&color, 0.5f, &out_color),
                  CMP_ERR_RANGE);
  color.r = 0.1f;
  CMP_TEST_OK(m3_progress_test_color_with_alpha(&color, 0.5f, &out_color));
  CMP_TEST_ASSERT(cmp_near(out_color.a, 0.2f, 0.001f));

  spec.mode = 99u;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(m3_progress_test_validate_measure_spec(spec),
                  CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_MEASURE_AT_MOST;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(m3_progress_test_validate_measure_spec(spec), CMP_ERR_RANGE);
  spec.mode = CMP_MEASURE_UNSPECIFIED;
  spec.size = -1.0f;
  CMP_TEST_OK(m3_progress_test_validate_measure_spec(spec));
  CMP_TEST_EXPECT(m3_progress_test_apply_measure(1.0f, spec, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_MEASURE_EXACTLY;
  spec.size = 5.0f;
  CMP_TEST_OK(m3_progress_test_apply_measure(1.0f, spec, &size));
  CMP_TEST_ASSERT(cmp_near(size, 5.0f, 0.001f));
  spec.mode = CMP_MEASURE_AT_MOST;
  spec.size = 3.0f;
  CMP_TEST_OK(m3_progress_test_apply_measure(5.0f, spec, &size));
  CMP_TEST_ASSERT(cmp_near(size, 3.0f, 0.001f));

  CMP_TEST_EXPECT(m3_progress_test_validate_rect((const CMPRect *)null_rect),
                  CMP_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 1.0f;
  CMP_TEST_EXPECT(m3_progress_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.width = 1.0f;
  rect.height = -1.0f;
  CMP_TEST_EXPECT(m3_progress_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.height = 1.0f;
  CMP_TEST_OK(m3_progress_test_validate_rect(&rect));

  CMP_TEST_EXPECT(m3_progress_test_validate_value01(-0.1f), CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_progress_test_validate_value01(1.1f), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_validate_value01(0.5f));

  CMP_TEST_OK(m3_progress_test_set_fail_point(M3_PROGRESS_TEST_FAIL_COLOR_SET));
  CMP_TEST_EXPECT(m3_linear_progress_style_init(&linear_style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(
      m3_progress_test_set_fail_point(M3_PROGRESS_TEST_FAIL_COLOR_WITH_ALPHA));
  CMP_TEST_EXPECT(m3_linear_progress_style_init(&linear_style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(m3_progress_test_set_color_set_fail_after(2u));
  CMP_TEST_EXPECT(m3_linear_progress_style_init(&linear_style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(m3_progress_test_set_color_alpha_fail_after(2u));
  CMP_TEST_EXPECT(m3_linear_progress_style_init(&linear_style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());

  CMP_TEST_OK(m3_linear_progress_style_init(&linear_style));
  CMP_TEST_EXPECT(m3_progress_test_linear_validate_style(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  linear_style.height = 0.0f;
  CMP_TEST_EXPECT(m3_progress_test_linear_validate_style(&linear_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_linear_progress_style_init(&linear_style));
  linear_style.min_width = -1.0f;
  CMP_TEST_EXPECT(m3_progress_test_linear_validate_style(&linear_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_linear_progress_style_init(&linear_style));
  linear_style.corner_radius = linear_style.height;
  CMP_TEST_EXPECT(m3_progress_test_linear_validate_style(&linear_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_linear_progress_style_init(&linear_style));
  linear_style.track_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_progress_test_linear_validate_style(&linear_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_linear_progress_style_init(&linear_style));
  linear_style.disabled_track_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_progress_test_linear_validate_style(&linear_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_linear_progress_style_init(&linear_style));
  linear_style.disabled_indicator_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_progress_test_linear_validate_style(&linear_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_linear_progress_style_init(&linear_style));
  CMP_TEST_OK(m3_progress_test_linear_validate_style(&linear_style));

  CMP_TEST_OK(m3_linear_progress_init(&linear, &linear_style, 0.5f));
  CMP_TEST_EXPECT(
      m3_progress_test_linear_resolve_colors(NULL, &color, &out_color),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_progress_test_linear_resolve_colors(&linear, NULL, &out_color),
      CMP_ERR_INVALID_ARGUMENT);
  linear.widget.flags = CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(
      m3_progress_test_linear_resolve_colors(&linear, &color, &out_color));
  linear.widget.flags = 0u;
  linear.style.track_color.r = -1.0f;
  CMP_TEST_EXPECT(
      m3_progress_test_linear_resolve_colors(&linear, &color, &out_color),
      CMP_ERR_RANGE);
  linear.style = linear_style;
  linear.style.indicator_color.r = -1.0f;
  CMP_TEST_EXPECT(
      m3_progress_test_linear_resolve_colors(&linear, &color, &out_color),
      CMP_ERR_RANGE);
  linear.style = linear_style;

  CMP_TEST_OK(m3_progress_test_set_fail_point(M3_PROGRESS_TEST_FAIL_COLOR_SET));
  CMP_TEST_EXPECT(m3_circular_progress_style_init(&circular_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(
      m3_progress_test_set_fail_point(M3_PROGRESS_TEST_FAIL_COLOR_WITH_ALPHA));
  CMP_TEST_EXPECT(m3_circular_progress_style_init(&circular_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(m3_progress_test_set_color_set_fail_after(2u));
  CMP_TEST_EXPECT(m3_circular_progress_style_init(&circular_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(m3_progress_test_set_color_alpha_fail_after(2u));
  CMP_TEST_EXPECT(m3_circular_progress_style_init(&circular_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());

  CMP_TEST_OK(m3_circular_progress_style_init(&circular_style));
  CMP_TEST_EXPECT(m3_progress_test_circular_validate_style(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  circular_style.diameter = 0.0f;
  CMP_TEST_EXPECT(m3_progress_test_circular_validate_style(&circular_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_circular_progress_style_init(&circular_style));
  circular_style.thickness = 0.0f;
  CMP_TEST_EXPECT(m3_progress_test_circular_validate_style(&circular_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_circular_progress_style_init(&circular_style));
  circular_style.thickness = circular_style.diameter;
  CMP_TEST_EXPECT(m3_progress_test_circular_validate_style(&circular_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_circular_progress_style_init(&circular_style));
  circular_style.segments = 2u;
  CMP_TEST_EXPECT(m3_progress_test_circular_validate_style(&circular_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_circular_progress_style_init(&circular_style));
  circular_style.track_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_progress_test_circular_validate_style(&circular_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_circular_progress_style_init(&circular_style));
  circular_style.indicator_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_progress_test_circular_validate_style(&circular_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_circular_progress_style_init(&circular_style));
  circular_style.disabled_track_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_progress_test_circular_validate_style(&circular_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_circular_progress_style_init(&circular_style));
  circular_style.disabled_indicator_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_progress_test_circular_validate_style(&circular_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_circular_progress_style_init(&circular_style));
  CMP_TEST_OK(m3_progress_test_circular_validate_style(&circular_style));

  CMP_TEST_OK(m3_circular_progress_init(&circular, &circular_style, 0.25f));
  CMP_TEST_EXPECT(
      m3_progress_test_circular_resolve_colors(NULL, &color, &out_color),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_progress_test_circular_resolve_colors(&circular, NULL, &out_color),
      CMP_ERR_INVALID_ARGUMENT);
  circular.widget.flags = CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(
      m3_progress_test_circular_resolve_colors(&circular, &color, &out_color));
  circular.widget.flags = 0u;
  circular.style.track_color.r = -1.0f;
  CMP_TEST_EXPECT(
      m3_progress_test_circular_resolve_colors(&circular, &color, &out_color),
      CMP_ERR_RANGE);
  circular.style = circular_style;
  circular.style.indicator_color.r = -1.0f;
  CMP_TEST_EXPECT(
      m3_progress_test_circular_resolve_colors(&circular, &color, &out_color),
      CMP_ERR_RANGE);
  circular.style = circular_style;

  CMP_TEST_OK(test_backend_init(&backend));
  gfx.ctx = &backend;
  gfx.vtable = NULL;
  gfx.text_vtable = NULL;
  CMP_TEST_EXPECT(m3_progress_test_circular_draw_arc(
                      NULL, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, color, 1.0f, 8u),
                  CMP_ERR_UNSUPPORTED);
  CMP_TEST_EXPECT(m3_progress_test_circular_draw_arc(
                      &gfx, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, color, 1.0f, 8u),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable_no_line;
  CMP_TEST_EXPECT(m3_progress_test_circular_draw_arc(
                      &gfx, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, color, 1.0f, 8u),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable;
  CMP_TEST_EXPECT(m3_progress_test_circular_draw_arc(
                      &gfx, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, color, 0.0f, 8u),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_progress_test_circular_draw_arc(
                      &gfx, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, color, 1.0f, 0u),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_circular_draw_arc(&gfx, 0.0f, 0.0f, 1.0f, 1.0f,
                                                 0.0f, color, 1.0f, 8u));
  backend.fail_draw_line = CMP_ERR_IO;
  CMP_TEST_EXPECT(m3_progress_test_circular_draw_arc(
                      &gfx, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, color, 1.0f, 8u),
                  CMP_ERR_IO);

  CMP_TEST_OK(m3_slider_style_init(&slider_style));
  CMP_TEST_EXPECT(m3_progress_test_slider_validate_style(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_progress_test_set_fail_point(M3_PROGRESS_TEST_FAIL_COLOR_SET));
  CMP_TEST_EXPECT(m3_slider_style_init(&slider_style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(
      m3_progress_test_set_fail_point(M3_PROGRESS_TEST_FAIL_COLOR_WITH_ALPHA));
  CMP_TEST_EXPECT(m3_slider_style_init(&slider_style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(m3_progress_test_set_color_set_fail_after(2u));
  CMP_TEST_EXPECT(m3_slider_style_init(&slider_style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(m3_progress_test_set_color_set_fail_after(3u));
  CMP_TEST_EXPECT(m3_slider_style_init(&slider_style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(m3_progress_test_set_color_alpha_fail_after(2u));
  CMP_TEST_EXPECT(m3_slider_style_init(&slider_style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(m3_progress_test_set_color_alpha_fail_after(3u));
  CMP_TEST_EXPECT(m3_slider_style_init(&slider_style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  slider_style.track_length = 0.0f;
  CMP_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_slider_style_init(&slider_style));
  slider_style.track_height = 0.0f;
  CMP_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_slider_style_init(&slider_style));
  slider_style.track_corner_radius = slider_style.track_height;
  CMP_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_slider_style_init(&slider_style));
  slider_style.thumb_radius = 0.0f;
  CMP_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_slider_style_init(&slider_style));
  slider_style.track_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_slider_style_init(&slider_style));
  slider_style.active_track_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_slider_style_init(&slider_style));
  slider_style.thumb_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_slider_style_init(&slider_style));
  slider_style.disabled_track_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_slider_style_init(&slider_style));
  slider_style.disabled_active_track_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_slider_style_init(&slider_style));
  slider_style.disabled_thumb_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_progress_test_slider_validate_style(&slider_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_slider_style_init(&slider_style));
  CMP_TEST_OK(m3_progress_test_slider_validate_style(&slider_style));

  memset(&slider, 0, sizeof(slider));
  slider.min_value = 1.0f;
  slider.max_value = 1.0f;
  slider.value = 1.0f;
  CMP_TEST_EXPECT(m3_progress_test_slider_value_to_fraction(NULL, &fraction),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_progress_test_slider_value_to_fraction(&slider, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_progress_test_slider_value_to_fraction(&slider, &fraction),
                  CMP_ERR_RANGE);
  slider.min_value = 0.0f;
  slider.max_value = 1.0f;
  slider.value = 2.0f;
  CMP_TEST_EXPECT(m3_progress_test_slider_value_to_fraction(&slider, &fraction),
                  CMP_ERR_RANGE);
  slider.value = 0.5f;
  CMP_TEST_OK(m3_progress_test_slider_value_to_fraction(&slider, &fraction));
  CMP_TEST_ASSERT(cmp_near(fraction, 0.5f, 0.001f));
  CMP_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_FRACTION_NEGATIVE));
  CMP_TEST_OK(m3_progress_test_slider_value_to_fraction(&slider, &fraction));
  CMP_TEST_ASSERT(cmp_near(fraction, 0.0f, 0.001f));
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_FRACTION_POSITIVE));
  CMP_TEST_OK(m3_progress_test_slider_value_to_fraction(&slider, &fraction));
  CMP_TEST_ASSERT(cmp_near(fraction, 1.0f, 0.001f));
  CMP_TEST_OK(m3_progress_test_clear_fail_points());

  slider.bounds.x = 0.0f;
  slider.bounds.y = 0.0f;
  slider.bounds.width = 0.0f;
  slider.bounds.height = 10.0f;
  CMP_TEST_EXPECT(m3_progress_test_slider_value_from_x(NULL, 1.0f, &value),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_progress_test_slider_value_from_x(&slider, 1.0f, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_progress_test_slider_value_from_x(&slider, 1.0f, &value),
                  CMP_ERR_RANGE);
  slider.bounds.width = 10.0f;
  slider.max_value = 0.0f;
  CMP_TEST_EXPECT(m3_progress_test_slider_value_from_x(&slider, 1.0f, &value),
                  CMP_ERR_RANGE);
  slider.max_value = 1.0f;
  slider.step = -1.0f;
  CMP_TEST_EXPECT(m3_progress_test_slider_value_from_x(&slider, 1.0f, &value),
                  CMP_ERR_RANGE);
  slider.step = 0.0f;
  CMP_TEST_OK(m3_progress_test_slider_value_from_x(&slider, -5.0f, &value));
  CMP_TEST_ASSERT(cmp_near(value, 0.0f, 0.001f));
  CMP_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_FROM_X_FRACTION_HIGH));
  CMP_TEST_OK(m3_progress_test_slider_value_from_x(&slider, 5.0f, &value));
  CMP_TEST_OK(m3_progress_test_clear_fail_points());

  CMP_TEST_EXPECT(
      m3_progress_test_slider_snap_value(0.0f, 0.0f, 1.0f, 0.0f, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_progress_test_slider_snap_value(0.0f, 0.0f, 1.0f, -1.0f, &value),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      m3_progress_test_slider_snap_value(0.0f, 1.0f, 0.0f, 0.0f, &value),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      m3_progress_test_slider_snap_value(0.0f, 0.0f, 1.0f, 2.0f, &value),
      CMP_ERR_RANGE);
  CMP_TEST_OK(
      m3_progress_test_slider_snap_value(-1.0f, 0.0f, 1.0f, 0.0f, &value));
  CMP_TEST_ASSERT(cmp_near(value, 0.0f, 0.001f));
  CMP_TEST_OK(
      m3_progress_test_slider_snap_value(2.0f, 0.0f, 1.0f, 0.0f, &value));
  CMP_TEST_ASSERT(cmp_near(value, 1.0f, 0.001f));
  CMP_TEST_OK(
      m3_progress_test_slider_snap_value(0.6f, 0.0f, 1.0f, 0.25f, &value));
  CMP_TEST_ASSERT(cmp_near(value, 0.5f, 0.001f));
  CMP_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_COUNT_NEGATIVE));
  CMP_TEST_EXPECT(
      m3_progress_test_slider_snap_value(0.6f, 0.0f, 1.0f, 0.25f, &value),
      CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_CLAMP_MIN));
  CMP_TEST_OK(
      m3_progress_test_slider_snap_value(0.6f, 0.0f, 1.0f, 0.25f, &value));
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_CLAMP_MAX));
  CMP_TEST_OK(
      m3_progress_test_slider_snap_value(0.6f, 0.0f, 1.0f, 0.25f, &value));
  CMP_TEST_OK(m3_progress_test_clear_fail_points());

  CMP_TEST_OK(m3_slider_init(&slider, &slider_style, 0.0f, 1.0f, 0.5f));
  CMP_TEST_EXPECT(m3_progress_test_slider_resolve_colors(
                      NULL, &color, &out_color, &out_color),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_progress_test_slider_resolve_colors(
                      &slider, NULL, &out_color, &out_color),
                  CMP_ERR_INVALID_ARGUMENT);
  slider.widget.flags = CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(m3_progress_test_slider_resolve_colors(&slider, &color,
                                                     &out_color, &out_color));
  slider.widget.flags = 0u;
  slider.style.track_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_progress_test_slider_resolve_colors(
                      &slider, &color, &out_color, &out_color),
                  CMP_ERR_RANGE);
  slider.style = slider_style;
  slider.style.active_track_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_progress_test_slider_resolve_colors(
                      &slider, &color, &out_color, &out_color),
                  CMP_ERR_RANGE);
  slider.style = slider_style;
  slider.style.thumb_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_progress_test_slider_resolve_colors(
                      &slider, &color, &out_color, &out_color),
                  CMP_ERR_RANGE);
  slider.style = slider_style;

  CMP_TEST_EXPECT(m3_progress_test_slider_update_value(NULL, 0.5f, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);

  return CMP_OK;
}

int main(void) {
  TestProgressBackend backend;
  CMPGfx gfx;
  CMPPaintContext paint_ctx;
  M3LinearProgressStyle linear_style;
  M3LinearProgressStyle bad_linear_style;
  M3LinearProgress linear;
  M3CircularProgressStyle circular_style;
  M3CircularProgressStyle bad_circular_style;
  M3CircularProgress circular;
  M3SliderStyle slider_style;
  M3SliderStyle bad_slider_style;
  M3Slider slider;
  CMPMeasureSpec width_spec;
  CMPMeasureSpec height_spec;
  CMPSize size;
  CMPRect bounds;
  CMPSemantics semantics;
  CMPBool handled;
  CMPScalar value;
  SliderCounter counter;
  CMPInputEvent event;

  CMP_TEST_OK(test_progress_helpers());

  CMP_TEST_OK(test_backend_init(&backend));
  gfx.ctx = &backend;
  gfx.vtable = &g_test_vtable;
  gfx.text_vtable = NULL;

  paint_ctx.gfx = &gfx;
  paint_ctx.dpi_scale = 1.0f;
  paint_ctx.clip.x = 0.0f;
  paint_ctx.clip.y = 0.0f;
  paint_ctx.clip.width = 200.0f;
  paint_ctx.clip.height = 200.0f;

  CMP_TEST_EXPECT(m3_linear_progress_style_init(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_linear_progress_style_init(&linear_style));
  CMP_TEST_ASSERT(
      cmp_near(linear_style.height, M3_LINEAR_PROGRESS_DEFAULT_HEIGHT, 0.001f));

  CMP_TEST_EXPECT(m3_linear_progress_init(NULL, &linear_style, 0.5f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_linear_progress_init(&linear, NULL, 0.5f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_linear_progress_init(&linear, &linear_style, -0.1f),
                  CMP_ERR_RANGE);

  bad_linear_style = linear_style;
  bad_linear_style.height = 0.0f;
  CMP_TEST_EXPECT(m3_linear_progress_init(&linear, &bad_linear_style, 0.5f),
                  CMP_ERR_RANGE);

  CMP_TEST_OK(m3_linear_progress_init(&linear, &linear_style, 0.25f));
  CMP_TEST_ASSERT(linear.widget.ctx == &linear);
  CMP_TEST_ASSERT(linear.widget.vtable != NULL);

  CMP_TEST_EXPECT(m3_linear_progress_set_value(NULL, 0.5f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_linear_progress_set_value(&linear, 1.5f), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_linear_progress_set_value(&linear, 0.75f));
  CMP_TEST_OK(m3_linear_progress_get_value(&linear, &value));
  CMP_TEST_ASSERT(cmp_near(value, 0.75f, 0.001f));
  CMP_TEST_EXPECT(m3_linear_progress_get_value(NULL, &value),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_linear_progress_get_value(&linear, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(m3_linear_progress_set_label(NULL, "A", 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_linear_progress_set_label(&linear, NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_linear_progress_set_label(&linear, "Loading", 7));

  CMP_TEST_EXPECT(m3_linear_progress_set_style(NULL, &linear_style),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_linear_progress_set_style(&linear, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  bad_linear_style = linear_style;
  bad_linear_style.track_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_linear_progress_set_style(&linear, &bad_linear_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_linear_progress_set_style(&linear, &linear_style));

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(linear.widget.vtable->measure(linear.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      linear.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(linear.widget.vtable->measure(linear.widget.ctx, width_spec,
                                                height_spec, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  CMP_TEST_EXPECT(linear.widget.vtable->measure(linear.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_RANGE);

  bad_linear_style = linear_style;
  bad_linear_style.min_width = -1.0f;
  linear.style = bad_linear_style;
  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(linear.widget.vtable->measure(linear.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_RANGE);
  linear.style = linear_style;

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 10.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = -1.0f;
  CMP_TEST_EXPECT(linear.widget.vtable->measure(linear.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_RANGE);

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_OK(linear.widget.vtable->measure(linear.widget.ctx, width_spec,
                                            height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, linear_style.min_width, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, linear_style.height, 0.001f));

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 10.0f;
  height_spec.mode = CMP_MEASURE_EXACTLY;
  height_spec.size = 12.0f;
  CMP_TEST_OK(linear.widget.vtable->measure(linear.widget.ctx, width_spec,
                                            height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 10.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 12.0f, 0.001f));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  CMP_TEST_EXPECT(linear.widget.vtable->layout(linear.widget.ctx, bounds),
                  CMP_ERR_RANGE);

  bounds.width = 120.0f;
  bounds.height = 8.0f;
  CMP_TEST_OK(linear.widget.vtable->layout(linear.widget.ctx, bounds));

  CMP_TEST_OK(test_backend_init(&backend));
  CMP_TEST_EXPECT(linear.widget.vtable->paint(NULL, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  gfx.vtable = NULL;
  CMP_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_vtable_no_rect;
  CMP_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable;

  linear.value = 2.0f;
  CMP_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  linear.value = 0.5f;

  linear.bounds.width = -1.0f;
  CMP_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  linear.bounds = bounds;

  bad_linear_style = linear_style;
  bad_linear_style.height = 0.0f;
  linear.style = bad_linear_style;
  CMP_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  linear.style = linear_style;

  linear.style.indicator_color.r = -1.0f;
  CMP_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  linear.style = linear_style;
  CMP_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_LINEAR_RESOLVE_COLORS));
  CMP_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_LINEAR_CORNER_RANGE));
  CMP_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_LINEAR_FILL_CORNER_CLAMP));
  CMP_TEST_OK(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx));
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_LINEAR_FILL_CORNER_RANGE));
  CMP_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());

  CMP_TEST_OK(test_backend_init(&backend));
  backend.fail_draw_rect = CMP_ERR_IO;
  CMP_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);

  CMP_TEST_OK(test_backend_init(&backend));
  linear.style = linear_style;
  linear.style.track_color.a = 0.0f;
  CMP_TEST_OK(m3_linear_progress_set_style(&linear, &linear.style));
  linear.value = 0.5f;
  backend.fail_draw_rect = CMP_ERR_IO;
  CMP_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend.fail_draw_rect = CMP_OK;
  linear.style = linear_style;
  CMP_TEST_OK(m3_linear_progress_set_style(&linear, &linear.style));

  CMP_TEST_OK(test_backend_init(&backend));
  linear.value = 0.0f;
  CMP_TEST_OK(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx));
  CMP_TEST_ASSERT(backend.draw_rect_calls == 1);

  CMP_TEST_OK(test_backend_init(&backend));
  linear.value = 0.5f;
  CMP_TEST_OK(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx));
  CMP_TEST_ASSERT(backend.draw_rect_calls == 2);

  linear.style = linear_style;
  linear.style.track_color.a = 0.0f;
  CMP_TEST_OK(m3_linear_progress_set_style(&linear, &linear.style));
  CMP_TEST_OK(test_backend_init(&backend));
  linear.value = 0.5f;
  CMP_TEST_OK(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx));

  linear.style = linear_style;
  linear.style.indicator_color.a = 0.0f;
  CMP_TEST_OK(m3_linear_progress_set_style(&linear, &linear.style));
  CMP_TEST_OK(test_backend_init(&backend));
  linear.value = 0.5f;
  CMP_TEST_OK(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx));

  linear.style = linear_style;
  linear.style.height = 8.0f;
  linear.style.corner_radius = 4.0f;
  CMP_TEST_OK(m3_linear_progress_set_style(&linear, &linear.style));
  bounds.width = 2.0f;
  bounds.height = 4.0f;
  CMP_TEST_OK(linear.widget.vtable->layout(linear.widget.ctx, bounds));
  CMP_TEST_OK(test_backend_init(&backend));
  linear.value = 1.0f;
  CMP_TEST_OK(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx));

  CMP_TEST_EXPECT(linear.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(linear.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      linear.widget.vtable->event(linear.widget.ctx, NULL, &handled),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(linear.widget.vtable->event(linear.widget.ctx, &event, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(linear.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(linear.widget.vtable->get_semantics(linear.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(linear.widget.vtable->destroy(NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  linear.widget.flags = CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(
      linear.widget.vtable->get_semantics(linear.widget.ctx, &semantics));
  CMP_TEST_ASSERT(semantics.role == CMP_SEMANTIC_NONE);
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_DISABLED) != 0u);
  linear.widget.flags = 0u;

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_OK(linear.widget.vtable->event(linear.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(linear.widget.vtable->destroy(linear.widget.ctx));
  CMP_TEST_ASSERT(linear.widget.vtable == NULL);

  CMP_TEST_EXPECT(m3_circular_progress_style_init(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_circular_progress_style_init(&circular_style));
  CMP_TEST_ASSERT(circular_style.segments ==
                  M3_CIRCULAR_PROGRESS_DEFAULT_SEGMENTS);

  CMP_TEST_EXPECT(m3_circular_progress_init(NULL, &circular_style, 0.5f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_circular_progress_init(&circular, NULL, 0.5f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_circular_progress_init(&circular, &circular_style, 1.5f),
                  CMP_ERR_RANGE);

  bad_circular_style = circular_style;
  bad_circular_style.segments = 2u;
  CMP_TEST_EXPECT(
      m3_circular_progress_init(&circular, &bad_circular_style, 0.5f),
      CMP_ERR_RANGE);

  CMP_TEST_OK(m3_circular_progress_init(&circular, &circular_style, 0.25f));
  CMP_TEST_ASSERT(circular.widget.ctx == &circular);
  CMP_TEST_ASSERT(circular.widget.vtable != NULL);

  CMP_TEST_EXPECT(m3_circular_progress_set_value(NULL, 0.5f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_circular_progress_set_value(&circular, -0.5f),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_circular_progress_set_value(&circular, 0.75f));
  CMP_TEST_OK(m3_circular_progress_get_value(&circular, &value));
  CMP_TEST_ASSERT(cmp_near(value, 0.75f, 0.001f));

  CMP_TEST_EXPECT(m3_circular_progress_get_value(NULL, &value),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_circular_progress_get_value(&circular, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(m3_circular_progress_set_label(NULL, "B", 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_circular_progress_set_label(&circular, NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_circular_progress_set_label(&circular, "Sync", 4));

  bad_circular_style = circular_style;
  bad_circular_style.track_color.a = 2.0f;
  CMP_TEST_EXPECT(
      m3_circular_progress_set_style(&circular, &bad_circular_style),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_circular_progress_set_style(NULL, &circular_style),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_circular_progress_set_style(&circular, &circular_style));

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(circular.widget.vtable->measure(
                      circular.widget.ctx, width_spec, height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      circular.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(circular.widget.vtable->measure(
                      circular.widget.ctx, width_spec, height_spec, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  CMP_TEST_EXPECT(circular.widget.vtable->measure(
                      circular.widget.ctx, width_spec, height_spec, &size),
                  CMP_ERR_RANGE);

  bad_circular_style = circular_style;
  bad_circular_style.diameter = -1.0f;
  circular.style = bad_circular_style;
  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(circular.widget.vtable->measure(
                      circular.widget.ctx, width_spec, height_spec, &size),
                  CMP_ERR_RANGE);
  circular.style = circular_style;

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 40.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = -1.0f;
  CMP_TEST_EXPECT(circular.widget.vtable->measure(
                      circular.widget.ctx, width_spec, height_spec, &size),
                  CMP_ERR_RANGE);

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_OK(circular.widget.vtable->measure(circular.widget.ctx, width_spec,
                                              height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, circular_style.diameter, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, circular_style.diameter, 0.001f));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  CMP_TEST_EXPECT(circular.widget.vtable->layout(circular.widget.ctx, bounds),
                  CMP_ERR_RANGE);

  bounds.width = 60.0f;
  bounds.height = 60.0f;
  CMP_TEST_OK(circular.widget.vtable->layout(circular.widget.ctx, bounds));

  CMP_TEST_OK(test_backend_init(&backend));
  CMP_TEST_EXPECT(circular.widget.vtable->paint(NULL, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(circular.widget.vtable->paint(circular.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  gfx.vtable = &g_test_vtable_no_line;
  CMP_TEST_EXPECT(
      circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
      CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable;

  circular.value = 2.0f;
  CMP_TEST_EXPECT(
      circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
      CMP_ERR_RANGE);
  circular.value = 0.25f;

  circular.bounds.width = -1.0f;
  CMP_TEST_EXPECT(
      circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
      CMP_ERR_RANGE);
  circular.bounds = bounds;

  bad_circular_style = circular_style;
  bad_circular_style.thickness = bad_circular_style.diameter;
  circular.style = bad_circular_style;
  CMP_TEST_EXPECT(
      circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
      CMP_ERR_RANGE);
  circular.style = circular_style;
  CMP_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_CIRCULAR_RESOLVE_COLORS));
  CMP_TEST_EXPECT(
      circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
      CMP_ERR_IO);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());

  CMP_TEST_OK(test_backend_init(&backend));
  backend.fail_draw_line = CMP_ERR_IO;
  CMP_TEST_EXPECT(
      circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
      CMP_ERR_IO);

  CMP_TEST_OK(test_backend_init(&backend));
  circular.style = circular_style;
  circular.style.track_color.a = 0.0f;
  CMP_TEST_OK(m3_circular_progress_set_style(&circular, &circular.style));
  backend.fail_draw_line = CMP_ERR_IO;
  CMP_TEST_EXPECT(
      circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
      CMP_ERR_IO);
  backend.fail_draw_line = CMP_OK;

  gfx.vtable = NULL;
  CMP_TEST_EXPECT(
      circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
      CMP_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_vtable;

  bounds.width = 60.0f;
  bounds.height = 40.0f;
  CMP_TEST_OK(circular.widget.vtable->layout(circular.widget.ctx, bounds));
  circular.style = circular_style;
  circular.style.track_color.a = 0.0f;
  CMP_TEST_OK(m3_circular_progress_set_style(&circular, &circular.style));
  CMP_TEST_OK(test_backend_init(&backend));
  circular.value = 0.5f;
  CMP_TEST_OK(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx));

  bounds.width = 0.0f;
  bounds.height = 10.0f;
  CMP_TEST_OK(circular.widget.vtable->layout(circular.widget.ctx, bounds));
  CMP_TEST_EXPECT(
      circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
      CMP_ERR_RANGE);
  bounds.width = 4.0f;
  bounds.height = 4.0f;
  CMP_TEST_OK(circular.widget.vtable->layout(circular.widget.ctx, bounds));
  CMP_TEST_EXPECT(
      circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx),
      CMP_ERR_RANGE);

  bounds.width = 60.0f;
  bounds.height = 40.0f;
  CMP_TEST_OK(circular.widget.vtable->layout(circular.widget.ctx, bounds));

  CMP_TEST_EXPECT(circular.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      circular.widget.vtable->event(circular.widget.ctx, NULL, &handled),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      circular.widget.vtable->event(circular.widget.ctx, &event, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(circular.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      circular.widget.vtable->get_semantics(circular.widget.ctx, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(circular.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(circular.widget.vtable->destroy(NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  circular.style = circular_style;
  CMP_TEST_OK(m3_circular_progress_set_style(&circular, &circular.style));
  CMP_TEST_OK(test_backend_init(&backend));
  circular.value = 0.0f;
  CMP_TEST_OK(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx));
  CMP_TEST_ASSERT(backend.draw_line_calls == (int)circular.style.segments);

  CMP_TEST_OK(test_backend_init(&backend));
  circular.value = 0.5f;
  CMP_TEST_OK(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx));
  CMP_TEST_ASSERT(backend.draw_line_calls > (int)circular.style.segments);

  circular.widget.flags = CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(
      circular.widget.vtable->get_semantics(circular.widget.ctx, &semantics));
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_DISABLED) != 0u);
  circular.widget.flags = 0u;

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_OK(
      circular.widget.vtable->event(circular.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(circular.widget.vtable->destroy(circular.widget.ctx));
  CMP_TEST_ASSERT(circular.widget.vtable == NULL);

  CMP_TEST_EXPECT(m3_slider_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_slider_style_init(&slider_style));
  CMP_TEST_ASSERT(
      cmp_near(slider_style.track_length, M3_SLIDER_DEFAULT_LENGTH, 0.001f));

  CMP_TEST_EXPECT(m3_slider_init(NULL, &slider_style, 0.0f, 1.0f, 0.5f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_slider_init(&slider, NULL, 0.0f, 1.0f, 0.5f),
                  CMP_ERR_INVALID_ARGUMENT);

  bad_slider_style = slider_style;
  bad_slider_style.thumb_radius = 0.0f;
  CMP_TEST_EXPECT(m3_slider_init(&slider, &bad_slider_style, 0.0f, 1.0f, 0.5f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_slider_init(&slider, &slider_style, 1.0f, 0.0f, 0.5f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_slider_init(&slider, &slider_style, 0.0f, 1.0f, 2.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_COUNT_NEGATIVE));
  CMP_TEST_EXPECT(m3_slider_init(&slider, &slider_style, 0.0f, 1.0f, 0.5f),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());

  CMP_TEST_OK(m3_slider_init(&slider, &slider_style, 0.0f, 1.0f, 0.25f));
  CMP_TEST_ASSERT(slider.widget.ctx == &slider);
  CMP_TEST_ASSERT(slider.widget.vtable != NULL);
  CMP_TEST_ASSERT((slider.widget.flags & CMP_WIDGET_FLAG_FOCUSABLE) != 0u);

  CMP_TEST_EXPECT(m3_slider_set_value(NULL, 0.5f), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_slider_set_value(&slider, 2.0f), CMP_ERR_RANGE);
  slider.min_value = 1.0f;
  slider.max_value = 1.0f;
  CMP_TEST_EXPECT(m3_slider_set_value(&slider, 1.0f), CMP_ERR_RANGE);
  slider.min_value = 0.0f;
  slider.max_value = 1.0f;
  CMP_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_COUNT_NEGATIVE));
  CMP_TEST_EXPECT(m3_slider_set_value(&slider, 0.5f), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_EXPECT(m3_slider_get_value(NULL, &value), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_slider_get_value(&slider, NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_slider_set_step(&slider, 0.25f));
  CMP_TEST_OK(m3_slider_set_value(&slider, 0.4f));
  CMP_TEST_OK(m3_slider_get_value(&slider, &value));
  CMP_TEST_ASSERT(cmp_near(value, 0.5f, 0.001f));

  CMP_TEST_EXPECT(m3_slider_set_step(NULL, 0.1f), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_slider_set_step(&slider, -1.0f), CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_slider_set_step(&slider, 2.0f), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_COUNT_NEGATIVE));
  CMP_TEST_EXPECT(m3_slider_set_step(&slider, 0.25f), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());

  CMP_TEST_EXPECT(m3_slider_set_range(NULL, 0.0f, 1.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_slider_set_range(&slider, 1.0f, 0.0f), CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_slider_set_range(&slider, 0.0f, 0.1f), CMP_ERR_RANGE);
  slider.step = -1.0f;
  CMP_TEST_EXPECT(m3_slider_set_range(&slider, 0.0f, 1.0f), CMP_ERR_RANGE);
  slider.step = 0.0f;
  CMP_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_SNAP_COUNT_NEGATIVE));
  CMP_TEST_EXPECT(m3_slider_set_range(&slider, 0.0f, 1.0f), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(m3_slider_set_step(&slider, 0.0f));
  CMP_TEST_OK(m3_slider_set_range(&slider, 0.0f, 1.0f));

  CMP_TEST_EXPECT(m3_slider_set_style(NULL, &slider_style),
                  CMP_ERR_INVALID_ARGUMENT);
  bad_slider_style = slider_style;
  bad_slider_style.track_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_slider_set_style(&slider, &bad_slider_style),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_slider_set_style(&slider, &slider_style));

  CMP_TEST_EXPECT(m3_slider_set_label(NULL, "S", 1), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_slider_set_label(&slider, NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_slider_set_label(&slider, "Volume", 6));

  CMP_TEST_EXPECT(m3_slider_set_on_change(NULL, test_slider_on_change, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  counter.calls = 0;
  counter.last_value = 0.0f;
  counter.fail = 0;
  CMP_TEST_OK(
      m3_slider_set_on_change(&slider, test_slider_on_change, &counter));

  CMP_TEST_EXPECT(slider.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      slider.widget.vtable->event(slider.widget.ctx, NULL, &handled),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(slider.widget.vtable->event(slider.widget.ctx, &event, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(slider.widget.vtable->measure(slider.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      slider.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(slider.widget.vtable->measure(slider.widget.ctx, width_spec,
                                                height_spec, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  CMP_TEST_EXPECT(slider.widget.vtable->measure(slider.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_RANGE);

  bad_slider_style = slider_style;
  bad_slider_style.track_height = 0.0f;
  slider.style = bad_slider_style;
  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(slider.widget.vtable->measure(slider.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_RANGE);
  slider.style = slider_style;

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 80.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = -1.0f;
  CMP_TEST_EXPECT(slider.widget.vtable->measure(slider.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_RANGE);

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_OK(slider.widget.vtable->measure(slider.widget.ctx, width_spec,
                                            height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, slider_style.track_length, 0.001f));
  CMP_TEST_ASSERT(
      cmp_near(size.height, slider_style.thumb_radius * 2.0f, 0.001f));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 20.0f;
  CMP_TEST_EXPECT(slider.widget.vtable->layout(slider.widget.ctx, bounds),
                  CMP_ERR_RANGE);

  bounds.width = 100.0f;
  bounds.height = 20.0f;
  CMP_TEST_OK(slider.widget.vtable->layout(slider.widget.ctx, bounds));

  CMP_TEST_OK(test_backend_init(&backend));
  CMP_TEST_EXPECT(slider.widget.vtable->paint(NULL, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  gfx.vtable = NULL;
  CMP_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_vtable_no_rect;
  CMP_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable;

  slider.bounds.width = 0.0f;
  CMP_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  slider.bounds = bounds;

  bad_slider_style = slider_style;
  bad_slider_style.track_length = 0.0f;
  slider.style = bad_slider_style;
  CMP_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  slider.style = slider_style;

  slider.bounds.width = -1.0f;
  CMP_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  slider.bounds = bounds;

  slider.step = -1.0f;
  CMP_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  slider.step = 0.0f;

  slider.max_value = slider.min_value;
  CMP_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  slider.max_value = 1.0f;

  slider.value = 2.0f;
  CMP_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  slider.value = 0.5f;

  CMP_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_RESOLVE_COLORS));
  CMP_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());

  CMP_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_CORNER_CLAMP));
  CMP_TEST_OK(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx));
  CMP_TEST_OK(m3_progress_test_clear_fail_points());
  CMP_TEST_OK(m3_progress_test_set_fail_point(
      M3_PROGRESS_TEST_FAIL_SLIDER_CORNER_RANGE));
  CMP_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_progress_test_clear_fail_points());

  CMP_TEST_OK(test_backend_init(&backend));
  backend.fail_draw_rect_after = 2;
  slider.value = 0.5f;
  CMP_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend.fail_draw_rect_after = 0;

  CMP_TEST_OK(test_backend_init(&backend));
  backend.fail_draw_rect_after = 3;
  slider.value = 0.5f;
  CMP_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend.fail_draw_rect_after = 0;

  CMP_TEST_OK(test_backend_init(&backend));
  backend.fail_draw_rect = CMP_ERR_IO;
  CMP_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);

  CMP_TEST_OK(test_backend_init(&backend));
  slider.value = 0.5f;
  CMP_TEST_OK(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx));
  CMP_TEST_ASSERT(backend.draw_rect_calls == 3);

  slider.step = 2.0f;
  CMP_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  slider.step = 0.0f;

  slider.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(
      slider.widget.vtable->get_semantics(slider.widget.ctx, &semantics));
  CMP_TEST_ASSERT(semantics.role == CMP_SEMANTIC_SLIDER);
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_DISABLED) != 0u);
  slider.widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;

  CMP_TEST_EXPECT(slider.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(slider.widget.vtable->get_semantics(slider.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(slider.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(slider.widget.vtable->destroy(NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  counter.calls = 0;
  counter.fail = 0;
  CMP_TEST_OK(
      m3_slider_set_on_change(&slider, test_slider_on_change, &counter));

  slider.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 50, 10));
  CMP_TEST_OK(slider.widget.vtable->event(slider.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  slider.widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 50, 10));
  CMP_TEST_OK(slider.widget.vtable->event(slider.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 50, 10));
  CMP_TEST_OK(slider.widget.vtable->event(slider.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(counter.calls == 1);
  CMP_TEST_ASSERT(cmp_near(counter.last_value, 0.5f, 0.01f));

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 80, 10));
  CMP_TEST_OK(slider.widget.vtable->event(slider.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(counter.calls == 2);
  CMP_TEST_ASSERT(cmp_near(counter.last_value, 0.8f, 0.02f));

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 80, 10));
  CMP_TEST_OK(slider.widget.vtable->event(slider.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 80, 10));
  CMP_TEST_OK(slider.widget.vtable->event(slider.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  slider.pressed = CMP_TRUE;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_EXPECT(
      slider.widget.vtable->event(slider.widget.ctx, &event, &handled),
      CMP_ERR_STATE);
  slider.pressed = CMP_FALSE;

  slider.step = -1.0f;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_EXPECT(
      slider.widget.vtable->event(slider.widget.ctx, &event, &handled),
      CMP_ERR_RANGE);
  slider.step = 0.0f;

  slider.pressed = CMP_TRUE;
  slider.bounds.width = 0.0f;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 10, 10));
  CMP_TEST_EXPECT(
      slider.widget.vtable->event(slider.widget.ctx, &event, &handled),
      CMP_ERR_RANGE);
  slider.bounds.width = bounds.width;
  slider.pressed = CMP_FALSE;

  counter.fail = 1;
  slider.value = 0.2f;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 90, 10));
  CMP_TEST_EXPECT(
      slider.widget.vtable->event(slider.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_ASSERT(cmp_near(slider.value, 0.2f, 0.001f));
  CMP_TEST_ASSERT(slider.pressed == CMP_FALSE);

  counter.fail = 1;
  slider.pressed = CMP_TRUE;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 90, 10));
  CMP_TEST_EXPECT(
      slider.widget.vtable->event(slider.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  slider.pressed = CMP_FALSE;
  counter.fail = 0;

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_KEY_DOWN, 0, 0));
  CMP_TEST_OK(slider.widget.vtable->event(slider.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(slider.widget.vtable->destroy(slider.widget.ctx));
  CMP_TEST_ASSERT(slider.widget.vtable == NULL);

  return 0;
}
