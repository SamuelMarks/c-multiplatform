#include "m3/m3_sheet.h"
#include "test_utils.h"

#include <string.h>

#define M3_SHEET_TEST_FAIL_NONE 0u
#define M3_SHEET_TEST_FAIL_SHADOW_INIT 1u
#define M3_SHEET_TEST_FAIL_COLOR_SET 2u
#define M3_SHEET_TEST_FAIL_ANIM_INIT 3u
#define M3_SHEET_TEST_FAIL_ANIM_START 4u
#define M3_SHEET_TEST_FAIL_ANIM_TARGET 5u
#define M3_SHEET_TEST_FAIL_APPLY_OFFSET 6u
#define M3_SHEET_TEST_FAIL_SCRIM_ALPHA 7u

int CMP_CALL m3_sheet_test_set_fail_point(cmp_u32 fail_point);
int CMP_CALL m3_sheet_test_set_color_fail_after(cmp_u32 call_count);
int CMP_CALL m3_sheet_test_clear_fail_points(void);
int CMP_CALL m3_sheet_test_validate_color(const CMPColor *color);
int CMP_CALL m3_sheet_test_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
                                    CMPScalar b, CMPScalar a);
int CMP_CALL m3_sheet_test_validate_edges(const CMPLayoutEdges *edges);
int CMP_CALL m3_sheet_test_validate_measure_spec(CMPMeasureSpec spec);
int CMP_CALL m3_sheet_test_validate_rect(const CMPRect *rect);
int CMP_CALL m3_sheet_test_validate_style(const M3SheetStyle *style);
int CMP_CALL m3_sheet_test_apply_offset(M3Sheet *sheet, CMPScalar offset,
                                       CMPBool reset_velocity,
                                       CMPBool *out_changed);
int CMP_CALL m3_sheet_test_compute_scrim_alpha(const M3Sheet *sheet,
                                              CMPScalar *out_alpha);

typedef struct TestSheetBackend {
  int draw_rect_calls;
  int push_clip_calls;
  int pop_clip_calls;
  int fail_draw_rect;
  int fail_push_clip;
  int fail_pop_clip;
  CMPRect rects[4];
  CMPColor colors[4];
  CMPScalar corners[4];
} TestSheetBackend;

typedef struct TestSheetAction {
  int calls;
  cmp_u32 last_action;
  int fail;
} TestSheetAction;

static int test_backend_init(TestSheetBackend *backend) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(backend, 0, sizeof(*backend));
  return CMP_OK;
}

static int test_gfx_draw_rect(void *gfx, const CMPRect *rect, CMPColor color,
                              CMPScalar corner_radius) {
  TestSheetBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestSheetBackend *)gfx;
  backend->draw_rect_calls += 1;
  if (backend->draw_rect_calls <= 4) {
    backend->rects[backend->draw_rect_calls - 1] = *rect;
    backend->colors[backend->draw_rect_calls - 1] = color;
    backend->corners[backend->draw_rect_calls - 1] = corner_radius;
  }
  if (backend->fail_draw_rect) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_gfx_push_clip(void *gfx, const CMPRect *rect) {
  TestSheetBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestSheetBackend *)gfx;
  backend->push_clip_calls += 1;
  if (backend->fail_push_clip) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_gfx_pop_clip(void *gfx) {
  TestSheetBackend *backend;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestSheetBackend *)gfx;
  backend->pop_clip_calls += 1;
  if (backend->fail_pop_clip) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static const CMPGfxVTable g_test_gfx_vtable = {NULL,
                                              NULL,
                                              NULL,
                                              test_gfx_draw_rect,
                                              NULL,
                                              NULL,
                                              test_gfx_push_clip,
                                              test_gfx_pop_clip,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL};

static const CMPGfxVTable g_test_gfx_vtable_no_draw = {
    NULL, NULL, NULL, NULL, NULL, NULL, test_gfx_push_clip, test_gfx_pop_clip,
    NULL, NULL, NULL, NULL, NULL};

