#include "m3/m3_pull_to_refresh.h"
#include "test_utils.h"
#include <string.h>

static int mock_draw_rect(void *ctx, const CMPRect *rect, CMPColor color,
                          CMPScalar radius) {
  return CMP_OK;
}
static const CMPGfxVTable mock_gfx_vtable = {
    NULL,           /* begin_frame */
    NULL,           /* end_frame */
    NULL,           /* clear */
    mock_draw_rect, /* draw_rect */
    NULL,           /* draw_line */
    NULL,           /* draw_path */
    NULL,           /* push_clip */
    NULL,           /* pop_clip */
    NULL,           /* set_transform */
    NULL,           /* create_texture */
    NULL,           /* destroy_texture */
    NULL            /* draw_texture */
};

static int mock_on_refresh(void *ctx) {
  int *called = (int *)ctx;
  if (called) {
    *called += 1;
  }
  return CMP_OK;
}

static int test_ptr_style(void) {
  M3PullToRefreshStyle style;
  CMP_TEST_EXPECT(m3_pull_to_refresh_style_init(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_pull_to_refresh_style_init(&style));
  CMP_TEST_ASSERT(style.size == 48.0f);
  CMP_TEST_ASSERT(style.max_drag == 80.0f);
  return 0;
}

static int test_ptr_init(void) {
  M3PullToRefresh ptr;
  M3PullToRefreshStyle style;
  CMPWidget child = {0};

  m3_pull_to_refresh_style_init(&style);

  CMP_TEST_EXPECT(m3_pull_to_refresh_init(NULL, &style, &child),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_pull_to_refresh_init(&ptr, NULL, &child),
                  CMP_ERR_INVALID_ARGUMENT);

  style.size = 0.0f;
  CMP_TEST_EXPECT(m3_pull_to_refresh_init(&ptr, &style, &child),
                  CMP_ERR_INVALID_ARGUMENT);
  style.size = 48.0f;

  style.max_drag = 0.0f;
  CMP_TEST_EXPECT(m3_pull_to_refresh_init(&ptr, &style, &child),
                  CMP_ERR_INVALID_ARGUMENT);
  style.max_drag = 80.0f;

  CMP_TEST_OK(m3_pull_to_refresh_init(&ptr, &style, &child));
  CMP_TEST_ASSERT(ptr.child == &child);
  CMP_TEST_ASSERT(ptr.scroll_parent.ctx == &ptr);
  CMP_TEST_ASSERT(ptr.scroll_parent.vtable != NULL);

  return 0;
}

static int test_ptr_methods(void) {
  M3PullToRefresh ptr;
  M3PullToRefreshStyle style;
  int refresh_called = 0;
  CMPWidget new_child = {0};

  m3_pull_to_refresh_style_init(&style);
  m3_pull_to_refresh_init(&ptr, &style, NULL);

  CMP_TEST_EXPECT(m3_pull_to_refresh_set_refreshing(NULL, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_pull_to_refresh_set_refreshing(&ptr, CMP_TRUE));
  CMP_TEST_ASSERT(ptr.is_refreshing == CMP_TRUE);

  ptr.drag_offset = 10.0f;
  CMP_TEST_OK(m3_pull_to_refresh_set_refreshing(&ptr, CMP_FALSE));
  CMP_TEST_ASSERT(ptr.is_refreshing == CMP_FALSE);
  CMP_TEST_ASSERT(ptr.drag_offset == 0.0f); /* Should clear drag offset */

  CMP_TEST_EXPECT(
      m3_pull_to_refresh_set_on_refresh(NULL, mock_on_refresh, &refresh_called),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_pull_to_refresh_set_on_refresh(&ptr, mock_on_refresh,
                                                &refresh_called));
  CMP_TEST_ASSERT(ptr.on_refresh == mock_on_refresh);
  CMP_TEST_ASSERT(ptr.on_refresh_ctx == &refresh_called);

  CMP_TEST_EXPECT(m3_pull_to_refresh_set_child(NULL, &new_child),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_pull_to_refresh_set_child(&ptr, &new_child));
  CMP_TEST_ASSERT(ptr.child == &new_child);

  return 0;
}

static int mock_child_measure(void *widget, CMPMeasureSpec width,
                              CMPMeasureSpec height, CMPSize *out_size) {
  out_size->width = 100.0f;
  out_size->height = 200.0f;
  return CMP_OK;
}
static int mock_child_layout(void *widget, CMPRect bounds) { return CMP_OK; }
static int mock_child_paint(void *widget, CMPPaintContext *ctx) {
  return CMP_OK;
}
static int mock_child_event(void *widget, const CMPInputEvent *event,
                            CMPBool *out_handled) {
  return CMP_OK;
}
static int mock_child_destroy(void *widget) { return CMP_OK; }

static const CMPWidgetVTable mock_child_vtable = {mock_child_measure,
                                                  mock_child_layout,
                                                  mock_child_paint,
                                                  mock_child_event,
                                                  NULL,
                                                  mock_child_destroy};

static int test_ptr_widget(void) {
  M3PullToRefresh ptr;
  M3PullToRefreshStyle style;
  CMPWidget child;
  CMPSize size;
  CMPRect bounds = {0, 0, 300, 500};
  CMPPaintContext paint_ctx = {0};
  CMPGfx gfx = {0};
  CMPInputEvent event = {0};
  CMPBool handled;
  CMPSemantics semantics;
  CMPMeasureSpec unspec = {CMP_MEASURE_UNSPECIFIED, 0.0f};
  CMPMeasureSpec exact100 = {CMP_MEASURE_EXACTLY, 100.0f};
  int refresh_called = 0;

  memset(&child, 0, sizeof(child));
  child.vtable = &mock_child_vtable;

  m3_pull_to_refresh_style_init(&style);
  m3_pull_to_refresh_init(&ptr, &style, &child);
  m3_pull_to_refresh_set_on_refresh(&ptr, mock_on_refresh, &refresh_called);

  /* Measure */
  CMP_TEST_EXPECT(ptr.widget.vtable->measure(NULL, unspec, unspec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(ptr.widget.vtable->measure(&ptr, unspec, unspec, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(ptr.widget.vtable->measure(&ptr, unspec, unspec, &size));
  CMP_TEST_ASSERT(size.width == 100.0f && size.height == 200.0f);
  CMP_TEST_OK(ptr.widget.vtable->measure(&ptr, exact100, exact100, &size));

  /* Layout */
  CMP_TEST_EXPECT(ptr.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(ptr.widget.vtable->layout(&ptr, bounds));

  /* Paint */
  paint_ctx.gfx = &gfx;
  gfx.vtable = &mock_gfx_vtable;
  CMP_TEST_EXPECT(ptr.widget.vtable->paint(NULL, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(ptr.widget.vtable->paint(&ptr, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Paint normal */
  CMP_TEST_OK(ptr.widget.vtable->paint(&ptr, &paint_ctx));

  /* Paint with drag offset */
  ptr.drag_offset = 20.0f;
  CMP_TEST_OK(ptr.widget.vtable->paint(&ptr, &paint_ctx));

  /* Paint refreshing */
  ptr.is_refreshing = CMP_TRUE;
  CMP_TEST_OK(ptr.widget.vtable->paint(&ptr, &paint_ctx));

  /* Event */
  CMP_TEST_EXPECT(ptr.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(ptr.widget.vtable->event(&ptr, NULL, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(ptr.widget.vtable->event(&ptr, &event, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  event.type = CMP_INPUT_POINTER_DOWN;
  CMP_TEST_OK(ptr.widget.vtable->event(&ptr, &event, &handled));

  event.type = CMP_INPUT_POINTER_UP;
  ptr.is_refreshing = CMP_FALSE;
  ptr.drag_offset = style.max_drag;
  refresh_called = 0;
  CMP_TEST_OK(ptr.widget.vtable->event(&ptr, &event, &handled));
  CMP_TEST_ASSERT(ptr.is_refreshing == CMP_TRUE);
  CMP_TEST_ASSERT(refresh_called == 1);

  ptr.is_refreshing = CMP_FALSE;
  ptr.drag_offset = 10.0f;
  CMP_TEST_OK(ptr.widget.vtable->event(&ptr, &event, &handled));
  CMP_TEST_ASSERT(ptr.is_refreshing == CMP_FALSE);
  CMP_TEST_ASSERT(ptr.drag_offset == 0.0f);

  /* Semantics */
  CMP_TEST_EXPECT(ptr.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(ptr.widget.vtable->get_semantics(&ptr, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(ptr.widget.vtable->get_semantics(&ptr, &semantics));

  /* Destroy */
  CMP_TEST_EXPECT(ptr.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(ptr.widget.vtable->destroy(&ptr));

  return 0;
}

static int test_ptr_scroll_parent(void) {
  M3PullToRefresh ptr;
  M3PullToRefreshStyle style;
  CMPScrollDelta delta, out_consumed, child_consumed;

  m3_pull_to_refresh_style_init(&style);
  m3_pull_to_refresh_init(&ptr, &style, NULL);

  /* pre_scroll invalid args */
  CMP_TEST_EXPECT(
      ptr.scroll_parent.vtable->pre_scroll(NULL, &delta, &out_consumed),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      ptr.scroll_parent.vtable->pre_scroll(&ptr, NULL, &out_consumed),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(ptr.scroll_parent.vtable->pre_scroll(&ptr, &delta, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* post_scroll invalid args */
  CMP_TEST_EXPECT(ptr.scroll_parent.vtable->post_scroll(
                      NULL, &delta, &child_consumed, &out_consumed),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(ptr.scroll_parent.vtable->post_scroll(
                      &ptr, NULL, &child_consumed, &out_consumed),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      ptr.scroll_parent.vtable->post_scroll(&ptr, &delta, NULL, &out_consumed),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(ptr.scroll_parent.vtable->post_scroll(&ptr, &delta,
                                                        &child_consumed, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Test pulling down (delta.y < 0 implies child content moves down, which hits
   * top) */
  delta.x = 0;
  delta.y = -50.0f;
  child_consumed.x = 0;
  child_consumed.y = 0;
  CMP_TEST_OK(ptr.scroll_parent.vtable->post_scroll(
      &ptr, &delta, &child_consumed, &out_consumed));
  CMP_TEST_ASSERT(out_consumed.y == -50.0f);
  CMP_TEST_ASSERT(ptr.drag_offset == 50.0f);

  /* Pulling down more, maxing out */
  delta.y = -50.0f;
  CMP_TEST_OK(ptr.scroll_parent.vtable->post_scroll(
      &ptr, &delta, &child_consumed, &out_consumed));
  CMP_TEST_ASSERT(ptr.drag_offset == style.max_drag);

  /* Test pre_scroll consuming pushing up */
  delta.y = 20.0f;
  CMP_TEST_OK(
      ptr.scroll_parent.vtable->pre_scroll(&ptr, &delta, &out_consumed));
  CMP_TEST_ASSERT(out_consumed.y == 20.0f);
  CMP_TEST_ASSERT(ptr.drag_offset == style.max_drag - 20.0f);

  /* Test pre_scroll consuming everything and capping */
  delta.y = 100.0f;
  CMP_TEST_OK(
      ptr.scroll_parent.vtable->pre_scroll(&ptr, &delta, &out_consumed));
  CMP_TEST_ASSERT(out_consumed.y == style.max_drag - 20.0f);
  CMP_TEST_ASSERT(ptr.drag_offset == 0.0f);

  /* post_scroll doesn't accumulate if refreshing */
  ptr.is_refreshing = CMP_TRUE;
  delta.y = -50.0f;
  CMP_TEST_OK(ptr.scroll_parent.vtable->post_scroll(
      &ptr, &delta, &child_consumed, &out_consumed));
  CMP_TEST_ASSERT(ptr.drag_offset == 0.0f);

  return 0;
}

int main(void) {
  CMP_TEST_ASSERT(test_ptr_style() == 0);
  CMP_TEST_ASSERT(test_ptr_init() == 0);
  CMP_TEST_ASSERT(test_ptr_methods() == 0);
  CMP_TEST_ASSERT(test_ptr_widget() == 0);
  CMP_TEST_ASSERT(test_ptr_scroll_parent() == 0);
  return 0;
}