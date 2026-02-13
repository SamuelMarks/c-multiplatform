#include "m3/m3_date_picker.h"
#include "test_utils.h"

#include <string.h>

typedef struct TestDatePickerBackend {
  int draw_rect_calls;
  int fail_draw_rect;
  int fail_draw_rect_after;
  CMPRect last_rect;
  CMPColor last_color;
  CMPScalar last_corner;
} TestDatePickerBackend;

typedef struct ChangeCounter {
  int calls;
  int fail;
  M3DateRange last;
} ChangeCounter;

static int test_backend_init(TestDatePickerBackend *backend) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(backend, 0, sizeof(*backend));
  backend->fail_draw_rect = CMP_OK;
  backend->fail_draw_rect_after = 0;
  return CMP_OK;
}

static int test_gfx_draw_rect(void *gfx, const CMPRect *rect, CMPColor color,
                              CMPScalar corner_radius) {
  TestDatePickerBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestDatePickerBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_color = color;
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

static const CMPGfxVTable g_test_vtable = {
    NULL, NULL, NULL, test_gfx_draw_rect, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL};

static const CMPGfxVTable g_test_vtable_no_rect = {NULL, NULL, NULL, NULL, NULL,
                                                   NULL, NULL, NULL, NULL, NULL,
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

static int test_on_change(void *ctx, M3DatePicker *picker,
                          const M3DateRange *range) {
  ChangeCounter *counter;

  if (ctx == NULL || picker == NULL || range == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  counter = (ChangeCounter *)ctx;
  counter->calls += 1;
  counter->last = *range;
  if (counter->fail) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_helpers(void) {
  M3DatePickerStyle style;
  CMPLayoutEdges edges;
  CMPColor color;
  CMPRect rect;
  CMPMeasureSpec spec;
  CMPDate date;
  CMPBool leap;
  CMPBool in_range;
  cmp_u32 days;
  cmp_u32 weekday;
  cmp_i32 cmp;
  cmp_u32 offset;
  cmp_i32 year;
  cmp_u32 month;

  CMP_TEST_EXPECT(m3_date_picker_test_validate_color(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  color.r = 1.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 0.0f;
  color.g = -0.1f;
  color.b = 0.0f;
  color.a = 0.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.1f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = 0.0f;
  color.a = -0.1f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_color(&color), CMP_ERR_RANGE);

  CMP_TEST_EXPECT(m3_date_picker_test_validate_edges(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_test_validate_style(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_test_validate_rect(NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  edges.left = -1.0f;
  edges.right = 0.0f;
  edges.top = 0.0f;
  edges.bottom = 0.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_edges(&edges), CMP_ERR_RANGE);

  CMP_TEST_OK(m3_date_picker_style_init(&style));
  CMP_TEST_OK(m3_date_picker_test_validate_style(&style));
  style.cell_width = 0.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_date_picker_style_init(&style));
  style.cell_spacing_x = -1.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_date_picker_style_init(&style));
  style.cell_corner_radius = style.cell_width;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_date_picker_style_init(&style));
  style.header_height = -1.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_date_picker_style_init(&style));
  style.weekday_height = -1.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_date_picker_style_init(&style));
  style.cell_corner_radius = -1.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_date_picker_style_init(&style));
  style.padding.left = -1.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_date_picker_style_init(&style));
  style.background_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_date_picker_style_init(&style));
  style.cell_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_date_picker_style_init(&style));
  style.cell_range_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_date_picker_style_init(&style));
  style.cell_selected_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_style(&style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_date_picker_style_init(&style));
  style.cell_disabled_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_style(&style), CMP_ERR_RANGE);

  spec.mode = 99u;
  spec.size = 10.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_measure_spec(spec),
                  CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_MEASURE_EXACTLY;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_measure_spec(spec),
                  CMP_ERR_RANGE);

  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 0.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_rect(&rect), CMP_ERR_RANGE);

  date.year = -1;
  date.month = 1u;
  date.day = 1u;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_date(&date), CMP_ERR_RANGE);
  date.year = 2024;
  date.month = 13u;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_date(&date), CMP_ERR_RANGE);
  date.year = 2024;
  date.month = 2u;
  date.day = 30u;
  CMP_TEST_EXPECT(m3_date_picker_test_validate_date(&date), CMP_ERR_RANGE);
  date.day = 29u;
  CMP_TEST_OK(m3_date_picker_test_validate_date(&date));

  CMP_TEST_EXPECT(m3_date_picker_test_is_leap_year(2024, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_test_is_leap_year(-1, &leap), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_date_picker_test_is_leap_year(2000, &leap));
  CMP_TEST_ASSERT(leap == CMP_TRUE);
  CMP_TEST_OK(m3_date_picker_test_is_leap_year(1900, &leap));
  CMP_TEST_ASSERT(leap == CMP_FALSE);

  CMP_TEST_EXPECT(m3_date_picker_test_days_in_month(2024, 1u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_test_days_in_month(-1, 1u, &days),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_date_picker_test_days_in_month(2024, 13u, &days),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_date_picker_test_days_in_month(2024, 2u, &days));
  CMP_TEST_ASSERT(days == 29u);
  CMP_TEST_OK(m3_date_picker_test_days_in_month(2023, 2u, &days));
  CMP_TEST_ASSERT(days == 28u);
  CMP_TEST_OK(m3_date_picker_test_days_in_month(2024, 1u, &days));
  CMP_TEST_ASSERT(days == 31u);
  CMP_TEST_OK(m3_date_picker_test_days_in_month(2024, 4u, &days));
  CMP_TEST_ASSERT(days == 30u);

  date.year = 2000;
  date.month = 1u;
  date.day = 1u;
  CMP_TEST_OK(m3_date_picker_test_day_of_week(&date, &weekday));
  CMP_TEST_ASSERT(weekday == M3_DATE_PICKER_WEEKDAY_SATURDAY);
  CMP_TEST_EXPECT(m3_date_picker_test_day_of_week(NULL, &weekday),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_test_day_of_week(&date, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  {
    CMPDate a;
    CMPDate b;

    a.year = 2024;
    a.month = 5u;
    a.day = 10u;
    b = a;
    CMP_TEST_OK(m3_date_picker_test_compare_dates(&a, &b, &cmp));
    CMP_TEST_ASSERT(cmp == 0);

    b.day = 11u;
    CMP_TEST_OK(m3_date_picker_test_compare_dates(&a, &b, &cmp));
    CMP_TEST_ASSERT(cmp < 0);

    CMP_TEST_OK(m3_date_picker_test_compare_dates(&b, &a, &cmp));
    CMP_TEST_ASSERT(cmp > 0);

    b = a;
    b.year = 2025;
    CMP_TEST_OK(m3_date_picker_test_compare_dates(&a, &b, &cmp));
    CMP_TEST_ASSERT(cmp < 0);
    CMP_TEST_OK(m3_date_picker_test_compare_dates(&b, &a, &cmp));
    CMP_TEST_ASSERT(cmp > 0);

    b = a;
    b.month = 6u;
    CMP_TEST_OK(m3_date_picker_test_compare_dates(&a, &b, &cmp));
    CMP_TEST_ASSERT(cmp < 0);
    CMP_TEST_OK(m3_date_picker_test_compare_dates(&b, &a, &cmp));
    CMP_TEST_ASSERT(cmp > 0);
  }
  CMP_TEST_EXPECT(m3_date_picker_test_compare_dates(NULL, &date, &cmp),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_test_compare_dates(&date, NULL, &cmp),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_test_compare_dates(&date, &date, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  {
    M3DateRange range;

    range.start.year = 2024;
    range.start.month = 1u;
    range.start.day = 1u;
    range.end.year = 2024;
    range.end.month = 12u;
    range.end.day = 31u;
    range.has_start = CMP_TRUE;
    range.has_end = CMP_TRUE;

    date.year = 2024;
    date.month = 6u;
    date.day = 1u;
    CMP_TEST_OK(m3_date_picker_test_date_in_range(&date, &range, &in_range));
    CMP_TEST_ASSERT(in_range == CMP_TRUE);

    date.year = 2025;
    CMP_TEST_OK(m3_date_picker_test_date_in_range(&date, &range, &in_range));
    CMP_TEST_ASSERT(in_range == CMP_FALSE);

    CMP_TEST_EXPECT(m3_date_picker_test_date_in_range(NULL, &range, &in_range),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_date_picker_test_date_in_range(&date, NULL, &in_range),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_date_picker_test_date_in_range(&date, &range, NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    range.start.year = 2024;
    range.start.month = 12u;
    range.start.day = 31u;
    range.end.year = 2024;
    range.end.month = 1u;
    range.end.day = 1u;
    CMP_TEST_EXPECT(m3_date_picker_test_date_in_range(&date, &range, &in_range),
                    CMP_ERR_RANGE);
  }

  CMP_TEST_EXPECT(m3_date_picker_test_compute_offset(7u, 0u, &offset),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_date_picker_test_compute_offset(0u, 7u, &offset),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_date_picker_test_compute_offset(0u, 0u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_date_picker_test_compute_offset(0u, 0u, &offset));
  CMP_TEST_ASSERT(offset == 0u);

  CMP_TEST_EXPECT(m3_date_picker_test_shift_month(2024, 1u, 1, NULL, &month),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_test_shift_month(2024, 1u, 1, &year, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_test_shift_month(-1, 1u, 1, &year, &month),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_date_picker_test_shift_month(2024, 13u, 1, &year, &month),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_date_picker_test_shift_month(2024, 0u, 1, &year, &month),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_date_picker_test_shift_month(CMP_DATE_MAX_YEAR + 1, 1u, 0,
                                                  &year, &month),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_date_picker_test_shift_month(2024, 1u, 1, &year, &month));
  CMP_TEST_ASSERT(year == 2024 && month == 2u);
  CMP_TEST_OK(m3_date_picker_test_shift_month(2024, 1u, -1, &year, &month));
  CMP_TEST_ASSERT(year == 2023 && month == 12u);
  CMP_TEST_EXPECT(
      m3_date_picker_test_shift_month(CMP_DATE_MIN_YEAR, 1u, -1, &year, &month),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      m3_date_picker_test_shift_month(CMP_DATE_MAX_YEAR, 12u, 1, &year, &month),
      CMP_ERR_RANGE);

  return 0;
}

static int test_init_and_display(void) {
  M3DatePickerStyle style;
  M3DatePicker picker;
  cmp_i32 year;
  cmp_u32 month;

  CMP_TEST_OK(m3_date_picker_style_init(&style));
  CMP_TEST_EXPECT(m3_date_picker_init(NULL, &style, 2024, 1u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_init(&picker, NULL, 2024, 1u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_init(&picker, &style, 0, 1u), CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_date_picker_init(&picker, &style, 2024, 13u),
                  CMP_ERR_RANGE);

  CMP_TEST_OK(m3_date_picker_init(&picker, &style, 2024, 12u));
  CMP_TEST_ASSERT(picker.widget.ctx == &picker);
  CMP_TEST_ASSERT(picker.widget.vtable != NULL);

  CMP_TEST_EXPECT(m3_date_picker_set_style(NULL, &style),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_set_style(&picker, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  {
    M3DatePickerStyle bad_style;

    bad_style = style;
    bad_style.cell_width = 0.0f;
    CMP_TEST_EXPECT(m3_date_picker_set_style(&picker, &bad_style),
                    CMP_ERR_RANGE);
  }

  CMP_TEST_EXPECT(m3_date_picker_get_display_month(NULL, &year, &month),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_get_display_month(&picker, NULL, &month),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_get_display_month(&picker, &year, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(m3_date_picker_set_week_start(NULL, 0u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_set_week_start(&picker, 7u), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_date_picker_set_week_start(&picker, 0u));

  CMP_TEST_OK(m3_date_picker_next_month(&picker));
  CMP_TEST_OK(m3_date_picker_get_display_month(&picker, &year, &month));
  CMP_TEST_ASSERT(year == 2025 && month == 1u);

  CMP_TEST_OK(m3_date_picker_prev_month(&picker));
  CMP_TEST_OK(m3_date_picker_get_display_month(&picker, &year, &month));
  CMP_TEST_ASSERT(year == 2024 && month == 12u);

  CMP_TEST_EXPECT(m3_date_picker_set_display_month(&picker, 2024, 0u),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_date_picker_set_display_month(&picker, 0, 1u),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_date_picker_set_display_month(&picker, -1, 1u),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_date_picker_set_display_month(NULL, 2024, 1u),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(
      m3_date_picker_set_display_month(&picker, CMP_DATE_MAX_YEAR - 1, 12u));
  CMP_TEST_OK(m3_date_picker_next_month(&picker));
  CMP_TEST_OK(m3_date_picker_get_display_month(&picker, &year, &month));
  CMP_TEST_ASSERT(year == CMP_DATE_MAX_YEAR && month == 1u);
  CMP_TEST_EXPECT(
      m3_date_picker_set_display_month(&picker, CMP_DATE_MAX_YEAR, 12u),
      CMP_ERR_RANGE);

  CMP_TEST_EXPECT(
      m3_date_picker_set_display_month(&picker, CMP_DATE_MIN_YEAR, 1u),
      CMP_ERR_RANGE);
  CMP_TEST_OK(
      m3_date_picker_set_display_month(&picker, CMP_DATE_MIN_YEAR + 1, 1u));
  CMP_TEST_OK(m3_date_picker_prev_month(&picker));
  CMP_TEST_EXPECT(m3_date_picker_next_month(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_prev_month(NULL), CMP_ERR_INVALID_ARGUMENT);

  return 0;
}

static int test_constraints_and_selection(void) {
  M3DatePickerStyle style;
  M3DatePicker picker;
  M3DateRange constraints;
  M3DateRange selection;
  M3DateRange out_range;
  ChangeCounter counter;

  CMP_TEST_OK(m3_date_picker_style_init(&style));
  CMP_TEST_OK(m3_date_picker_init(&picker, &style, 2024, 6u));

  CMP_TEST_EXPECT(m3_date_picker_set_constraints(NULL, &constraints),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_set_constraints(&picker, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  constraints.start.year = 2024;
  constraints.start.month = 1u;
  constraints.start.day = 1u;
  constraints.end.year = 2024;
  constraints.end.month = 12u;
  constraints.end.day = 31u;
  constraints.has_start = CMP_TRUE;
  constraints.has_end = CMP_TRUE;
  CMP_TEST_OK(m3_date_picker_set_constraints(&picker, &constraints));

  CMP_TEST_EXPECT(m3_date_picker_get_constraints(NULL, &out_range),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_get_constraints(&picker, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  constraints.has_start = 2;
  CMP_TEST_EXPECT(m3_date_picker_set_constraints(&picker, &constraints),
                  CMP_ERR_RANGE);
  constraints.has_start = CMP_TRUE;
  constraints.has_end = CMP_TRUE;

  selection.start.year = 2024;
  selection.start.month = 5u;
  selection.start.day = 10u;
  selection.end = selection.start;
  selection.has_start = CMP_TRUE;
  selection.has_end = CMP_FALSE;

  counter.calls = 0;
  counter.fail = 0;
  CMP_TEST_EXPECT(m3_date_picker_set_on_change(NULL, test_on_change, &counter),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_date_picker_set_on_change(&picker, test_on_change, &counter));
  CMP_TEST_EXPECT(m3_date_picker_set_selection(NULL, &selection),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_set_selection(&picker, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_date_picker_set_selection(&picker, &selection));
  CMP_TEST_ASSERT(counter.calls == 1);

  picker.style.cell_width = 0.0f;
  CMP_TEST_EXPECT(m3_date_picker_set_selection(&picker, &selection),
                  CMP_ERR_RANGE);
  picker.style.cell_width = style.cell_width;

  CMP_TEST_EXPECT(m3_date_picker_get_selection(NULL, &out_range),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_get_selection(&picker, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  counter.fail = 1;
  CMP_TEST_EXPECT(m3_date_picker_set_selection(&picker, &selection),
                  CMP_ERR_IO);
  counter.fail = 0;

  selection.end.year = 2024;
  selection.end.month = 5u;
  selection.end.day = 11u;
  selection.has_end = CMP_TRUE;
  CMP_TEST_EXPECT(m3_date_picker_set_selection(&picker, &selection),
                  CMP_ERR_RANGE);
  selection.has_end = CMP_FALSE;

  selection.start.year = 2025;
  CMP_TEST_EXPECT(m3_date_picker_set_selection(&picker, &selection),
                  CMP_ERR_RANGE);

  selection.has_start = CMP_FALSE;
  selection.has_end = CMP_TRUE;
  CMP_TEST_EXPECT(m3_date_picker_set_selection(&picker, &selection),
                  CMP_ERR_RANGE);
  selection.has_start = CMP_TRUE;
  selection.has_end = CMP_FALSE;

  selection.start.year = 2024;
  selection.end.year = 2024;
  selection.end.month = 4u;
  selection.end.day = 1u;
  selection.has_end = CMP_TRUE;
  CMP_TEST_OK(m3_date_picker_set_mode(&picker, M3_DATE_PICKER_MODE_RANGE));
  CMP_TEST_EXPECT(m3_date_picker_set_selection(&picker, &selection),
                  CMP_ERR_RANGE);

  selection.end.year = 2024;
  selection.end.month = 6u;
  selection.end.day = 1u;
  CMP_TEST_OK(m3_date_picker_set_selection(&picker, &selection));

  CMP_TEST_OK(m3_date_picker_get_selection(&picker, &out_range));
  CMP_TEST_ASSERT(out_range.has_start == CMP_TRUE);
  CMP_TEST_ASSERT(out_range.has_end == CMP_TRUE);

  constraints.end.year = 2024;
  constraints.end.month = 5u;
  constraints.end.day = 1u;
  CMP_TEST_EXPECT(m3_date_picker_set_constraints(&picker, &constraints),
                  CMP_ERR_RANGE);

  CMP_TEST_EXPECT(m3_date_picker_set_mode(&picker, M3_DATE_PICKER_MODE_SINGLE),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_date_picker_set_mode(NULL, M3_DATE_PICKER_MODE_SINGLE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_set_mode(&picker, 99u), CMP_ERR_RANGE);

  return 0;
}

static int find_cell(const M3DatePickerCell *cells, cmp_usize count,
                     cmp_i32 year, cmp_u32 month, cmp_u32 day,
                     cmp_usize *out_index) {
  cmp_usize i;

  if (cells == NULL || out_index == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < count; ++i) {
    if (cells[i].date.year == year && cells[i].date.month == month &&
        cells[i].date.day == day) {
      *out_index = i;
      return CMP_OK;
    }
  }

  return CMP_ERR_NOT_FOUND;
}

static int test_grid_and_cells(void) {
  M3DatePickerStyle style;
  M3DatePicker picker;
  M3DateRange constraints;
  M3DateRange selection;
  const M3DatePickerCell *cells;
  cmp_usize count;
  cmp_usize index;

  CMP_TEST_OK(m3_date_picker_style_init(&style));
  CMP_TEST_OK(m3_date_picker_init(&picker, &style, 2024, 2u));

  constraints.start.year = 2024;
  constraints.start.month = 2u;
  constraints.start.day = 10u;
  constraints.end.year = 2024;
  constraints.end.month = 2u;
  constraints.end.day = 20u;
  constraints.has_start = CMP_TRUE;
  constraints.has_end = CMP_TRUE;
  CMP_TEST_OK(m3_date_picker_set_constraints(&picker, &constraints));

  selection.start.year = 2024;
  selection.start.month = 2u;
  selection.start.day = 14u;
  selection.end = selection.start;
  selection.has_start = CMP_TRUE;
  selection.has_end = CMP_FALSE;
  CMP_TEST_OK(m3_date_picker_set_selection(&picker, &selection));

  CMP_TEST_EXPECT(m3_date_picker_get_cells(NULL, &cells, &count),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_get_cells(&picker, NULL, &count),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_get_cells(&picker, &cells, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_date_picker_get_cells(&picker, &cells, &count));
  CMP_TEST_ASSERT(count == M3_DATE_PICKER_GRID_COUNT);

  CMP_TEST_OK(find_cell(cells, count, 2024, 2u, 14u, &index));
  CMP_TEST_ASSERT(cells[index].flags & M3_DATE_CELL_FLAG_SELECTED);

  CMP_TEST_OK(find_cell(cells, count, 2024, 2u, 5u, &index));
  CMP_TEST_ASSERT(cells[index].flags & M3_DATE_CELL_FLAG_OUT_OF_RANGE);

  CMP_TEST_EXPECT(m3_date_picker_get_cell(NULL, 0, &picker.cells[0]),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_date_picker_get_cell(&picker, M3_DATE_PICKER_GRID_COUNT, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_get_cell(&picker, M3_DATE_PICKER_GRID_COUNT,
                                          &picker.cells[0]),
                  CMP_ERR_RANGE);

  return 0;
}

static int test_update_grid_errors(void) {
  M3DatePickerStyle style;
  M3DatePicker picker;

  CMP_TEST_EXPECT(m3_date_picker_update(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_test_update_grid(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_date_picker_style_init(&style));
  CMP_TEST_OK(m3_date_picker_init(&picker, &style, 2024, 3u));

  picker.style.cell_width = 0.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_update_grid(&picker), CMP_ERR_RANGE);
  picker.style.cell_width = style.cell_width;

  picker.bounds.width = -1.0f;
  CMP_TEST_EXPECT(m3_date_picker_test_update_grid(&picker), CMP_ERR_RANGE);
  picker.bounds.width = 0.0f;

  picker.week_start = 7u;
  CMP_TEST_EXPECT(m3_date_picker_test_update_grid(&picker), CMP_ERR_RANGE);
  picker.week_start = M3_DATE_PICKER_WEEK_START_SUNDAY;

  picker.constraints.has_start = 2;
  CMP_TEST_EXPECT(m3_date_picker_test_update_grid(&picker), CMP_ERR_RANGE);
  picker.constraints.has_start = CMP_FALSE;

  picker.selection.has_start = CMP_FALSE;
  picker.selection.has_end = CMP_TRUE;
  CMP_TEST_EXPECT(m3_date_picker_test_update_grid(&picker), CMP_ERR_RANGE);

  picker.mode = 99u;
  CMP_TEST_EXPECT(m3_date_picker_test_update_grid(&picker), CMP_ERR_RANGE);

  return 0;
}

static int test_widget_paths(void) {
  TestDatePickerBackend backend;
  CMPGfx gfx;
  CMPPaintContext paint_ctx;
  M3DatePickerStyle style;
  M3DatePicker picker;
  CMPMeasureSpec width_spec;
  CMPMeasureSpec height_spec;
  CMPSize size;
  CMPRect bounds;
  CMPSemantics semantics;
  CMPInputEvent event;
  CMPBool handled;
  cmp_usize index;
  M3DateRange constraints;
  M3DateRange selection;
  CMPScalar hit_x;
  CMPScalar hit_y;

  CMP_TEST_OK(test_backend_init(&backend));
  gfx.ctx = &backend;
  gfx.vtable = &g_test_vtable;
  gfx.text_vtable = NULL;

  paint_ctx.gfx = &gfx;
  paint_ctx.dpi_scale = 1.0f;
  paint_ctx.clip.x = 0.0f;
  paint_ctx.clip.y = 0.0f;
  paint_ctx.clip.width = 400.0f;
  paint_ctx.clip.height = 400.0f;

  CMP_TEST_OK(m3_date_picker_style_init(&style));
  CMP_TEST_OK(m3_date_picker_init(&picker, &style, 2024, 2u));

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 300.0f;
  height_spec.mode = CMP_MEASURE_EXACTLY;
  height_spec.size = 300.0f;

  CMP_TEST_EXPECT(
      picker.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(picker.widget.vtable->measure(picker.widget.ctx, width_spec,
                                                height_spec, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  width_spec.mode = 99u;
  CMP_TEST_EXPECT(picker.widget.vtable->measure(picker.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  width_spec.mode = CMP_MEASURE_EXACTLY;
  height_spec.size = -1.0f;
  CMP_TEST_EXPECT(picker.widget.vtable->measure(picker.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_RANGE);
  height_spec.size = 300.0f;

  CMP_TEST_OK(picker.widget.vtable->measure(picker.widget.ctx, width_spec,
                                            height_spec, &size));
  CMP_TEST_ASSERT(size.width == 300.0f && size.height == 300.0f);

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 300.0f;
  bounds.height = 300.0f;
  CMP_TEST_EXPECT(picker.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  bounds.width = -1.0f;
  CMP_TEST_EXPECT(picker.widget.vtable->layout(picker.widget.ctx, bounds),
                  CMP_ERR_RANGE);
  bounds.width = 300.0f;
  CMP_TEST_OK(picker.widget.vtable->layout(picker.widget.ctx, bounds));

  CMP_TEST_EXPECT(picker.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(picker.widget.vtable->get_semantics(picker.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(
      picker.widget.vtable->get_semantics(picker.widget.ctx, &semantics));
  CMP_TEST_ASSERT(semantics.role == CMP_SEMANTIC_NONE);

  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(picker.widget.vtable->paint(NULL, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = NULL;
  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = &gfx;
  gfx.vtable = NULL;
  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_vtable_no_rect;
  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable;

  backend.fail_draw_rect_after = 1;
  CMP_TEST_EXPECT(picker.widget.vtable->paint(picker.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend.fail_draw_rect_after = 0;
  backend.fail_draw_rect = CMP_OK;
  CMP_TEST_OK(picker.widget.vtable->paint(picker.widget.ctx, &paint_ctx));

  constraints.start.year = 2024;
  constraints.start.month = 2u;
  constraints.start.day = 5u;
  constraints.end.year = 2024;
  constraints.end.month = 2u;
  constraints.end.day = 25u;
  constraints.has_start = CMP_TRUE;
  constraints.has_end = CMP_TRUE;
  CMP_TEST_OK(m3_date_picker_set_constraints(&picker, &constraints));
  CMP_TEST_OK(m3_date_picker_set_mode(&picker, M3_DATE_PICKER_MODE_RANGE));
  selection.start.year = 2024;
  selection.start.month = 2u;
  selection.start.day = 10u;
  selection.end.year = 2024;
  selection.end.month = 2u;
  selection.end.day = 12u;
  selection.has_start = CMP_TRUE;
  selection.has_end = CMP_TRUE;
  CMP_TEST_OK(m3_date_picker_set_selection(&picker, &selection));
  CMP_TEST_OK(picker.widget.vtable->paint(picker.widget.ctx, &paint_ctx));

  CMP_TEST_EXPECT(
      picker.widget.vtable->event(picker.widget.ctx, NULL, &handled),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(picker.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(picker.widget.vtable->event(picker.widget.ctx, &event, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(
      m3_date_picker_test_compute_cell_bounds(&picker, 0u, 0u, &bounds));
  hit_x = bounds.x + 1.0f;
  hit_y = bounds.y + 1.0f;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_EXPECT(
      picker.widget.vtable->event(picker.widget.ctx, &event, &handled),
      CMP_ERR_STATE);
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  {
    cmp_usize start_index;
    cmp_usize other_index;
    cmp_usize row;
    cmp_usize col;
    CMPRect cell_bounds;

    start_index = 0u;
    while (start_index < M3_DATE_PICKER_GRID_COUNT) {
      if ((picker.cells[start_index].flags & M3_DATE_CELL_FLAG_OUT_OF_RANGE) ==
          0u) {
        break;
      }
      start_index += 1u;
    }
    other_index = start_index + 1u;
    while (other_index < M3_DATE_PICKER_GRID_COUNT) {
      if ((picker.cells[other_index].flags & M3_DATE_CELL_FLAG_OUT_OF_RANGE) ==
          0u) {
        break;
      }
      other_index += 1u;
    }
    CMP_TEST_ASSERT(other_index < M3_DATE_PICKER_GRID_COUNT);

    selection.start = picker.cells[start_index].date;
    selection.end = selection.start;
    selection.has_start = CMP_TRUE;
    selection.has_end = CMP_FALSE;
    CMP_TEST_OK(m3_date_picker_set_selection(&picker, &selection));

    row = other_index / M3_DATE_PICKER_GRID_COLS;
    col = other_index % M3_DATE_PICKER_GRID_COLS;
    CMP_TEST_OK(m3_date_picker_test_compute_cell_bounds(&picker, row, col,
                                                        &cell_bounds));
    hit_x = cell_bounds.x + 1.0f;
    hit_y = cell_bounds.y + 1.0f;
    CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN,
                                   (cmp_i32)hit_x, (cmp_i32)hit_y));
    CMP_TEST_OK(
        picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
    CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, (cmp_i32)hit_x,
                                   (cmp_i32)hit_y));
    CMP_TEST_OK(
        picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  }

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, -10, -10));
  CMP_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  picker.mode = 99u;
  index = 0u;
  while (index < M3_DATE_PICKER_GRID_COUNT) {
    if ((picker.cells[index].flags & M3_DATE_CELL_FLAG_OUT_OF_RANGE) == 0u) {
      break;
    }
    index += 1u;
  }
  CMP_TEST_ASSERT(index < M3_DATE_PICKER_GRID_COUNT);
  hit_x = picker.cells[index].bounds.x + 1.0f;
  hit_y = picker.cells[index].bounds.y + 1.0f;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_EXPECT(
      picker.widget.vtable->event(picker.widget.ctx, &event, &handled),
      CMP_ERR_RANGE);
  picker.mode = M3_DATE_PICKER_MODE_SINGLE;
  picker.selection.has_end = CMP_FALSE;

  selection.has_start = CMP_FALSE;
  selection.has_end = CMP_FALSE;
  CMP_TEST_OK(m3_date_picker_set_selection(&picker, &selection));

  constraints.start.year = 2024;
  constraints.start.month = 2u;
  constraints.start.day = 15u;
  constraints.end = constraints.start;
  constraints.has_start = CMP_TRUE;
  constraints.has_end = CMP_TRUE;
  CMP_TEST_OK(m3_date_picker_set_constraints(&picker, &constraints));
  CMP_TEST_OK(m3_date_picker_update(&picker));
  index = 0u;
  while (index < M3_DATE_PICKER_GRID_COUNT) {
    if (picker.cells[index].flags & M3_DATE_CELL_FLAG_OUT_OF_RANGE) {
      CMPScalar hit_x;
      CMPScalar hit_y;

      hit_x = picker.cells[index].bounds.x + 1.0f;
      hit_y = picker.cells[index].bounds.y + 1.0f;
      CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN,
                                     (cmp_i32)hit_x, (cmp_i32)hit_y));
      CMP_TEST_OK(
          picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
      CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP,
                                     (cmp_i32)hit_x, (cmp_i32)hit_y));
      CMP_TEST_OK(
          picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
      break;
    }
    index += 1u;
  }

  picker.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_OK(picker.widget.vtable->event(picker.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  picker.widget.flags = 0u;

  CMP_TEST_EXPECT(m3_date_picker_test_hit_test(NULL, 0.0f, 0.0f, &index),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_test_hit_test(&picker, 0.0f, 0.0f, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_date_picker_test_hit_test(&picker, -1.0f, -1.0f, &index));
  CMP_TEST_ASSERT(index == M3_DATE_PICKER_INVALID_INDEX);

  CMP_TEST_EXPECT(
      m3_date_picker_test_compute_cell_bounds(NULL, 0u, 0u, &bounds),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_date_picker_test_compute_cell_bounds(&picker, 0u, 0u, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_date_picker_test_compute_cell_bounds(
                      &picker, M3_DATE_PICKER_GRID_ROWS, 0u, &bounds),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_date_picker_test_compute_cell_bounds(
                      &picker, 0u, M3_DATE_PICKER_GRID_COLS, &bounds),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(
      m3_date_picker_test_compute_cell_bounds(&picker, 0u, 0u, &bounds));

  CMP_TEST_OK(picker.widget.vtable->destroy(picker.widget.ctx));

  return 0;
}

int main(void) {
  CMP_TEST_OK(test_helpers());
  CMP_TEST_OK(test_init_and_display());
  CMP_TEST_OK(test_constraints_and_selection());
  CMP_TEST_OK(test_grid_and_cells());
  CMP_TEST_OK(test_update_grid_errors());
  CMP_TEST_OK(test_widget_paths());
  return 0;
}