static int test_sheet_on_action(void *ctx, struct M3Sheet *sheet,
                                cmp_u32 action) {
  TestSheetAction *state;

  if (ctx == NULL || sheet == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestSheetAction *)ctx;
  state->calls += 1;
  state->last_action = action;
  if (state->fail) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
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

static int init_gesture_event(CMPInputEvent *event, cmp_u32 type, CMPScalar x,
                              CMPScalar y, CMPScalar start_x, CMPScalar start_y,
                              CMPScalar total_y, CMPScalar velocity_y) {
  if (event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(event, 0, sizeof(*event));
  event->type = type;
  event->data.gesture.x = x;
  event->data.gesture.y = y;
  event->data.gesture.start_x = start_x;
  event->data.gesture.start_y = start_y;
  event->data.gesture.total_y = total_y;
  event->data.gesture.velocity_y = velocity_y;
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

static int test_sheet_helpers(void) {
  CMPColor color;
  CMPLayoutEdges edges;
  CMPMeasureSpec spec;
  CMPRect rect;
  M3SheetStyle base_style;
  M3SheetStyle style;
  M3Sheet sheet;
  CMPBool changed;
  CMPScalar alpha;

  CMP_TEST_EXPECT(m3_sheet_test_validate_color(NULL), CMP_ERR_INVALID_ARGUMENT);

  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_color(&color), CMP_ERR_RANGE);

  color.r = 0.0f;
  color.g = -0.1f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_color(&color), CMP_ERR_RANGE);

  color.g = 0.0f;
  color.b = -0.1f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_color(&color), CMP_ERR_RANGE);

  color.b = 0.0f;
  color.a = -0.1f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_color(&color), CMP_ERR_RANGE);

  CMP_TEST_EXPECT(m3_sheet_test_color_set(NULL, 0.0f, 0.0f, 0.0f, 0.0f),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_sheet_test_color_set(&color, -0.1f, 0.0f, 0.0f, 0.0f),
                 CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_sheet_test_color_set(&color, 0.0f, -0.1f, 0.0f, 0.0f),
                 CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_sheet_test_color_set(&color, 0.0f, 0.0f, -0.1f, 0.0f),
                 CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_sheet_test_color_set(&color, 0.0f, 0.0f, 0.0f, -0.1f),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(m3_sheet_test_color_set(&color, 0.1f, 0.2f, 0.3f, 0.4f));

  CMP_TEST_EXPECT(m3_sheet_test_validate_edges(NULL), CMP_ERR_INVALID_ARGUMENT);
  edges.left = -1.0f;
  edges.right = 0.0f;
  edges.top = 0.0f;
  edges.bottom = 0.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.left = 0.0f;
  edges.right = -1.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.right = 0.0f;
  edges.top = -1.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.top = 0.0f;
  edges.bottom = -1.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.bottom = 0.0f;
  CMP_TEST_OK(m3_sheet_test_validate_edges(&edges));

  spec.mode = 99u;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_measure_spec(spec),
                 CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_MEASURE_AT_MOST;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_measure_spec(spec), CMP_ERR_RANGE);
  spec.mode = CMP_MEASURE_EXACTLY;
  spec.size = 12.0f;
  CMP_TEST_OK(m3_sheet_test_validate_measure_spec(spec));

  CMP_TEST_EXPECT(m3_sheet_test_validate_rect(NULL), CMP_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 1.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.width = 1.0f;
  rect.height = -1.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.height = 1.0f;
  CMP_TEST_OK(m3_sheet_test_validate_rect(&rect));

  CMP_TEST_EXPECT(m3_sheet_test_validate_style(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_sheet_style_init_standard(&base_style));
  CMP_TEST_OK(m3_sheet_test_validate_style(&base_style));

  memcpy(&style, &base_style, sizeof(style));
  style.variant = 99u;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  memcpy(&style, &base_style, sizeof(style));
  style.padding.left = -1.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  memcpy(&style, &base_style, sizeof(style));
  style.height = -1.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  memcpy(&style, &base_style, sizeof(style));
  style.min_height = -1.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  memcpy(&style, &base_style, sizeof(style));
  style.max_height = -1.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  memcpy(&style, &base_style, sizeof(style));
  style.max_height = 10.0f;
  style.min_height = 20.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  memcpy(&style, &base_style, sizeof(style));
  style.max_width = -1.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  memcpy(&style, &base_style, sizeof(style));
  style.corner_radius = -1.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  memcpy(&style, &base_style, sizeof(style));
  style.dismiss_threshold = -0.1f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);
  style.dismiss_threshold = 1.1f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  memcpy(&style, &base_style, sizeof(style));
  style.min_fling_velocity = -1.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  memcpy(&style, &base_style, sizeof(style));
  style.spring_stiffness = 0.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  memcpy(&style, &base_style, sizeof(style));
  style.spring_damping = -0.1f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  memcpy(&style, &base_style, sizeof(style));
  style.spring_mass = 0.0f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  memcpy(&style, &base_style, sizeof(style));
  style.spring_tolerance = -0.1f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  memcpy(&style, &base_style, sizeof(style));
  style.spring_rest_velocity = -0.1f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  memcpy(&style, &base_style, sizeof(style));
  style.shadow_enabled = 5;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  memcpy(&style, &base_style, sizeof(style));
  style.scrim_enabled = 5;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  memcpy(&style, &base_style, sizeof(style));
  style.background_color.a = -0.1f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  memcpy(&style, &base_style, sizeof(style));
  style.scrim_color.a = -0.1f;
  CMP_TEST_EXPECT(m3_sheet_test_validate_style(&style), CMP_ERR_RANGE);

  CMP_TEST_OK(m3_sheet_test_clear_fail_points());
  CMP_TEST_OK(m3_sheet_test_set_fail_point(M3_SHEET_TEST_FAIL_SHADOW_INIT));
  CMP_TEST_EXPECT(m3_sheet_style_init_modal(&style), CMP_ERR_IO);

  CMP_TEST_OK(m3_sheet_test_clear_fail_points());
  CMP_TEST_OK(m3_sheet_test_set_color_fail_after(1u));
  CMP_TEST_EXPECT(m3_sheet_style_init_standard(&style), CMP_ERR_IO);
  CMP_TEST_OK(m3_sheet_test_clear_fail_points());

  CMP_TEST_OK(m3_sheet_style_init_standard(&style));
  CMP_TEST_OK(m3_sheet_init(&sheet, &style));
  sheet.sheet_bounds.height = 100.0f;
  sheet.offset = 0.0f;
  sheet.anim.spring.velocity = 5.0f;
  CMP_TEST_EXPECT(m3_sheet_test_apply_offset(NULL, 0.0f, CMP_FALSE, &changed),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_sheet_test_apply_offset(&sheet, -10.0f, CMP_FALSE, &changed));
  CMP_TEST_ASSERT(sheet.offset == 0.0f);
  CMP_TEST_ASSERT(sheet.anim.spring.velocity == 0.0f);
  CMP_TEST_ASSERT(changed == CMP_FALSE);
  CMP_TEST_OK(m3_sheet_test_apply_offset(&sheet, 200.0f, CMP_FALSE, &changed));
  CMP_TEST_ASSERT(sheet.offset == 100.0f);
  CMP_TEST_ASSERT(changed == CMP_TRUE);
  sheet.sheet_bounds.height = -1.0f;
  CMP_TEST_EXPECT(m3_sheet_test_apply_offset(&sheet, 0.0f, CMP_TRUE, &changed),
                 CMP_ERR_RANGE);
  sheet.sheet_bounds.height = 100.0f;
  CMP_TEST_OK(m3_sheet_test_set_fail_point(M3_SHEET_TEST_FAIL_APPLY_OFFSET));
  CMP_TEST_EXPECT(m3_sheet_test_apply_offset(&sheet, 0.0f, CMP_TRUE, &changed),
                 CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_sheet_test_clear_fail_points());

  CMP_TEST_EXPECT(m3_sheet_test_compute_scrim_alpha(NULL, &alpha),
                 CMP_ERR_INVALID_ARGUMENT);
  sheet.sheet_bounds.height = -1.0f;
  CMP_TEST_EXPECT(m3_sheet_test_compute_scrim_alpha(&sheet, &alpha),
                 CMP_ERR_RANGE);
  sheet.sheet_bounds.height = 0.0f;
  sheet.offset = 0.0f;
  CMP_TEST_OK(m3_sheet_test_compute_scrim_alpha(&sheet, &alpha));
  CMP_TEST_ASSERT(alpha == 0.0f);
  sheet.sheet_bounds.height = 100.0f;
  sheet.offset = -10.0f;
  CMP_TEST_OK(m3_sheet_test_compute_scrim_alpha(&sheet, &alpha));
  CMP_TEST_ASSERT(alpha == 1.0f);
  sheet.offset = 120.0f;
  CMP_TEST_OK(m3_sheet_test_compute_scrim_alpha(&sheet, &alpha));
  CMP_TEST_ASSERT(alpha == 0.0f);
  CMP_TEST_OK(m3_sheet_test_set_fail_point(M3_SHEET_TEST_FAIL_SCRIM_ALPHA));
  CMP_TEST_EXPECT(m3_sheet_test_compute_scrim_alpha(&sheet, &alpha),
                 CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_sheet_test_clear_fail_points());

  return 0;
}

