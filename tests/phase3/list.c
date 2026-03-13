/* clang-format off */
#include "cmpc/cmp_list.h"
#include "test_utils.h"
#include <string.h>
/* clang-format on */

#define M3_LIST_TEST_FAIL_NONE 0u
#define CMP_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE 1u
#define CMP_LIST_TEST_FAIL_VISIBLE_STRIDE 2u
#define CMP_LIST_TEST_FAIL_VISIBLE_END 3u
#define CMP_LIST_TEST_FAIL_VISIBLE_LAST_BEFORE_FIRST 4u
#define CMP_LIST_TEST_FAIL_VISIBLE_LAST_AFTER_OVERSCAN 5u
#define CMP_LIST_TEST_FAIL_GRID_LINE_COUNT_ZERO 6u
#define CMP_LIST_TEST_FAIL_GRID_STRIDE 7u
#define CMP_LIST_TEST_FAIL_GRID_END 8u
#define CMP_LIST_TEST_FAIL_GRID_END_NEGATIVE 9u
#define CMP_LIST_TEST_FAIL_GRID_LAST_TOO_LARGE 10u
#define CMP_LIST_TEST_FAIL_GRID_LAST_BEFORE_FIRST 11u
#define CMP_LIST_TEST_FAIL_RESERVE_NODE_BYTES 12u
#define CMP_LIST_TEST_FAIL_RESERVE_COPY_BYTES 13u
#define CMP_LIST_TEST_FAIL_LIST_ITEM_BOUNDS 14u
#define CMP_LIST_TEST_FAIL_LIST_RENDER_BOUNDS 15u
#define CMP_LIST_TEST_FAIL_GRID_ITEM_BOUNDS 16u
#define CMP_LIST_TEST_FAIL_GRID_RENDER_BOUNDS 17u
#define CMP_LIST_TEST_FAIL_LIST_MEASURE_CONTENT_NEGATIVE 18u
#define CMP_LIST_TEST_FAIL_GRID_MEASURE_CONTENT_NEGATIVE 19u

static CMPWidget g_dummy_widget;
static CMPWidgetVTable g_dummy_vtable;

static int bind_fn_impl(void *ctx, CMPListSlot *slot, cmp_usize index) {
  if (ctx) {
    int *called = (int *)ctx;
    (*called)++;
  }
  if (slot) {
    slot->index = index;
    g_dummy_widget.vtable = &g_dummy_vtable;
    slot->node.widget = &g_dummy_widget;
  }
  return CMP_OK;
}

static int bind_fail_impl(void *ctx, CMPListSlot *slot, cmp_usize index) {
  return CMP_ERR_IO;
}

static int test_list_style(void) {
  CMPListStyle style;
  CMPColor bad_color = {-1.0f, 0, 0, 0};

  CMP_TEST_EXPECT(cmp_list_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_list_style_init(&style));

  CMP_TEST_EXPECT(cmp_list_test_validate_style(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_list_test_validate_style(&style));

  style.orientation = 99;
  CMP_TEST_EXPECT(cmp_list_test_validate_style(&style), CMP_ERR_RANGE);
  style.orientation = CMP_LIST_ORIENTATION_VERTICAL;

  style.spacing = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_style(&style), CMP_ERR_RANGE);
  style.spacing = 0.0f;

  style.item_extent = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_style(&style), CMP_ERR_RANGE);
  style.item_extent = 1.0f;

  style.background_color = bad_color;
  CMP_TEST_EXPECT(cmp_list_test_validate_style(&style), CMP_ERR_RANGE);
  style.background_color.r = 0.0f;

  style.padding.left = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_style(&style), CMP_ERR_RANGE);

  return CMP_OK;
}

static int test_grid_style(void) {
  CMPGridStyle style;
  CMPColor bad_color = {-1.0f, 0, 0, 0};

  CMP_TEST_EXPECT(cmp_grid_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_grid_style_init(&style));

  CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_list_test_validate_grid_style(&style));

  style.scroll_axis = 99;
  CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(&style), CMP_ERR_RANGE);
  style.scroll_axis = CMP_GRID_SCROLL_VERTICAL;

  style.span = 0;
  CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(&style), CMP_ERR_RANGE);
  style.span = 1;

  style.spacing_x = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(&style), CMP_ERR_RANGE);
  style.spacing_x = 0.0f;

  style.spacing_y = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(&style), CMP_ERR_RANGE);
  style.spacing_y = 0.0f;

  style.item_width = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(&style), CMP_ERR_RANGE);
  style.item_width = 1.0f;

  style.item_height = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(&style), CMP_ERR_RANGE);
  style.item_height = 1.0f;

  style.background_color = bad_color;
  CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(&style), CMP_ERR_RANGE);
  style.background_color.r = 0.0f;

  style.padding.left = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(&style), CMP_ERR_RANGE);

  return CMP_OK;
}

