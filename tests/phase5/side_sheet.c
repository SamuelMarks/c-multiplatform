#include "cmpc/cmp_core.h"
#include "m3/m3_sheet.h"
#include "test_utils.h"

#include <stdio.h>
#include <string.h>

typedef struct TestSideSheetBackend {
  int draw_rect_calls;
  int fail_draw_rect;
  CMPRect last_rect;
  CMPColor last_rect_color;
  CMPScalar last_corner;
} TestSideSheetBackend;

static void test_backend_init(TestSideSheetBackend *backend) {
  memset(backend, 0, sizeof(*backend));
}

static int test_gfx_draw_rect(void *gfx, const CMPRect *rect, CMPColor color,
                              CMPScalar corner_radius) {
  TestSideSheetBackend *backend = (TestSideSheetBackend *)gfx;
  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_rect_color = color;
  backend->last_corner = corner_radius;
  if (backend->fail_draw_rect) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static const CMPGfxVTable g_test_gfx_vtable = {
    NULL, NULL, NULL, test_gfx_draw_rect, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL};

static int test_action_calls = 0;
static int test_action_last_action = 0;

static int test_side_sheet_action(void *ctx, struct M3SideSheet *sheet,
                                  cmp_u32 action) {
  test_action_calls += 1;
  test_action_last_action = action;
  if (ctx != NULL) {
    int *fail = (int *)ctx;
    if (*fail)
      return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_side_sheet(void) {
  M3SideSheetStyle style;
  M3SideSheet sheet;
  CMPMeasureSpec spec_any = {CMP_MEASURE_UNSPECIFIED, 0};
  CMPMeasureSpec spec_exact = {CMP_MEASURE_EXACTLY, 500.0f};
  CMPMeasureSpec spec_most = {CMP_MEASURE_AT_MOST, 500.0f};
  CMPSize size;
  CMPRect bounds = {0.0f, 0.0f, 800.0f, 600.0f};
  CMPPaintContext ctx;
  CMPGfx gfx;
  TestSideSheetBackend backend;
  CMPInputEvent event;
  CMPBool handled;
  CMPSemantics semantics;
  CMPBool changed;
  CMPBool is_open;
  int action_fail = 1;

  test_backend_init(&backend);
  memset(&gfx, 0, sizeof(gfx));
  gfx.vtable = &g_test_gfx_vtable;
  gfx.ctx = &backend;

  memset(&ctx, 0, sizeof(ctx));
  ctx.gfx = &gfx;
  ctx.clip = bounds;
  ctx.dpi_scale = 1.0f;

  /* Style init */
  CMP_TEST_EXPECT(m3_side_sheet_style_init_standard(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_side_sheet_style_init_standard(&style));
  CMP_TEST_EXPECT(m3_side_sheet_style_init_modal(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_side_sheet_style_init_modal(&style));

  /* Init */
  CMP_TEST_EXPECT(m3_side_sheet_init(NULL, &style), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_side_sheet_init(&sheet, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_side_sheet_init(&sheet, &style));

  /* Invalid styles test via set_style */
  CMP_TEST_EXPECT(m3_side_sheet_set_style(NULL, &style),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_side_sheet_set_style(&sheet, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  style.variant = 99u;
  CMP_TEST_EXPECT(m3_side_sheet_set_style(&sheet, &style), CMP_ERR_RANGE);
  style.variant = M3_SIDE_SHEET_VARIANT_MODAL;

  style.placement = 99u;
  CMP_TEST_EXPECT(m3_side_sheet_set_style(&sheet, &style), CMP_ERR_RANGE);
  style.placement = M3_SIDE_SHEET_PLACEMENT_END;

  style.width = -1.0f;
  CMP_TEST_EXPECT(m3_side_sheet_set_style(&sheet, &style), CMP_ERR_RANGE);
  style.width = 400.0f;

  style.min_width = 500.0f;
  CMP_TEST_EXPECT(m3_side_sheet_set_style(&sheet, &style), CMP_ERR_RANGE);
  style.min_width = 100.0f;

  style.max_width = 200.0f;
  CMP_TEST_EXPECT(m3_side_sheet_set_style(&sheet, &style), CMP_ERR_RANGE);
  style.max_width = 0.0f;

  style.padding.left = -1.0f;
  CMP_TEST_EXPECT(m3_side_sheet_set_style(&sheet, &style), CMP_ERR_RANGE);
  style.padding.left = 0.0f;

  style.corner_radius = -1.0f;
  CMP_TEST_EXPECT(m3_side_sheet_set_style(&sheet, &style), CMP_ERR_RANGE);
  style.corner_radius = 0.0f;

  CMP_TEST_OK(m3_side_sheet_set_style(&sheet, &style));

  /* Measure */
  CMP_TEST_EXPECT(sheet.widget.vtable->measure(NULL, spec_any, spec_any, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      sheet.widget.vtable->measure(&sheet, spec_any, spec_any, NULL),
      CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(sheet.widget.vtable->measure(&sheet, spec_any, spec_any, &size));
  CMP_TEST_ASSERT(size.width == 400.0f);
  CMP_TEST_ASSERT(size.height == 0.0f);

  CMP_TEST_OK(
      sheet.widget.vtable->measure(&sheet, spec_exact, spec_exact, &size));
  CMP_TEST_ASSERT(size.width == 500.0f);
  CMP_TEST_ASSERT(size.height == 500.0f);

  CMP_TEST_OK(
      sheet.widget.vtable->measure(&sheet, spec_most, spec_most, &size));
  CMP_TEST_ASSERT(size.width == 400.0f);
  CMP_TEST_ASSERT(size.height == 0.0f);

  /* Layout */
  CMP_TEST_EXPECT(sheet.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(sheet.widget.vtable->layout(&sheet, bounds));

  /* Verify layout calculations for placement END */
  CMP_TEST_ASSERT(sheet.sheet_bounds.x ==
                  bounds.x + bounds.width - sheet.style.width);
  CMP_TEST_ASSERT(sheet.offset == sheet.style.width); /* Closed offset */

  /* Open and layout again */
  CMP_TEST_OK(m3_side_sheet_set_open(&sheet, CMP_TRUE));
  /* Wait for spring to finish */
  int loop_count = 0;
  while (sheet.anim.mode != CMP_ANIM_MODE_NONE && loop_count < 1000) {
    CMP_TEST_OK(m3_side_sheet_step(&sheet, 0.016f, &changed));
    loop_count++;
  }
  CMP_TEST_OK(sheet.widget.vtable->layout(&sheet, bounds));
  CMP_TEST_ASSERT(sheet.offset == 0.0f); /* Open offset */

  /* Layout calculations for placement START */
  style.placement = M3_SIDE_SHEET_PLACEMENT_START;
  CMP_TEST_OK(m3_side_sheet_set_style(&sheet, &style));
  CMP_TEST_OK(m3_side_sheet_set_open(&sheet, CMP_FALSE));
  loop_count = 0;
  while (sheet.anim.mode != CMP_ANIM_MODE_NONE && loop_count < 1000) {
    CMP_TEST_OK(m3_side_sheet_step(&sheet, 0.016f, &changed));
    loop_count++;
  }
  CMP_TEST_OK(sheet.widget.vtable->layout(&sheet, bounds));
  CMP_TEST_ASSERT(sheet.sheet_bounds.x == bounds.x);
  CMP_TEST_ASSERT(sheet.offset == -sheet.style.width); /* Closed offset */

  /* Paint */
  CMP_TEST_EXPECT(sheet.widget.vtable->paint(NULL, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(sheet.widget.vtable->paint(&sheet, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Paint without gfx/vtable */
  ctx.gfx = NULL;
  CMP_TEST_EXPECT(sheet.widget.vtable->paint(&sheet, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  ctx.gfx = &gfx;

  gfx.vtable = NULL;
  CMP_TEST_EXPECT(sheet.widget.vtable->paint(&sheet, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_gfx_vtable;

  CMP_TEST_OK(sheet.widget.vtable->paint(&sheet, &ctx));

  /* Test offset clipping */
  sheet.offset = -1000.0f;
  CMP_TEST_OK(sheet.widget.vtable->paint(&sheet, &ctx));
  sheet.offset = 1000.0f;
  style.placement = M3_SIDE_SHEET_PLACEMENT_END;
  CMP_TEST_OK(m3_side_sheet_set_style(&sheet, &style));
  CMP_TEST_OK(sheet.widget.vtable->paint(&sheet, &ctx));

  /* Test scrim */
  sheet.offset = 100.0f;
  CMP_TEST_OK(sheet.widget.vtable->paint(&sheet, &ctx));

  backend.fail_draw_rect = 1;
  CMP_TEST_EXPECT(sheet.widget.vtable->paint(&sheet, &ctx), CMP_ERR_IO);
  backend.fail_draw_rect = 0;

  /* Event */
  CMP_TEST_EXPECT(sheet.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);

  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_POINTER_DOWN;
  event.data.pointer.x = 0;
  event.data.pointer.y = 0;

  CMP_TEST_OK(m3_side_sheet_set_open(&sheet, CMP_FALSE));
  CMP_TEST_OK(sheet.widget.vtable->layout(&sheet, bounds));
  CMP_TEST_OK(sheet.widget.vtable->event(&sheet, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(m3_side_sheet_set_open(&sheet, CMP_TRUE));
  sheet.offset = 0.0f;
  CMP_TEST_OK(sheet.widget.vtable->layout(&sheet, bounds));

  CMP_TEST_OK(
      m3_side_sheet_set_on_action(&sheet, test_side_sheet_action, NULL));

  /* Tap outside (scrim) */
  event.data.pointer.x =
      bounds.x + 10.0f; /* Since placement is END, x=10 is outside */
  CMP_TEST_OK(sheet.widget.vtable->event(&sheet, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(sheet.open == CMP_FALSE);

  /* Drag events */
  CMP_TEST_OK(m3_side_sheet_set_open(&sheet, CMP_TRUE));
  sheet.offset = 0.0f;
  event.type = CMP_INPUT_GESTURE_DRAG_START;
  event.data.gesture.start_x =
      bounds.x + bounds.width - 10.0f; /* Inside sheet */
  event.data.gesture.start_y = bounds.y + 10.0f;
  CMP_TEST_OK(sheet.widget.vtable->event(&sheet, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(sheet.dragging == CMP_TRUE);

  event.type = CMP_INPUT_GESTURE_DRAG_UPDATE;
  event.data.gesture.total_x = 50.0f;
  CMP_TEST_OK(sheet.widget.vtable->event(&sheet, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(sheet.offset == 50.0f);

  event.type = CMP_INPUT_GESTURE_DRAG_END;
  event.data.gesture.velocity_x =
      2000.0f; /* Fling right (dismiss for END placement) */
  CMP_TEST_OK(sheet.widget.vtable->event(&sheet, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(sheet.dragging == CMP_FALSE);
  CMP_TEST_ASSERT(sheet.open == CMP_FALSE);

  /* Animation step */
  CMP_TEST_EXPECT(m3_side_sheet_step(NULL, 0.1f, &changed),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_side_sheet_step(&sheet, 0.1f, &changed));

  /* Get bounds */
  CMPRect content_bounds;
  CMP_TEST_EXPECT(m3_side_sheet_get_bounds(NULL, &content_bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_side_sheet_get_content_bounds(NULL, &content_bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_side_sheet_get_bounds(&sheet, &content_bounds));
  CMP_TEST_OK(m3_side_sheet_get_content_bounds(&sheet, &content_bounds));

  /* Semantics */
  CMP_TEST_EXPECT(sheet.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(sheet.widget.vtable->get_semantics(&sheet, &semantics));
  CMP_TEST_ASSERT(semantics.role == CMP_SEMANTIC_NONE);

  /* Destroy */
  CMP_TEST_EXPECT(sheet.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(sheet.widget.vtable->destroy(&sheet));

  /* Extra drag logic branches */
  CMP_TEST_OK(m3_side_sheet_set_open(&sheet, CMP_TRUE));
  sheet.offset = 0.0f;
  style.placement = M3_SIDE_SHEET_PLACEMENT_START;
  CMP_TEST_OK(m3_side_sheet_set_style(&sheet, &style));
  CMP_TEST_OK(sheet.widget.vtable->layout(&sheet, bounds));

  event.type = CMP_INPUT_GESTURE_DRAG_START;
  event.data.gesture.start_x = bounds.x + 10.0f;
  CMP_TEST_OK(sheet.widget.vtable->event(&sheet, &event, &handled));

  event.type = CMP_INPUT_GESTURE_DRAG_UPDATE;
  event.data.gesture.total_x = -50.0f; /* Drag left */
  CMP_TEST_OK(sheet.widget.vtable->event(&sheet, &event, &handled));

  event.type = CMP_INPUT_GESTURE_DRAG_END;
  event.data.gesture.velocity_x = -2000.0f; /* Fling left */
  CMP_TEST_OK(sheet.widget.vtable->event(&sheet, &event, &handled));
  CMP_TEST_ASSERT(sheet.open == CMP_FALSE);

  /* Drag but over-clamped */
  sheet.open = CMP_TRUE;
  sheet.offset = 0.0f;
  event.type = CMP_INPUT_GESTURE_DRAG_START;
  event.data.gesture.start_x = bounds.x + 10.0f;
  CMP_TEST_OK(sheet.widget.vtable->event(&sheet, &event, &handled));
  event.type = CMP_INPUT_GESTURE_DRAG_UPDATE;
  event.data.gesture.total_x =
      50.0f; /* Drag right when start placed is overscroll */
  CMP_TEST_OK(sheet.widget.vtable->event(&sheet, &event, &handled));
  CMP_TEST_ASSERT(sheet.offset == 0.0f); /* Clamped */

  event.type = CMP_INPUT_GESTURE_DRAG_END;
  event.data.gesture.velocity_x = 2000.0f; /* Fling right */
  CMP_TEST_OK(sheet.widget.vtable->event(&sheet, &event, &handled));
  CMP_TEST_ASSERT(sheet.open == CMP_TRUE);

  return 0;
}

int main(void) {
  if (test_side_sheet() != 0) {
    return 1;
  }
  return 0;
}