static int test_sheet_init_and_setters(void) {
  M3SheetStyle style;
  M3Sheet sheet;
  CMPBool open;

  CMP_TEST_OK(m3_sheet_style_init_standard(&style));

  CMP_TEST_EXPECT(m3_sheet_init(NULL, &style), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_sheet_init(&sheet, NULL), CMP_ERR_INVALID_ARGUMENT);

  style.variant = 99u;
  CMP_TEST_EXPECT(m3_sheet_init(&sheet, &style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_sheet_style_init_standard(&style));

  CMP_TEST_OK(m3_sheet_test_set_fail_point(M3_SHEET_TEST_FAIL_ANIM_INIT));
  CMP_TEST_EXPECT(m3_sheet_init(&sheet, &style), CMP_ERR_IO);
  CMP_TEST_OK(m3_sheet_test_clear_fail_points());

  CMP_TEST_OK(m3_sheet_test_set_fail_point(M3_SHEET_TEST_FAIL_ANIM_START));
  CMP_TEST_EXPECT(m3_sheet_init(&sheet, &style), CMP_ERR_IO);
  CMP_TEST_OK(m3_sheet_test_clear_fail_points());

  CMP_TEST_OK(m3_sheet_init(&sheet, &style));
  sheet.sheet_bounds.height = 120.0f;
  sheet.offset = 0.0f;

  CMP_TEST_EXPECT(m3_sheet_set_style(NULL, &style), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_sheet_set_style(&sheet, NULL), CMP_ERR_INVALID_ARGUMENT);
  style.variant = 99u;
  CMP_TEST_EXPECT(m3_sheet_set_style(&sheet, &style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_sheet_style_init_standard(&style));

  CMP_TEST_EXPECT(m3_sheet_set_on_action(NULL, NULL, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_sheet_set_on_action(&sheet, NULL, NULL));

  CMP_TEST_EXPECT(m3_sheet_set_open(NULL, CMP_TRUE), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_sheet_set_open(&sheet, 3), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_sheet_get_open(NULL, &open), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_sheet_get_open(&sheet, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_sheet_get_open(&sheet, &open));
  CMP_TEST_ASSERT(open == CMP_TRUE);

  CMP_TEST_OK(m3_sheet_test_set_fail_point(M3_SHEET_TEST_FAIL_ANIM_TARGET));
  CMP_TEST_EXPECT(m3_sheet_set_open(&sheet, CMP_FALSE), CMP_ERR_IO);
  CMP_TEST_OK(m3_sheet_test_clear_fail_points());

  sheet.anim.mode = CMP_ANIM_MODE_TIMING;
  CMP_TEST_EXPECT(m3_sheet_set_open(&sheet, CMP_FALSE), CMP_ERR_STATE);

  {
    CMPSemantics semantics;

    CMP_TEST_EXPECT(sheet.widget.vtable->get_semantics(NULL, &semantics),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(sheet.widget.vtable->get_semantics(sheet.widget.ctx, NULL),
                   CMP_ERR_INVALID_ARGUMENT);

    sheet.widget.flags = CMP_WIDGET_FLAG_DISABLED | CMP_WIDGET_FLAG_FOCUSABLE;
    CMP_TEST_OK(
        sheet.widget.vtable->get_semantics(sheet.widget.ctx, &semantics));
    CMP_TEST_ASSERT(semantics.role == CMP_SEMANTIC_NONE);
    CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_DISABLED) != 0u);
    CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_FOCUSABLE) != 0u);
    CMP_TEST_ASSERT(semantics.utf8_label == NULL);
    CMP_TEST_ASSERT(semantics.utf8_hint == NULL);
    CMP_TEST_ASSERT(semantics.utf8_value == NULL);
    sheet.widget.flags = 0u;
  }

  CMP_TEST_EXPECT(sheet.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(sheet.widget.vtable->destroy(sheet.widget.ctx));
  CMP_TEST_ASSERT(sheet.widget.vtable == NULL);

  return 0;
}

static int test_sheet_measure_layout(void) {
  M3SheetStyle style;
  M3Sheet sheet;
  CMPMeasureSpec width_spec;
  CMPMeasureSpec height_spec;
  CMPSize size;
  CMPRect bounds;
  CMPRect content;

  CMP_TEST_OK(m3_sheet_style_init_standard(&style));
  style.height = 200.0f;
  style.min_height = 100.0f;
  style.max_height = 250.0f;
  style.max_width = 300.0f;
  style.padding.left = 10.0f;
  style.padding.right = 20.0f;
  style.padding.top = 5.0f;
  style.padding.bottom = 15.0f;

  CMP_TEST_OK(m3_sheet_init(&sheet, &style));

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 500.0f;
  height_spec.mode = CMP_MEASURE_EXACTLY;
  height_spec.size = 600.0f;
  CMP_TEST_OK(sheet.widget.vtable->measure(sheet.widget.ctx, width_spec,
                                          height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 500.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 600.0f, 0.001f));

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 250.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 150.0f;
  CMP_TEST_OK(sheet.widget.vtable->measure(sheet.widget.ctx, width_spec,
                                          height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 250.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 150.0f, 0.001f));

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_OK(sheet.widget.vtable->measure(sheet.widget.ctx, width_spec,
                                          height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 300.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 200.0f, 0.001f));

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(sheet.widget.vtable->measure(sheet.widget.ctx, width_spec,
                                              height_spec, &size),
                 CMP_ERR_INVALID_ARGUMENT);

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 100.0f;
  CMP_TEST_EXPECT(sheet.widget.vtable->layout(sheet.widget.ctx, bounds),
                 CMP_ERR_RANGE);

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 800.0f;
  bounds.height = 600.0f;
  CMP_TEST_OK(sheet.widget.vtable->layout(sheet.widget.ctx, bounds));

  CMP_TEST_OK(m3_sheet_test_set_fail_point(M3_SHEET_TEST_FAIL_APPLY_OFFSET));
  CMP_TEST_EXPECT(sheet.widget.vtable->layout(sheet.widget.ctx, bounds),
                 CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_sheet_test_clear_fail_points());

  CMP_TEST_OK(m3_sheet_get_bounds(&sheet, &bounds));
  CMP_TEST_ASSERT(cmp_near(bounds.width, 300.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bounds.height, 200.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bounds.x, 250.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bounds.y, 400.0f, 0.001f));

  CMP_TEST_OK(m3_sheet_get_content_bounds(&sheet, &content));
  CMP_TEST_ASSERT(cmp_near(content.x, 260.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(content.y, 405.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(content.width, 270.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(content.height, 180.0f, 0.001f));

  CMP_TEST_EXPECT(m3_sheet_get_bounds(NULL, &bounds), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_sheet_get_bounds(&sheet, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_sheet_get_content_bounds(NULL, &content),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_sheet_get_content_bounds(&sheet, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  return 0;
}

static int test_sheet_paint(void) {
  M3SheetStyle style;
  M3Sheet sheet;
  TestSheetBackend backend;
  CMPGfx gfx;
  CMPPaintContext ctx;
  CMPRect overlay;
  CMPScalar expected_alpha;
  CMPBool changed;
  int rc;

  CMP_TEST_OK(m3_sheet_style_init_modal(&style));
  style.height = 200.0f;
  style.min_height = 200.0f;
  style.max_height = 200.0f;
  style.shadow_enabled = CMP_FALSE;
  style.scrim_color.a = 0.4f;

  CMP_TEST_OK(m3_sheet_init(&sheet, &style));

  overlay.x = 0.0f;
  overlay.y = 0.0f;
  overlay.width = 400.0f;
  overlay.height = 400.0f;
  CMP_TEST_OK(sheet.widget.vtable->layout(sheet.widget.ctx, overlay));

  CMP_TEST_OK(m3_sheet_test_apply_offset(
      &sheet, sheet.sheet_bounds.height * 0.5f, CMP_TRUE, &changed));
  CMP_TEST_ASSERT(changed == CMP_TRUE);

  CMP_TEST_OK(test_backend_init(&backend));
  gfx.ctx = &backend;
  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = NULL;

  ctx.gfx = &gfx;
  ctx.clip = overlay;
  ctx.dpi_scale = 1.0f;

  rc = sheet.widget.vtable->paint(sheet.widget.ctx, &ctx);
  CMP_TEST_OK(rc);
  CMP_TEST_ASSERT(backend.draw_rect_calls == 2);

  expected_alpha = 0.4f * 0.5f;
  CMP_TEST_ASSERT(cmp_near(backend.colors[0].a, expected_alpha, 0.01f));
  CMP_TEST_ASSERT(cmp_near(backend.rects[0].width, overlay.width, 0.001f));
  CMP_TEST_ASSERT(
      cmp_near(backend.rects[1].y, sheet.sheet_bounds.y + sheet.offset, 0.001f));

  CMP_TEST_OK(test_backend_init(&backend));
  style.scrim_enabled = CMP_FALSE;
  CMP_TEST_OK(m3_sheet_set_style(&sheet, &style));
  rc = sheet.widget.vtable->paint(sheet.widget.ctx, &ctx);
  CMP_TEST_OK(rc);
  CMP_TEST_ASSERT(backend.draw_rect_calls == 1);

  CMP_TEST_OK(m3_sheet_test_set_fail_point(M3_SHEET_TEST_FAIL_SCRIM_ALPHA));
  style.scrim_enabled = CMP_TRUE;
  CMP_TEST_OK(m3_sheet_set_style(&sheet, &style));
  CMP_TEST_EXPECT(sheet.widget.vtable->paint(sheet.widget.ctx, &ctx),
                 CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_sheet_test_clear_fail_points());

  CMP_TEST_OK(test_backend_init(&backend));
  gfx.vtable = &g_test_gfx_vtable;
  backend.fail_draw_rect = 1;
  style.scrim_enabled = CMP_TRUE;
  CMP_TEST_OK(m3_sheet_set_style(&sheet, &style));
  CMP_TEST_EXPECT(sheet.widget.vtable->paint(sheet.widget.ctx, &ctx), CMP_ERR_IO);
  backend.fail_draw_rect = 0;

  CMP_TEST_OK(test_backend_init(&backend));
  gfx.vtable = &g_test_gfx_vtable;
  backend.fail_draw_rect = 1;
  style.scrim_enabled = CMP_FALSE;
  CMP_TEST_OK(m3_sheet_set_style(&sheet, &style));
  CMP_TEST_EXPECT(sheet.widget.vtable->paint(sheet.widget.ctx, &ctx), CMP_ERR_IO);
  backend.fail_draw_rect = 0;

  sheet.style.corner_radius = -1.0f;
  CMP_TEST_EXPECT(sheet.widget.vtable->paint(sheet.widget.ctx, &ctx),
                 CMP_ERR_RANGE);
  sheet.style.corner_radius = style.corner_radius;

  sheet.overlay_bounds.width = -1.0f;
  CMP_TEST_EXPECT(sheet.widget.vtable->paint(sheet.widget.ctx, &ctx),
                 CMP_ERR_RANGE);
  sheet.overlay_bounds.width = overlay.width;

  sheet.sheet_bounds.height = -1.0f;
  CMP_TEST_EXPECT(sheet.widget.vtable->paint(sheet.widget.ctx, &ctx),
                 CMP_ERR_RANGE);
  sheet.sheet_bounds.height = overlay.height - style.height;

  sheet.style.variant = 99u;
  CMP_TEST_EXPECT(sheet.widget.vtable->paint(sheet.widget.ctx, &ctx),
                 CMP_ERR_RANGE);
  sheet.style.variant = style.variant;

  gfx.vtable = &g_test_gfx_vtable_no_draw;
  CMP_TEST_EXPECT(sheet.widget.vtable->paint(sheet.widget.ctx, &ctx),
                 CMP_ERR_UNSUPPORTED);

  gfx.vtable = &g_test_gfx_vtable;
  style.shadow_enabled = CMP_TRUE;
  style.shadow.blur_radius = -1.0f;
  CMP_TEST_OK(m3_sheet_set_style(&sheet, &style));
  CMP_TEST_EXPECT(sheet.widget.vtable->paint(sheet.widget.ctx, &ctx),
                 CMP_ERR_RANGE);

  CMP_TEST_EXPECT(sheet.widget.vtable->paint(NULL, &ctx),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(sheet.widget.vtable->paint(sheet.widget.ctx, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  return 0;
}

static int test_sheet_event(void) {
  M3SheetStyle style;
  M3Sheet sheet;
  CMPRect overlay;
  CMPInputEvent event;
  CMPBool handled;
  TestSheetAction action;

  CMP_TEST_OK(m3_sheet_style_init_modal(&style));
  style.height = 200.0f;
  style.min_height = 200.0f;
  style.max_height = 200.0f;

  CMP_TEST_OK(m3_sheet_init(&sheet, &style));
  overlay.x = 0.0f;
  overlay.y = 0.0f;
  overlay.width = 400.0f;
  overlay.height = 400.0f;
  CMP_TEST_OK(sheet.widget.vtable->layout(sheet.widget.ctx, overlay));

  memset(&action, 0, sizeof(action));
  CMP_TEST_OK(m3_sheet_set_on_action(&sheet, test_sheet_on_action, &action));

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(action.calls == 1);
  CMP_TEST_ASSERT(action.last_action == M3_SHEET_ACTION_DISMISS);
  CMP_TEST_ASSERT(sheet.open == CMP_FALSE);

  CMP_TEST_OK(m3_sheet_set_open(&sheet, CMP_TRUE));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 200, 350));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  action.fail = 1;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_EXPECT(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled),
                 CMP_ERR_IO);
  action.fail = 0;

  CMP_TEST_OK(m3_sheet_style_init_standard(&style));
  style.height = 200.0f;
  style.min_height = 200.0f;
  style.max_height = 200.0f;
  CMP_TEST_OK(m3_sheet_set_style(&sheet, &style));
  CMP_TEST_OK(m3_sheet_set_open(&sheet, CMP_TRUE));

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(init_gesture_event(&event, CMP_INPUT_GESTURE_DRAG_START, 200.0f,
                                350.0f, 200.0f, 350.0f, 0.0f, 0.0f));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(sheet.dragging == CMP_TRUE);

  CMP_TEST_EXPECT(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled),
                 CMP_ERR_STATE);

  CMP_TEST_OK(init_gesture_event(&event, CMP_INPUT_GESTURE_DRAG_UPDATE, 200.0f,
                                360.0f, 200.0f, 350.0f, 50.0f, 0.0f));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(sheet.offset > 0.0f);

  CMP_TEST_OK(m3_sheet_test_set_fail_point(M3_SHEET_TEST_FAIL_APPLY_OFFSET));
  CMP_TEST_OK(init_gesture_event(&event, CMP_INPUT_GESTURE_DRAG_UPDATE, 200.0f,
                                360.0f, 200.0f, 350.0f, 30.0f, 0.0f));
  CMP_TEST_EXPECT(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled),
                 CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_sheet_test_clear_fail_points());

  CMP_TEST_OK(init_gesture_event(&event, CMP_INPUT_GESTURE_DRAG_END, 200.0f,
                                360.0f, 200.0f, 350.0f, 50.0f, 0.0f));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(sheet.open == CMP_TRUE);

  CMP_TEST_OK(init_gesture_event(&event, CMP_INPUT_GESTURE_DRAG_START, 200.0f,
                                350.0f, 200.0f, 350.0f, 0.0f, 0.0f));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_OK(init_gesture_event(&event, CMP_INPUT_GESTURE_DRAG_UPDATE, 200.0f,
                                380.0f, 200.0f, 350.0f, 120.0f, 0.0f));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_OK(init_gesture_event(&event, CMP_INPUT_GESTURE_DRAG_END, 200.0f,
                                380.0f, 200.0f, 350.0f, 120.0f, 0.0f));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(sheet.open == CMP_FALSE);

  CMP_TEST_OK(m3_sheet_set_open(&sheet, CMP_TRUE));
  sheet.dragging = CMP_TRUE;
  sheet.offset = 10.0f;
  CMP_TEST_OK(init_gesture_event(&event, CMP_INPUT_GESTURE_DRAG_END, 200.0f,
                                350.0f, 200.0f, 350.0f, 10.0f, 2000.0f));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(sheet.open == CMP_FALSE);

  CMP_TEST_OK(init_gesture_event(&event, CMP_INPUT_GESTURE_DRAG_START, 10.0f,
                                10.0f, 10.0f, 10.0f, 0.0f, 0.0f));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  CMP_TEST_ASSERT(sheet.dragging == CMP_FALSE);

  CMP_TEST_OK(init_gesture_event(&event, CMP_INPUT_GESTURE_DRAG_END, 10.0f, 10.0f,
                                10.0f, 10.0f, 0.0f, 0.0f));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(init_gesture_event(&event, CMP_INPUT_GESTURE_DRAG_UPDATE, 10.0f,
                                10.0f, 10.0f, 10.0f, 5.0f, 0.0f));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  sheet.dragging = CMP_TRUE;
  sheet.sheet_bounds.height = 0.0f;
  CMP_TEST_OK(init_gesture_event(&event, CMP_INPUT_GESTURE_DRAG_END, 200.0f,
                                360.0f, 200.0f, 350.0f, 50.0f, 0.0f));
  CMP_TEST_EXPECT(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled),
                 CMP_ERR_RANGE);
  sheet.sheet_bounds.height = 200.0f;

  sheet.dragging = CMP_TRUE;
  CMP_TEST_OK(init_gesture_event(&event, CMP_INPUT_GESTURE_FLING, 200.0f, 360.0f,
                                200.0f, 350.0f, 50.0f, 2000.0f));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(sheet.open == CMP_FALSE);

  CMP_TEST_OK(m3_sheet_set_open(&sheet, CMP_TRUE));
  sheet.dragging = CMP_TRUE;
  CMP_TEST_OK(init_gesture_event(&event, CMP_INPUT_GESTURE_FLING, 200.0f, 360.0f,
                                200.0f, 350.0f, 10.0f, 0.0f));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(sheet.open == CMP_TRUE);

  sheet.style.scrim_enabled = CMP_TRUE;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 10, 10));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_SCROLL, 10, 10));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  sheet.style.scrim_enabled = CMP_FALSE;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 200, 350));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_SCROLL, 200, 350));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  sheet.widget.flags |= CMP_WIDGET_FLAG_HIDDEN;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 200, 350));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  sheet.widget.flags &= (cmp_u32)~CMP_WIDGET_FLAG_HIDDEN;

  sheet.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 200, 350));
  CMP_TEST_OK(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  sheet.widget.flags &= (cmp_u32)~CMP_WIDGET_FLAG_DISABLED;

  sheet.style.corner_radius = -1.0f;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 200, 350));
  CMP_TEST_EXPECT(sheet.widget.vtable->event(sheet.widget.ctx, &event, &handled),
                 CMP_ERR_RANGE);

  CMP_TEST_EXPECT(sheet.widget.vtable->event(NULL, &event, &handled),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(sheet.widget.vtable->event(sheet.widget.ctx, NULL, &handled),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(sheet.widget.vtable->event(sheet.widget.ctx, &event, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  return 0;
}

static int test_sheet_step(void) {
  M3SheetStyle style;
  M3Sheet sheet;
  CMPRect overlay;
  CMPBool changed;

  CMP_TEST_OK(m3_sheet_style_init_standard(&style));
  style.height = 200.0f;
  style.min_height = 200.0f;
  style.max_height = 200.0f;

  CMP_TEST_OK(m3_sheet_init(&sheet, &style));
  overlay.x = 0.0f;
  overlay.y = 0.0f;
  overlay.width = 400.0f;
  overlay.height = 400.0f;
  CMP_TEST_OK(sheet.widget.vtable->layout(sheet.widget.ctx, overlay));

  CMP_TEST_EXPECT(m3_sheet_step(NULL, 0.1f, &changed), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_sheet_step(&sheet, 0.1f, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_sheet_step(&sheet, -0.1f, &changed), CMP_ERR_RANGE);

  sheet.style.spring_stiffness = 0.0f;
  CMP_TEST_EXPECT(m3_sheet_step(&sheet, 0.1f, &changed), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_sheet_style_init_standard(&style));
  style.height = 200.0f;
  style.min_height = 200.0f;
  style.max_height = 200.0f;
  CMP_TEST_OK(m3_sheet_set_style(&sheet, &style));

  CMP_TEST_OK(m3_sheet_set_open(&sheet, CMP_FALSE));
  CMP_TEST_OK(m3_sheet_step(&sheet, 0.1f, &changed));
  CMP_TEST_ASSERT(changed == CMP_TRUE);
  CMP_TEST_ASSERT(sheet.offset >= 0.0f);

  sheet.anim.mode = CMP_ANIM_MODE_NONE;
  sheet.anim.running = CMP_TRUE;
  CMP_TEST_EXPECT(m3_sheet_step(&sheet, 0.1f, &changed), CMP_ERR_STATE);

  sheet.anim.mode = CMP_ANIM_MODE_SPRING;
  sheet.anim.running = CMP_FALSE;
  sheet.open = CMP_FALSE;
  sheet.offset = sheet.sheet_bounds.height;
  CMP_TEST_OK(m3_sheet_step(&sheet, 0.0f, &changed));
  CMP_TEST_ASSERT(sheet.widget.flags & CMP_WIDGET_FLAG_HIDDEN);

  return 0;
}

int main(void) {
  int rc;

  rc = test_sheet_helpers();
  if (rc != 0) {
    return rc;
  }
  rc = test_sheet_init_and_setters();
  if (rc != 0) {
    return rc;
  }
  rc = test_sheet_measure_layout();
  if (rc != 0) {
    return rc;
  }
  rc = test_sheet_paint();
  if (rc != 0) {
    return rc;
  }
  rc = test_sheet_event();
  if (rc != 0) {
    return rc;
  }
  rc = test_sheet_step();
  if (rc != 0) {
    return rc;
  }

  return 0;
}