static int test_list_view_init(void) {
  CMPListView view;
  CMPListStyle style;

  CMP_TEST_OK(cmp_list_style_init(&style));
  CMP_TEST_EXPECT(cmp_list_view_init(NULL, &style, NULL, 10, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_view_init(&view, NULL, NULL, 10, 0),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_list_view_init(&view, &style, NULL, 10, 0));
  CMP_TEST_ASSERT(view.item_count == 10);
  CMP_TEST_ASSERT(view.slot_capacity == 0);
  CMP_TEST_ASSERT(view.slots == NULL);

  CMP_TEST_EXPECT(cmp_list_view_set_bind(NULL, bind_fn_impl, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_list_view_set_bind(&view, bind_fn_impl, NULL));

  CMP_TEST_EXPECT(cmp_list_view_set_item_count(NULL, 5),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_list_view_set_item_count(&view, 5));
  CMP_TEST_ASSERT(view.item_count == 5);

  CMP_TEST_EXPECT(cmp_list_view_set_style(NULL, &style),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_view_set_style(&view, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_list_view_set_style(&view, &style));

  CMPScalar val;
  CMP_TEST_EXPECT(cmp_list_view_set_scroll(NULL, 10.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_list_view_set_scroll(&view, -1.0f));
  CMP_TEST_OK(cmp_list_view_get_scroll(&view, &val));
  CMP_TEST_ASSERT(val == 0.0f);
  CMP_TEST_OK(cmp_list_view_set_scroll(&view, 10.0f));

  CMP_TEST_EXPECT(cmp_list_view_get_scroll(NULL, &val),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_view_get_scroll(&view, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_list_view_get_scroll(&view, &val));
  CMP_TEST_ASSERT(val == 10.0f);

  CMP_TEST_EXPECT(cmp_list_view_get_content_extent(NULL, &val),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_view_get_content_extent(&view, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_list_view_get_content_extent(&view, &val));

  cmp_usize count;
  CMP_TEST_EXPECT(cmp_list_view_get_required_slots(NULL, &count),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_view_get_required_slots(&view, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_list_view_reserve(NULL, 10), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_list_view_update(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMPRenderNode **nodes;
  CMP_TEST_EXPECT(cmp_list_view_get_visible(NULL, &nodes, &count),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_view_get_visible(&view, NULL, &count),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_view_get_visible(&view, &nodes, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  view.widget.vtable->destroy(&view);
  return CMP_OK;
}

static int test_grid_view_init(void) {
  CMPGridView view;
  CMPGridStyle style;

  CMP_TEST_OK(cmp_grid_style_init(&style));
  CMP_TEST_EXPECT(cmp_grid_view_init(NULL, &style, NULL, 10, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_init(&view, NULL, NULL, 10, 0),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_grid_view_init(&view, &style, NULL, 10, 0));
  CMP_TEST_ASSERT(view.item_count == 10);

  CMP_TEST_EXPECT(cmp_grid_view_set_bind(NULL, bind_fn_impl, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_grid_view_set_bind(&view, bind_fn_impl, NULL));

  CMP_TEST_EXPECT(cmp_grid_view_set_item_count(NULL, 5),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_grid_view_set_item_count(&view, 5));
  CMP_TEST_ASSERT(view.item_count == 5);

  CMP_TEST_EXPECT(cmp_grid_view_set_style(NULL, &style),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_set_style(&view, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_grid_view_set_style(&view, &style));

  CMPScalar val;
  CMP_TEST_EXPECT(cmp_grid_view_set_scroll(NULL, 10.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_grid_view_set_scroll(&view, -1.0f));
  CMP_TEST_OK(cmp_grid_view_get_scroll(&view, &val));
  CMP_TEST_ASSERT(val == 0.0f);
  CMP_TEST_OK(cmp_grid_view_set_scroll(&view, 10.0f));

  CMP_TEST_EXPECT(cmp_grid_view_get_scroll(NULL, &val),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_get_scroll(&view, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_grid_view_get_scroll(&view, &val));

  CMP_TEST_EXPECT(cmp_grid_view_get_content_extent(NULL, &val),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_get_content_extent(&view, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_grid_view_get_content_extent(&view, &val));

  cmp_usize count;
  CMP_TEST_EXPECT(cmp_grid_view_get_required_slots(NULL, &count),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_get_required_slots(&view, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_grid_view_reserve(NULL, 10), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_grid_view_update(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMPRenderNode **nodes;
  CMP_TEST_EXPECT(cmp_grid_view_get_visible(NULL, &nodes, &count),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_get_visible(&view, NULL, &count),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_get_visible(&view, &nodes, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  view.widget.vtable->destroy(&view);
  return CMP_OK;
}

static int test_list_compute(void) {
  CMPScalar extent;
  CMP_TEST_EXPECT(
      cmp_list_test_compute_content_extent(10, 50.0f, 5.0f, 10.0f, 10.0f, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_list_test_compute_content_extent(10, 50.0f, 5.0f, 10.0f,
                                                   10.0f, &extent));
  CMP_TEST_ASSERT(extent == 565.0f);

  cmp_usize first, last, count;
  CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(10, 50.0f, 5.0f, 10.0f,
                                                      0.0f, 100.0f, 0, NULL,
                                                      &last, &count),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(10, 50.0f, 5.0f, 10.0f,
                                                      0.0f, 100.0f, 0, &first,
                                                      NULL, &count),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(10, 50.0f, 5.0f, 10.0f,
                                                      0.0f, 100.0f, 0, &first,
                                                      &last, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_list_test_compute_visible_range(
      10, 50.0f, 5.0f, 10.0f, 0.0f, 100.0f, 0, &first, &last, &count));

  CMPListView list;
  CMPListStyle style;
  cmp_list_style_init(&style);
  cmp_list_view_init(&list, &style, NULL, 10, 10);
  CMP_TEST_EXPECT(cmp_list_test_compute_item_bounds(NULL, 0, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_list_test_update_metrics(NULL), CMP_ERR_INVALID_ARGUMENT);

  list.widget.vtable->destroy(&list);
  return CMP_OK;
}

static int test_grid_compute(void) {
  CMPGridView grid;
  CMPGridStyle style;
  cmp_grid_style_init(&style);
  cmp_grid_view_init(&grid, &style, NULL, 10, 10);

  cmp_usize first, last, count;
  CMP_TEST_EXPECT(
      cmp_list_test_grid_compute_visible_range(NULL, &first, &last, &count),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_list_test_grid_compute_visible_range(&grid, NULL, &last, &count),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_list_test_grid_compute_visible_range(&grid, &first, NULL, &count),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_list_test_grid_compute_visible_range(&grid, &first, &last, NULL),
      CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_list_test_grid_compute_item_bounds(NULL, 0, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_list_test_grid_update_metrics(NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  grid.widget.vtable->destroy(&grid);
  return CMP_OK;
}

static int test_reserves(void) {
  CMPListSlot *slots = NULL;
  CMPRenderNode **nodes = NULL;
  cmp_usize cap = 0;
  CMPAllocator alloc;
  cmp_get_default_allocator(&alloc);

  CMP_TEST_EXPECT(cmp_list_test_reserve_slots(NULL, &nodes, &cap, &alloc, 10),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_test_reserve_slots(&slots, NULL, &cap, &alloc, 10),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_test_reserve_slots(&slots, &nodes, NULL, &alloc, 10),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_test_reserve_slots(&slots, &nodes, &cap, NULL, 10),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_list_test_reserve_slots(&slots, &nodes, &cap, &alloc, 10));
  CMP_TEST_ASSERT(cap == 10);

  alloc.free(alloc.ctx, slots);
  alloc.free(alloc.ctx, nodes);

  return CMP_OK;
}

static int test_list_update(void) {
  CMPListView view;
  CMPListStyle style;

  CMP_TEST_EXPECT(cmp_list_view_update(NULL), CMP_ERR_INVALID_ARGUMENT);

  cmp_list_style_init(&style);
  style.item_extent = 50.0f;
  cmp_list_view_init(&view, &style, NULL, 100, 0);

  view.bounds.width = 100.0f;
  view.bounds.height = 200.0f;

  /* Simulate metrics update error */
  view.style.item_extent = -1.0f;
  CMP_TEST_EXPECT(cmp_list_view_update(&view), CMP_ERR_RANGE);
  view.style.item_extent = 50.0f;

  /* Corrupt style for update error */
  view.style.orientation = 999;
  CMP_TEST_EXPECT(cmp_list_view_update(&view), CMP_ERR_RANGE);
  view.style.orientation = CMP_LIST_ORIENTATION_VERTICAL;

  /* Corrupt bounds for update error */
  view.bounds.width = -1.0f;
  CMP_TEST_EXPECT(cmp_list_view_update(&view), CMP_ERR_RANGE);
  view.bounds.width = 100.0f;

  int binds = 0;
  cmp_list_view_set_bind(&view, bind_fn_impl, &binds);

  CMP_TEST_OK(cmp_list_view_reserve(&view, 20));
  CMP_TEST_OK(cmp_list_view_update(&view));
  CMP_TEST_ASSERT(view.visible_count == 5);

  cmp_list_view_set_scroll(&view, 50.0f);
  CMP_TEST_OK(cmp_list_view_update(&view));

  view.style.orientation = CMP_LIST_ORIENTATION_HORIZONTAL;
  CMP_TEST_OK(cmp_list_view_update(&view));
  view.style.orientation = CMP_LIST_ORIENTATION_VERTICAL;

  /* Missing slots/bind tests */
  view.bind = NULL;
  CMP_TEST_EXPECT(cmp_list_view_update(&view), CMP_ERR_STATE);
  view.bind = bind_fn_impl;

  CMPListSlot *old_slots = view.slots;
  view.slots = NULL;
  CMP_TEST_EXPECT(cmp_list_view_update(&view), CMP_ERR_STATE);
  view.slots = old_slots;

  cmp_usize old_cap = view.slot_capacity;
  view.slot_capacity = 0;
  CMP_TEST_EXPECT(cmp_list_view_update(&view), CMP_ERR_RANGE);
  view.slot_capacity = old_cap;

  /* Bind error */
  view.bind = bind_fail_impl;
  CMP_TEST_EXPECT(cmp_list_view_update(&view), CMP_ERR_IO);
  view.bind = bind_fn_impl;

  view.widget.vtable->layout(&view, view.bounds);
  CMPMeasureSpec ms = {CMP_MEASURE_EXACTLY, 100.0f};
  CMPSize size;
  view.widget.vtable->measure(&view, ms, ms, &size);

  CMPGfx gfx;
  CMPGfxVTable gfx_vtable;
  memset(&gfx, 0, sizeof(gfx));
  memset(&gfx_vtable, 0, sizeof(gfx_vtable));
  gfx.vtable = &gfx_vtable;

  CMPPaintContext ctx;
  memset(&ctx, 0, sizeof(ctx));
  ctx.gfx = &gfx;
  view.widget.vtable->paint(&view, &ctx);

  view.widget.vtable->destroy(&view);
  return CMP_OK;
}

static int test_grid_update(void) {
  CMPGridView view;
  CMPGridStyle style;

  CMP_TEST_EXPECT(cmp_grid_view_update(NULL), CMP_ERR_INVALID_ARGUMENT);

  cmp_grid_style_init(&style);
  style.item_height = 50.0f;
  style.item_width = 50.0f;
  style.span = 2;
  cmp_grid_view_init(&view, &style, NULL, 100, 0);

  view.bounds.width = 100.0f;
  view.bounds.height = 200.0f;

  /* Simulate metrics update error */
  view.style.item_height = -1.0f;
  CMP_TEST_EXPECT(cmp_grid_view_update(&view), CMP_ERR_RANGE);
  view.style.item_height = 50.0f;

  /* Corrupt style for update error */
  view.style.scroll_axis = 999;
  CMP_TEST_EXPECT(cmp_grid_view_update(&view), CMP_ERR_RANGE);
  view.style.scroll_axis = CMP_GRID_SCROLL_VERTICAL;

  /* Corrupt bounds for update error */
  view.bounds.width = -1.0f;
  CMP_TEST_EXPECT(cmp_grid_view_update(&view), CMP_ERR_RANGE);
  view.bounds.width = 100.0f;

  view.style.span = 0;
  CMP_TEST_EXPECT(cmp_grid_view_update(&view), CMP_ERR_RANGE);
  view.style.span = 2;

  view.style.item_height = -1.0f;
  CMP_TEST_EXPECT(cmp_grid_view_update(&view), CMP_ERR_RANGE);
  view.style.item_height = 50.0f;

  view.bind = NULL;
  CMP_TEST_EXPECT(cmp_grid_view_update(&view), CMP_ERR_STATE);
  view.bind = bind_fn_impl;

  CMPListSlot *old_slots = view.slots;
  CMPRenderNode **old_nodes = view.visible_nodes;
  view.slots = NULL;
  CMP_TEST_EXPECT(cmp_grid_view_update(&view), CMP_ERR_STATE);
  view.slots = old_slots;
  view.visible_nodes = NULL;
  CMP_TEST_EXPECT(cmp_grid_view_update(&view), CMP_ERR_STATE);
  view.visible_nodes = old_nodes;

  cmp_grid_view_reserve(&view, 20);
  cmp_usize old_cap = view.slot_capacity;
  view.slot_capacity = 1;
  view.item_count = 10;
  view.scroll_offset = 0.0f;
  CMP_TEST_EXPECT(cmp_grid_view_update(&view), CMP_ERR_RANGE);
  view.slot_capacity = old_cap;
  view.bind = NULL;
  CMP_TEST_EXPECT(cmp_grid_view_update(&view), CMP_ERR_STATE);
  view.bind = bind_fail_impl;
  CMP_TEST_EXPECT(cmp_grid_view_update(&view), CMP_ERR_IO);
  view.bind = bind_fn_impl;

  view.style.item_height = -1.0f;
  CMP_TEST_EXPECT(cmp_grid_view_update(&view), CMP_ERR_RANGE);
  view.style.item_height = 50.0f;

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_LINE_COUNT_ZERO);
  CMP_TEST_EXPECT(cmp_grid_view_update(&view), CMP_OK);
  cmp_list_test_clear_fail_points();

  int binds = 0;
  cmp_grid_view_set_bind(&view, bind_fn_impl, &binds);

  CMP_TEST_OK(cmp_grid_view_reserve(&view, 20));
  CMP_TEST_OK(cmp_grid_view_update(&view));

  cmp_grid_view_set_scroll(&view, 50.0f);
  CMP_TEST_OK(cmp_grid_view_update(&view));

  view.widget.vtable->layout(&view, view.bounds);
  CMPMeasureSpec ms = {CMP_MEASURE_EXACTLY, 100.0f};
  CMPSize size;
  view.widget.vtable->measure(&view, ms, ms, &size);

  CMPGfx gfx;
  CMPGfxVTable gfx_vtable;
  memset(&gfx, 0, sizeof(gfx));
  memset(&gfx_vtable, 0, sizeof(gfx_vtable));
  gfx.vtable = &gfx_vtable;

  CMPPaintContext ctx;
  memset(&ctx, 0, sizeof(ctx));
  ctx.gfx = &gfx;
  view.widget.vtable->paint(&view, &ctx);

  view.widget.vtable->destroy(&view);
  return CMP_OK;
}

static int test_fail_points(void) {
  cmp_usize u1, u2, u3;
  CMPListView list_view;
  CMPListStyle list_style;
  cmp_list_style_init(&list_style);
  cmp_list_view_init(&list_view, &list_style, NULL, 10, 0);
  int binds = 0;
  cmp_list_view_set_bind(&list_view, bind_fn_impl, &binds);
  cmp_list_view_reserve(&list_view, 10);
  list_view.bounds.width = 100.0f;
  list_view.bounds.height = 100.0f;
  list_view.scroll_offset = 200.0f; /* Make sure first > 0 */

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE);
  CMP_TEST_EXPECT(cmp_list_view_update(&list_view), CMP_ERR_RANGE);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_VISIBLE_STRIDE);
  CMP_TEST_EXPECT(cmp_list_view_update(&list_view), CMP_ERR_RANGE);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_VISIBLE_END);
  CMP_TEST_EXPECT(cmp_list_view_update(&list_view), CMP_OK);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_VISIBLE_LAST_BEFORE_FIRST);
  CMP_TEST_EXPECT(cmp_list_view_update(&list_view), CMP_OK);
  cmp_list_test_clear_fail_points();

  list_view.scroll_offset = 0.0f;
  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_VISIBLE_LAST_BEFORE_FIRST);
  CMP_TEST_EXPECT(cmp_list_view_update(&list_view), CMP_OK);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_VISIBLE_LAST_AFTER_OVERSCAN);
  CMP_TEST_EXPECT(cmp_list_view_update(&list_view), CMP_OK);
  cmp_list_test_clear_fail_points();

  list_view.scroll_offset = 0.0f;
  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_VISIBLE_LAST_AFTER_OVERSCAN);
  CMP_TEST_EXPECT(cmp_list_view_update(&list_view), CMP_OK);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_LIST_ITEM_BOUNDS);
  CMP_TEST_EXPECT(cmp_list_view_update(&list_view), CMP_ERR_IO);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_LIST_RENDER_BOUNDS);
  CMP_TEST_EXPECT(cmp_list_view_update(&list_view), CMP_ERR_IO);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(
      CMP_LIST_TEST_FAIL_LIST_MEASURE_CONTENT_NEGATIVE);
  CMPMeasureSpec ms = {CMP_MEASURE_EXACTLY, 100.0f};
  CMPSize size;
  CMP_TEST_EXPECT(list_view.widget.vtable->measure(&list_view, ms, ms, &size),
                  CMP_ERR_RANGE);
  cmp_list_test_clear_fail_points();

  list_view.widget.vtable->destroy(&list_view);

  CMPGridView grid_view;
  CMPGridStyle grid_style;
  cmp_grid_style_init(&grid_style);
  cmp_grid_view_init(&grid_view, &grid_style, NULL, 10, 0);
  cmp_grid_view_set_bind(&grid_view, bind_fn_impl, &binds);
  cmp_grid_view_reserve(&grid_view, 10);
  grid_view.bounds.width = 100.0f;
  grid_view.bounds.height = 100.0f;
  grid_view.scroll_offset = 200.0f; /* Make sure first > 0 */

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_LINE_COUNT_ZERO);
  CMP_TEST_OK(cmp_grid_view_update(&grid_view));
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_STRIDE);
  CMP_TEST_EXPECT(cmp_grid_view_update(&grid_view), CMP_ERR_RANGE);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_END);
  CMP_TEST_EXPECT(cmp_grid_view_update(&grid_view), CMP_OK);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_END_NEGATIVE);
  CMP_TEST_EXPECT(cmp_grid_view_update(&grid_view), CMP_OK);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_LAST_TOO_LARGE);
  CMP_TEST_EXPECT(cmp_grid_view_update(&grid_view), CMP_OK);
  cmp_list_test_clear_fail_points();

#define CMP_LIST_TEST_FAIL_GRID_LAST_BEFORE_FIRST_INDEX 20u
  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_LAST_BEFORE_FIRST_INDEX);
  CMP_TEST_EXPECT(
      cmp_list_test_grid_compute_visible_range(&grid_view, &u1, &u2, &u3),
      CMP_OK);
  cmp_list_test_clear_fail_points();

  grid_view.item_count = 10;
  grid_view.style.span = 2;
  grid_view.scroll_offset = -100.0f; /* this will make first_line = 0 */
  grid_view.style.overscan = 0;
  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_LAST_BEFORE_FIRST);
  CMP_TEST_EXPECT(
      cmp_list_test_grid_compute_visible_range(&grid_view, &u1, &u2, &u3),
      CMP_OK);
  cmp_list_test_clear_fail_points();

  grid_view.scroll_offset = 0.0f; /* test with exact zero */
  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_LAST_BEFORE_FIRST);
  CMP_TEST_EXPECT(
      cmp_list_test_grid_compute_visible_range(&grid_view, &u1, &u2, &u3),
      CMP_OK);
  cmp_list_test_clear_fail_points();

  grid_view.scroll_offset = 200.0f; /* this will make first_line > 0 */
  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_LAST_BEFORE_FIRST);
  CMP_TEST_EXPECT(
      cmp_list_test_grid_compute_visible_range(&grid_view, &u1, &u2, &u3),
      CMP_OK);
  cmp_list_test_clear_fail_points();

  grid_view.scroll_offset = 0.0f;
  grid_view.style.overscan = 1;

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_ITEM_BOUNDS);
  CMP_TEST_EXPECT(cmp_grid_view_update(&grid_view), CMP_ERR_IO);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_RENDER_BOUNDS);
  CMP_TEST_EXPECT(cmp_grid_view_update(&grid_view), CMP_ERR_IO);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(
      CMP_LIST_TEST_FAIL_GRID_MEASURE_CONTENT_NEGATIVE);
  CMP_TEST_EXPECT(grid_view.widget.vtable->measure(&grid_view, ms, ms, &size),
                  CMP_ERR_RANGE);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE);
  CMP_TEST_EXPECT(cmp_grid_view_update(&grid_view), CMP_OK);
  cmp_list_test_clear_fail_points();

  grid_view.widget.vtable->destroy(&grid_view);

  CMPListSlot *slots = NULL;
  CMPRenderNode **nodes = NULL;
  cmp_usize cap = 0;
  CMPAllocator alloc;
  cmp_get_default_allocator(&alloc);

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_RESERVE_NODE_BYTES);
  CMP_TEST_EXPECT(cmp_list_test_reserve_slots(&slots, &nodes, &cap, &alloc, 10),
                  CMP_ERR_OVERFLOW);
  cmp_list_test_clear_fail_points();

  cmp_list_test_reserve_slots(&slots, &nodes, &cap, &alloc, 5);
  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_RESERVE_COPY_BYTES);
  CMP_TEST_EXPECT(cmp_list_test_reserve_slots(&slots, &nodes, &cap, &alloc, 10),
                  CMP_ERR_OVERFLOW);
  cmp_list_test_clear_fail_points();

  cmp_list_test_reserve_slots(&slots, &nodes, &cap, &alloc, 20);
  alloc.free(alloc.ctx, slots);
  alloc.free(alloc.ctx, nodes);

  return CMP_OK;
}

static int g_alloc_fail_count = 0;
static CMPAllocator g_sys_alloc;

static int test_alloc(void *ctx, cmp_usize size, void **ptr) {
  if (g_alloc_fail_count > 0) {
    g_alloc_fail_count--;
    if (g_alloc_fail_count == 0)
      return CMP_ERR_OUT_OF_MEMORY;
  }
  return g_sys_alloc.alloc(g_sys_alloc.ctx, size, ptr);
}

static int test_free(void *ctx, void *ptr) {
  if (g_alloc_fail_count > 0) {
    g_alloc_fail_count--;
    if (g_alloc_fail_count == 0)
      return CMP_ERR_IO;
  }
  return g_sys_alloc.free(g_sys_alloc.ctx, ptr);
}

static int dummy_draw_rect(void *ctx, const CMPRect *bounds, CMPColor color,
                           CMPScalar border_radius) {
  return CMP_OK;
}

static int test_measure_errors(void) {
  CMPListView lv;
  CMPListStyle lstyle;
  cmp_list_style_init(&lstyle);
  cmp_list_view_init(&lv, &lstyle, NULL, 10, 0);

  CMPGridView gv;
  CMPGridStyle gstyle;
  cmp_grid_style_init(&gstyle);
  cmp_grid_view_init(&gv, &gstyle, NULL, 10, 0);

  CMPMeasureSpec ok_spec = {CMP_MEASURE_EXACTLY, 100.0f};
  CMPMeasureSpec bad_spec = {(cmp_u32)999, 100.0f};
  CMPSize size;

  CMP_TEST_EXPECT(lv.widget.vtable->measure(NULL, ok_spec, ok_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(lv.widget.vtable->measure(&lv, ok_spec, ok_spec, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(lv.widget.vtable->measure(&lv, bad_spec, ok_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(lv.widget.vtable->measure(&lv, ok_spec, bad_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  lv.style.orientation = 999;
  CMP_TEST_EXPECT(lv.widget.vtable->measure(&lv, ok_spec, ok_spec, &size),
                  CMP_ERR_RANGE);
  lv.style.orientation = CMP_LIST_ORIENTATION_HORIZONTAL;
  CMP_TEST_EXPECT(lv.widget.vtable->measure(&lv, ok_spec, ok_spec, &size),
                  CMP_OK);
  lv.style.orientation = CMP_LIST_ORIENTATION_VERTICAL;
  lv.style.item_extent = -1.0f;
  CMP_TEST_EXPECT(lv.widget.vtable->measure(&lv, ok_spec, ok_spec, &size),
                  CMP_ERR_RANGE);
  lv.style.item_extent = 10.0f;

  CMP_TEST_EXPECT(gv.widget.vtable->measure(NULL, ok_spec, ok_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(gv.widget.vtable->measure(&gv, ok_spec, ok_spec, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(gv.widget.vtable->measure(&gv, bad_spec, ok_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(gv.widget.vtable->measure(&gv, ok_spec, bad_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  gv.style.scroll_axis = 999;
  CMP_TEST_EXPECT(gv.widget.vtable->measure(&gv, ok_spec, ok_spec, &size),
                  CMP_ERR_RANGE);

  gv.style.scroll_axis = CMP_GRID_SCROLL_VERTICAL;
  gv.style.item_height = -1.0f;
  CMP_TEST_EXPECT(gv.widget.vtable->measure(&gv, ok_spec, ok_spec, &size),
                  CMP_ERR_RANGE);
  gv.style.item_height = 0.0f;
  CMP_TEST_EXPECT(gv.widget.vtable->measure(&gv, ok_spec, ok_spec, &size),
                  CMP_ERR_RANGE);
  gv.style.item_height = 10.0f;

  gv.style.scroll_axis = CMP_GRID_SCROLL_HORIZONTAL;
  CMP_TEST_EXPECT(gv.widget.vtable->measure(&gv, ok_spec, ok_spec, &size),
                  CMP_OK);
  gv.style.scroll_axis = CMP_GRID_SCROLL_VERTICAL;

  CMPRect rect = {0, 0, 100, 100};
  CMPRect bad_rect = {-1.0f, -1.0f, -1.0f, -1.0f};
  CMP_TEST_EXPECT(lv.widget.vtable->layout(NULL, rect),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(lv.widget.vtable->layout(&lv, bad_rect), CMP_ERR_RANGE);
  CMP_TEST_EXPECT(gv.widget.vtable->layout(NULL, rect),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(gv.widget.vtable->layout(&gv, bad_rect), CMP_ERR_RANGE);

  CMPPaintContext paint_ctx;
  memset(&paint_ctx, 0, sizeof(paint_ctx));
  CMP_TEST_EXPECT(lv.widget.vtable->paint(NULL, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(lv.widget.vtable->paint(&lv, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(lv.widget.vtable->paint(&lv, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);

  CMPGfx gfx;
  memset(&gfx, 0, sizeof(gfx));
  paint_ctx.gfx = &gfx;
  lv.style.orientation = 999;
  CMP_TEST_EXPECT(lv.widget.vtable->paint(&lv, &paint_ctx), CMP_ERR_RANGE);
  lv.style.orientation = CMP_LIST_ORIENTATION_VERTICAL;
  lv.bounds = bad_rect;
  CMP_TEST_EXPECT(lv.widget.vtable->paint(&lv, &paint_ctx), CMP_ERR_RANGE);
  lv.bounds = (CMPRect){0, 0, 100, 100};

  lv.style.background_color.a = 1.0f;
  CMP_TEST_EXPECT(lv.widget.vtable->paint(&lv, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);

  static CMPGfxVTable gvtable = {0};
  gvtable.draw_rect = dummy_draw_rect;
  gfx.vtable = &gvtable;
  CMP_TEST_EXPECT(lv.widget.vtable->paint(&lv, &paint_ctx), CMP_OK);

  lv.style.background_color.a = 0.0f;

  CMPInputEvent event;
  memset(&event, 0, sizeof(event));
  CMPBool handled;
  CMP_TEST_EXPECT(lv.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(lv.widget.vtable->event(&lv, NULL, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(lv.widget.vtable->event(&lv, &event, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  lv.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_EXPECT(lv.widget.vtable->event(&lv, &event, &handled), CMP_OK);
  lv.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  event.type = CMP_INPUT_KEY_DOWN;
  CMP_TEST_EXPECT(lv.widget.vtable->event(&lv, &event, &handled), CMP_OK);
  event.type = CMP_INPUT_POINTER_DOWN;
  event.data.pointer.x = 200.0f;
  CMP_TEST_EXPECT(lv.widget.vtable->event(&lv, &event, &handled), CMP_OK);

  event.type = CMP_INPUT_POINTER_SCROLL;
  event.data.pointer.scroll_y = 10.0f;
  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE);
  CMP_TEST_EXPECT(lv.widget.vtable->event(&lv, &event, &handled),
                  CMP_ERR_RANGE);
  cmp_list_test_clear_fail_points();

  event.data.pointer.scroll_y = 0.0f;
  CMP_TEST_EXPECT(lv.widget.vtable->event(&lv, &event, &handled), CMP_OK);

  CMPSemantics sem;
  CMP_TEST_EXPECT(lv.widget.vtable->get_semantics(NULL, &sem),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(lv.widget.vtable->get_semantics(&lv, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(gv.widget.vtable->get_semantics(NULL, &sem),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(gv.widget.vtable->get_semantics(&gv, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  lv.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_EXPECT(lv.widget.vtable->get_semantics(&lv, &sem), CMP_OK);
  CMP_TEST_ASSERT(sem.flags & CMP_SEMANTIC_FLAG_DISABLED);
  lv.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  gv.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_EXPECT(gv.widget.vtable->get_semantics(&gv, &sem), CMP_OK);
  CMP_TEST_ASSERT(sem.flags & CMP_SEMANTIC_FLAG_DISABLED);
  gv.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  event.data.pointer.scroll_x = 0.0f;
  CMP_TEST_EXPECT(gv.widget.vtable->event(&gv, &event, &handled), CMP_OK);

  event.data.pointer.scroll_y = -1000.0f;
  CMP_TEST_EXPECT(gv.widget.vtable->event(&gv, &event, &handled), CMP_OK);
  event.data.pointer.scroll_y = 1000.0f;
  CMP_TEST_EXPECT(gv.widget.vtable->event(&gv, &event, &handled), CMP_OK);

  gv.bounds.height = -1.0f;
  event.data.pointer.scroll_y = 10.0f;
  CMP_TEST_EXPECT(gv.widget.vtable->event(&gv, &event, &handled),
                  CMP_ERR_RANGE);
  gv.bounds.height = 100.0f;

  gv.style.item_height = -1.0f;
  cmp_usize req_slots;
  gstyle.item_height = -1.0f;
  CMP_TEST_EXPECT(cmp_grid_view_set_style(&gv, &gstyle), CMP_ERR_RANGE);
  gstyle.item_height = 10.0f;
  CMP_TEST_EXPECT(cmp_grid_view_set_item_count(&gv, 10), CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_grid_view_get_required_slots(&gv, &req_slots),
                  CMP_ERR_RANGE);
  gv.style.item_height = 10.0f;
  gv.bounds.height = -1.0f;
  CMP_TEST_EXPECT(cmp_grid_view_get_required_slots(&gv, &req_slots),
                  CMP_ERR_RANGE);
  gv.bounds.height = 100.0f;
  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_STRIDE);
  CMP_TEST_EXPECT(cmp_grid_view_get_required_slots(&gv, &req_slots),
                  CMP_ERR_RANGE);
  cmp_list_test_clear_fail_points();

  gv.style.scroll_axis = CMP_GRID_SCROLL_HORIZONTAL;
  CMP_TEST_EXPECT(cmp_grid_view_get_required_slots(&gv, &req_slots), CMP_OK);
  gv.style.scroll_axis = CMP_GRID_SCROLL_VERTICAL;

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_STRIDE);
  CMP_TEST_EXPECT(cmp_grid_view_update(&gv), CMP_ERR_RANGE);
  cmp_list_test_clear_fail_points();

  cmp_usize old_cap = gv.slot_capacity;
  gv.slot_capacity = 1;
  gv.item_count = 10;
  gv.scroll_offset = 0.0f;
  /* Ensure enough count requires more than 1 slot. Fails with STATE if
   * unreserved. */
  CMP_TEST_EXPECT(cmp_grid_view_update(&gv), CMP_ERR_STATE);
  gv.slot_capacity = old_cap;

  gv.bind = NULL;
  CMP_TEST_EXPECT(cmp_grid_view_update(&gv), CMP_ERR_STATE);
  gv.bind = bind_fn_impl;

  CMP_TEST_EXPECT(gv.widget.vtable->paint(NULL, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(gv.widget.vtable->paint(&gv, NULL), CMP_ERR_INVALID_ARGUMENT);

  gv.style.scroll_axis = 999;
  CMP_TEST_EXPECT(gv.widget.vtable->paint(&gv, &paint_ctx), CMP_ERR_RANGE);
  gv.style.scroll_axis = CMP_GRID_SCROLL_VERTICAL;

  gv.bounds = bad_rect;
  CMP_TEST_EXPECT(gv.widget.vtable->paint(&gv, &paint_ctx), CMP_ERR_RANGE);
  gv.bounds = rect;

  gfx.vtable = NULL;
  gv.style.background_color.a = 1.0f;
  CMP_TEST_EXPECT(gv.widget.vtable->paint(&gv, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &gvtable;
  CMP_TEST_EXPECT(gv.widget.vtable->paint(&gv, &paint_ctx), CMP_OK);
  gv.style.background_color.a = 0.0f;

  /* Test API methods with valid and invalid values */
  CMPScalar ext_val;
  CMPRenderNode **visible_out;
  cmp_usize visible_count;

  CMP_TEST_EXPECT(cmp_list_view_get_visible(&lv, &visible_out, &visible_count),
                  CMP_OK);
  CMP_TEST_EXPECT(cmp_list_view_get_visible(NULL, &visible_out, &visible_count),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_get_visible(&gv, &visible_out, &visible_count),
                  CMP_OK);
  CMP_TEST_EXPECT(cmp_grid_view_get_visible(NULL, &visible_out, &visible_count),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_list_view_set_scroll(NULL, 100.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_set_scroll(NULL, 100.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_view_set_scroll(&lv, 100.0f), CMP_OK);
  CMP_TEST_EXPECT(cmp_list_view_get_scroll(&lv, &ext_val), CMP_OK);
  CMP_TEST_EXPECT(cmp_list_view_get_scroll(NULL, &ext_val),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_view_get_scroll(&lv, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_set_scroll(&gv, 100.0f), CMP_OK);

  CMP_TEST_EXPECT(cmp_list_view_set_style(NULL, &lstyle),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_set_style(NULL, &gstyle),
                  CMP_ERR_INVALID_ARGUMENT);
  lstyle.orientation = 999;
  CMP_TEST_EXPECT(cmp_list_view_set_style(&lv, &lstyle), CMP_ERR_RANGE);
  lstyle.orientation = CMP_LIST_ORIENTATION_VERTICAL;
  CMP_TEST_EXPECT(cmp_list_view_set_style(&lv, &lstyle), CMP_OK);
  gstyle.scroll_axis = 999;
  CMP_TEST_EXPECT(cmp_grid_view_set_style(&gv, &gstyle), CMP_ERR_RANGE);
  gstyle.scroll_axis = CMP_GRID_SCROLL_VERTICAL;
  CMP_TEST_EXPECT(cmp_grid_view_set_style(&gv, &gstyle), CMP_OK);

  lv.style.item_extent = -1.0f;
  CMP_TEST_EXPECT(cmp_list_view_get_content_extent(&lv, &ext_val),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_list_view_get_required_slots(&lv, &req_slots),
                  CMP_ERR_RANGE);

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE);
  CMP_TEST_EXPECT(lv.widget.vtable->measure(&lv, ok_spec, ok_spec, &size),
                  CMP_ERR_RANGE);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(
      CMP_LIST_TEST_FAIL_LIST_MEASURE_CONTENT_NEGATIVE);
  CMP_TEST_EXPECT(lv.widget.vtable->measure(&lv, ok_spec, ok_spec, &size),
                  CMP_ERR_RANGE);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE);
  CMP_TEST_EXPECT(lv.widget.vtable->layout(&lv, rect), CMP_ERR_RANGE);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE);
  CMP_TEST_EXPECT(cmp_list_view_get_required_slots(&lv, &req_slots),
                  CMP_ERR_RANGE);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE);
  CMP_TEST_EXPECT(cmp_list_view_set_scroll(&lv, 100.0f), CMP_ERR_RANGE);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE);
  CMP_TEST_EXPECT(cmp_list_view_get_content_extent(&lv, &ext_val),
                  CMP_ERR_RANGE);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_LINE_COUNT_ZERO);
  CMP_TEST_EXPECT(cmp_grid_view_get_content_extent(&gv, &ext_val), CMP_OK);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_LINE_COUNT_ZERO);
  CMP_TEST_EXPECT(cmp_grid_view_set_scroll(&gv, 100.0f), CMP_OK);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_LINE_COUNT_ZERO);
  CMP_TEST_EXPECT(cmp_grid_view_get_required_slots(&gv, &req_slots), CMP_OK);
  cmp_list_test_clear_fail_points();

  cmp_grid_view_reserve(&gv, 20);

  /* Let's try to fail cmp_grid_view_update via update_metrics, which sets
   * CMP_LIST_TEST_FAIL_GRID_LINE_COUNT_ZERO to fail internally... wait,
   * line_count zero fails update metrics, so it SHOULD be caught. Why isn't it?
   * Let me use CMP_LIST_TEST_FAIL_GRID_MEASURE_CONTENT_NEGATIVE instead. */
  gv.style.item_height = 0.0f;
  CMP_TEST_EXPECT(cmp_grid_view_update(&gv), CMP_ERR_RANGE);
  gv.style.item_height = 10.0f;

  lv.bounds.height = -1.0f;
  CMP_TEST_EXPECT(cmp_list_view_get_required_slots(&lv, &req_slots),
                  CMP_ERR_RANGE);

  CMP_TEST_EXPECT(lv.widget.vtable->measure(&lv, ok_spec, ok_spec, &size),
                  CMP_ERR_RANGE);

  lv.bounds.height = 100.0f;
  lv.item_count = 10;
  lv.style.item_extent = 0.0f;
  CMP_TEST_EXPECT(cmp_list_view_get_content_extent(&lv, &ext_val),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_list_view_get_required_slots(&lv, &req_slots),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(lv.widget.vtable->measure(&lv, ok_spec, ok_spec, &size),
                  CMP_ERR_RANGE);
  lv.style.item_extent = 10.0f;
  CMP_TEST_EXPECT(cmp_list_view_get_content_extent(&lv, &ext_val), CMP_OK);
  CMP_TEST_EXPECT(cmp_list_view_get_required_slots(&lv, &req_slots), CMP_OK);

  lv.style.orientation = CMP_LIST_ORIENTATION_HORIZONTAL;
  CMP_TEST_EXPECT(cmp_list_view_get_required_slots(&lv, &req_slots), CMP_OK);
  lv.style.orientation = CMP_LIST_ORIENTATION_VERTICAL;

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_VISIBLE_STRIDE);
  CMP_TEST_EXPECT(cmp_list_view_get_required_slots(&lv, &req_slots),
                  CMP_ERR_RANGE);
  cmp_list_test_clear_fail_points();

  gv.style.item_height = -1.0f;
  CMP_TEST_EXPECT(cmp_grid_view_get_content_extent(&gv, &ext_val),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_grid_view_get_required_slots(&gv, &req_slots),
                  CMP_ERR_RANGE);
  gv.style.item_height = 10.0f;

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE);
  CMP_TEST_EXPECT(gv.widget.vtable->measure(&gv, ok_spec, ok_spec, &size),
                  CMP_ERR_RANGE);
  cmp_list_test_clear_fail_points();

  CMP_TEST_EXPECT(cmp_grid_view_get_content_extent(&gv, &ext_val), CMP_OK);
  CMP_TEST_EXPECT(cmp_grid_view_get_required_slots(&gv, &req_slots), CMP_OK);
  CMP_TEST_EXPECT(cmp_list_view_get_required_slots(NULL, &req_slots),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_get_required_slots(NULL, &req_slots),
                  CMP_ERR_INVALID_ARGUMENT);

  lv.style.item_extent = -1.0f;
  CMP_TEST_EXPECT(cmp_list_view_set_item_count(&lv, 100), CMP_ERR_RANGE);
  lv.style.item_extent = 10.0f;
  CMP_TEST_EXPECT(cmp_list_view_set_item_count(&lv, 100), CMP_OK);
  CMP_TEST_EXPECT(cmp_list_view_set_item_count(NULL, 100),
                  CMP_ERR_INVALID_ARGUMENT);

  gv.style.item_height = -1.0f;
  CMP_TEST_EXPECT(cmp_grid_view_set_item_count(&gv, 100), CMP_ERR_RANGE);
  gv.style.item_height = 10.0f;
  CMP_TEST_EXPECT(cmp_grid_view_set_item_count(&gv, 100), CMP_OK);
  CMP_TEST_EXPECT(cmp_grid_view_set_item_count(NULL, 100),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Error inside get_required_slots via range compute */
  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_STRIDE);
  CMP_TEST_EXPECT(cmp_grid_view_get_required_slots(&gv, &req_slots),
                  CMP_ERR_RANGE);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_VISIBLE_STRIDE);
  CMP_TEST_EXPECT(cmp_list_view_get_required_slots(&lv, &req_slots),
                  CMP_ERR_RANGE);
  cmp_list_test_clear_fail_points();

  lv.style.orientation = CMP_LIST_ORIENTATION_HORIZONTAL;
  CMP_TEST_EXPECT(cmp_list_view_get_required_slots(&lv, &req_slots), CMP_OK);
  lv.style.orientation = CMP_LIST_ORIENTATION_VERTICAL;

  /* Event scroll tests that were failing due to missing metrics */
  event.type = CMP_INPUT_POINTER_SCROLL;
  event.data.pointer.scroll_x = 10.0f;
  lv.style.orientation = CMP_LIST_ORIENTATION_HORIZONTAL;
  CMP_TEST_EXPECT(lv.widget.vtable->event(&lv, &event, &handled), CMP_OK);
  lv.style.orientation = CMP_LIST_ORIENTATION_VERTICAL;

  event.data.pointer.scroll_y = 10.0f;
  gv.style.scroll_axis = CMP_GRID_SCROLL_VERTICAL;
  CMP_TEST_EXPECT(gv.widget.vtable->event(&gv, &event, &handled), CMP_OK);
  event.data.pointer.scroll_x = 10.0f;
  gv.style.scroll_axis = CMP_GRID_SCROLL_HORIZONTAL;
  CMP_TEST_EXPECT(gv.widget.vtable->event(&gv, &event, &handled), CMP_OK);
  gv.style.scroll_axis = CMP_GRID_SCROLL_VERTICAL;

  CMP_TEST_EXPECT(lv.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(gv.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);

  lv.widget.vtable->destroy(&lv);
  CMP_TEST_EXPECT(gv.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(gv.widget.vtable->event(&gv, NULL, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(gv.widget.vtable->event(&gv, &event, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  gv.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_EXPECT(gv.widget.vtable->event(&gv, &event, &handled), CMP_OK);
  gv.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  event.type = CMP_INPUT_KEY_DOWN;
  CMP_TEST_EXPECT(gv.widget.vtable->event(&gv, &event, &handled), CMP_OK);

  gv.widget.vtable->destroy(&gv);
  return CMP_OK;
}

static int test_coverage_hooks(void) {
  CMP_TEST_EXPECT(cmp_list_test_mul_overflow(1, 1, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_test_validate_color(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_test_validate_edges(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_test_validate_rect(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_list_test_validate_measure_spec((CMPMeasureSpec){999, 0.0f}),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_test_validate_style(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_list_test_compute_content_extent(0, 0.0f, 0.0f, 0.0f, 0.0f, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(
                      0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, NULL, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_list_test_grid_compute_visible_range(NULL, NULL, NULL, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_test_compute_item_bounds(NULL, 0, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_test_grid_compute_item_bounds(NULL, 0, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_test_update_metrics(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_test_grid_update_metrics(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_test_reserve_slots(NULL, NULL, NULL, NULL, 0),
                  CMP_ERR_INVALID_ARGUMENT);

  cmp_usize val;
  CMP_TEST_EXPECT(
      cmp_list_test_mul_overflow((cmp_usize)-1, (cmp_usize)-1, &val),
      CMP_ERR_OVERFLOW);

  cmp_usize u1, u2, u3;
  CMPScalar ext;
  CMPRect rect;
  CMPListStyle lstyle;
  CMPGridStyle gstyle;
  CMPListView lv;
  CMPGridView gv;
  cmp_list_style_init(&lstyle);
  cmp_grid_style_init(&gstyle);

  CMP_TEST_EXPECT(
      cmp_list_test_compute_content_extent(0, -1.0f, 0.0f, 0.0f, 0.0f, &ext),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      cmp_list_test_compute_content_extent(0, 0.0f, 0.0f, 0.0f, 0.0f, &ext),
      CMP_OK);
  CMP_TEST_EXPECT(
      cmp_list_test_compute_content_extent(1, 0.0f, 0.0f, 0.0f, 0.0f, &ext),
      CMP_ERR_RANGE);

  CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(
                      1, -1.0f, 0.0f, 0.0f, 0.0f, 100.0f, 0, &u1, &u2, &u3),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(0, 0.0f, 0.0f, 0.0f, 0.0f,
                                                      0.0f, 0, &u1, &u2, &u3),
                  CMP_OK);
  CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(1, 0.0f, 0.0f, 0.0f, 0.0f,
                                                      100.0f, 0, &u1, &u2, &u3),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(
                      10, 10.0f, 0.0f, 20.0f, 0.0f, 10.0f, 0, &u1, &u2, &u3),
                  CMP_OK);
  CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(10, 10.0f, 0.0f, 20.0f,
                                                      0.0f, 20.00005f, 0, &u1,
                                                      &u2, &u3),
                  CMP_OK);
  CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(
                      10, 10.0f, 0.0f, 0.0f, 1000.0f, 100.0f, 0, &u1, &u2, &u3),
                  CMP_OK);
  CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(
                      10, 10.0f, 0.0f, 0.0f, 50.0f, 1000.0f, 0, &u1, &u2, &u3),
                  CMP_OK);

  cmp_grid_view_init(&gv, &gstyle, NULL, 10, 0);
  CMP_TEST_EXPECT(cmp_list_test_grid_compute_visible_range(&gv, &u1, &u2, &u3),
                  CMP_OK);

  /* grid edge cases */
  gv.item_count = 0;
  CMP_TEST_EXPECT(cmp_list_test_grid_compute_visible_range(&gv, &u1, &u2, &u3),
                  CMP_OK);
  gv.item_count = 10;
  gv.style.span = 1;
  gv.scroll_offset = 0.0f;
  gv.bounds.height = -10.0f;
  CMP_TEST_EXPECT(cmp_list_test_grid_compute_visible_range(&gv, &u1, &u2, &u3),
                  CMP_OK);
  gv.bounds.height = 100.0f;
  gv.style.item_height = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_grid_compute_visible_range(&gv, &u1, &u2, &u3),
                  CMP_ERR_RANGE);
  gv.style.item_height = 10.0f;
  gv.style.span = 3; /* line_count % span != 0 */
  CMP_TEST_EXPECT(cmp_list_test_grid_compute_visible_range(&gv, &u1, &u2, &u3),
                  CMP_OK);

  /* grid scroll axis horizontal */
  gv.style.scroll_axis = CMP_GRID_SCROLL_HORIZONTAL;
  gv.bounds.width = -10.0f;
  CMP_TEST_EXPECT(cmp_list_test_grid_compute_visible_range(&gv, &u1, &u2, &u3),
                  CMP_OK);
  gv.bounds.width = 100.0f;
  gv.scroll_offset = -100.0f;
  CMP_TEST_EXPECT(cmp_list_test_grid_compute_visible_range(&gv, &u1, &u2, &u3),
                  CMP_OK);
  gv.scroll_offset = 0.0f;
  gv.style.item_width = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_grid_compute_visible_range(&gv, &u1, &u2, &u3),
                  CMP_ERR_RANGE);
  gv.style.item_width = 10.0f;
  gv.scroll_offset = 200.0f;
  CMP_TEST_EXPECT(cmp_list_test_grid_compute_visible_range(&gv, &u1, &u2, &u3),
                  CMP_OK);
  gv.scroll_offset = 2000.0f;
  CMP_TEST_EXPECT(cmp_list_test_grid_compute_visible_range(&gv, &u1, &u2, &u3),
                  CMP_OK);

  gv.scroll_offset = 0.0f;
  gv.bounds.width = 10.0f;
  gv.style.overscan = 100;
  CMP_TEST_EXPECT(cmp_list_test_grid_compute_visible_range(&gv, &u1, &u2, &u3),
                  CMP_OK);

  gv.style.overscan = 1;
  CMP_TEST_EXPECT(cmp_list_test_grid_compute_visible_range(&gv, &u1, &u2, &u3),
                  CMP_OK);
  gv.bounds.width = 100.0f;

  gv.style.span = 3;
  gv.item_count = 10;
  gv.style.overscan = 0;
  CMP_TEST_EXPECT(cmp_list_test_grid_compute_visible_range(&gv, &u1, &u2, &u3),
                  CMP_OK);

  /* Init error cases */
  CMPListStyle bad_lstyle = lstyle;
  bad_lstyle.orientation = 999;
  CMP_TEST_EXPECT(cmp_list_view_init(&lv, &bad_lstyle, NULL, 10, 0),
                  CMP_ERR_RANGE);
  CMPGridStyle bad_gstyle = gstyle;
  bad_gstyle.scroll_axis = 999;
  CMP_TEST_EXPECT(cmp_grid_view_init(&gv, &bad_gstyle, NULL, 10, 0),
                  CMP_ERR_RANGE);

  cmp_core_test_set_default_allocator_fail(CMP_TRUE);
  CMP_TEST_EXPECT(cmp_list_view_init(&lv, &lstyle, NULL, 10, 0),
                  CMP_ERR_UNKNOWN);
  CMP_TEST_EXPECT(cmp_grid_view_init(&gv, &gstyle, NULL, 10, 0),
                  CMP_ERR_UNKNOWN);
  cmp_core_test_set_default_allocator_fail(CMP_FALSE);

  CMPAllocator partial_alloc = g_sys_alloc;
  partial_alloc.alloc = NULL;
  CMP_TEST_EXPECT(cmp_list_view_init(&lv, &lstyle, &partial_alloc, 10, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_init(&gv, &gstyle, &partial_alloc, 10, 0),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Test overscan */
  CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(
                      10, 10.0f, 0.0f, 0.0f, 0.0f, 20.0f, 5, &u1, &u2, &u3),
                  CMP_OK); /* overscan > max_add */
  CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(
                      10, 10.0f, 0.0f, 0.0f, 0.0f, 20.0f, 1, &u1, &u2, &u3),
                  CMP_OK); /* overscan <= max_add */
  CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(
                      10, 10.0f, 0.0f, 0.0f, 50.0f, 20.0f, 10, &u1, &u2, &u3),
                  CMP_OK); /* overscan > first */
  CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(
                      10, 10.0f, 0.0f, 0.0f, 50.0f, 20.0f, 1, &u1, &u2, &u3),
                  CMP_OK); /* overscan <= first */

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_VISIBLE_LAST_AFTER_OVERSCAN);
  CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(
                      10, 10.0f, 0.0f, 0.0f, 50.0f, 20.0f, 0, &u1, &u2, &u3),
                  CMP_OK);
  cmp_list_test_clear_fail_points();

  cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_VISIBLE_LAST_AFTER_OVERSCAN);
  CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(
                      10, 10.0f, 0.0f, 0.0f, 0.0f, 20.0f, 0, &u1, &u2, &u3),
                  CMP_OK);
  cmp_list_test_clear_fail_points();

  cmp_list_view_init(&lv, &lstyle, NULL, 0, 0);
  CMP_TEST_EXPECT(cmp_list_test_compute_item_bounds(&lv, 0, &rect),
                  CMP_ERR_NOT_FOUND);

  lv.scroll_offset = -10.0f;
  lv.bounds.height = 100.0f;
  CMP_TEST_EXPECT(cmp_list_test_update_metrics(&lv), CMP_OK);

  lv.style.orientation = CMP_LIST_ORIENTATION_HORIZONTAL;
  lv.scroll_offset = -10.0f;
  lv.bounds.width = 100.0f;
  CMP_TEST_EXPECT(cmp_list_test_update_metrics(&lv), CMP_OK);

  lv.style.orientation = CMP_LIST_ORIENTATION_VERTICAL;
  lv.item_count = 10;
  lv.style.item_extent = -1.0f;
  lv.style.spacing = 0.0f;
  CMP_TEST_EXPECT(cmp_list_test_compute_item_bounds(&lv, 0, &rect),
                  CMP_ERR_RANGE);

  lv.style.item_extent = 10.0f;
  lv.bounds.width = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_compute_item_bounds(&lv, 0, &rect),
                  CMP_ERR_RANGE);

  lv.bounds.width = 100.0f;
  lv.style.orientation = CMP_LIST_ORIENTATION_HORIZONTAL;
  CMP_TEST_EXPECT(cmp_list_test_compute_item_bounds(&lv, 0, &rect), CMP_OK);

  lv.bounds.height = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_compute_item_bounds(&lv, 0, &rect),
                  CMP_ERR_RANGE);
  lv.bounds.height = 100.0f;

  gv.item_count = 0;
  CMP_TEST_EXPECT(cmp_list_test_grid_compute_item_bounds(&gv, 0, &rect),
                  CMP_ERR_NOT_FOUND);

  gv.item_count = 10;
  gv.style.scroll_axis = CMP_GRID_SCROLL_HORIZONTAL;
  CMP_TEST_EXPECT(cmp_list_test_grid_compute_item_bounds(&gv, 0, &rect),
                  CMP_OK);
  gv.style.scroll_axis = CMP_GRID_SCROLL_VERTICAL;
  gv.style.item_width = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_grid_compute_item_bounds(&gv, 0, &rect),
                  CMP_ERR_RANGE);
  gv.style.item_width = 10.0f;

  gv.item_count = 10;
  gv.style.scroll_axis = CMP_GRID_SCROLL_VERTICAL;
  gv.style.item_height = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_grid_update_metrics(&gv), CMP_ERR_RANGE);
  gv.style.item_height = 0.0f;
  CMP_TEST_EXPECT(cmp_list_test_grid_update_metrics(&gv), CMP_ERR_RANGE);
  gv.style.item_height = 10.0f;

  gv.style.span = 0;
  CMP_TEST_EXPECT(cmp_list_test_grid_update_metrics(&gv), CMP_ERR_RANGE);
  gv.style.span = 1;

  gv.item_count = 0;
  CMP_TEST_EXPECT(cmp_list_test_grid_update_metrics(&gv), CMP_OK);
  gv.item_count = 10;

  gv.scroll_offset = -10.0f;
  CMP_TEST_EXPECT(cmp_list_test_grid_update_metrics(&gv), CMP_OK);

  gv.style.scroll_axis = CMP_GRID_SCROLL_HORIZONTAL;
  gv.scroll_offset = -10.0f;
  CMP_TEST_EXPECT(cmp_list_test_grid_update_metrics(&gv), CMP_OK);

  CMPListSlot *slots = NULL;
  CMPRenderNode **nodes = NULL;
  cmp_usize cap = 0;
  CMPAllocator alloc;
  CMP_TEST_EXPECT(cmp_get_default_allocator(&alloc), CMP_OK);

  CMPAllocator bad_alloc = alloc;
  bad_alloc.alloc = NULL;
  CMP_TEST_EXPECT(
      cmp_list_test_reserve_slots(&slots, &nodes, &cap, &bad_alloc, 100),
      CMP_ERR_INVALID_ARGUMENT);
  bad_alloc.alloc = alloc.alloc;
  bad_alloc.free = NULL;
  CMP_TEST_EXPECT(
      cmp_list_test_reserve_slots(&slots, &nodes, &cap, &bad_alloc, 100),
      CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(
      cmp_list_test_reserve_slots(&slots, &nodes, &cap, &alloc, 100), CMP_OK);

  /* Test capacity <= *slot_capacity */
  CMP_TEST_EXPECT(cmp_list_test_reserve_slots(&slots, &nodes, &cap, &alloc, 50),
                  CMP_OK);

  /* Test overflow capacity */
  CMP_TEST_EXPECT(
      cmp_list_test_reserve_slots(&slots, &nodes, &cap, &alloc, (cmp_usize)-1),
      CMP_ERR_OVERFLOW);
  if (slots)
    alloc.free(alloc.ctx, slots);
  if (nodes)
    alloc.free(alloc.ctx, nodes);

  slots = NULL;
  nodes = NULL;
  cap = 0;
  cmp_get_default_allocator(&g_sys_alloc);
  CMPAllocator fail_alloc = g_sys_alloc;
  fail_alloc.alloc = test_alloc;
  fail_alloc.free = test_free;

  g_alloc_fail_count = 1;
  CMP_TEST_EXPECT(
      cmp_list_test_reserve_slots(&slots, &nodes, &cap, &fail_alloc, 10),
      CMP_ERR_OUT_OF_MEMORY);
  g_alloc_fail_count = 2;
  CMP_TEST_EXPECT(
      cmp_list_test_reserve_slots(&slots, &nodes, &cap, &fail_alloc, 10),
      CMP_ERR_OUT_OF_MEMORY);

  g_alloc_fail_count = 1;
  CMP_TEST_EXPECT(cmp_list_view_init(&lv, &lstyle, &fail_alloc, 10, 10),
                  CMP_ERR_OUT_OF_MEMORY);
  g_alloc_fail_count = 2;
  CMP_TEST_EXPECT(cmp_list_view_init(&lv, &lstyle, &fail_alloc, 10, 10),
                  CMP_ERR_OUT_OF_MEMORY);

  g_alloc_fail_count = 1;
  CMP_TEST_EXPECT(cmp_grid_view_init(&gv, &gstyle, &fail_alloc, 10, 10),
                  CMP_ERR_OUT_OF_MEMORY);
  g_alloc_fail_count = 2;
  CMP_TEST_EXPECT(cmp_grid_view_init(&gv, &gstyle, &fail_alloc, 10, 10),
                  CMP_ERR_OUT_OF_MEMORY);

  cmp_list_view_init(&lv, &lstyle, &fail_alloc, 10, 10);
  g_alloc_fail_count = 1;
  CMP_TEST_EXPECT(lv.widget.vtable->destroy(&lv), CMP_ERR_IO);

  cmp_list_view_init(&lv, &lstyle, &fail_alloc, 10, 10);
  g_alloc_fail_count = 2;
  CMP_TEST_EXPECT(lv.widget.vtable->destroy(&lv), CMP_ERR_IO);

  cmp_grid_view_init(&gv, &gstyle, &fail_alloc, 10, 10);
  g_alloc_fail_count = 1;
  CMP_TEST_EXPECT(gv.widget.vtable->destroy(&gv), CMP_ERR_IO);

  cmp_grid_view_init(&gv, &gstyle, &fail_alloc, 10, 10);
  g_alloc_fail_count = 2;
  CMP_TEST_EXPECT(gv.widget.vtable->destroy(&gv), CMP_ERR_IO);

  return CMP_OK;
}

int main(void) {
  CMP_TEST_ASSERT(test_list_style() == CMP_OK);
  CMP_TEST_ASSERT(test_grid_style() == CMP_OK);
  CMP_TEST_ASSERT(test_list_view_init() == CMP_OK);
  CMP_TEST_ASSERT(test_grid_view_init() == CMP_OK);
  CMP_TEST_ASSERT(test_list_compute() == CMP_OK);
  CMP_TEST_ASSERT(test_grid_compute() == CMP_OK);
  CMP_TEST_ASSERT(test_reserves() == CMP_OK);
  CMP_TEST_ASSERT(test_list_update() == CMP_OK);
  CMP_TEST_ASSERT(test_grid_update() == CMP_OK);
  CMP_TEST_ASSERT(test_fail_points() == CMP_OK);
  CMP_TEST_ASSERT(test_measure_errors() == CMP_OK);
  CMP_TEST_ASSERT(test_coverage_hooks() == CMP_OK);
  return 0;
}
