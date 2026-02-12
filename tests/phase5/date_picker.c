#include "m3/m3_date_picker.h"
#include "test_utils.h"

#include <string.h>

typedef struct TestDatePickerBackend {
  int draw_rect_calls;
  int fail_draw_rect;
  int fail_draw_rect_after;
  M3Rect last_rect;
  M3Color last_color;
  M3Scalar last_corner;
} TestDatePickerBackend;

typedef struct ChangeCounter {
  int calls;
  int fail;
  M3DateRange last;
} ChangeCounter;

static int test_backend_init(TestDatePickerBackend *backend) {
  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(backend, 0, sizeof(*backend));
  backend->fail_draw_rect = M3_OK;
  backend->fail_draw_rect_after = 0;
  return M3_OK;
}

static int test_gfx_draw_rect(void *gfx, const M3Rect *rect, M3Color color,
                              M3Scalar corner_radius) {
  TestDatePickerBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestDatePickerBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_color = color;
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

static const M3GfxVTable g_test_vtable = {
    NULL, NULL, NULL, test_gfx_draw_rect, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL};

static const M3GfxVTable g_test_vtable_no_rect = {NULL, NULL, NULL, NULL, NULL,
                                                  NULL, NULL, NULL, NULL, NULL,
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

static int test_on_change(void *ctx, M3DatePicker *picker,
                          const M3DateRange *range) {
  ChangeCounter *counter;

  if (ctx == NULL || picker == NULL || range == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  counter = (ChangeCounter *)ctx;
  counter->calls += 1;
  counter->last = *range;
  if (counter->fail) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_helpers(void) {
  M3DatePickerStyle style;
  M3LayoutEdges edges;
  M3Color color;
  M3Rect rect;
  M3MeasureSpec spec;
  M3Date date;
  M3Bool leap;
  M3Bool in_range;
  m3_u32 days;
  m3_u32 weekday;
  m3_i32 cmp;
  m3_u32 offset;
  m3_i32 year;
  m3_u32 month;

  M3_TEST_EXPECT(m3_date_picker_test_validate_color(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  color.r = 1.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_color(&color), M3_ERR_RANGE);
  color.r = 0.0f;
  color.g = -0.1f;
  color.b = 0.0f;
  color.a = 0.0f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_color(&color), M3_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.1f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_color(&color), M3_ERR_RANGE);
  color.b = 0.0f;
  color.a = -0.1f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_color(&color), M3_ERR_RANGE);

  M3_TEST_EXPECT(m3_date_picker_test_validate_edges(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_test_validate_style(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_test_validate_rect(NULL),
                 M3_ERR_INVALID_ARGUMENT);

  edges.left = -1.0f;
  edges.right = 0.0f;
  edges.top = 0.0f;
  edges.bottom = 0.0f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_edges(&edges), M3_ERR_RANGE);

  M3_TEST_OK(m3_date_picker_style_init(&style));
  M3_TEST_OK(m3_date_picker_test_validate_style(&style));
  style.cell_width = 0.0f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_date_picker_style_init(&style));
  style.cell_spacing_x = -1.0f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_date_picker_style_init(&style));
  style.cell_corner_radius = style.cell_width;
  M3_TEST_EXPECT(m3_date_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_date_picker_style_init(&style));
  style.header_height = -1.0f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_date_picker_style_init(&style));
  style.weekday_height = -1.0f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_date_picker_style_init(&style));
  style.cell_corner_radius = -1.0f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_date_picker_style_init(&style));
  style.padding.left = -1.0f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_date_picker_style_init(&style));
  style.background_color.r = -1.0f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_date_picker_style_init(&style));
  style.cell_color.r = -1.0f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_date_picker_style_init(&style));
  style.cell_range_color.r = -1.0f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_date_picker_style_init(&style));
  style.cell_selected_color.r = -1.0f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_date_picker_style_init(&style));
  style.cell_disabled_color.r = -1.0f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_style(&style), M3_ERR_RANGE);

  spec.mode = 99u;
  spec.size = 10.0f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_measure_spec(spec),
                 M3_ERR_INVALID_ARGUMENT);
  spec.mode = M3_MEASURE_EXACTLY;
  spec.size = -1.0f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_measure_spec(spec), M3_ERR_RANGE);

  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 0.0f;
  M3_TEST_EXPECT(m3_date_picker_test_validate_rect(&rect), M3_ERR_RANGE);

  date.year = -1;
  date.month = 1u;
  date.day = 1u;
  M3_TEST_EXPECT(m3_date_picker_test_validate_date(&date), M3_ERR_RANGE);
  date.year = 2024;
  date.month = 13u;
  M3_TEST_EXPECT(m3_date_picker_test_validate_date(&date), M3_ERR_RANGE);
  date.year = 2024;
  date.month = 2u;
  date.day = 30u;
  M3_TEST_EXPECT(m3_date_picker_test_validate_date(&date), M3_ERR_RANGE);
  date.day = 29u;
  M3_TEST_OK(m3_date_picker_test_validate_date(&date));

  M3_TEST_EXPECT(m3_date_picker_test_is_leap_year(2024, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_test_is_leap_year(-1, &leap), M3_ERR_RANGE);
  M3_TEST_OK(m3_date_picker_test_is_leap_year(2000, &leap));
  M3_TEST_ASSERT(leap == M3_TRUE);
  M3_TEST_OK(m3_date_picker_test_is_leap_year(1900, &leap));
  M3_TEST_ASSERT(leap == M3_FALSE);

  M3_TEST_EXPECT(m3_date_picker_test_days_in_month(2024, 1u, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_test_days_in_month(-1, 1u, &days),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_date_picker_test_days_in_month(2024, 13u, &days),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_date_picker_test_days_in_month(2024, 2u, &days));
  M3_TEST_ASSERT(days == 29u);
  M3_TEST_OK(m3_date_picker_test_days_in_month(2023, 2u, &days));
  M3_TEST_ASSERT(days == 28u);
  M3_TEST_OK(m3_date_picker_test_days_in_month(2024, 1u, &days));
  M3_TEST_ASSERT(days == 31u);
  M3_TEST_OK(m3_date_picker_test_days_in_month(2024, 4u, &days));
  M3_TEST_ASSERT(days == 30u);

  date.year = 2000;
  date.month = 1u;
  date.day = 1u;
  M3_TEST_OK(m3_date_picker_test_day_of_week(&date, &weekday));
  M3_TEST_ASSERT(weekday == M3_DATE_PICKER_WEEKDAY_SATURDAY);
  M3_TEST_EXPECT(m3_date_picker_test_day_of_week(NULL, &weekday),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_test_day_of_week(&date, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  {
    M3Date a;
    M3Date b;

    a.year = 2024;
    a.month = 5u;
    a.day = 10u;
    b = a;
    M3_TEST_OK(m3_date_picker_test_compare_dates(&a, &b, &cmp));
    M3_TEST_ASSERT(cmp == 0);

    b.day = 11u;
    M3_TEST_OK(m3_date_picker_test_compare_dates(&a, &b, &cmp));
    M3_TEST_ASSERT(cmp < 0);

    M3_TEST_OK(m3_date_picker_test_compare_dates(&b, &a, &cmp));
    M3_TEST_ASSERT(cmp > 0);

    b = a;
    b.year = 2025;
    M3_TEST_OK(m3_date_picker_test_compare_dates(&a, &b, &cmp));
    M3_TEST_ASSERT(cmp < 0);
    M3_TEST_OK(m3_date_picker_test_compare_dates(&b, &a, &cmp));
    M3_TEST_ASSERT(cmp > 0);

    b = a;
    b.month = 6u;
    M3_TEST_OK(m3_date_picker_test_compare_dates(&a, &b, &cmp));
    M3_TEST_ASSERT(cmp < 0);
    M3_TEST_OK(m3_date_picker_test_compare_dates(&b, &a, &cmp));
    M3_TEST_ASSERT(cmp > 0);
  }
  M3_TEST_EXPECT(m3_date_picker_test_compare_dates(NULL, &date, &cmp),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_test_compare_dates(&date, NULL, &cmp),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_test_compare_dates(&date, &date, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  {
    M3DateRange range;

    range.start.year = 2024;
    range.start.month = 1u;
    range.start.day = 1u;
    range.end.year = 2024;
    range.end.month = 12u;
    range.end.day = 31u;
    range.has_start = M3_TRUE;
    range.has_end = M3_TRUE;

    date.year = 2024;
    date.month = 6u;
    date.day = 1u;
    M3_TEST_OK(m3_date_picker_test_date_in_range(&date, &range, &in_range));
    M3_TEST_ASSERT(in_range == M3_TRUE);

    date.year = 2025;
    M3_TEST_OK(m3_date_picker_test_date_in_range(&date, &range, &in_range));
    M3_TEST_ASSERT(in_range == M3_FALSE);

    M3_TEST_EXPECT(m3_date_picker_test_date_in_range(NULL, &range, &in_range),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_date_picker_test_date_in_range(&date, NULL, &in_range),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_date_picker_test_date_in_range(&date, &range, NULL),
                   M3_ERR_INVALID_ARGUMENT);

    range.start.year = 2024;
    range.start.month = 12u;
    range.start.day = 31u;
    range.end.year = 2024;
    range.end.month = 1u;
    range.end.day = 1u;
    M3_TEST_EXPECT(m3_date_picker_test_date_in_range(&date, &range, &in_range),
                   M3_ERR_RANGE);
  }

  M3_TEST_EXPECT(m3_date_picker_test_compute_offset(7u, 0u, &offset),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_date_picker_test_compute_offset(0u, 7u, &offset),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_date_picker_test_compute_offset(0u, 0u, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_date_picker_test_compute_offset(0u, 0u, &offset));
  M3_TEST_ASSERT(offset == 0u);

  M3_TEST_EXPECT(m3_date_picker_test_shift_month(2024, 1u, 1, NULL, &month),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_test_shift_month(2024, 1u, 1, &year, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_test_shift_month(-1, 1u, 1, &year, &month),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_date_picker_test_shift_month(2024, 13u, 1, &year, &month),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_date_picker_test_shift_month(2024, 0u, 1, &year, &month),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_date_picker_test_shift_month(M3_DATE_MAX_YEAR + 1, 1u, 0,
                                                 &year, &month),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_date_picker_test_shift_month(2024, 1u, 1, &year, &month));
  M3_TEST_ASSERT(year == 2024 && month == 2u);
  M3_TEST_OK(m3_date_picker_test_shift_month(2024, 1u, -1, &year, &month));
  M3_TEST_ASSERT(year == 2023 && month == 12u);
  M3_TEST_EXPECT(
      m3_date_picker_test_shift_month(M3_DATE_MIN_YEAR, 1u, -1, &year, &month),
      M3_ERR_RANGE);
  M3_TEST_EXPECT(
      m3_date_picker_test_shift_month(M3_DATE_MAX_YEAR, 12u, 1, &year, &month),
      M3_ERR_RANGE);

  return 0;
}

static int test_init_and_display(void) {
  M3DatePickerStyle style;
  M3DatePicker picker;
  m3_i32 year;
  m3_u32 month;

  M3_TEST_OK(m3_date_picker_style_init(&style));
  M3_TEST_EXPECT(m3_date_picker_init(NULL, &style, 2024, 1u),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_init(&picker, NULL, 2024, 1u),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_init(&picker, &style, 0, 1u), M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_date_picker_init(&picker, &style, 2024, 13u), M3_ERR_RANGE);

  M3_TEST_OK(m3_date_picker_init(&picker, &style, 2024, 12u));
  M3_TEST_ASSERT(picker.widget.ctx == &picker);
  M3_TEST_ASSERT(picker.widget.vtable != NULL);

  M3_TEST_EXPECT(m3_date_picker_set_style(NULL, &style),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_set_style(&picker, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  {
    M3DatePickerStyle bad_style;

    bad_style = style;
    bad_style.cell_width = 0.0f;
    M3_TEST_EXPECT(m3_date_picker_set_style(&picker, &bad_style), M3_ERR_RANGE);
  }

  M3_TEST_EXPECT(m3_date_picker_get_display_month(NULL, &year, &month),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_get_display_month(&picker, NULL, &month),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_get_display_month(&picker, &year, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_date_picker_set_week_start(NULL, 0u),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_set_week_start(&picker, 7u), M3_ERR_RANGE);
  M3_TEST_OK(m3_date_picker_set_week_start(&picker, 0u));

  M3_TEST_OK(m3_date_picker_next_month(&picker));
  M3_TEST_OK(m3_date_picker_get_display_month(&picker, &year, &month));
  M3_TEST_ASSERT(year == 2025 && month == 1u);

  M3_TEST_OK(m3_date_picker_prev_month(&picker));
  M3_TEST_OK(m3_date_picker_get_display_month(&picker, &year, &month));
  M3_TEST_ASSERT(year == 2024 && month == 12u);

  M3_TEST_EXPECT(m3_date_picker_set_display_month(&picker, 2024, 0u),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_date_picker_set_display_month(&picker, 0, 1u),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_date_picker_set_display_month(&picker, -1, 1u),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_date_picker_set_display_month(NULL, 2024, 1u),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(
      m3_date_picker_set_display_month(&picker, M3_DATE_MAX_YEAR - 1, 12u));
  M3_TEST_OK(m3_date_picker_next_month(&picker));
  M3_TEST_OK(m3_date_picker_get_display_month(&picker, &year, &month));
  M3_TEST_ASSERT(year == M3_DATE_MAX_YEAR && month == 1u);
  M3_TEST_EXPECT(
      m3_date_picker_set_display_month(&picker, M3_DATE_MAX_YEAR, 12u),
      M3_ERR_RANGE);

  M3_TEST_EXPECT(
      m3_date_picker_set_display_month(&picker, M3_DATE_MIN_YEAR, 1u),
      M3_ERR_RANGE);
  M3_TEST_OK(
      m3_date_picker_set_display_month(&picker, M3_DATE_MIN_YEAR + 1, 1u));
  M3_TEST_OK(m3_date_picker_prev_month(&picker));
  M3_TEST_EXPECT(m3_date_picker_next_month(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_prev_month(NULL), M3_ERR_INVALID_ARGUMENT);

  return 0;
}

static int test_constraints_and_selection(void) {
  M3DatePickerStyle style;
  M3DatePicker picker;
  M3DateRange constraints;
  M3DateRange selection;
  M3DateRange out_range;
  ChangeCounter counter;

  M3_TEST_OK(m3_date_picker_style_init(&style));
  M3_TEST_OK(m3_date_picker_init(&picker, &style, 2024, 6u));

  M3_TEST_EXPECT(m3_date_picker_set_constraints(NULL, &constraints),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_set_constraints(&picker, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  constraints.start.year = 2024;
  constraints.start.month = 1u;
  constraints.start.day = 1u;
  constraints.end.year = 2024;
  constraints.end.month = 12u;
  constraints.end.day = 31u;
  constraints.has_start = M3_TRUE;
  constraints.has_end = M3_TRUE;
  M3_TEST_OK(m3_date_picker_set_constraints(&picker, &constraints));

  M3_TEST_EXPECT(m3_date_picker_get_constraints(NULL, &out_range),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_get_constraints(&picker, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  constraints.has_start = 2;
  M3_TEST_EXPECT(m3_date_picker_set_constraints(&picker, &constraints),
                 M3_ERR_RANGE);
  constraints.has_start = M3_TRUE;
  constraints.has_end = M3_TRUE;

  selection.start.year = 2024;
  selection.start.month = 5u;
  selection.start.day = 10u;
  selection.end = selection.start;
  selection.has_start = M3_TRUE;
  selection.has_end = M3_FALSE;

  counter.calls = 0;
  counter.fail = 0;
  M3_TEST_EXPECT(m3_date_picker_set_on_change(NULL, test_on_change, &counter),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_date_picker_set_on_change(&picker, test_on_change, &counter));
  M3_TEST_EXPECT(m3_date_picker_set_selection(NULL, &selection),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_set_selection(&picker, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_date_picker_set_selection(&picker, &selection));
  M3_TEST_ASSERT(counter.calls == 1);

  picker.style.cell_width = 0.0f;
  M3_TEST_EXPECT(m3_date_picker_set_selection(&picker, &selection),
                 M3_ERR_RANGE);
  picker.style.cell_width = style.cell_width;

  M3_TEST_EXPECT(m3_date_picker_get_selection(NULL, &out_range),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_get_selection(&picker, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  counter.fail = 1;
  M3_TEST_EXPECT(m3_date_picker_set_selection(&picker, &selection), M3_ERR_IO);
  counter.fail = 0;

  selection.end.year = 2024;
  selection.end.month = 5u;
  selection.end.day = 11u;
  selection.has_end = M3_TRUE;
  M3_TEST_EXPECT(m3_date_picker_set_selection(&picker, &selection),
                 M3_ERR_RANGE);
  selection.has_end = M3_FALSE;

  selection.start.year = 2025;
  M3_TEST_EXPECT(m3_date_picker_set_selection(&picker, &selection),
                 M3_ERR_RANGE);

  selection.has_start = M3_FALSE;
  selection.has_end = M3_TRUE;
  M3_TEST_EXPECT(m3_date_picker_set_selection(&picker, &selection),
                 M3_ERR_RANGE);
  selection.has_start = M3_TRUE;
  selection.has_end = M3_FALSE;

  selection.start.year = 2024;
  selection.end.year = 2024;
  selection.end.month = 4u;
  selection.end.day = 1u;
  selection.has_end = M3_TRUE;
  M3_TEST_OK(m3_date_picker_set_mode(&picker, M3_DATE_PICKER_MODE_RANGE));
  M3_TEST_EXPECT(m3_date_picker_set_selection(&picker, &selection),
                 M3_ERR_RANGE);

  selection.end.year = 2024;
  selection.end.month = 6u;
  selection.end.day = 1u;
  M3_TEST_OK(m3_date_picker_set_selection(&picker, &selection));

  M3_TEST_OK(m3_date_picker_get_selection(&picker, &out_range));
  M3_TEST_ASSERT(out_range.has_start == M3_TRUE);
  M3_TEST_ASSERT(out_range.has_end == M3_TRUE);

  constraints.end.year = 2024;
  constraints.end.month = 5u;
  constraints.end.day = 1u;
  M3_TEST_EXPECT(m3_date_picker_set_constraints(&picker, &constraints),
                 M3_ERR_RANGE);

  M3_TEST_EXPECT(m3_date_picker_set_mode(&picker, M3_DATE_PICKER_MODE_SINGLE),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_date_picker_set_mode(NULL, M3_DATE_PICKER_MODE_SINGLE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_set_mode(&picker, 99u), M3_ERR_RANGE);

  return 0;
}

static int find_cell(const M3DatePickerCell *cells, m3_usize count, m3_i32 year,
                     m3_u32 month, m3_u32 day, m3_usize *out_index) {
  m3_usize i;

  if (cells == NULL || out_index == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < count; ++i) {
    if (cells[i].date.year == year && cells[i].date.month == month &&
        cells[i].date.day == day) {
      *out_index = i;
      return M3_OK;
    }
  }

  return M3_ERR_NOT_FOUND;
}

static int test_grid_and_cells(void) {
  M3DatePickerStyle style;
  M3DatePicker picker;
  M3DateRange constraints;
  M3DateRange selection;
  const M3DatePickerCell *cells;
  m3_usize count;
  m3_usize index;

  M3_TEST_OK(m3_date_picker_style_init(&style));
  M3_TEST_OK(m3_date_picker_init(&picker, &style, 2024, 2u));

  constraints.start.year = 2024;
  constraints.start.month = 2u;
  constraints.start.day = 10u;
  constraints.end.year = 2024;
  constraints.end.month = 2u;
  constraints.end.day = 20u;
  constraints.has_start = M3_TRUE;
  constraints.has_end = M3_TRUE;
  M3_TEST_OK(m3_date_picker_set_constraints(&picker, &constraints));

  selection.start.year = 2024;
  selection.start.month = 2u;
  selection.start.day = 14u;
  selection.end = selection.start;
  selection.has_start = M3_TRUE;
  selection.has_end = M3_FALSE;
  M3_TEST_OK(m3_date_picker_set_selection(&picker, &selection));

  M3_TEST_EXPECT(m3_date_picker_get_cells(NULL, &cells, &count),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_get_cells(&picker, NULL, &count),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_get_cells(&picker, &cells, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_date_picker_get_cells(&picker, &cells, &count));
  M3_TEST_ASSERT(count == M3_DATE_PICKER_GRID_COUNT);

  M3_TEST_OK(find_cell(cells, count, 2024, 2u, 14u, &index));
  M3_TEST_ASSERT(cells[index].flags & M3_DATE_CELL_FLAG_SELECTED);

  M3_TEST_OK(find_cell(cells, count, 2024, 2u, 5u, &index));
  M3_TEST_ASSERT(cells[index].flags & M3_DATE_CELL_FLAG_OUT_OF_RANGE);

  M3_TEST_EXPECT(m3_date_picker_get_cell(NULL, 0, &picker.cells[0]),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_date_picker_get_cell(&picker, M3_DATE_PICKER_GRID_COUNT, NULL),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_get_cell(&picker, M3_DATE_PICKER_GRID_COUNT,
                                         &picker.cells[0]),
                 M3_ERR_RANGE);

  return 0;
}

static int test_update_grid_errors(void) {
  M3DatePickerStyle style;
  M3DatePicker picker;

  M3_TEST_EXPECT(m3_date_picker_update(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_test_update_grid(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_date_picker_style_init(&style));
  M3_TEST_OK(m3_date_picker_init(&picker, &style, 2024, 3u));

  picker.style.cell_width = 0.0f;
  M3_TEST_EXPECT(m3_date_picker_test_update_grid(&picker), M3_ERR_RANGE);
  picker.style.cell_width = style.cell_width;

  picker.bounds.width = -1.0f;
  M3_TEST_EXPECT(m3_date_picker_test_update_grid(&picker), M3_ERR_RANGE);
  picker.bounds.width = 0.0f;

  picker.week_start = 7u;
  M3_TEST_EXPECT(m3_date_picker_test_update_grid(&picker), M3_ERR_RANGE);
  picker.week_start = M3_DATE_PICKER_WEEK_START_SUNDAY;

  picker.constraints.has_start = 2;
  M3_TEST_EXPECT(m3_date_picker_test_update_grid(&picker), M3_ERR_RANGE);
  picker.constraints.has_start = M3_FALSE;

  picker.selection.has_start = M3_FALSE;
  picker.selection.has_end = M3_TRUE;
  M3_TEST_EXPECT(m3_date_picker_test_update_grid(&picker), M3_ERR_RANGE);

  picker.mode = 99u;
  M3_TEST_EXPECT(m3_date_picker_test_update_grid(&picker), M3_ERR_RANGE);

  return 0;
}

static int test_widget_paths(void) {
  TestDatePickerBackend backend;
  M3Gfx gfx;
  M3PaintContext paint_ctx;
  M3DatePickerStyle style;
  M3DatePicker picker;
  M3MeasureSpec width_spec;
  M3MeasureSpec height_spec;
  M3Size size;
  M3Rect bounds;
  M3Semantics semantics;
  M3InputEvent event;
  M3Bool handled;
  m3_usize index;
  M3DateRange constraints;
  M3DateRange selection;
  M3Scalar hit_x;
  M3Scalar hit_y;

  M3_TEST_OK(test_backend_init(&backend));
  gfx.ctx = &backend;
  gfx.vtable = &g_test_vtable;
  gfx.text_vtable = NULL;

  paint_ctx.gfx = &gfx;
  paint_ctx.dpi_scale = 1.0f;
  paint_ctx.clip.x = 0.0f;
  paint_ctx.clip.y = 0.0f;
  paint_ctx.clip.width = 400.0f;
  paint_ctx.clip.height = 400.0f;

  M3_TEST_OK(m3_date_picker_style_init(&style));
  M3_TEST_OK(m3_date_picker_init(&picker, &style, 2024, 2u));

  width_spec.mode = M3_MEASURE_EXACTLY;
  width_spec.size = 300.0f;
  height_spec.mode = M3_MEASURE_EXACTLY;
  height_spec.size = 300.0f;

  M3_TEST_EXPECT(
      picker.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(picker.widget.vtable->measure(picker.widget.ctx, width_spec,
                                               height_spec, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  width_spec.mode = 99u;
  M3_TEST_EXPECT(picker.widget.vtable->measure(picker.widget.ctx, width_spec,
                                               height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);
  width_spec.mode = M3_MEASURE_EXACTLY;
  height_spec.size = -1.0f;
  M3_TEST_EXPECT(picker.widget.vtable->measure(picker.widget.ctx, width_spec,
                                               height_spec, &size),
                 M3_ERR_RANGE);
  height_spec.size = 300.0f;

  M3_TEST_OK(picker.widget.vtable->measure(picker.widget.ctx, width_spec,
                                           height_spec, &size));
  M3_TEST_ASSERT(size.width == 300.0f && size.height == 300.0f);

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 300.0f;
  bounds.height = 300.0f;
  M3_TEST_EXPECT(picker.widget.vtable->layout(NULL, bounds),
                 M3_ERR_INVALID_ARGUMENT);
  bounds.width = -1.0f;
  M3_TEST_EXPECT(picker.widget.vtable->layout(picker.widget.ctx, bounds),
                 M3_ERR_RANGE);
  bounds.width = 300.0f;
  M3_TEST_OK(picker.widget.vtable->layout(picker.widget.ctx, bounds));

  M3_TEST_EXPECT(picker.widget.vtable->get_semantics(NULL, &semantics),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(picker.widget.vtable->get_semantics(picker.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(
      picker.widget.vtable->get_semantics(picker.widget.ctx, &semantics));
  M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_NONE);

  M3_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(picker.widget.vtable->paint(NULL, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = NULL;
  M3_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = &gfx;
  gfx.vtable = NULL;
  M3_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_vtable_no_rect;
  M3_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable;

  backend.fail_draw_rect_after = 1;
  M3_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend.fail_draw_rect_after = 0;
  backend.fail_draw_rect = M3_OK;
  M3_TEST_OK(picker.widget.vtable->paint(picker.widget.ctx, &paint_ctx));

  constraints.start.year = 2024;
  constraints.start.month = 2u;
  constraints.start.day = 5u;
  constraints.end.year = 2024;
  constraints.end.month = 2u;
  constraints.end.day = 25u;
  constraints.has_start = M3_TRUE;
  constraints.has_end = M3_TRUE;
  M3_TEST_OK(m3_date_picker_set_constraints(&picker, &constraints));
  M3_TEST_OK(m3_date_picker_set_mode(&picker, M3_DATE_PICKER_MODE_RANGE));
  selection.start.year = 2024;
  selection.start.month = 2u;
  selection.start.day = 10u;
  selection.end.year = 2024;
  selection.end.month = 2u;
  selection.end.day = 12u;
  selection.has_start = M3_TRUE;
  selection.has_end = M3_TRUE;
  M3_TEST_OK(m3_date_picker_set_selection(&picker, &selection));
  M3_TEST_OK(picker.widget.vtable->paint(picker.widget.ctx, &paint_ctx));

  M3_TEST_EXPECT(picker.widget.vtable->event(picker.widget.ctx, NULL, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(picker.widget.vtable->event(NULL, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(picker.widget.vtable->event(picker.widget.ctx, &event, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_date_picker_test_compute_cell_bounds(&picker, 0u, 0u, &bounds));
  hit_x = bounds.x + 1.0f;
  hit_y = bounds.y + 1.0f;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, (m3_i32)hit_x,
                                (m3_i32)hit_y));
  M3_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_EXPECT(
      picker.widget.vtable->event(picker.widget.ctx, &event, &handled),
      M3_ERR_STATE);
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, (m3_i32)hit_x,
                                (m3_i32)hit_y));
  M3_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, -10, -10));
  M3_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  picker.mode = 99u;
  index = 0u;
  while (index < M3_DATE_PICKER_GRID_COUNT) {
    if ((picker.cells[index].flags & M3_DATE_CELL_FLAG_OUT_OF_RANGE) == 0u) {
      break;
    }
    index += 1u;
  }
  M3_TEST_ASSERT(index < M3_DATE_PICKER_GRID_COUNT);
  hit_x = picker.cells[index].bounds.x + 1.0f;
  hit_y = picker.cells[index].bounds.y + 1.0f;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, (m3_i32)hit_x,
                                (m3_i32)hit_y));
  M3_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, (m3_i32)hit_x,
                                (m3_i32)hit_y));
  M3_TEST_EXPECT(
      picker.widget.vtable->event(picker.widget.ctx, &event, &handled),
      M3_ERR_RANGE);
  picker.mode = M3_DATE_PICKER_MODE_SINGLE;
  picker.selection.has_end = M3_FALSE;

  selection.has_start = M3_FALSE;
  selection.has_end = M3_FALSE;
  M3_TEST_OK(m3_date_picker_set_selection(&picker, &selection));

  constraints.start.year = 2024;
  constraints.start.month = 2u;
  constraints.start.day = 15u;
  constraints.end = constraints.start;
  constraints.has_start = M3_TRUE;
  constraints.has_end = M3_TRUE;
  M3_TEST_OK(m3_date_picker_set_constraints(&picker, &constraints));
  M3_TEST_OK(m3_date_picker_update(&picker));
  index = 0u;
  while (index < M3_DATE_PICKER_GRID_COUNT) {
    if (picker.cells[index].flags & M3_DATE_CELL_FLAG_OUT_OF_RANGE) {
      M3Scalar hit_x;
      M3Scalar hit_y;

      hit_x = picker.cells[index].bounds.x + 1.0f;
      hit_y = picker.cells[index].bounds.y + 1.0f;
      M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN,
                                    (m3_i32)hit_x, (m3_i32)hit_y));
      M3_TEST_OK(
          picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
      M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, (m3_i32)hit_x,
                                    (m3_i32)hit_y));
      M3_TEST_OK(
          picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
      break;
    }
    index += 1u;
  }

  picker.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
  M3_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  picker.widget.flags = 0u;

  M3_TEST_EXPECT(m3_date_picker_test_hit_test(NULL, 0.0f, 0.0f, &index),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_test_hit_test(&picker, 0.0f, 0.0f, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_date_picker_test_hit_test(&picker, -1.0f, -1.0f, &index));
  M3_TEST_ASSERT(index == M3_DATE_PICKER_INVALID_INDEX);

  M3_TEST_EXPECT(m3_date_picker_test_compute_cell_bounds(NULL, 0u, 0u, &bounds),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_test_compute_cell_bounds(&picker, 0u, 0u, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_date_picker_test_compute_cell_bounds(
                     &picker, M3_DATE_PICKER_GRID_ROWS, 0u, &bounds),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_date_picker_test_compute_cell_bounds(
                     &picker, 0u, M3_DATE_PICKER_GRID_COLS, &bounds),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_date_picker_test_compute_cell_bounds(&picker, 0u, 0u, &bounds));

  M3_TEST_OK(picker.widget.vtable->destroy(picker.widget.ctx));

  return 0;
}

int main(void) {
  M3_TEST_OK(test_helpers());
  M3_TEST_OK(test_init_and_display());
  M3_TEST_OK(test_constraints_and_selection());
  M3_TEST_OK(test_grid_and_cells());
  M3_TEST_OK(test_update_grid_errors());
  M3_TEST_OK(test_widget_paths());
  return 0;
}
