#include "m3/m3_time_picker.h"
#include "test_utils.h"

#include <string.h>

#define M3_TEST_PI 3.14159265358979323846f

typedef struct TestTimePickerBackend {
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
} TestTimePickerBackend;

typedef struct ChangeCounter {
  int calls;
  int fail;
  M3Time last;
} ChangeCounter;

static int test_backend_init(TestTimePickerBackend *backend) {
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
  TestTimePickerBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestTimePickerBackend *)gfx;
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
  TestTimePickerBackend *backend;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestTimePickerBackend *)gfx;
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
                                          NULL,
                                          NULL};

static const M3GfxVTable g_test_vtable_no_rect = {
    NULL, NULL, NULL, NULL, test_gfx_draw_line, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL};

static const M3GfxVTable g_test_vtable_no_line = {
    NULL, NULL, NULL, test_gfx_draw_rect, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL};

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

static int test_on_change(void *ctx, M3TimePicker *picker, const M3Time *time) {
  ChangeCounter *counter;

  if (ctx == NULL || picker == NULL || time == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  counter = (ChangeCounter *)ctx;
  counter->calls += 1;
  counter->last = *time;
  if (counter->fail) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int m3_near(M3Scalar a, M3Scalar b, M3Scalar tol) {
  M3Scalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

static int test_helpers(void) {
  M3TimePickerStyle style;
  M3LayoutEdges edges;
  M3Color color;
  M3Rect rect;
  M3MeasureSpec spec;
  M3Time time;
  M3TimePicker picker;
  M3TimePickerMetrics metrics;
  M3Scalar angle;
  m3_u32 index;
  m3_u32 hour;
  M3Bool inner;
  M3Color out_background;
  M3Color out_ring;
  M3Color out_hand;
  M3Color out_selection;
  volatile const M3Color *null_color;
  volatile const M3LayoutEdges *null_edges;
  volatile const M3TimePickerStyle *null_style;
  volatile const M3Rect *null_rect;
  volatile const M3Time *null_time;

  null_color = NULL;
  null_edges = NULL;
  null_style = NULL;
  null_rect = NULL;
  null_time = NULL;

  M3_TEST_EXPECT(
      m3_time_picker_test_validate_color((const M3Color *)null_color),
      M3_ERR_INVALID_ARGUMENT);
  color.r = 1.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_color(&color), M3_ERR_RANGE);
  color.r = 0.0f;
  color.g = -0.1f;
  color.b = 0.0f;
  color.a = 0.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_color(&color), M3_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.1f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_color(&color), M3_ERR_RANGE);
  color.b = 0.0f;
  color.a = -0.1f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_color(&color), M3_ERR_RANGE);

  M3_TEST_EXPECT(
      m3_time_picker_test_validate_edges((const M3LayoutEdges *)null_edges),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_time_picker_test_validate_style((const M3TimePickerStyle *)null_style),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_test_validate_rect((const M3Rect *)null_rect),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_test_validate_time((const M3Time *)null_time),
                 M3_ERR_INVALID_ARGUMENT);

  edges.left = -1.0f;
  edges.right = 0.0f;
  edges.top = 0.0f;
  edges.bottom = 0.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_edges(&edges), M3_ERR_RANGE);

  M3_TEST_OK(m3_time_picker_style_init(&style));
  M3_TEST_OK(m3_time_picker_test_validate_style(&style));
  style.diameter = 0.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_time_picker_style_init(&style));
  style.ring_thickness = 0.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_time_picker_style_init(&style));
  style.ring_thickness = style.diameter;
  M3_TEST_EXPECT(m3_time_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_time_picker_style_init(&style));
  style.inner_ring_ratio = 0.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_time_picker_style_init(&style));
  style.inner_ring_ratio = 1.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_time_picker_style_init(&style));
  style.hand_thickness = 0.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_time_picker_style_init(&style));
  style.hand_center_radius = -1.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_time_picker_style_init(&style));
  style.hand_center_radius = style.diameter;
  M3_TEST_EXPECT(m3_time_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_time_picker_style_init(&style));
  style.padding.left = -1.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_time_picker_style_init(&style));
  style.background_color.r = -1.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_time_picker_style_init(&style));
  style.ring_color.r = -1.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_time_picker_style_init(&style));
  style.hand_color.r = -1.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_time_picker_style_init(&style));
  style.selection_color.r = -1.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_time_picker_style_init(&style));
  style.disabled_color.r = -1.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_style(&style), M3_ERR_RANGE);

  spec.mode = 99u;
  spec.size = 10.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_measure_spec(spec),
                 M3_ERR_INVALID_ARGUMENT);
  spec.mode = M3_MEASURE_AT_MOST;
  spec.size = -1.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_measure_spec(spec), M3_ERR_RANGE);
  spec.mode = M3_MEASURE_EXACTLY;
  spec.size = 10.0f;
  M3_TEST_OK(m3_time_picker_test_validate_measure_spec(spec));

  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 10.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_rect(&rect), M3_ERR_RANGE);
  rect.width = 10.0f;
  rect.height = -1.0f;
  M3_TEST_EXPECT(m3_time_picker_test_validate_rect(&rect), M3_ERR_RANGE);

  time.hour = 24u;
  time.minute = 0u;
  M3_TEST_EXPECT(m3_time_picker_test_validate_time(&time), M3_ERR_RANGE);
  time.hour = 23u;
  time.minute = 60u;
  M3_TEST_EXPECT(m3_time_picker_test_validate_time(&time), M3_ERR_RANGE);
  time.hour = 23u;
  time.minute = 59u;
  M3_TEST_OK(m3_time_picker_test_validate_time(&time));

  M3_TEST_OK(
      m3_time_picker_test_angle_from_point(0.0f, 0.0f, 0.0f, -1.0f, &angle));
  M3_TEST_ASSERT(m3_near(angle, 0.0f, 1e-3f));
  M3_TEST_EXPECT(
      m3_time_picker_test_angle_from_point(0.0f, 0.0f, 0.0f, 0.0f, NULL),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(
      m3_time_picker_test_angle_from_point(0.0f, 0.0f, 1.0f, 0.0f, &angle));
  M3_TEST_ASSERT(m3_near(angle, M3_TEST_PI * 0.5f, 1e-3f));
  M3_TEST_OK(
      m3_time_picker_test_angle_from_point(-1.0f, -1.0f, -2.0f, -2.0f, &angle));
  M3_TEST_ASSERT(m3_near(angle, (M3Scalar)(M3_TEST_PI * 1.75f), 1e-3f));

  M3_TEST_EXPECT(m3_time_picker_test_angle_to_index(0.0f, 0u, &index),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_time_picker_test_angle_to_index(0.0f, 12u, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_time_picker_test_angle_to_index(0.0f, 12u, &index));
  M3_TEST_ASSERT(index == 0u);
  angle = (M3Scalar)(M3_TEST_PI * 2.0f / 12.0f) * 0.6f;
  M3_TEST_OK(m3_time_picker_test_angle_to_index(angle, 12u, &index));
  M3_TEST_ASSERT(index == 1u);
  M3_TEST_OK(m3_time_picker_test_angle_to_index(-0.1f, 12u, &index));
  M3_TEST_ASSERT(index == 0u);
  M3_TEST_OK(m3_time_picker_test_angle_to_index(7.0f, 12u, &index));
  M3_TEST_ASSERT(index == 1u);

  M3_TEST_EXPECT(m3_time_picker_test_hour_from_index(
                     12u, M3_TIME_PICKER_FORMAT_24H, M3_FALSE, 0u, &hour),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_time_picker_test_hour_from_index(
                     0u, M3_TIME_PICKER_FORMAT_24H, M3_FALSE, 0u, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_time_picker_test_hour_from_index(0u, 99u, M3_FALSE, 0u, &hour),
      M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_time_picker_test_hour_from_index(
                     0u, M3_TIME_PICKER_FORMAT_12H, M3_FALSE, 99u, &hour),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_time_picker_test_hour_from_index(0u, M3_TIME_PICKER_FORMAT_24H,
                                                 M3_TRUE, 0u, &hour));
  M3_TEST_ASSERT(hour == 0u);
  M3_TEST_OK(m3_time_picker_test_hour_from_index(0u, M3_TIME_PICKER_FORMAT_24H,
                                                 M3_FALSE, 0u, &hour));
  M3_TEST_ASSERT(hour == 12u);
  M3_TEST_OK(m3_time_picker_test_hour_from_index(1u, M3_TIME_PICKER_FORMAT_24H,
                                                 M3_TRUE, 0u, &hour));
  M3_TEST_ASSERT(hour == 13u);
  M3_TEST_OK(m3_time_picker_test_hour_from_index(1u, M3_TIME_PICKER_FORMAT_12H,
                                                 M3_FALSE, 15u, &hour));
  M3_TEST_ASSERT(hour == 13u);
  M3_TEST_OK(m3_time_picker_test_hour_from_index(0u, M3_TIME_PICKER_FORMAT_12H,
                                                 M3_FALSE, 3u, &hour));
  M3_TEST_ASSERT(hour == 0u);

  M3_TEST_EXPECT(m3_time_picker_test_hour_to_index(
                     24u, M3_TIME_PICKER_FORMAT_24H, &index, &inner),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_time_picker_test_hour_to_index(0u, 99u, &index, &inner),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_time_picker_test_hour_to_index(
                     0u, M3_TIME_PICKER_FORMAT_24H, NULL, &inner),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_test_hour_to_index(
                     0u, M3_TIME_PICKER_FORMAT_24H, &index, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_time_picker_test_hour_to_index(0u, M3_TIME_PICKER_FORMAT_24H,
                                               &index, &inner));
  M3_TEST_ASSERT(index == 0u);
  M3_TEST_ASSERT(inner == M3_TRUE);
  M3_TEST_OK(m3_time_picker_test_hour_to_index(12u, M3_TIME_PICKER_FORMAT_24H,
                                               &index, &inner));
  M3_TEST_ASSERT(index == 0u);
  M3_TEST_ASSERT(inner == M3_FALSE);
  M3_TEST_OK(m3_time_picker_test_hour_to_index(23u, M3_TIME_PICKER_FORMAT_24H,
                                               &index, &inner));
  M3_TEST_ASSERT(index == 11u);
  M3_TEST_ASSERT(inner == M3_TRUE);
  M3_TEST_OK(m3_time_picker_test_hour_to_index(5u, M3_TIME_PICKER_FORMAT_12H,
                                               &index, &inner));
  M3_TEST_ASSERT(index == 5u);
  M3_TEST_ASSERT(inner == M3_FALSE);

  M3_TEST_OK(m3_time_picker_style_init(&style));
  M3_TEST_OK(
      m3_time_picker_init(&picker, &style, 0u, 0u, M3_TIME_PICKER_FORMAT_24H));
  M3_TEST_EXPECT(m3_time_picker_test_update_metrics(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_test_compute_metrics(NULL, &metrics),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_test_compute_metrics(&picker, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  picker.bounds.width = -1.0f;
  M3_TEST_EXPECT(m3_time_picker_test_compute_metrics(&picker, &metrics),
                 M3_ERR_RANGE);
  picker.bounds.width = 0.0f;
  picker.bounds.height = 0.0f;
  picker.style.diameter = 0.0f;
  M3_TEST_EXPECT(m3_time_picker_test_compute_metrics(&picker, &metrics),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_time_picker_style_init(&style));
  M3_TEST_OK(
      m3_time_picker_init(&picker, &style, 0u, 0u, M3_TIME_PICKER_FORMAT_24H));
  M3_TEST_EXPECT(m3_time_picker_test_resolve_colors(NULL, &out_background,
                                                    &out_ring, &out_hand,
                                                    &out_selection),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_test_resolve_colors(&picker, NULL, &out_ring,
                                                    &out_hand, &out_selection),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_time_picker_test_resolve_colors(
      &picker, &out_background, &out_ring, &out_hand, &out_selection));
  picker.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(m3_time_picker_test_resolve_colors(
      &picker, &out_background, &out_ring, &out_hand, &out_selection));
  picker.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;
  picker.style.background_color.r = -1.0f;
  M3_TEST_EXPECT(m3_time_picker_test_resolve_colors(&picker, &out_background,
                                                    &out_ring, &out_hand,
                                                    &out_selection),
                 M3_ERR_RANGE);

  return 0;
}

static int test_metrics_and_picking(void) {
  M3TimePickerStyle style;
  M3TimePicker picker;
  M3TimePickerMetrics metrics;
  M3Time picked;
  M3Bool valid;
  M3Scalar hand_angle;
  M3Scalar hand_radius;
  M3Scalar saved_outer;
  M3Scalar saved_inner;

  M3_TEST_OK(m3_time_picker_style_init(&style));
  M3_TEST_OK(m3_time_picker_init(&picker, &style, 15u, 30u,
                                 M3_TIME_PICKER_FORMAT_24H));

  picker.bounds.x = 0.0f;
  picker.bounds.y = 0.0f;
  picker.bounds.width = 200.0f;
  picker.bounds.height = 200.0f;

  M3_TEST_OK(m3_time_picker_update(&picker));
  M3_TEST_OK(m3_time_picker_test_compute_metrics(&picker, &metrics));
  M3_TEST_ASSERT(m3_near(metrics.center_x, 100.0f, 0.01f));
  M3_TEST_ASSERT(m3_near(metrics.center_y, 100.0f, 0.01f));
  M3_TEST_ASSERT(metrics.outer_radius > metrics.inner_radius);
  saved_outer = metrics.outer_radius;
  saved_inner = metrics.inner_radius;

  M3_TEST_EXPECT(m3_time_picker_test_compute_hand(NULL, &metrics, &hand_angle,
                                                  &hand_radius),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_test_compute_hand(&picker, NULL, &hand_angle,
                                                  &hand_radius),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_time_picker_test_compute_hand(&picker, &metrics, NULL, &hand_radius),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_time_picker_test_compute_hand(&picker, &metrics, &hand_angle, NULL),
      M3_ERR_INVALID_ARGUMENT);

  picker.active_field = 99u;
  M3_TEST_EXPECT(m3_time_picker_test_compute_hand(&picker, &metrics,
                                                  &hand_angle, &hand_radius),
                 M3_ERR_RANGE);
  picker.active_field = M3_TIME_PICKER_FIELD_HOUR;
  picker.format = 99u;
  M3_TEST_EXPECT(m3_time_picker_test_compute_hand(&picker, &metrics,
                                                  &hand_angle, &hand_radius),
                 M3_ERR_RANGE);
  picker.format = M3_TIME_PICKER_FORMAT_24H;
  picker.time.hour = 99u;
  M3_TEST_EXPECT(m3_time_picker_test_compute_hand(&picker, &metrics,
                                                  &hand_angle, &hand_radius),
                 M3_ERR_RANGE);
  picker.time.hour = 15u;
  picker.active_field = M3_TIME_PICKER_FIELD_MINUTE;
  picker.time.minute = 99u;
  M3_TEST_EXPECT(m3_time_picker_test_compute_hand(&picker, &metrics,
                                                  &hand_angle, &hand_radius),
                 M3_ERR_RANGE);
  picker.time.minute = 30u;
  M3_TEST_OK(m3_time_picker_test_compute_hand(&picker, &metrics, &hand_angle,
                                              &hand_radius));
  picker.active_field = M3_TIME_PICKER_FIELD_HOUR;
  M3_TEST_OK(m3_time_picker_test_compute_hand(&picker, &metrics, &hand_angle,
                                              &hand_radius));
  metrics.outer_radius = 0.0f;
  metrics.inner_radius = 0.0f;
  M3_TEST_OK(m3_time_picker_test_compute_hand(&picker, &metrics, &hand_angle,
                                              &hand_radius));
  M3_TEST_ASSERT(hand_radius == 0.0f);
  metrics.outer_radius = saved_outer;
  metrics.inner_radius = saved_inner;

  M3_TEST_OK(m3_time_picker_test_pick_time(
      &picker, metrics.center_x, metrics.center_y - metrics.outer_radius,
      M3_TIME_PICKER_FIELD_HOUR, &picked, &valid));
  M3_TEST_ASSERT(valid == M3_TRUE);
  M3_TEST_ASSERT(picked.hour == 12u);

  M3_TEST_OK(m3_time_picker_test_pick_time(
      &picker, metrics.center_x, metrics.center_y - metrics.inner_radius,
      M3_TIME_PICKER_FIELD_HOUR, &picked, &valid));
  M3_TEST_ASSERT(valid == M3_TRUE);
  M3_TEST_ASSERT(picked.hour == 0u);

  M3_TEST_OK(m3_time_picker_test_pick_time(
      &picker, metrics.center_x + metrics.outer_radius, metrics.center_y,
      M3_TIME_PICKER_FIELD_MINUTE, &picked, &valid));
  M3_TEST_ASSERT(valid == M3_TRUE);
  M3_TEST_ASSERT(picked.minute == 15u);

  M3_TEST_OK(m3_time_picker_test_pick_time(
      &picker, metrics.center_x + metrics.outer_radius + 10.0f,
      metrics.center_y, M3_TIME_PICKER_FIELD_HOUR, &picked, &valid));
  M3_TEST_ASSERT(valid == M3_FALSE);

  M3_TEST_OK(m3_time_picker_set_format(&picker, M3_TIME_PICKER_FORMAT_12H));
  picker.time.hour = 15u;
  M3_TEST_OK(m3_time_picker_test_pick_time(
      &picker, metrics.center_x, metrics.center_y - metrics.outer_radius,
      M3_TIME_PICKER_FIELD_HOUR, &picked, &valid));
  M3_TEST_ASSERT(valid == M3_TRUE);
  M3_TEST_ASSERT(picked.hour == 12u);

  picker.time.hour = 3u;
  M3_TEST_OK(m3_time_picker_test_pick_time(
      &picker, metrics.center_x, metrics.center_y - metrics.outer_radius,
      M3_TIME_PICKER_FIELD_HOUR, &picked, &valid));
  M3_TEST_ASSERT(valid == M3_TRUE);
  M3_TEST_ASSERT(picked.hour == 0u);

  M3_TEST_EXPECT(m3_time_picker_test_pick_time(NULL, 0.0f, 0.0f,
                                               M3_TIME_PICKER_FIELD_HOUR,
                                               &picked, &valid),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_time_picker_test_pick_time(&picker, 0.0f, 0.0f, 99u, &picked, &valid),
      M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_time_picker_test_pick_time(&picker, 0.0f, 0.0f,
                                               M3_TIME_PICKER_FIELD_HOUR, NULL,
                                               &valid),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_test_pick_time(&picker, 0.0f, 0.0f,
                                               M3_TIME_PICKER_FIELD_HOUR,
                                               &picked, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  picker.bounds.width = 0.0f;
  picker.bounds.height = 0.0f;
  M3_TEST_OK(m3_time_picker_test_compute_metrics(&picker, &metrics));
  M3_TEST_ASSERT(metrics.outer_radius == 0.0f);
  M3_TEST_OK(m3_time_picker_test_pick_time(
      &picker, 0.0f, 0.0f, M3_TIME_PICKER_FIELD_HOUR, &picked, &valid));
  M3_TEST_ASSERT(valid == M3_FALSE);

  return 0;
}

static int test_init_and_setters(void) {
  M3TimePickerStyle style;
  M3TimePicker picker;
  M3Time time;
  m3_u32 format;
  m3_u32 field;
  m3_u32 period;
  ChangeCounter counter;
  M3Scalar prev_diameter;
  m3_u32 prev_format;

  M3_TEST_OK(m3_time_picker_style_init(&style));
  M3_TEST_EXPECT(
      m3_time_picker_init(NULL, &style, 0u, 0u, M3_TIME_PICKER_FORMAT_12H),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_time_picker_init(&picker, NULL, 0u, 0u, M3_TIME_PICKER_FORMAT_12H),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_time_picker_init(&picker, &style, 24u, 0u, M3_TIME_PICKER_FORMAT_12H),
      M3_ERR_RANGE);
  M3_TEST_EXPECT(
      m3_time_picker_init(&picker, &style, 0u, 60u, M3_TIME_PICKER_FORMAT_12H),
      M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_time_picker_init(&picker, &style, 0u, 0u, 99u),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_time_picker_init(&picker, &style, 13u, 15u,
                                 M3_TIME_PICKER_FORMAT_24H));

  M3_TEST_OK(m3_time_picker_get_format(&picker, &format));
  M3_TEST_ASSERT(format == M3_TIME_PICKER_FORMAT_24H);
  M3_TEST_OK(m3_time_picker_get_active_field(&picker, &field));
  M3_TEST_ASSERT(field == M3_TIME_PICKER_FIELD_HOUR);
  M3_TEST_OK(m3_time_picker_get_time(&picker, &time));
  M3_TEST_ASSERT(time.hour == 13u);
  M3_TEST_ASSERT(time.minute == 15u);

  M3_TEST_EXPECT(m3_time_picker_get_format(NULL, &format),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_get_format(&picker, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_get_active_field(NULL, &field),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_get_active_field(&picker, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_get_time(NULL, &time), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_get_time(&picker, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_time_picker_set_style(NULL, &style),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_set_style(&picker, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  style.diameter = 0.0f;
  M3_TEST_EXPECT(m3_time_picker_set_style(&picker, &style), M3_ERR_RANGE);
  M3_TEST_OK(m3_time_picker_style_init(&style));

  picker.style = style;
  picker.style.diameter = 180.0f;
  prev_diameter = picker.style.diameter;
  picker.bounds.width = -1.0f;
  M3_TEST_EXPECT(m3_time_picker_set_style(&picker, &style), M3_ERR_RANGE);
  M3_TEST_ASSERT(picker.style.diameter == prev_diameter);
  picker.bounds.width = 200.0f;

  M3_TEST_EXPECT(m3_time_picker_set_format(NULL, M3_TIME_PICKER_FORMAT_12H),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_set_format(&picker, 99u), M3_ERR_RANGE);

  prev_format = picker.format;
  picker.bounds.width = -1.0f;
  M3_TEST_EXPECT(m3_time_picker_set_format(&picker, M3_TIME_PICKER_FORMAT_12H),
                 M3_ERR_RANGE);
  M3_TEST_ASSERT(picker.format == prev_format);
  picker.bounds.width = 200.0f;

  M3_TEST_EXPECT(m3_time_picker_set_period(NULL, M3_TIME_PICKER_PERIOD_AM),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_set_period(&picker, M3_TIME_PICKER_PERIOD_AM),
                 M3_ERR_STATE);

  M3_TEST_OK(m3_time_picker_set_format(&picker, M3_TIME_PICKER_FORMAT_12H));

  M3_TEST_EXPECT(
      m3_time_picker_set_active_field(NULL, M3_TIME_PICKER_FIELD_HOUR),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_set_active_field(&picker, 99u), M3_ERR_RANGE);
  M3_TEST_OK(
      m3_time_picker_set_active_field(&picker, M3_TIME_PICKER_FIELD_MINUTE));

  time.hour = 10u;
  time.minute = 5u;
  M3_TEST_EXPECT(m3_time_picker_set_time(NULL, &time), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_set_time(&picker, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  time.hour = 30u;
  time.minute = 5u;
  M3_TEST_EXPECT(m3_time_picker_set_time(&picker, &time), M3_ERR_RANGE);
  time.hour = 10u;
  time.minute = 70u;
  M3_TEST_EXPECT(m3_time_picker_set_time(&picker, &time), M3_ERR_RANGE);
  time.hour = 10u;
  time.minute = 5u;
  M3_TEST_OK(m3_time_picker_set_time(&picker, &time));

  M3_TEST_EXPECT(m3_time_picker_set_on_change(NULL, test_on_change, &counter),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_time_picker_set_format(&picker, M3_TIME_PICKER_FORMAT_12H));
  M3_TEST_EXPECT(m3_time_picker_set_period(&picker, 99u), M3_ERR_RANGE);
  picker.time.hour = 13u;
  M3_TEST_OK(m3_time_picker_set_period(&picker, M3_TIME_PICKER_PERIOD_AM));
  M3_TEST_ASSERT(picker.time.hour == 1u);
  M3_TEST_OK(m3_time_picker_set_period(&picker, M3_TIME_PICKER_PERIOD_PM));
  M3_TEST_ASSERT(picker.time.hour == 13u);

  M3_TEST_EXPECT(m3_time_picker_get_period(NULL, &period),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_time_picker_get_period(&picker, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_time_picker_get_period(&picker, &period));
  M3_TEST_ASSERT(period == M3_TIME_PICKER_PERIOD_PM);

  memset(&counter, 0, sizeof(counter));
  counter.fail = 1;
  M3_TEST_OK(m3_time_picker_set_on_change(&picker, test_on_change, &counter));
  time.hour = 9u;
  time.minute = 45u;
  M3_TEST_EXPECT(m3_time_picker_set_time(&picker, &time), M3_ERR_IO);
  M3_TEST_ASSERT(picker.time.hour == 13u);

  counter.fail = 0;
  M3_TEST_OK(m3_time_picker_set_time(&picker, &time));
  M3_TEST_ASSERT(counter.calls >= 1);
  M3_TEST_ASSERT(picker.time.hour == 9u);

  M3_TEST_EXPECT(m3_time_picker_update(NULL), M3_ERR_INVALID_ARGUMENT);

  return 0;
}

static int test_widget_api(void) {
  M3TimePickerStyle style;
  M3TimePicker picker;
  M3MeasureSpec spec;
  M3Size size;
  M3Rect bounds;
  M3Semantics semantics;
  int rc;

  M3_TEST_OK(m3_time_picker_style_init(&style));
  M3_TEST_OK(
      m3_time_picker_init(&picker, &style, 8u, 0u, M3_TIME_PICKER_FORMAT_24H));

  spec.mode = 99u;
  spec.size = 0.0f;
  M3_TEST_EXPECT(
      picker.widget.vtable->measure(picker.widget.ctx, spec, spec, &size),
      M3_ERR_INVALID_ARGUMENT);
  spec.mode = M3_MEASURE_EXACTLY;
  spec.size = 123.0f;
  M3_TEST_OK(
      picker.widget.vtable->measure(picker.widget.ctx, spec, spec, &size));
  M3_TEST_ASSERT(size.width == 123.0f);
  M3_TEST_ASSERT(size.height == 123.0f);

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  M3_TEST_EXPECT(picker.widget.vtable->layout(picker.widget.ctx, bounds),
                 M3_ERR_RANGE);
  bounds.width = 100.0f;
  bounds.height = 100.0f;
  M3_TEST_OK(picker.widget.vtable->layout(picker.widget.ctx, bounds));

  M3_TEST_EXPECT(picker.widget.vtable->get_semantics(NULL, &semantics),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(picker.widget.vtable->get_semantics(picker.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  rc = picker.widget.vtable->get_semantics(picker.widget.ctx, &semantics);
  M3_TEST_OK(rc);
  M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_NONE);

  picker.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  rc = picker.widget.vtable->get_semantics(picker.widget.ctx, &semantics);
  M3_TEST_OK(rc);
  M3_TEST_ASSERT(semantics.flags & M3_SEMANTIC_FLAG_DISABLED);

  M3_TEST_EXPECT(picker.widget.vtable->destroy(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(picker.widget.vtable->destroy(picker.widget.ctx));

  return 0;
}

static int test_event_and_paint(void) {
  M3TimePickerStyle style;
  M3TimePicker picker;
  M3TimePickerMetrics metrics;
  TestTimePickerBackend backend;
  M3Gfx gfx;
  M3PaintContext ctx;
  M3InputEvent event;
  M3Bool handled;
  ChangeCounter counter;
  M3Time time;

  M3_TEST_OK(m3_time_picker_style_init(&style));
  M3_TEST_OK(
      m3_time_picker_init(&picker, &style, 6u, 0u, M3_TIME_PICKER_FORMAT_24H));

  picker.bounds.x = 0.0f;
  picker.bounds.y = 0.0f;
  picker.bounds.width = 200.0f;
  picker.bounds.height = 200.0f;
  M3_TEST_OK(m3_time_picker_update(&picker));

  M3_TEST_OK(m3_time_picker_test_compute_metrics(&picker, &metrics));

  M3_TEST_OK(test_backend_init(&backend));
  gfx.ctx = &backend;
  gfx.vtable = &g_test_vtable;
  ctx.gfx = &gfx;
  ctx.clip.x = 0.0f;
  ctx.clip.y = 0.0f;
  ctx.clip.width = 200.0f;
  ctx.clip.height = 200.0f;
  ctx.dpi_scale = 1.0f;

  M3_TEST_EXPECT(picker.widget.vtable->paint(NULL, &ctx),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  ctx.gfx = NULL;
  M3_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                 M3_ERR_INVALID_ARGUMENT);
  ctx.gfx = &gfx;
  gfx.vtable = NULL;
  M3_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                 M3_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_vtable_no_rect;
  M3_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable_no_line;
  M3_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                 M3_ERR_UNSUPPORTED);

  gfx.vtable = &g_test_vtable;
  M3_TEST_OK(picker.widget.vtable->paint(picker.widget.ctx, &ctx));
  M3_TEST_ASSERT(backend.draw_rect_calls > 0);
  M3_TEST_ASSERT(backend.draw_line_calls > 0);

  picker.time.hour = 0u;
  picker.active_field = M3_TIME_PICKER_FIELD_HOUR;
  M3_TEST_OK(picker.widget.vtable->paint(picker.widget.ctx, &ctx));
  picker.active_field = M3_TIME_PICKER_FIELD_MINUTE;
  M3_TEST_OK(picker.widget.vtable->paint(picker.widget.ctx, &ctx));
  picker.active_field = M3_TIME_PICKER_FIELD_HOUR;

  picker.bounds.width = 0.0f;
  picker.bounds.height = 0.0f;
  M3_TEST_OK(picker.widget.vtable->layout(picker.widget.ctx, picker.bounds));
  M3_TEST_OK(picker.widget.vtable->paint(picker.widget.ctx, &ctx));
  picker.bounds.width = 200.0f;
  picker.bounds.height = 200.0f;
  M3_TEST_OK(picker.widget.vtable->layout(picker.widget.ctx, picker.bounds));

  backend.fail_draw_rect = M3_ERR_IO;
  M3_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                 M3_ERR_IO);
  backend.fail_draw_rect = M3_OK;

  backend.fail_draw_line = M3_ERR_IO;
  M3_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                 M3_ERR_IO);
  backend.fail_draw_line = M3_OK;

  picker.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(picker.widget.vtable->paint(picker.widget.ctx, &ctx));
  picker.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  picker.style.hand_center_radius = 0.0f;
  M3_TEST_OK(picker.widget.vtable->paint(picker.widget.ctx, &ctx));
  picker.style.hand_center_radius = M3_TIME_PICKER_DEFAULT_HAND_CENTER_RADIUS;

  M3_TEST_OK(test_backend_init(&backend));
  gfx.vtable = &g_test_vtable;
  gfx.ctx = &backend;
  ctx.gfx = &gfx;

  M3_TEST_EXPECT(picker.widget.vtable->event(NULL, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(picker.widget.vtable->event(picker.widget.ctx, NULL, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(picker.widget.vtable->event(picker.widget.ctx, &event, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_MOVE,
                                (m3_i32)metrics.center_x,
                                (m3_i32)metrics.center_y));
  M3_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(init_pointer_event(
      &event, M3_INPUT_POINTER_DOWN,
      (m3_i32)(metrics.center_x + metrics.outer_radius + 10.0f),
      (m3_i32)metrics.center_y));
  M3_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  M3_TEST_ASSERT(picker.pressed == M3_FALSE);

  memset(&counter, 0, sizeof(counter));
  M3_TEST_OK(m3_time_picker_set_on_change(&picker, test_on_change, &counter));
  M3_TEST_OK(init_pointer_event(
      &event, M3_INPUT_POINTER_DOWN, (m3_i32)metrics.center_x,
      (m3_i32)(metrics.center_y - metrics.outer_radius)));
  M3_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(picker.pressed == M3_TRUE);
  M3_TEST_ASSERT(counter.calls == 1);
  M3_TEST_ASSERT(picker.time.hour == 12u);

  M3_TEST_EXPECT(
      picker.widget.vtable->event(picker.widget.ctx, &event, &handled),
      M3_ERR_STATE);

  M3_TEST_OK(
      init_pointer_event(&event, M3_INPUT_POINTER_MOVE,
                         (m3_i32)(metrics.center_x + metrics.outer_radius),
                         (m3_i32)metrics.center_y));
  M3_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(picker.time.hour == 3u);

  M3_TEST_OK(init_pointer_event(
      &event, M3_INPUT_POINTER_MOVE,
      (m3_i32)(metrics.center_x + metrics.outer_radius + 20.0f),
      (m3_i32)metrics.center_y));
  M3_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP,
                                (m3_i32)metrics.center_x,
                                (m3_i32)metrics.center_y));
  M3_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(picker.pressed == M3_FALSE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP,
                                (m3_i32)metrics.center_x,
                                (m3_i32)metrics.center_y));
  M3_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_MOVE,
                                (m3_i32)metrics.center_x,
                                (m3_i32)metrics.center_y));
  M3_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  picker.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(init_pointer_event(
      &event, M3_INPUT_POINTER_DOWN, (m3_i32)metrics.center_x,
      (m3_i32)(metrics.center_y - metrics.outer_radius)));
  M3_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  picker.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  counter.fail = 1;
  time = picker.time;
  M3_TEST_OK(init_pointer_event(
      &event, M3_INPUT_POINTER_DOWN, (m3_i32)metrics.center_x,
      (m3_i32)(metrics.center_y - metrics.outer_radius)));
  M3_TEST_EXPECT(
      picker.widget.vtable->event(picker.widget.ctx, &event, &handled),
      M3_ERR_IO);
  M3_TEST_ASSERT(picker.time.hour == time.hour);
  counter.fail = 0;

  M3_TEST_OK(m3_time_picker_set_format(&picker, M3_TIME_PICKER_FORMAT_12H));
  picker.time.hour = 0u;
  picker.active_field = M3_TIME_PICKER_FIELD_MINUTE;
  M3_TEST_OK(
      init_pointer_event(&event, M3_INPUT_POINTER_DOWN,
                         (m3_i32)(metrics.center_x + metrics.outer_radius),
                         (m3_i32)metrics.center_y));
  M3_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(picker.time.minute == 15u);

  M3_TEST_EXPECT(m3_time_picker_test_draw_circle(NULL, 0.0f, 0.0f, 1.0f,
                                                 picker.style.background_color),
                 M3_ERR_UNSUPPORTED);
  M3_TEST_EXPECT(m3_time_picker_test_draw_circle(&gfx, 0.0f, 0.0f, -1.0f,
                                                 picker.style.background_color),
                 M3_ERR_RANGE);
  backend.draw_rect_calls = 0;
  M3_TEST_OK(m3_time_picker_test_draw_circle(&gfx, 0.0f, 0.0f, 0.0f,
                                             picker.style.background_color));
  M3_TEST_ASSERT(backend.draw_rect_calls == 0);

  M3_TEST_EXPECT(m3_time_picker_test_draw_ring(&gfx, 0.0f, 0.0f, 1.0f, 0.0f,
                                               picker.style.ring_color,
                                               picker.style.background_color),
                 M3_ERR_RANGE);
  backend.draw_rect_calls = 0;
  M3_TEST_OK(m3_time_picker_test_draw_ring(&gfx, 0.0f, 0.0f, 0.0f, 1.0f,
                                           picker.style.ring_color,
                                           picker.style.background_color));
  M3_TEST_ASSERT(backend.draw_rect_calls == 0);
  backend.draw_rect_calls = 0;
  M3_TEST_OK(m3_time_picker_test_draw_ring(&gfx, 0.0f, 0.0f, 1.0f, 5.0f,
                                           picker.style.ring_color,
                                           picker.style.background_color));
  M3_TEST_ASSERT(backend.draw_rect_calls == 1);

  picker.bounds.width = 6.0f;
  picker.bounds.height = 6.0f;
  picker.style.diameter = 100.0f;
  picker.style.ring_thickness = 4.0f;
  picker.style.padding.left = 0.0f;
  picker.style.padding.right = 0.0f;
  picker.style.padding.top = 0.0f;
  picker.style.padding.bottom = 0.0f;
  M3_TEST_OK(picker.widget.vtable->layout(picker.widget.ctx, picker.bounds));
  M3_TEST_OK(picker.widget.vtable->paint(picker.widget.ctx, &ctx));

  return 0;
}

int main(void) {
  if (test_helpers() != 0) {
    return 1;
  }
  if (test_metrics_and_picking() != 0) {
    return 1;
  }
  if (test_init_and_setters() != 0) {
    return 1;
  }
  if (test_widget_api() != 0) {
    return 1;
  }
  if (test_event_and_paint() != 0) {
    return 1;
  }
  return 0;
}
