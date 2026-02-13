#include "m3/m3_time_picker.h"
#include "test_utils.h"

#include <string.h>

#define CMP_TEST_PI 3.14159265358979323846f

typedef struct TestTimePickerBackend {
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
} TestTimePickerBackend;

typedef struct ChangeCounter {
  int calls;
  int fail;
  CMPTime last;
} ChangeCounter;

static int test_backend_init(TestTimePickerBackend *backend) {
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
  TestTimePickerBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestTimePickerBackend *)gfx;
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
  TestTimePickerBackend *backend;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestTimePickerBackend *)gfx;
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

static int test_on_change(void *ctx, M3TimePicker *picker,
                          const CMPTime *time) {
  ChangeCounter *counter;

  if (ctx == NULL || picker == NULL || time == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  counter = (ChangeCounter *)ctx;
  counter->calls += 1;
  counter->last = *time;
  if (counter->fail) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int cmp_near(CMPScalar a, CMPScalar b, CMPScalar tol) {
  CMPScalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

static int test_helpers(void) {
  M3TimePickerStyle style;
  CMPLayoutEdges edges;
  CMPColor color;
  CMPRect rect;
  CMPMeasureSpec spec;
  CMPTime time;
  M3TimePicker picker;
  M3TimePickerMetrics metrics;
  CMPScalar angle;
  cmp_u32 index;
  cmp_u32 hour;
  CMPBool inner;
  CMPColor out_background;
  CMPColor out_ring;
  CMPColor out_hand;
  CMPColor out_selection;
  volatile const CMPColor *null_color;
  volatile const CMPLayoutEdges *null_edges;
  volatile const M3TimePickerStyle *null_style;
  volatile const CMPRect *null_rect;
  volatile const CMPTime *null_time;

  null_color = NULL;
  null_edges = NULL;
  null_style = NULL;
  null_rect = NULL;
  null_time = NULL;

  CMP_TEST_EXPECT(
      m3_time_picker_test_validate_color((const CMPColor *)null_color),
      CMP_ERR_INVALID_ARGUMENT);
  color.r = 1.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 0.0f;
  color.g = -0.1f;
  color.b = 0.0f;
  color.a = 0.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.1f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = 0.0f;
  color.a = -0.1f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_color(&color), CMP_ERR_RANGE);

  CMP_TEST_EXPECT(
      m3_time_picker_test_validate_edges((const CMPLayoutEdges *)null_edges),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_time_picker_test_validate_style((const M3TimePickerStyle *)null_style),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_test_validate_rect((const CMPRect *)null_rect),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_test_validate_time((const CMPTime *)null_time),
                  CMP_ERR_INVALID_ARGUMENT);

  edges.left = -1.0f;
  edges.right = 0.0f;
  edges.top = 0.0f;
  edges.bottom = 0.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_edges(&edges), CMP_ERR_RANGE);

  CMP_TEST_OK(m3_time_picker_style_init(&style));
  CMP_TEST_OK(m3_time_picker_test_validate_style(&style));
  style.diameter = 0.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  style.ring_thickness = 0.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  style.ring_thickness = style.diameter;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  style.inner_ring_ratio = 0.5f;
  style.ring_thickness = (style.diameter * 0.5f) * style.inner_ring_ratio;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  style.inner_ring_ratio = 0.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  style.inner_ring_ratio = 1.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  style.hand_thickness = 0.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  style.hand_center_radius = -1.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  style.hand_center_radius = style.diameter;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  style.padding.left = -1.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  style.background_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  style.ring_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  style.hand_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  style.selection_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  style.disabled_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_style(&style), CMP_ERR_RANGE);

  spec.mode = 99u;
  spec.size = 10.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_measure_spec(spec),
                  CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_MEASURE_AT_MOST;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_measure_spec(spec),
                  CMP_ERR_RANGE);
  spec.mode = CMP_MEASURE_EXACTLY;
  spec.size = 10.0f;
  CMP_TEST_OK(m3_time_picker_test_validate_measure_spec(spec));

  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 10.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.width = 10.0f;
  rect.height = -1.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_rect(&rect), CMP_ERR_RANGE);

  time.hour = 24u;
  time.minute = 0u;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_time(&time), CMP_ERR_RANGE);
  time.hour = 23u;
  time.minute = 60u;
  CMP_TEST_EXPECT(m3_time_picker_test_validate_time(&time), CMP_ERR_RANGE);
  time.hour = 23u;
  time.minute = 59u;
  CMP_TEST_OK(m3_time_picker_test_validate_time(&time));

  CMP_TEST_OK(
      m3_time_picker_test_angle_from_point(0.0f, 0.0f, 0.0f, -1.0f, &angle));
  CMP_TEST_ASSERT(cmp_near(angle, 0.0f, 1e-3f));
  CMP_TEST_EXPECT(
      m3_time_picker_test_angle_from_point(0.0f, 0.0f, 0.0f, 0.0f, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(
      m3_time_picker_test_angle_from_point(0.0f, 0.0f, 1.0f, 0.0f, &angle));
  CMP_TEST_ASSERT(cmp_near(angle, CMP_TEST_PI * 0.5f, 1e-3f));
  CMP_TEST_OK(
      m3_time_picker_test_angle_from_point(-1.0f, -1.0f, -2.0f, -2.0f, &angle));
  CMP_TEST_ASSERT(cmp_near(angle, (CMPScalar)(CMP_TEST_PI * 1.75f), 1e-3f));

  CMP_TEST_EXPECT(m3_time_picker_test_angle_to_index(0.0f, 0u, &index),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_time_picker_test_angle_to_index(0.0f, 12u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_time_picker_test_angle_to_index(0.0f, 12u, &index));
  CMP_TEST_ASSERT(index == 0u);
  angle = (CMPScalar)(CMP_TEST_PI * 2.0f / 12.0f) * 0.6f;
  CMP_TEST_OK(m3_time_picker_test_angle_to_index(angle, 12u, &index));
  CMP_TEST_ASSERT(index == 1u);
  CMP_TEST_OK(m3_time_picker_test_angle_to_index(-0.1f, 12u, &index));
  CMP_TEST_ASSERT(index == 0u);
  CMP_TEST_OK(m3_time_picker_test_angle_to_index(7.0f, 12u, &index));
  CMP_TEST_ASSERT(index == 1u);

  CMP_TEST_EXPECT(m3_time_picker_test_hour_from_index(
                      12u, M3_TIME_PICKER_FORMAT_24H, CMP_FALSE, 0u, &hour),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_time_picker_test_hour_from_index(
                      0u, M3_TIME_PICKER_FORMAT_24H, CMP_FALSE, 0u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_time_picker_test_hour_from_index(0u, 99u, CMP_FALSE, 0u, &hour),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_time_picker_test_hour_from_index(
                      0u, M3_TIME_PICKER_FORMAT_12H, CMP_FALSE, 99u, &hour),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_test_hour_from_index(0u, M3_TIME_PICKER_FORMAT_24H,
                                                  CMP_TRUE, 0u, &hour));
  CMP_TEST_ASSERT(hour == 0u);
  CMP_TEST_OK(m3_time_picker_test_hour_from_index(0u, M3_TIME_PICKER_FORMAT_24H,
                                                  CMP_FALSE, 0u, &hour));
  CMP_TEST_ASSERT(hour == 12u);
  CMP_TEST_OK(m3_time_picker_test_hour_from_index(1u, M3_TIME_PICKER_FORMAT_24H,
                                                  CMP_TRUE, 0u, &hour));
  CMP_TEST_ASSERT(hour == 13u);
  CMP_TEST_OK(m3_time_picker_test_hour_from_index(1u, M3_TIME_PICKER_FORMAT_12H,
                                                  CMP_FALSE, 15u, &hour));
  CMP_TEST_ASSERT(hour == 13u);
  CMP_TEST_OK(m3_time_picker_test_hour_from_index(0u, M3_TIME_PICKER_FORMAT_12H,
                                                  CMP_FALSE, 3u, &hour));
  CMP_TEST_ASSERT(hour == 0u);

  CMP_TEST_EXPECT(m3_time_picker_test_hour_to_index(
                      24u, M3_TIME_PICKER_FORMAT_24H, &index, &inner),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_time_picker_test_hour_to_index(0u, 99u, &index, &inner),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_time_picker_test_hour_to_index(
                      0u, M3_TIME_PICKER_FORMAT_24H, NULL, &inner),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_test_hour_to_index(
                      0u, M3_TIME_PICKER_FORMAT_24H, &index, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_time_picker_test_hour_to_index(0u, M3_TIME_PICKER_FORMAT_24H,
                                                &index, &inner));
  CMP_TEST_ASSERT(index == 0u);
  CMP_TEST_ASSERT(inner == CMP_TRUE);
  CMP_TEST_OK(m3_time_picker_test_hour_to_index(12u, M3_TIME_PICKER_FORMAT_24H,
                                                &index, &inner));
  CMP_TEST_ASSERT(index == 0u);
  CMP_TEST_ASSERT(inner == CMP_FALSE);
  CMP_TEST_OK(m3_time_picker_test_hour_to_index(23u, M3_TIME_PICKER_FORMAT_24H,
                                                &index, &inner));
  CMP_TEST_ASSERT(index == 11u);
  CMP_TEST_ASSERT(inner == CMP_TRUE);
  CMP_TEST_OK(m3_time_picker_test_hour_to_index(5u, M3_TIME_PICKER_FORMAT_12H,
                                                &index, &inner));
  CMP_TEST_ASSERT(index == 5u);
  CMP_TEST_ASSERT(inner == CMP_FALSE);

  CMP_TEST_OK(m3_time_picker_style_init(&style));
  CMP_TEST_OK(
      m3_time_picker_init(&picker, &style, 0u, 0u, M3_TIME_PICKER_FORMAT_24H));
  CMP_TEST_EXPECT(m3_time_picker_test_update_metrics(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_test_compute_metrics(NULL, &metrics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_test_compute_metrics(&picker, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  picker.bounds.width = -1.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_compute_metrics(&picker, &metrics),
                  CMP_ERR_RANGE);
  picker.bounds.width = 0.0f;
  picker.bounds.height = 0.0f;
  picker.style.diameter = 0.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_compute_metrics(&picker, &metrics),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  CMP_TEST_OK(
      m3_time_picker_init(&picker, &style, 0u, 0u, M3_TIME_PICKER_FORMAT_24H));
  picker.bounds.width = 100.0f;
  picker.bounds.height = 80.0f;
  CMP_TEST_OK(m3_time_picker_test_set_force_dial_size_zero(CMP_TRUE));
  CMP_TEST_OK(m3_time_picker_test_compute_metrics(&picker, &metrics));
  CMP_TEST_ASSERT(metrics.outer_radius == 0.0f);
  CMP_TEST_EXPECT(m3_time_picker_test_resolve_colors(NULL, &out_background,
                                                     &out_ring, &out_hand,
                                                     &out_selection),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_test_resolve_colors(&picker, NULL, &out_ring,
                                                     &out_hand, &out_selection),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_time_picker_test_resolve_colors(
      &picker, &out_background, &out_ring, &out_hand, &out_selection));
  picker.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(m3_time_picker_test_resolve_colors(
      &picker, &out_background, &out_ring, &out_hand, &out_selection));
  picker.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;
  picker.style.background_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_resolve_colors(&picker, &out_background,
                                                     &out_ring, &out_hand,
                                                     &out_selection),
                  CMP_ERR_RANGE);
  picker.style.background_color.r = 0.0f;
  picker.style.ring_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_resolve_colors(&picker, &out_background,
                                                     &out_ring, &out_hand,
                                                     &out_selection),
                  CMP_ERR_RANGE);
  picker.style.ring_color.r = 0.0f;
  picker.style.hand_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_resolve_colors(&picker, &out_background,
                                                     &out_ring, &out_hand,
                                                     &out_selection),
                  CMP_ERR_RANGE);
  picker.style.hand_color.r = 0.0f;
  picker.style.selection_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_resolve_colors(&picker, &out_background,
                                                     &out_ring, &out_hand,
                                                     &out_selection),
                  CMP_ERR_RANGE);

  return 0;
}

static int test_metrics_and_picking(void) {
  M3TimePickerStyle style;
  M3TimePicker picker;
  M3TimePickerMetrics metrics;
  CMPTime picked;
  CMPTime saved_time;
  CMPRect saved_bounds;
  CMPBool valid;
  CMPScalar hand_angle;
  CMPScalar hand_radius;
  CMPScalar saved_outer;
  CMPScalar saved_inner;

  CMP_TEST_OK(m3_time_picker_style_init(&style));
  CMP_TEST_OK(m3_time_picker_init(&picker, &style, 15u, 30u,
                                  M3_TIME_PICKER_FORMAT_24H));

  picker.bounds.x = 0.0f;
  picker.bounds.y = 0.0f;
  picker.bounds.width = 200.0f;
  picker.bounds.height = 200.0f;

  CMP_TEST_OK(m3_time_picker_update(&picker));
  CMP_TEST_OK(m3_time_picker_test_compute_metrics(&picker, &metrics));
  CMP_TEST_ASSERT(cmp_near(metrics.center_x, 100.0f, 0.01f));
  CMP_TEST_ASSERT(cmp_near(metrics.center_y, 100.0f, 0.01f));
  CMP_TEST_ASSERT(metrics.outer_radius > metrics.inner_radius);
  saved_outer = metrics.outer_radius;
  saved_inner = metrics.inner_radius;

  CMP_TEST_EXPECT(m3_time_picker_test_compute_hand(NULL, &metrics, &hand_angle,
                                                   &hand_radius),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_test_compute_hand(&picker, NULL, &hand_angle,
                                                   &hand_radius),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_time_picker_test_compute_hand(&picker, &metrics, NULL, &hand_radius),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_time_picker_test_compute_hand(&picker, &metrics, &hand_angle, NULL),
      CMP_ERR_INVALID_ARGUMENT);

  picker.active_field = 99u;
  CMP_TEST_EXPECT(m3_time_picker_test_compute_hand(&picker, &metrics,
                                                   &hand_angle, &hand_radius),
                  CMP_ERR_RANGE);
  picker.active_field = M3_TIME_PICKER_FIELD_HOUR;
  picker.format = 99u;
  CMP_TEST_EXPECT(m3_time_picker_test_compute_hand(&picker, &metrics,
                                                   &hand_angle, &hand_radius),
                  CMP_ERR_RANGE);
  picker.format = M3_TIME_PICKER_FORMAT_24H;
  picker.time.hour = 99u;
  CMP_TEST_EXPECT(m3_time_picker_test_compute_hand(&picker, &metrics,
                                                   &hand_angle, &hand_radius),
                  CMP_ERR_RANGE);
  picker.time.hour = 15u;
  picker.active_field = M3_TIME_PICKER_FIELD_MINUTE;
  picker.time.minute = 99u;
  CMP_TEST_EXPECT(m3_time_picker_test_compute_hand(&picker, &metrics,
                                                   &hand_angle, &hand_radius),
                  CMP_ERR_RANGE);
  picker.time.minute = 30u;
  CMP_TEST_OK(m3_time_picker_test_compute_hand(&picker, &metrics, &hand_angle,
                                               &hand_radius));
  CMP_TEST_OK(
      m3_time_picker_test_set_force_compute_hand_minute_error(CMP_TRUE));
  CMP_TEST_EXPECT(m3_time_picker_test_compute_hand(&picker, &metrics,
                                                   &hand_angle, &hand_radius),
                  CMP_ERR_RANGE);
  picker.active_field = M3_TIME_PICKER_FIELD_HOUR;
  picker.format = M3_TIME_PICKER_FORMAT_24H;
  picker.time.hour = 15u;
  CMP_TEST_OK(m3_time_picker_test_set_force_hour_to_index_error(CMP_TRUE));
  CMP_TEST_EXPECT(m3_time_picker_test_compute_hand(&picker, &metrics,
                                                   &hand_angle, &hand_radius),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_test_compute_hand(&picker, &metrics, &hand_angle,
                                               &hand_radius));
  picker.style.ring_thickness = metrics.outer_radius * 4.0f;
  metrics.outer_radius = 1.0f;
  CMP_TEST_OK(m3_time_picker_test_compute_hand(&picker, &metrics, &hand_angle,
                                               &hand_radius));
  CMP_TEST_ASSERT(hand_radius == 0.0f);
  picker.style.ring_thickness = M3_TIME_PICKER_DEFAULT_RING_THICKNESS;
  metrics.outer_radius = 0.0f;
  metrics.inner_radius = 0.0f;
  CMP_TEST_OK(m3_time_picker_test_compute_hand(&picker, &metrics, &hand_angle,
                                               &hand_radius));
  CMP_TEST_ASSERT(hand_radius == 0.0f);
  metrics.outer_radius = saved_outer;
  metrics.inner_radius = saved_inner;

  CMP_TEST_OK(m3_time_picker_test_pick_time(
      &picker, metrics.center_x, metrics.center_y - metrics.outer_radius,
      M3_TIME_PICKER_FIELD_HOUR, &picked, &valid));
  CMP_TEST_ASSERT(valid == CMP_TRUE);
  CMP_TEST_ASSERT(picked.hour == 12u);

  CMP_TEST_OK(m3_time_picker_test_pick_time(
      &picker, metrics.center_x, metrics.center_y - metrics.inner_radius,
      M3_TIME_PICKER_FIELD_HOUR, &picked, &valid));
  CMP_TEST_ASSERT(valid == CMP_TRUE);
  CMP_TEST_ASSERT(picked.hour == 0u);

  CMP_TEST_OK(m3_time_picker_test_pick_time(
      &picker, metrics.center_x + metrics.outer_radius, metrics.center_y,
      M3_TIME_PICKER_FIELD_MINUTE, &picked, &valid));
  CMP_TEST_ASSERT(valid == CMP_TRUE);
  CMP_TEST_ASSERT(picked.minute == 15u);

  CMP_TEST_OK(m3_time_picker_test_pick_time(
      &picker, metrics.center_x + metrics.outer_radius + 10.0f,
      metrics.center_y, M3_TIME_PICKER_FIELD_HOUR, &picked, &valid));
  CMP_TEST_ASSERT(valid == CMP_FALSE);

  CMP_TEST_OK(m3_time_picker_test_set_force_angle_error(CMP_TRUE));
  CMP_TEST_EXPECT(m3_time_picker_test_pick_time(
                      &picker, metrics.center_x, metrics.center_y - 10.0f,
                      M3_TIME_PICKER_FIELD_HOUR, &picked, &valid),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_test_set_force_angle_index_error(CMP_TRUE));
  CMP_TEST_EXPECT(m3_time_picker_test_pick_time(
                      &picker, metrics.center_x, metrics.center_y - 10.0f,
                      M3_TIME_PICKER_FIELD_HOUR, &picked, &valid),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_test_set_force_hour_invalid(CMP_TRUE));
  CMP_TEST_EXPECT(m3_time_picker_test_pick_time(
                      &picker, metrics.center_x, metrics.center_y - 10.0f,
                      M3_TIME_PICKER_FIELD_HOUR, &picked, &valid),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_test_set_force_angle_index_error(CMP_TRUE));
  CMP_TEST_EXPECT(m3_time_picker_test_pick_time(
                      &picker, metrics.center_x + metrics.outer_radius,
                      metrics.center_y, M3_TIME_PICKER_FIELD_MINUTE, &picked,
                      &valid),
                  CMP_ERR_RANGE);

  CMP_TEST_OK(m3_time_picker_set_format(&picker, M3_TIME_PICKER_FORMAT_12H));
  picker.time.hour = 15u;
  CMP_TEST_OK(m3_time_picker_test_pick_time(
      &picker, metrics.center_x, metrics.center_y - metrics.outer_radius,
      M3_TIME_PICKER_FIELD_HOUR, &picked, &valid));
  CMP_TEST_ASSERT(valid == CMP_TRUE);
  CMP_TEST_ASSERT(picked.hour == 12u);

  picker.time.hour = 3u;
  CMP_TEST_OK(m3_time_picker_test_pick_time(
      &picker, metrics.center_x, metrics.center_y - metrics.outer_radius,
      M3_TIME_PICKER_FIELD_HOUR, &picked, &valid));
  CMP_TEST_ASSERT(valid == CMP_TRUE);
  CMP_TEST_ASSERT(picked.hour == 0u);

  CMP_TEST_EXPECT(m3_time_picker_test_pick_time(NULL, 0.0f, 0.0f,
                                                M3_TIME_PICKER_FIELD_HOUR,
                                                &picked, &valid),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_time_picker_test_pick_time(&picker, 0.0f, 0.0f, 99u, &picked, &valid),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_time_picker_test_pick_time(&picker, 0.0f, 0.0f,
                                                M3_TIME_PICKER_FIELD_HOUR, NULL,
                                                &valid),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_test_pick_time(&picker, 0.0f, 0.0f,
                                                M3_TIME_PICKER_FIELD_HOUR,
                                                &picked, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  saved_bounds = picker.bounds;
  picker.bounds.width = -1.0f;
  CMP_TEST_EXPECT(m3_time_picker_test_pick_time(&picker, 0.0f, 0.0f,
                                                M3_TIME_PICKER_FIELD_HOUR,
                                                &picked, &valid),
                  CMP_ERR_RANGE);
  picker.bounds = saved_bounds;

  saved_time = picker.time;
  picker.time.hour = 24u;
  CMP_TEST_EXPECT(
      m3_time_picker_test_pick_time(&picker, metrics.center_x, metrics.center_y,
                                    M3_TIME_PICKER_FIELD_HOUR, &picked, &valid),
      CMP_ERR_RANGE);
  picker.time = saved_time;

  picker.bounds.width = 0.0f;
  picker.bounds.height = 0.0f;
  CMP_TEST_OK(m3_time_picker_test_compute_metrics(&picker, &metrics));
  CMP_TEST_ASSERT(metrics.outer_radius == 0.0f);
  CMP_TEST_OK(m3_time_picker_test_pick_time(
      &picker, 0.0f, 0.0f, M3_TIME_PICKER_FIELD_HOUR, &picked, &valid));
  CMP_TEST_ASSERT(valid == CMP_FALSE);

  return 0;
}

static int test_init_and_setters(void) {
  M3TimePickerStyle style;
  M3TimePicker picker;
  CMPTime time;
  cmp_u32 format;
  cmp_u32 field;
  cmp_u32 period;
  ChangeCounter counter;
  CMPScalar prev_diameter;
  cmp_u32 prev_format;

  CMP_TEST_EXPECT(m3_time_picker_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  CMP_TEST_EXPECT(
      m3_time_picker_init(NULL, &style, 0u, 0u, M3_TIME_PICKER_FORMAT_12H),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_time_picker_init(&picker, NULL, 0u, 0u, M3_TIME_PICKER_FORMAT_12H),
      CMP_ERR_INVALID_ARGUMENT);
  style.diameter = 0.0f;
  CMP_TEST_EXPECT(
      m3_time_picker_init(&picker, &style, 0u, 0u, M3_TIME_PICKER_FORMAT_12H),
      CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  CMP_TEST_EXPECT(
      m3_time_picker_init(&picker, &style, 24u, 0u, M3_TIME_PICKER_FORMAT_12H),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      m3_time_picker_init(&picker, &style, 0u, 60u, M3_TIME_PICKER_FORMAT_12H),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_time_picker_init(&picker, &style, 0u, 0u, 99u),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_init(&picker, &style, 13u, 15u,
                                  M3_TIME_PICKER_FORMAT_24H));

  CMP_TEST_OK(m3_time_picker_get_format(&picker, &format));
  CMP_TEST_ASSERT(format == M3_TIME_PICKER_FORMAT_24H);
  CMP_TEST_OK(m3_time_picker_get_active_field(&picker, &field));
  CMP_TEST_ASSERT(field == M3_TIME_PICKER_FIELD_HOUR);
  CMP_TEST_OK(m3_time_picker_get_time(&picker, &time));
  CMP_TEST_ASSERT(time.hour == 13u);
  CMP_TEST_ASSERT(time.minute == 15u);

  CMP_TEST_EXPECT(m3_time_picker_get_format(NULL, &format),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_get_format(&picker, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_get_active_field(NULL, &field),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_get_active_field(&picker, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_get_time(NULL, &time),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_get_time(&picker, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(m3_time_picker_set_style(NULL, &style),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_set_style(&picker, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  style.diameter = 0.0f;
  CMP_TEST_EXPECT(m3_time_picker_set_style(&picker, &style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));

  picker.style = style;
  picker.style.diameter = 180.0f;
  prev_diameter = picker.style.diameter;
  picker.bounds.width = -1.0f;
  CMP_TEST_EXPECT(m3_time_picker_set_style(&picker, &style), CMP_ERR_RANGE);
  CMP_TEST_ASSERT(picker.style.diameter == prev_diameter);
  picker.bounds.width = 200.0f;
  CMP_TEST_OK(m3_time_picker_set_style(&picker, &style));

  CMP_TEST_EXPECT(m3_time_picker_set_format(NULL, M3_TIME_PICKER_FORMAT_12H),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_set_format(&picker, 99u), CMP_ERR_RANGE);

  prev_format = picker.format;
  picker.bounds.width = -1.0f;
  CMP_TEST_EXPECT(m3_time_picker_set_format(&picker, M3_TIME_PICKER_FORMAT_12H),
                  CMP_ERR_RANGE);
  CMP_TEST_ASSERT(picker.format == prev_format);
  picker.bounds.width = 200.0f;

  CMP_TEST_EXPECT(m3_time_picker_set_period(NULL, M3_TIME_PICKER_PERIOD_AM),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_set_period(&picker, M3_TIME_PICKER_PERIOD_AM),
                  CMP_ERR_STATE);
  picker.format = 99u;
  CMP_TEST_EXPECT(m3_time_picker_set_period(&picker, M3_TIME_PICKER_PERIOD_AM),
                  CMP_ERR_RANGE);
  picker.format = M3_TIME_PICKER_FORMAT_24H;

  CMP_TEST_OK(m3_time_picker_set_format(&picker, M3_TIME_PICKER_FORMAT_12H));

  CMP_TEST_EXPECT(
      m3_time_picker_set_active_field(NULL, M3_TIME_PICKER_FIELD_HOUR),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_set_active_field(&picker, 99u), CMP_ERR_RANGE);
  CMP_TEST_OK(
      m3_time_picker_set_active_field(&picker, M3_TIME_PICKER_FIELD_MINUTE));

  time.hour = 10u;
  time.minute = 5u;
  CMP_TEST_EXPECT(m3_time_picker_set_time(NULL, &time),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_set_time(&picker, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  time.hour = 30u;
  time.minute = 5u;
  CMP_TEST_EXPECT(m3_time_picker_set_time(&picker, &time), CMP_ERR_RANGE);
  time.hour = 10u;
  time.minute = 70u;
  CMP_TEST_EXPECT(m3_time_picker_set_time(&picker, &time), CMP_ERR_RANGE);
  time.hour = 10u;
  time.minute = 5u;
  CMP_TEST_OK(m3_time_picker_set_time(&picker, &time));

  CMP_TEST_EXPECT(m3_time_picker_set_on_change(NULL, test_on_change, &counter),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_time_picker_set_format(&picker, M3_TIME_PICKER_FORMAT_12H));
  CMP_TEST_EXPECT(m3_time_picker_set_period(&picker, 99u), CMP_ERR_RANGE);
  picker.time.hour = 13u;
  CMP_TEST_OK(m3_time_picker_set_period(&picker, M3_TIME_PICKER_PERIOD_AM));
  CMP_TEST_ASSERT(picker.time.hour == 1u);
  CMP_TEST_OK(m3_time_picker_set_period(&picker, M3_TIME_PICKER_PERIOD_PM));
  CMP_TEST_ASSERT(picker.time.hour == 13u);

  CMP_TEST_EXPECT(m3_time_picker_get_period(NULL, &period),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_time_picker_get_period(&picker, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_time_picker_get_period(&picker, &period));
  CMP_TEST_ASSERT(period == M3_TIME_PICKER_PERIOD_PM);
  picker.time.hour = 1u;
  CMP_TEST_OK(m3_time_picker_get_period(&picker, &period));
  CMP_TEST_ASSERT(period == M3_TIME_PICKER_PERIOD_AM);

  picker.time.hour = 13u;
  memset(&counter, 0, sizeof(counter));
  counter.fail = 1;
  CMP_TEST_OK(m3_time_picker_set_on_change(&picker, test_on_change, &counter));
  time.hour = 9u;
  time.minute = 45u;
  CMP_TEST_EXPECT(m3_time_picker_set_time(&picker, &time), CMP_ERR_IO);
  CMP_TEST_ASSERT(picker.time.hour == 13u);

  counter.fail = 0;
  CMP_TEST_OK(m3_time_picker_set_time(&picker, &time));
  CMP_TEST_ASSERT(counter.calls >= 1);
  CMP_TEST_ASSERT(picker.time.hour == 9u);

  CMP_TEST_EXPECT(m3_time_picker_update(NULL), CMP_ERR_INVALID_ARGUMENT);

  return 0;
}

static int test_widget_api(void) {
  M3TimePickerStyle style;
  M3TimePicker picker;
  CMPMeasureSpec spec;
  CMPMeasureSpec width_spec;
  CMPMeasureSpec height_spec;
  CMPSize size;
  CMPRect bounds;
  CMPSemantics semantics;
  int rc;

  CMP_TEST_OK(m3_time_picker_style_init(&style));
  CMP_TEST_OK(
      m3_time_picker_init(&picker, &style, 8u, 0u, M3_TIME_PICKER_FORMAT_24H));

  spec.mode = 99u;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(picker.widget.vtable->measure(NULL, spec, spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      picker.widget.vtable->measure(picker.widget.ctx, spec, spec, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      picker.widget.vtable->measure(picker.widget.ctx, spec, spec, &size),
      CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_MEASURE_EXACTLY;
  spec.size = 123.0f;
  CMP_TEST_OK(
      picker.widget.vtable->measure(picker.widget.ctx, spec, spec, &size));
  CMP_TEST_ASSERT(size.width == 123.0f);
  CMP_TEST_ASSERT(size.height == 123.0f);
  picker.style.diameter = 0.0f;
  CMP_TEST_EXPECT(
      picker.widget.vtable->measure(picker.widget.ctx, spec, spec, &size),
      CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  picker.style = style;

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 10.0f;
  height_spec.mode = CMP_MEASURE_EXACTLY;
  height_spec.size = 50.0f;
  CMP_TEST_OK(picker.widget.vtable->measure(picker.widget.ctx, width_spec,
                                            height_spec, &size));
  CMP_TEST_ASSERT(size.width == width_spec.size);
  CMP_TEST_ASSERT(size.height == height_spec.size);

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = 99u;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(picker.widget.vtable->measure(picker.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 40.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 10.0f;
  CMP_TEST_OK(picker.widget.vtable->measure(picker.widget.ctx, width_spec,
                                            height_spec, &size));
  CMP_TEST_ASSERT(size.width == width_spec.size);
  CMP_TEST_ASSERT(size.height == height_spec.size);

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 40.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_OK(picker.widget.vtable->measure(picker.widget.ctx, width_spec,
                                            height_spec, &size));
  CMP_TEST_ASSERT(size.width == width_spec.size);

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  CMP_TEST_EXPECT(picker.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  CMP_TEST_EXPECT(picker.widget.vtable->layout(picker.widget.ctx, bounds),
                  CMP_ERR_RANGE);
  bounds.width = 100.0f;
  bounds.height = 100.0f;
  CMP_TEST_OK(picker.widget.vtable->layout(picker.widget.ctx, bounds));

  CMP_TEST_EXPECT(picker.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(picker.widget.vtable->get_semantics(picker.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  rc = picker.widget.vtable->get_semantics(picker.widget.ctx, &semantics);
  CMP_TEST_OK(rc);
  CMP_TEST_ASSERT(semantics.role == CMP_SEMANTIC_NONE);

  picker.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  rc = picker.widget.vtable->get_semantics(picker.widget.ctx, &semantics);
  CMP_TEST_OK(rc);
  CMP_TEST_ASSERT(semantics.flags & CMP_SEMANTIC_FLAG_DISABLED);

  CMP_TEST_EXPECT(picker.widget.vtable->destroy(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(picker.widget.vtable->destroy(picker.widget.ctx));

  return 0;
}

static int test_event_and_paint(void) {
  M3TimePickerStyle style;
  M3TimePicker picker;
  M3TimePickerMetrics metrics;
  TestTimePickerBackend backend;
  CMPGfx gfx;
  CMPPaintContext ctx;
  CMPInputEvent event;
  CMPBool handled;
  ChangeCounter counter;
  CMPTime time;

  CMP_TEST_OK(m3_time_picker_style_init(&style));
  CMP_TEST_OK(
      m3_time_picker_init(&picker, &style, 6u, 0u, M3_TIME_PICKER_FORMAT_24H));

  picker.bounds.x = 0.0f;
  picker.bounds.y = 0.0f;
  picker.bounds.width = 200.0f;
  picker.bounds.height = 200.0f;
  CMP_TEST_OK(m3_time_picker_update(&picker));

  CMP_TEST_OK(m3_time_picker_test_compute_metrics(&picker, &metrics));

  CMP_TEST_OK(test_backend_init(&backend));
  gfx.ctx = &backend;
  gfx.vtable = &g_test_vtable;
  ctx.gfx = &gfx;
  ctx.clip.x = 0.0f;
  ctx.clip.y = 0.0f;
  ctx.clip.width = 200.0f;
  ctx.clip.height = 200.0f;
  ctx.dpi_scale = 1.0f;

  CMP_TEST_EXPECT(picker.widget.vtable->paint(NULL, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  ctx.gfx = NULL;
  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  ctx.gfx = &gfx;
  gfx.vtable = NULL;
  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_vtable_no_rect;
  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable_no_line;
  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                  CMP_ERR_UNSUPPORTED);

  gfx.vtable = &g_test_vtable;
  picker.style.diameter = 0.0f;
  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_time_picker_style_init(&style));
  picker.style = style;
  picker.bounds.width = -1.0f;
  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                  CMP_ERR_RANGE);
  picker.bounds.width = 200.0f;
  picker.time.minute = 99u;
  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                  CMP_ERR_RANGE);
  picker.time.minute = 0u;
  CMP_TEST_OK(m3_time_picker_test_set_force_resolve_colors_error(CMP_TRUE));
  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(picker.widget.vtable->paint(picker.widget.ctx, &ctx));
  CMP_TEST_ASSERT(backend.draw_rect_calls > 0);
  CMP_TEST_ASSERT(backend.draw_line_calls > 0);

  backend.draw_rect_calls = 0;
  backend.fail_draw_rect_after = 3;
  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                  CMP_ERR_IO);
  backend.fail_draw_rect_after = 0;

  picker.active_field = 99u;
  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                  CMP_ERR_RANGE);
  picker.active_field = M3_TIME_PICKER_FIELD_HOUR;

  backend.draw_rect_calls = 0;
  backend.fail_draw_rect_after = 5;
  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                  CMP_ERR_IO);
  backend.fail_draw_rect_after = 0;

  picker.time.hour = 0u;
  picker.active_field = M3_TIME_PICKER_FIELD_HOUR;
  CMP_TEST_OK(picker.widget.vtable->paint(picker.widget.ctx, &ctx));
  picker.active_field = M3_TIME_PICKER_FIELD_MINUTE;
  CMP_TEST_OK(picker.widget.vtable->paint(picker.widget.ctx, &ctx));
  picker.active_field = M3_TIME_PICKER_FIELD_HOUR;

  picker.bounds.width = 0.0f;
  picker.bounds.height = 0.0f;
  CMP_TEST_OK(picker.widget.vtable->layout(picker.widget.ctx, picker.bounds));
  CMP_TEST_OK(picker.widget.vtable->paint(picker.widget.ctx, &ctx));
  picker.bounds.width = 200.0f;
  picker.bounds.height = 200.0f;
  CMP_TEST_OK(picker.widget.vtable->layout(picker.widget.ctx, picker.bounds));

  backend.fail_draw_rect = CMP_ERR_IO;
  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                  CMP_ERR_IO);
  backend.fail_draw_rect = CMP_OK;

  backend.fail_draw_line = CMP_ERR_IO;
  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &ctx),
                  CMP_ERR_IO);
  backend.fail_draw_line = CMP_OK;

  picker.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(picker.widget.vtable->paint(picker.widget.ctx, &ctx));
  picker.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  picker.style.hand_center_radius = 0.0f;
  CMP_TEST_OK(picker.widget.vtable->paint(picker.widget.ctx, &ctx));
  picker.style.hand_center_radius = M3_TIME_PICKER_DEFAULT_HAND_CENTER_RADIUS;

  CMP_TEST_OK(test_backend_init(&backend));
  gfx.vtable = &g_test_vtable;
  gfx.ctx = &backend;
  ctx.gfx = &gfx;

  CMP_TEST_EXPECT(picker.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      picker.widget.vtable->event(picker.widget.ctx, NULL, &handled),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(picker.widget.vtable->event(picker.widget.ctx, &event, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_MOVE,
                                 (cmp_i32)metrics.center_x,
                                 (cmp_i32)metrics.center_y));
  CMP_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(init_pointer_event(
      &event, CMP_INPUT_POINTER_DOWN,
      (cmp_i32)(metrics.center_x + metrics.outer_radius + 10.0f),
      (cmp_i32)metrics.center_y));
  CMP_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  CMP_TEST_ASSERT(picker.pressed == CMP_FALSE);

  memset(&counter, 0, sizeof(counter));
  CMP_TEST_OK(m3_time_picker_set_on_change(&picker, test_on_change, &counter));
  CMP_TEST_OK(init_pointer_event(
      &event, CMP_INPUT_POINTER_DOWN, (cmp_i32)metrics.center_x,
      (cmp_i32)(metrics.center_y - metrics.outer_radius)));
  CMP_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(picker.pressed == CMP_TRUE);
  CMP_TEST_ASSERT(counter.calls == 1);
  CMP_TEST_ASSERT(picker.time.hour == 12u);

  CMP_TEST_EXPECT(
      picker.widget.vtable->event(picker.widget.ctx, &event, &handled),
      CMP_ERR_STATE);

  CMP_TEST_OK(
      init_pointer_event(&event, CMP_INPUT_POINTER_MOVE,
                         (cmp_i32)(metrics.center_x + metrics.outer_radius),
                         (cmp_i32)metrics.center_y));
  CMP_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(picker.time.hour == 3u);

  CMP_TEST_OK(init_pointer_event(
      &event, CMP_INPUT_POINTER_MOVE,
      (cmp_i32)(metrics.center_x + metrics.outer_radius + 20.0f),
      (cmp_i32)metrics.center_y));
  CMP_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP,
                                 (cmp_i32)metrics.center_x,
                                 (cmp_i32)metrics.center_y));
  CMP_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(picker.pressed == CMP_FALSE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP,
                                 (cmp_i32)metrics.center_x,
                                 (cmp_i32)metrics.center_y));
  CMP_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_MOVE,
                                 (cmp_i32)metrics.center_x,
                                 (cmp_i32)metrics.center_y));
  CMP_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  picker.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(init_pointer_event(
      &event, CMP_INPUT_POINTER_DOWN, (cmp_i32)metrics.center_x,
      (cmp_i32)(metrics.center_y - metrics.outer_radius)));
  CMP_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  picker.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  counter.fail = 1;
  time = picker.time;
  CMP_TEST_OK(init_pointer_event(
      &event, CMP_INPUT_POINTER_DOWN, (cmp_i32)metrics.center_x,
      (cmp_i32)(metrics.center_y - metrics.outer_radius)));
  CMP_TEST_EXPECT(
      picker.widget.vtable->event(picker.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_ASSERT(picker.time.hour == time.hour);
  counter.fail = 0;

  picker.format = 99u;
  CMP_TEST_OK(init_pointer_event(
      &event, CMP_INPUT_POINTER_DOWN, (cmp_i32)metrics.center_x,
      (cmp_i32)(metrics.center_y - metrics.outer_radius)));
  CMP_TEST_EXPECT(
      picker.widget.vtable->event(picker.widget.ctx, &event, &handled),
      CMP_ERR_RANGE);
  picker.format = M3_TIME_PICKER_FORMAT_24H;

  picker.format = 99u;
  picker.pressed = CMP_TRUE;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_MOVE,
                                 (cmp_i32)metrics.center_x,
                                 (cmp_i32)metrics.center_y));
  CMP_TEST_EXPECT(
      picker.widget.vtable->event(picker.widget.ctx, &event, &handled),
      CMP_ERR_RANGE);
  picker.pressed = CMP_FALSE;
  picker.format = M3_TIME_PICKER_FORMAT_24H;

  picker.pressed = CMP_TRUE;
  counter.fail = 1;
  CMP_TEST_OK(init_pointer_event(
      &event, CMP_INPUT_POINTER_MOVE, (cmp_i32)metrics.center_x,
      (cmp_i32)(metrics.center_y - metrics.outer_radius)));
  CMP_TEST_EXPECT(
      picker.widget.vtable->event(picker.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  counter.fail = 0;
  picker.pressed = CMP_FALSE;

  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_KEY_DOWN;
  handled = CMP_TRUE;
  CMP_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(m3_time_picker_set_format(&picker, M3_TIME_PICKER_FORMAT_12H));
  picker.time.hour = 0u;
  picker.active_field = M3_TIME_PICKER_FIELD_MINUTE;
  CMP_TEST_OK(
      init_pointer_event(&event, CMP_INPUT_POINTER_DOWN,
                         (cmp_i32)(metrics.center_x + metrics.outer_radius),
                         (cmp_i32)metrics.center_y));
  CMP_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(picker.time.minute == 15u);

  CMP_TEST_EXPECT(m3_time_picker_test_draw_circle(
                      NULL, 0.0f, 0.0f, 1.0f, picker.style.background_color),
                  CMP_ERR_UNSUPPORTED);
  CMP_TEST_EXPECT(m3_time_picker_test_draw_circle(
                      &gfx, 0.0f, 0.0f, -1.0f, picker.style.background_color),
                  CMP_ERR_RANGE);
  backend.draw_rect_calls = 0;
  CMP_TEST_OK(m3_time_picker_test_draw_circle(&gfx, 0.0f, 0.0f, 0.0f,
                                              picker.style.background_color));
  CMP_TEST_ASSERT(backend.draw_rect_calls == 0);
  CMP_TEST_OK(m3_time_picker_test_set_force_rect_error(CMP_TRUE));
  CMP_TEST_EXPECT(m3_time_picker_test_draw_circle(
                      &gfx, 0.0f, 0.0f, 1.0f, picker.style.background_color),
                  CMP_ERR_RANGE);

  CMP_TEST_EXPECT(m3_time_picker_test_draw_ring(&gfx, 0.0f, 0.0f, 1.0f, 0.0f,
                                                picker.style.ring_color,
                                                picker.style.background_color),
                  CMP_ERR_RANGE);
  backend.draw_rect_calls = 0;
  CMP_TEST_OK(m3_time_picker_test_draw_ring(&gfx, 0.0f, 0.0f, 0.0f, 1.0f,
                                            picker.style.ring_color,
                                            picker.style.background_color));
  CMP_TEST_ASSERT(backend.draw_rect_calls == 0);
  backend.draw_rect_calls = 0;
  CMP_TEST_OK(m3_time_picker_test_draw_ring(&gfx, 0.0f, 0.0f, 1.0f, 5.0f,
                                            picker.style.ring_color,
                                            picker.style.background_color));
  CMP_TEST_ASSERT(backend.draw_rect_calls == 1);

  picker.bounds.width = 6.0f;
  picker.bounds.height = 6.0f;
  picker.style.diameter = 100.0f;
  picker.style.ring_thickness = 4.0f;
  picker.style.padding.left = 0.0f;
  picker.style.padding.right = 0.0f;
  picker.style.padding.top = 0.0f;
  picker.style.padding.bottom = 0.0f;
  CMP_TEST_OK(picker.widget.vtable->layout(picker.widget.ctx, picker.bounds));
  CMP_TEST_OK(picker.widget.vtable->paint(picker.widget.ctx, &ctx));

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
