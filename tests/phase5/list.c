#include "cmpc/cmp_list.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>

#define CMP_LIST_TEST_FAIL_NONE 0u
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

typedef struct TestAllocator {
  cmp_usize alloc_calls;
  cmp_usize realloc_calls;
  cmp_usize free_calls;
  cmp_usize fail_alloc_on;
  cmp_usize fail_free_on;
} TestAllocator;

static int test_alloc(void *ctx, cmp_usize size, void **out_ptr) {
  TestAllocator *alloc;

  if (ctx == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->alloc_calls += 1;
  if (alloc->fail_alloc_on != 0 && alloc->alloc_calls == alloc->fail_alloc_on) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = malloc(size);
  if (*out_ptr == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  return CMP_OK;
}

static int test_realloc(void *ctx, void *ptr, cmp_usize size, void **out_ptr) {
  void *mem;

  if (ctx == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  mem = realloc(ptr, size);
  if (mem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  *out_ptr = mem;
  return CMP_OK;
}

static int test_free(void *ctx, void *ptr) {
  TestAllocator *alloc;

  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->free_calls += 1;
  if (alloc->fail_free_on != 0 && alloc->free_calls == alloc->fail_free_on) {
    return CMP_ERR_IO;
  }

  free(ptr);
  return CMP_OK;
}

static void test_allocator_init(TestAllocator *alloc) {
  alloc->alloc_calls = 0;
  alloc->realloc_calls = 0;
  alloc->free_calls = 0;
  alloc->fail_alloc_on = 0;
  alloc->fail_free_on = 0;
}

typedef struct TestWidget {
  CMPWidget widget;
  CMPRect last_bounds;
} TestWidget;

static int test_widget_measure(void *widget, CMPMeasureSpec width,
                               CMPMeasureSpec height, CMPSize *out_size) {
  CMP_UNUSED(widget);
  if (out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  CMP_UNUSED(width);
  CMP_UNUSED(height);
  out_size->width = 0.0f;
  out_size->height = 0.0f;
  return CMP_OK;
}

static int test_widget_layout(void *widget, CMPRect bounds) {
  TestWidget *w;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  w = (TestWidget *)widget;
  w->last_bounds = bounds;
  return CMP_OK;
}

static int test_widget_paint(void *widget, CMPPaintContext *ctx) {
  CMP_UNUSED(widget);
  CMP_UNUSED(ctx);
  return CMP_OK;
}

static int test_widget_event(void *widget, const CMPInputEvent *event,
                             CMPBool *out_handled) {
  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_handled = CMP_FALSE;
  return CMP_OK;
}

static int test_widget_semantics(void *widget, CMPSemantics *out_semantics) {
  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  out_semantics->role = CMP_SEMANTIC_NONE;
  out_semantics->flags = 0;
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int test_widget_destroy(void *widget) {
  CMP_UNUSED(widget);
  return CMP_OK;
}

static const CMPWidgetVTable g_test_widget_vtable = {
    test_widget_measure, test_widget_layout,    test_widget_paint,
    test_widget_event,   test_widget_semantics, test_widget_destroy};

static void test_widget_init(TestWidget *widget) {
  memset(widget, 0, sizeof(*widget));
  widget->widget.ctx = widget;
  widget->widget.vtable = &g_test_widget_vtable;
  widget->widget.flags = 0u;
}

typedef struct TestBindCtx {
  TestWidget *pool;
  cmp_usize pool_count;
  cmp_usize next;
  cmp_usize calls;
  cmp_usize last_index;
  int fail;
} TestBindCtx;

static int test_bind(void *ctx, CMPListSlot *slot, cmp_usize index) {
  TestBindCtx *bind;
  TestWidget *widget;
  CMPRect zero;
  int rc;

  if (ctx == NULL || slot == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  bind = (TestBindCtx *)ctx;
  if (bind->fail) {
    return CMP_ERR_UNKNOWN;
  }

  bind->calls += 1;
  bind->last_index = index;

  if (slot->user == NULL) {
    if (bind->next >= bind->pool_count) {
      return CMP_ERR_OUT_OF_MEMORY;
    }
    widget = &bind->pool[bind->next];
    bind->next += 1;
    test_widget_init(widget);
    slot->user = widget;
  } else {
    widget = (TestWidget *)slot->user;
  }

  zero.x = 0.0f;
  zero.y = 0.0f;
  zero.width = 0.0f;
  zero.height = 0.0f;
  rc = cmp_render_node_init(&slot->node, &widget->widget, &zero);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int test_bind_missing_widget(void *ctx, CMPListSlot *slot,
                                    cmp_usize index) {
  CMP_UNUSED(ctx);
  CMP_UNUSED(slot);
  CMP_UNUSED(index);
  return CMP_OK;
}

typedef struct TestGfxBackend {
  int draw_rect_calls;
} TestGfxBackend;

static int test_draw_rect(void *gfx, const CMPRect *rect, CMPColor color,
                          CMPScalar radius) {
  TestGfxBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestGfxBackend *)gfx;
  backend->draw_rect_calls += 1;
  CMP_UNUSED(color);
  CMP_UNUSED(radius);
  return CMP_OK;
}

static const CMPGfxVTable g_test_gfx_vtable = {
    NULL, NULL, NULL, test_draw_rect, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,           NULL, NULL};

static const CMPGfxVTable g_test_gfx_vtable_no_draw = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL};

int main(void) {
  CMPListStyle list_style;
  CMPListView list;
  CMPListView list_small;
  CMPGridStyle grid_style;
  CMPGridView grid;
  CMPAllocator bad_alloc;
  TestAllocator alloc;
  CMPAllocator alloc_iface;
  TestWidget pool[16];
  TestBindCtx bind_ctx;
  CMPMeasureSpec width_spec;
  CMPMeasureSpec height_spec;
  CMPSize measured;
  CMPRect bounds;
  CMPRenderNode **visible;
  cmp_usize visible_count;
  cmp_usize overflow_value;
  CMPScalar content_extent;
  CMPScalar scroll;
  CMPBool handled;
  CMPInputEvent event;
  CMPPaintContext paint_ctx;
  TestGfxBackend gfx_backend;
  CMPGfx gfx;

  CMP_TEST_OK(cmp_list_style_init(&list_style));
  CMP_TEST_EXPECT(cmp_list_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_ASSERT(list_style.orientation == CMP_LIST_ORIENTATION_VERTICAL);
  CMP_TEST_ASSERT(list_style.spacing == CMP_LIST_DEFAULT_SPACING);
  CMP_TEST_ASSERT(list_style.item_extent == CMP_LIST_DEFAULT_ITEM_EXTENT);
  CMP_TEST_ASSERT(list_style.overscan == CMP_LIST_DEFAULT_OVERSCAN);

  {
    CMPColor color;
    CMPLayoutEdges edges;
    CMPRect rect;
    CMPMeasureSpec spec;
    CMPListStyle style;
    CMPGridStyle grid_style_local;
    CMPScalar extent;
    cmp_usize first;
    cmp_usize last;
    cmp_usize count;
    cmp_usize huge_span;
    CMPListView temp_list;
    CMPGridView temp_grid;

    CMP_TEST_EXPECT(cmp_list_test_validate_color(NULL), CMP_ERR_INVALID_ARGUMENT);
    color.r = -0.1f;
    color.g = 0.5f;
    color.b = 0.5f;
    color.a = 0.5f;
    CMP_TEST_EXPECT(cmp_list_test_validate_color(&color), CMP_ERR_RANGE);
    color.r = 0.0f;
    color.g = 1.5f;
    CMP_TEST_EXPECT(cmp_list_test_validate_color(&color), CMP_ERR_RANGE);
    color.g = 0.0f;
    color.b = -0.2f;
    CMP_TEST_EXPECT(cmp_list_test_validate_color(&color), CMP_ERR_RANGE);
    color.b = 0.0f;
    color.a = 2.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_color(&color), CMP_ERR_RANGE);
    color.a = 1.0f;
    CMP_TEST_OK(cmp_list_test_validate_color(&color));

    CMP_TEST_EXPECT(cmp_list_test_validate_edges(NULL), CMP_ERR_INVALID_ARGUMENT);
    edges.left = -1.0f;
    edges.right = 0.0f;
    edges.top = 0.0f;
    edges.bottom = 0.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_edges(&edges), CMP_ERR_RANGE);
    edges.left = 0.0f;
    edges.right = -1.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_edges(&edges), CMP_ERR_RANGE);
    edges.right = 0.0f;
    edges.top = -1.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_edges(&edges), CMP_ERR_RANGE);
    edges.top = 0.0f;
    edges.bottom = -1.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_edges(&edges), CMP_ERR_RANGE);
    edges.bottom = 0.0f;
    CMP_TEST_OK(cmp_list_test_validate_edges(&edges));

    CMP_TEST_EXPECT(cmp_list_test_validate_rect(NULL), CMP_ERR_INVALID_ARGUMENT);
    rect.x = 0.0f;
    rect.y = 0.0f;
    rect.width = -1.0f;
    rect.height = 1.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_rect(&rect), CMP_ERR_RANGE);
    rect.width = 1.0f;
    rect.height = -1.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_rect(&rect), CMP_ERR_RANGE);
    rect.height = 1.0f;
    CMP_TEST_OK(cmp_list_test_validate_rect(&rect));

    spec.mode = 99u;
    spec.size = 1.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_measure_spec(spec),
                   CMP_ERR_INVALID_ARGUMENT);
    spec.mode = CMP_MEASURE_EXACTLY;
    spec.size = -1.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_measure_spec(spec), CMP_ERR_RANGE);
    spec.mode = CMP_MEASURE_AT_MOST;
    spec.size = 1.0f;
    CMP_TEST_OK(cmp_list_test_validate_measure_spec(spec));

    CMP_TEST_OK(cmp_list_style_init(&style));
    CMP_TEST_EXPECT(cmp_list_test_validate_style(NULL), CMP_ERR_INVALID_ARGUMENT);
    style.orientation = 99u;
    CMP_TEST_EXPECT(cmp_list_test_validate_style(&style), CMP_ERR_RANGE);
    style.orientation = CMP_LIST_ORIENTATION_VERTICAL;
    style.spacing = -1.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_style(&style), CMP_ERR_RANGE);
    style.spacing = CMP_LIST_DEFAULT_SPACING;
    style.item_extent = -1.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_style(&style), CMP_ERR_RANGE);
    style.item_extent = CMP_LIST_DEFAULT_ITEM_EXTENT;
    style.padding.left = -1.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_style(&style), CMP_ERR_RANGE);
    style.padding.left = 0.0f;
    style.background_color.g = 2.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_style(&style), CMP_ERR_RANGE);
    style.background_color.g = 0.0f;
    CMP_TEST_OK(cmp_list_test_validate_style(&style));

    CMP_TEST_OK(cmp_grid_style_init(&grid_style_local));
    CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(NULL),
                   CMP_ERR_INVALID_ARGUMENT);
    grid_style_local.scroll_axis = 99u;
    CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(&grid_style_local),
                   CMP_ERR_RANGE);
    grid_style_local.scroll_axis = CMP_GRID_SCROLL_VERTICAL;
    grid_style_local.span = 0u;
    CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(&grid_style_local),
                   CMP_ERR_RANGE);
    grid_style_local.span = 2u;
    grid_style_local.spacing_x = -1.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(&grid_style_local),
                   CMP_ERR_RANGE);
    grid_style_local.spacing_x = 0.0f;
    grid_style_local.spacing_y = -1.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(&grid_style_local),
                   CMP_ERR_RANGE);
    grid_style_local.spacing_y = 0.0f;
    grid_style_local.item_width = -1.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(&grid_style_local),
                   CMP_ERR_RANGE);
    grid_style_local.item_width = CMP_GRID_DEFAULT_ITEM_WIDTH;
    grid_style_local.item_height = -1.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(&grid_style_local),
                   CMP_ERR_RANGE);
    grid_style_local.item_height = CMP_GRID_DEFAULT_ITEM_HEIGHT;
    grid_style_local.padding.left = -1.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(&grid_style_local),
                   CMP_ERR_RANGE);
    grid_style_local.padding.left = 0.0f;
    grid_style_local.background_color.a = 2.0f;
    CMP_TEST_EXPECT(cmp_list_test_validate_grid_style(&grid_style_local),
                   CMP_ERR_RANGE);
    grid_style_local.background_color.a = 0.0f;
    CMP_TEST_OK(cmp_list_test_validate_grid_style(&grid_style_local));

    CMP_TEST_EXPECT(
        cmp_list_test_compute_content_extent(1u, 1.0f, 1.0f, 0.0f, 0.0f, NULL),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_list_test_compute_content_extent(1u, -1.0f, 1.0f, 0.0f,
                                                       0.0f, &extent),
                   CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_list_test_compute_content_extent(0u, 1.0f, 1.0f, 1.0f, 2.0f,
                                                   &extent));
    CMP_TEST_ASSERT(extent > 2.999f && extent < 3.001f);
    CMP_TEST_EXPECT(cmp_list_test_compute_content_extent(1u, 0.0f, 1.0f, 0.0f,
                                                       0.0f, &extent),
                   CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_list_test_compute_content_extent(2u, 1.0f, 1.0f, 0.0f, 0.0f,
                                                   &extent));
    CMP_TEST_OK(
        cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE));
    CMP_TEST_EXPECT(cmp_list_test_compute_content_extent(2u, 1.0f, 1.0f, 0.0f,
                                                       0.0f, &extent),
                   CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());

    CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(1u, 1.0f, 0.0f, 0.0f,
                                                      0.0f, 1.0f, 0u, NULL,
                                                      &last, &count),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_list_test_compute_visible_range(
        0u, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0u, &first, &last, &count));
    CMP_TEST_ASSERT(count == 0u);
    CMP_TEST_OK(cmp_list_test_compute_visible_range(
        1u, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0u, &first, &last, &count));
    CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(1u, 0.0f, 0.0f, 0.0f,
                                                      0.0f, 1.0f, 0u, &first,
                                                      &last, &count),
                   CMP_ERR_RANGE);
    CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(1u, 1.0f, -1.0f, 0.0f,
                                                      0.0f, 1.0f, 0u, &first,
                                                      &last, &count),
                   CMP_ERR_RANGE);
    CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(1u, 1.0f, 0.0f, -1.0f,
                                                      0.0f, 1.0f, 0u, &first,
                                                      &last, &count),
                   CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_list_test_compute_visible_range(
        1u, 1.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0u, &first, &last, &count));
    CMP_TEST_OK(cmp_list_test_compute_visible_range(
        1u, 1.0f, 0.0f, 0.0f, -10.0f, 5.0f, 0u, &first, &last, &count));
    CMP_TEST_OK(cmp_list_test_compute_visible_range(
        2u, 1.0f, 0.0f, 0.0f, 0.0f, 0.00005f, 0u, &first, &last, &count));
    CMP_TEST_OK(cmp_list_test_compute_visible_range(
        1u, 1.0f, 0.0f, 0.0f, 100.0f, 1.0f, 0u, &first, &last, &count));
    CMP_TEST_OK(cmp_list_test_compute_visible_range(
        2u, 1.0f, 0.0f, 0.0f, 0.0f, 10.0f, 0u, &first, &last, &count));
    CMP_TEST_OK(cmp_list_test_compute_visible_range(
        3u, 1.0f, 0.0f, 0.0f, 0.0f, 1.1f, 1u, &first, &last, &count));
    CMP_TEST_OK(cmp_list_test_compute_visible_range(
        3u, 1.0f, 0.0f, 0.0f, 0.0f, 1.1f, 10u, &first, &last, &count));
    CMP_TEST_OK(cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_VISIBLE_STRIDE));
    CMP_TEST_EXPECT(cmp_list_test_compute_visible_range(1u, 1.0f, 0.0f, 0.0f,
                                                      0.0f, 1.0f, 0u, &first,
                                                      &last, &count),
                   CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());
    CMP_TEST_OK(cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_VISIBLE_END));
    CMP_TEST_OK(cmp_list_test_compute_visible_range(
        1u, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0u, &first, &last, &count));
    CMP_TEST_ASSERT(count == 0u);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());
    CMP_TEST_OK(cmp_list_test_set_fail_point(
        CMP_LIST_TEST_FAIL_VISIBLE_LAST_BEFORE_FIRST));
    CMP_TEST_OK(cmp_list_test_compute_visible_range(
        20u, 1.0f, 0.0f, 0.0f, 10.0f, 1.0f, 0u, &first, &last, &count));
    CMP_TEST_ASSERT(count == 0u);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());
    CMP_TEST_OK(cmp_list_test_set_fail_point(
        CMP_LIST_TEST_FAIL_VISIBLE_LAST_AFTER_OVERSCAN));
    CMP_TEST_OK(cmp_list_test_compute_visible_range(
        20u, 1.0f, 0.0f, 0.0f, 2.0f, 1.0f, 1u, &first, &last, &count));
    CMP_TEST_ASSERT(count == 0u);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());

    CMP_TEST_OK(cmp_list_test_set_fail_point(
        CMP_LIST_TEST_FAIL_VISIBLE_LAST_BEFORE_FIRST));
    CMP_TEST_OK(cmp_list_test_compute_visible_range(
        5u, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0u, &first, &last, &count));
    CMP_TEST_ASSERT(count == 1u);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());
    CMP_TEST_OK(cmp_list_test_set_fail_point(
        CMP_LIST_TEST_FAIL_VISIBLE_LAST_AFTER_OVERSCAN));
    CMP_TEST_OK(cmp_list_test_compute_visible_range(
        5u, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0u, &first, &last, &count));
    CMP_TEST_ASSERT(count == 1u);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());

    memset(&temp_list, 0, sizeof(temp_list));
    temp_list.item_count = 2u;
    CMP_TEST_EXPECT(cmp_list_test_compute_item_bounds(NULL, 0u, &rect),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_list_test_compute_item_bounds(&temp_list, 2u, &rect),
                   CMP_ERR_NOT_FOUND);
    temp_list.style.item_extent = 0.0f;
    temp_list.style.spacing = 0.0f;
    CMP_TEST_EXPECT(cmp_list_test_compute_item_bounds(&temp_list, 0u, &rect),
                   CMP_ERR_RANGE);
    temp_list.style.item_extent = 10.0f;
    temp_list.style.spacing = 1.0f;
    temp_list.bounds.width = 0.0f;
    temp_list.style.padding.left = 2.0f;
    temp_list.style.padding.right = 2.0f;
    CMP_TEST_EXPECT(cmp_list_test_compute_item_bounds(&temp_list, 0u, &rect),
                   CMP_ERR_RANGE);
    temp_list.bounds.width = 20.0f;
    temp_list.bounds.height = 20.0f;
    temp_list.style.padding.left = 0.0f;
    temp_list.style.padding.right = 0.0f;
    temp_list.style.padding.top = 0.0f;
    temp_list.style.padding.bottom = 0.0f;
    temp_list.style.orientation = CMP_LIST_ORIENTATION_VERTICAL;
    CMP_TEST_OK(cmp_list_test_compute_item_bounds(&temp_list, 0u, &rect));

    memset(&temp_grid, 0, sizeof(temp_grid));
    CMP_TEST_EXPECT(
        cmp_list_test_grid_compute_visible_range(NULL, &first, &last, &count),
        CMP_ERR_INVALID_ARGUMENT);
    temp_grid.style = grid_style_local;
    temp_grid.item_count = 0u;
    CMP_TEST_OK(cmp_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    temp_grid.item_count = 1u;
    temp_grid.style.item_height = -1.0f;
    CMP_TEST_EXPECT(cmp_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                           &last, &count),
                   CMP_ERR_RANGE);
    temp_grid.style.item_height = 10.0f;
    temp_grid.bounds.height = 0.0f;
    CMP_TEST_OK(cmp_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    temp_grid.bounds.height = 10.0f;
    temp_grid.style.padding.top = 20.0f;
    CMP_TEST_OK(cmp_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    temp_grid.style.padding.top = 0.0f;
    temp_grid.style.span = 2u;
    temp_grid.item_count = 3u;
    temp_grid.style.spacing_y = 0.0f;
    temp_grid.scroll_offset = 0.0f;
    CMP_TEST_OK(cmp_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    temp_grid.scroll_offset = 100.0f;
    CMP_TEST_OK(cmp_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    CMP_TEST_OK(
        cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_LINE_COUNT_ZERO));
    CMP_TEST_OK(cmp_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    CMP_TEST_ASSERT(count == 0u);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());
    CMP_TEST_OK(cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_STRIDE));
    CMP_TEST_EXPECT(cmp_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                           &last, &count),
                   CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());
    CMP_TEST_OK(cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_END));
    CMP_TEST_OK(cmp_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    CMP_TEST_ASSERT(count == 0u);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());
    CMP_TEST_OK(
        cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_END_NEGATIVE));
    CMP_TEST_OK(cmp_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    CMP_TEST_OK(cmp_list_test_clear_fail_points());
    CMP_TEST_OK(
        cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_LAST_TOO_LARGE));
    CMP_TEST_OK(cmp_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    CMP_TEST_OK(cmp_list_test_clear_fail_points());
    CMP_TEST_OK(
        cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_LAST_BEFORE_FIRST));
    CMP_TEST_OK(cmp_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    CMP_TEST_ASSERT(count == 0u);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());

    temp_grid.item_count = 4u;
    temp_grid.style.span = 2u;
    temp_grid.style.scroll_axis = CMP_GRID_SCROLL_VERTICAL;
    temp_grid.style.item_height = 10.0f;
    temp_grid.style.spacing_y = 0.0f;
    temp_grid.style.padding.top = 0.0f;
    temp_grid.style.padding.bottom = 0.0f;
    temp_grid.bounds.height = 25.0f;
    temp_grid.scroll_offset = 0.0f;
    CMP_TEST_OK(
        cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_LAST_TOO_LARGE));
    CMP_TEST_OK(cmp_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    CMP_TEST_ASSERT(count > 0u);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());

    temp_grid.item_count = 6u;
    temp_grid.style.span = 2u;
    temp_grid.style.scroll_axis = CMP_GRID_SCROLL_VERTICAL;
    temp_grid.style.item_height = 10.0f;
    temp_grid.style.spacing_y = 0.0f;
    temp_grid.style.padding.top = 0.0f;
    temp_grid.style.padding.bottom = 0.0f;
    temp_grid.bounds.height = 5.0f;
    temp_grid.scroll_offset = 15.0f;
    CMP_TEST_OK(
        cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_LAST_BEFORE_FIRST));
    CMP_TEST_OK(cmp_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    CMP_TEST_ASSERT(count == 0u);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());

    temp_grid.scroll_offset = 0.0f;
    CMP_TEST_OK(
        cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_LAST_BEFORE_FIRST));
    CMP_TEST_OK(cmp_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    CMP_TEST_ASSERT(count > 0u);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());

    temp_grid.item_count = 6u;
    temp_grid.style.span = 2u;
    temp_grid.style.overscan = 2u;
    temp_grid.bounds.height = 15.0f;
    temp_grid.scroll_offset = 0.0f;
    CMP_TEST_OK(cmp_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    CMP_TEST_ASSERT(count > 0u);

    huge_span = ((cmp_usize) ~(cmp_usize)0 / 2u) + 2u;
    memset(&temp_grid, 0, sizeof(temp_grid));
    temp_grid.style.scroll_axis = CMP_GRID_SCROLL_VERTICAL;
    temp_grid.style.span = huge_span;
    temp_grid.style.item_height = 1.0f;
    temp_grid.style.spacing_y = 0.0f;
    temp_grid.style.padding.top = 0.0f;
    temp_grid.style.padding.bottom = 0.0f;
    temp_grid.item_count = huge_span + 1u;
    temp_grid.bounds.height = 1.0f;
    temp_grid.scroll_offset = 1.2f;
    CMP_TEST_OK(cmp_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    CMP_TEST_ASSERT(count == 0u);

    memset(&temp_grid, 0, sizeof(temp_grid));
    temp_grid.style = grid_style_local;
    temp_grid.item_count = 1u;

    CMP_TEST_EXPECT(cmp_list_test_grid_compute_item_bounds(NULL, 0u, &rect),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_list_test_grid_compute_item_bounds(&temp_grid, 10u, &rect),
        CMP_ERR_NOT_FOUND);
    temp_grid.style.item_width = 0.0f;
    CMP_TEST_EXPECT(cmp_list_test_grid_compute_item_bounds(&temp_grid, 0u, &rect),
                   CMP_ERR_RANGE);
    temp_grid.style.item_width = 10.0f;
    temp_grid.style.item_height = 10.0f;
    CMP_TEST_OK(cmp_list_test_grid_compute_item_bounds(&temp_grid, 0u, &rect));
  }

  CMP_TEST_EXPECT(cmp_list_view_init(NULL, &list_style, NULL, 0, 0),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_view_init(&list, NULL, NULL, 0, 0),
                 CMP_ERR_INVALID_ARGUMENT);

  list_style.orientation = 99u;
  CMP_TEST_EXPECT(cmp_list_view_init(&list, &list_style, NULL, 0, 0),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_list_style_init(&list_style));

  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_list_view_init(&list, &list_style, NULL, 0, 0),
                 CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_FALSE));

  CMP_TEST_EXPECT(cmp_list_view_set_bind(NULL, NULL, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_view_set_item_count(NULL, 1), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_view_set_style(NULL, &list_style),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_view_set_scroll(NULL, 0.0f), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_view_get_scroll(NULL, &scroll),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_view_get_content_extent(NULL, &content_extent),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_view_get_required_slots(NULL, &visible_count),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_view_update(NULL), CMP_ERR_INVALID_ARGUMENT);

  {
    CMPListView metrics_view;
    CMPListStyle metrics_style;

    CMP_TEST_EXPECT(cmp_list_test_update_metrics(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_list_style_init(&metrics_style));
    CMP_TEST_OK(cmp_list_view_init(&metrics_view, &metrics_style, NULL, 0, 0));
    metrics_view.bounds.height = -1.0f;
    CMP_TEST_EXPECT(cmp_list_test_update_metrics(&metrics_view), CMP_ERR_RANGE);
    metrics_view.bounds.height = 10.0f;
    metrics_view.scroll_offset = -5.0f;
    CMP_TEST_OK(cmp_list_test_update_metrics(&metrics_view));
    CMP_TEST_ASSERT(metrics_view.scroll_offset == 0.0f);
    metrics_view.scroll_offset = 5.0f;
    CMP_TEST_OK(cmp_list_test_update_metrics(&metrics_view));
    CMP_TEST_OK(metrics_view.widget.vtable->destroy(metrics_view.widget.ctx));
  }

  {
    CMPGridView metrics_grid;
    CMPGridStyle metrics_grid_style;

    CMP_TEST_EXPECT(cmp_list_test_grid_update_metrics(NULL),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_grid_style_init(&metrics_grid_style));
    CMP_TEST_OK(
        cmp_grid_view_init(&metrics_grid, &metrics_grid_style, NULL, 0, 0));
    metrics_grid.style.spacing_y = -1.0f;
    CMP_TEST_EXPECT(cmp_list_test_grid_update_metrics(&metrics_grid),
                   CMP_ERR_RANGE);
    metrics_grid.style.spacing_y = 0.0f;
    metrics_grid.style.span = 0u;
    CMP_TEST_EXPECT(cmp_list_test_grid_update_metrics(&metrics_grid),
                   CMP_ERR_RANGE);
    metrics_grid.style.span = 1u;
    metrics_grid.item_count = 0u;
    metrics_grid.bounds.height = 10.0f;
    CMP_TEST_OK(cmp_list_test_grid_update_metrics(&metrics_grid));
    metrics_grid.item_count = 2u;
    metrics_grid.style.item_height = 0.0f;
    CMP_TEST_EXPECT(cmp_list_test_grid_update_metrics(&metrics_grid),
                   CMP_ERR_RANGE);
    metrics_grid.style.item_height = metrics_grid_style.item_height;
    metrics_grid.bounds.height = -1.0f;
    CMP_TEST_EXPECT(cmp_list_test_grid_update_metrics(&metrics_grid),
                   CMP_ERR_RANGE);
    metrics_grid.bounds.height = 10.0f;
    metrics_grid.scroll_offset = -5.0f;
    CMP_TEST_OK(cmp_list_test_grid_update_metrics(&metrics_grid));
    CMP_TEST_ASSERT(metrics_grid.scroll_offset == 0.0f);
    metrics_grid.scroll_offset = 50.0f;
    CMP_TEST_OK(cmp_list_test_grid_update_metrics(&metrics_grid));
    CMP_TEST_OK(metrics_grid.widget.vtable->destroy(metrics_grid.widget.ctx));
  }

  {
    CMPListSlot *slots;
    CMPRenderNode **nodes;
    cmp_usize capacity;

    slots = NULL;
    nodes = NULL;
    capacity = 0;
    CMP_TEST_EXPECT(cmp_list_test_reserve_slots(NULL, NULL, NULL, NULL, 1),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_list_test_reserve_slots(&slots, &nodes, &capacity, NULL, 1),
        CMP_ERR_INVALID_ARGUMENT);
  }

  CMP_TEST_EXPECT(cmp_list_test_mul_overflow(1, 1, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_list_test_mul_overflow(2, 3, &overflow_value));
  CMP_TEST_ASSERT(overflow_value == 6);
  overflow_value = ((cmp_usize) ~(cmp_usize)0) / (cmp_usize)sizeof(CMPListSlot) + 1;
  CMP_TEST_EXPECT(cmp_list_test_mul_overflow(overflow_value,
                                           (cmp_usize)sizeof(CMPListSlot),
                                           &overflow_value),
                 CMP_ERR_OVERFLOW);

  bad_alloc.ctx = NULL;
  bad_alloc.alloc = NULL;
  bad_alloc.realloc = NULL;
  bad_alloc.free = NULL;
  CMP_TEST_EXPECT(cmp_list_view_init(&list, &list_style, &bad_alloc, 0, 0),
                 CMP_ERR_INVALID_ARGUMENT);

  test_allocator_init(&alloc);
  alloc.fail_alloc_on = 1;
  alloc_iface.ctx = &alloc;
  alloc_iface.alloc = test_alloc;
  alloc_iface.realloc = test_realloc;
  alloc_iface.free = test_free;
  CMP_TEST_EXPECT(cmp_list_view_init(&list, &list_style, &alloc_iface, 0, 2),
                 CMP_ERR_OUT_OF_MEMORY);

  alloc.fail_alloc_on = 0;
  CMP_TEST_OK(cmp_list_view_init(&list, &list_style, &alloc_iface, 5, 0));
  CMP_TEST_EXPECT(cmp_list_view_reserve(NULL, 1), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_list_view_reserve(&list, 4));
  CMP_TEST_ASSERT(list.slot_capacity == 4);
  CMP_TEST_ASSERT(list.slots != NULL);
  CMP_TEST_ASSERT(list.visible_nodes != NULL);
  CMP_TEST_OK(cmp_list_view_reserve(&list, 2));
  CMP_TEST_EXPECT(cmp_list_view_reserve(&list, overflow_value), CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_RESERVE_NODE_BYTES));
  CMP_TEST_EXPECT(cmp_list_view_reserve(&list, 8), CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_list_test_clear_fail_points());
  CMP_TEST_OK(cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_RESERVE_COPY_BYTES));
  CMP_TEST_EXPECT(cmp_list_view_reserve(&list, 8), CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_list_test_clear_fail_points());
  {
    CMPListView list_bad;

    list_bad = list;
    list_bad.allocator.alloc = NULL;
    list_bad.allocator.free = NULL;
    CMP_TEST_EXPECT(cmp_list_view_reserve(&list_bad, 1), CMP_ERR_INVALID_ARGUMENT);
  }
  {
    CMPListView list_fail;
    TestAllocator alloc_fail;
    CMPAllocator alloc_iface_fail;

    test_allocator_init(&alloc_fail);
    alloc_fail.fail_alloc_on = 2;
    alloc_iface_fail.ctx = &alloc_fail;
    alloc_iface_fail.alloc = test_alloc;
    alloc_iface_fail.realloc = test_realloc;
    alloc_iface_fail.free = test_free;
    CMP_TEST_OK(
        cmp_list_view_init(&list_fail, &list_style, &alloc_iface_fail, 0, 0));
    CMP_TEST_EXPECT(cmp_list_view_reserve(&list_fail, 1), CMP_ERR_OUT_OF_MEMORY);
    CMP_TEST_OK(list_fail.widget.vtable->destroy(list_fail.widget.ctx));
  }
  {
    CMPListView list_bad;
    memset(&list_bad, 0, sizeof(list_bad));
    CMP_TEST_EXPECT(cmp_list_view_reserve(&list_bad, 1), CMP_ERR_INVALID_ARGUMENT);
  }

  list_style = list.style;
  list_style.padding.left = -1.0f;
  CMP_TEST_EXPECT(cmp_list_view_set_style(&list, &list_style), CMP_ERR_RANGE);
  list_style.padding.left = 0.0f;
  list_style.background_color.r = -0.1f;
  CMP_TEST_EXPECT(cmp_list_view_set_style(&list, &list_style), CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_list_style_init(&list_style));

  list_style = list.style;
  list_style.item_extent = 10.0f;
  list_style.spacing = 2.0f;
  list_style.padding.left = 1.0f;
  list_style.padding.top = 1.0f;
  list_style.padding.right = 1.0f;
  list_style.padding.bottom = 1.0f;
  list_style.overscan = 1u;
  CMP_TEST_OK(cmp_list_view_set_style(&list, &list_style));
  CMP_TEST_OK(cmp_list_view_set_item_count(&list, 5));

  CMP_TEST_OK(cmp_list_view_set_item_count(&list, 0));
  CMP_TEST_OK(cmp_list_view_get_content_extent(&list, &content_extent));
  CMP_TEST_ASSERT(content_extent == 2.0f);
  CMP_TEST_OK(cmp_list_view_set_item_count(&list, 5));

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 5.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 30.0f;
  CMP_TEST_EXPECT(
      list.widget.vtable->measure(NULL, width_spec, height_spec, &measured),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                             height_spec, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  list_style = list.style;
  list.style.orientation = 99u;
  CMP_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                             height_spec, &measured),
                 CMP_ERR_RANGE);
  list.style = list_style;

  list.bounds.height = -1.0f;
  CMP_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                             height_spec, &measured),
                 CMP_ERR_RANGE);
  list.bounds.height = 0.0f;

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 10.0f;
  CMP_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                             height_spec, &measured),
                 CMP_ERR_RANGE);

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 5.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 30.0f;
  CMP_TEST_OK(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                         height_spec, &measured));
  CMP_TEST_ASSERT(measured.width == 2.0f);
  CMP_TEST_ASSERT(measured.height == 30.0f);

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_OK(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                         height_spec, &measured));
  CMP_TEST_ASSERT(measured.width == 2.0f);
  CMP_TEST_ASSERT(measured.height == 60.0f);

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 100.0f;
  height_spec.mode = CMP_MEASURE_EXACTLY;
  height_spec.size = 50.0f;
  CMP_TEST_OK(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                         height_spec, &measured));
  CMP_TEST_ASSERT(measured.width == 100.0f);
  CMP_TEST_ASSERT(measured.height == 50.0f);

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  CMP_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                             height_spec, &measured),
                 CMP_ERR_INVALID_ARGUMENT);

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 5.0f;
  height_spec.mode = 99u;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                             height_spec, &measured),
                 CMP_ERR_INVALID_ARGUMENT);

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 10.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 10.0f;
  CMP_TEST_OK(cmp_list_test_set_fail_point(
      CMP_LIST_TEST_FAIL_LIST_MEASURE_CONTENT_NEGATIVE));
  CMP_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                             height_spec, &measured),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_list_test_clear_fail_points());

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  CMP_TEST_EXPECT(list.widget.vtable->layout(list.widget.ctx, bounds),
                 CMP_ERR_RANGE);
  CMP_TEST_EXPECT(list.widget.vtable->layout(NULL, bounds),
                 CMP_ERR_INVALID_ARGUMENT);

  bounds.width = 50.0f;
  bounds.height = 0.0f;
  CMP_TEST_OK(list.widget.vtable->layout(list.widget.ctx, bounds));
  CMP_TEST_OK(cmp_list_view_get_required_slots(&list, &visible_count));
  CMP_TEST_ASSERT(visible_count == 0);

  bounds.height = 25.0f;
  CMP_TEST_OK(list.widget.vtable->layout(list.widget.ctx, bounds));
  CMP_TEST_ASSERT(list.bounds.width == 50.0f);

  bind_ctx.pool = pool;
  bind_ctx.pool_count = CMP_COUNTOF(pool);
  bind_ctx.next = 0;
  bind_ctx.calls = 0;
  bind_ctx.last_index = 0;
  bind_ctx.fail = 0;
  CMP_TEST_OK(cmp_list_view_set_bind(&list, test_bind, &bind_ctx));

  CMP_TEST_OK(cmp_list_view_get_required_slots(&list, &visible_count));
  CMP_TEST_ASSERT(visible_count == 3);

  CMP_TEST_OK(cmp_list_view_reserve(&list, 5));
  CMP_TEST_OK(cmp_list_view_update(&list));
  CMP_TEST_ASSERT(list.visible_count == 3);
  CMP_TEST_ASSERT(list.visible_first == 0);
  CMP_TEST_ASSERT(list.visible_last == 2);
  CMP_TEST_ASSERT(list.slots[0].index == 0);
  CMP_TEST_ASSERT(list.slots[1].index == 1);
  CMP_TEST_ASSERT(list.slots[2].index == 2);
  CMP_TEST_ASSERT(list.slots[0].node.bounds.y == 1.0f);
  CMP_TEST_ASSERT(list.slots[1].node.bounds.y == 13.0f);

  CMP_TEST_OK(cmp_list_view_get_visible(&list, &visible, &visible_count));
  CMP_TEST_ASSERT(visible_count == 3);
  CMP_TEST_ASSERT(visible[0] == &list.slots[0].node);

  CMP_TEST_OK(cmp_list_view_init(&list_small, &list_style, NULL, 5, 2));
  CMP_TEST_OK(cmp_list_view_set_bind(&list_small, test_bind, &bind_ctx));
  CMP_TEST_OK(list_small.widget.vtable->layout(list_small.widget.ctx, bounds));
  CMP_TEST_EXPECT(cmp_list_view_update(&list_small), CMP_ERR_RANGE);
  CMP_TEST_OK(list_small.widget.vtable->destroy(list_small.widget.ctx));

  CMP_TEST_OK(cmp_list_view_set_scroll(&list, 100.0f));
  CMP_TEST_OK(cmp_list_view_get_scroll(&list, &scroll));
  CMP_TEST_ASSERT(scroll == 35.0f);

  list_style.overscan = 0u;
  CMP_TEST_OK(cmp_list_view_set_style(&list, &list_style));
  CMP_TEST_OK(cmp_list_view_set_scroll(&list, 15.0f));
  CMP_TEST_OK(cmp_list_view_update(&list));
  CMP_TEST_ASSERT(list.visible_first == 1);
  CMP_TEST_ASSERT(list.visible_last == 3);
  CMP_TEST_ASSERT(list.visible_count == 3);

  list_style.item_extent = 0.0f;
  CMP_TEST_EXPECT(cmp_list_view_set_style(&list, &list_style), CMP_ERR_RANGE);
  list.style.item_extent = 0.0f;
  CMP_TEST_EXPECT(cmp_list_view_update(&list), CMP_ERR_RANGE);
  list_style.item_extent = 10.0f;
  CMP_TEST_OK(cmp_list_view_set_style(&list, &list_style));

  CMP_TEST_OK(cmp_list_view_set_bind(&list, NULL, NULL));
  CMP_TEST_EXPECT(cmp_list_view_update(&list), CMP_ERR_STATE);
  CMP_TEST_OK(cmp_list_view_set_bind(&list, test_bind, &bind_ctx));

  bind_ctx.fail = 1;
  CMP_TEST_EXPECT(cmp_list_view_update(&list), CMP_ERR_UNKNOWN);
  bind_ctx.fail = 0;

  CMP_TEST_OK(cmp_list_view_set_bind(&list, test_bind_missing_widget, NULL));
  list.slots[0].node.widget = NULL;
  CMP_TEST_EXPECT(cmp_list_view_update(&list), CMP_ERR_STATE);
  CMP_TEST_OK(cmp_list_view_set_bind(&list, test_bind, &bind_ctx));

  CMP_TEST_EXPECT(cmp_list_view_get_visible(&list, NULL, &visible_count),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_list_view_get_visible(&list, &visible, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  gfx_backend.draw_rect_calls = 0;
  gfx.ctx = &gfx_backend;
  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = NULL;
  paint_ctx.gfx = &gfx;
  paint_ctx.clip = bounds;
  paint_ctx.dpi_scale = 1.0f;
  CMP_TEST_EXPECT(list.widget.vtable->paint(NULL, &paint_ctx),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(list.widget.vtable->paint(list.widget.ctx, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  {
    CMPPaintContext bad_ctx;

    bad_ctx = paint_ctx;
    bad_ctx.gfx = NULL;
    CMP_TEST_EXPECT(list.widget.vtable->paint(list.widget.ctx, &bad_ctx),
                   CMP_ERR_INVALID_ARGUMENT);
  }
  list.style.background_color.a = 1.0f;
  CMP_TEST_OK(list.widget.vtable->paint(list.widget.ctx, &paint_ctx));
  CMP_TEST_ASSERT(gfx_backend.draw_rect_calls == 1);

  gfx.vtable = &g_test_gfx_vtable_no_draw;
  list.style.background_color.a = 0.0f;
  CMP_TEST_OK(list.widget.vtable->paint(list.widget.ctx, &paint_ctx));

  list.style.background_color.a = 1.0f;
  CMP_TEST_EXPECT(list.widget.vtable->paint(list.widget.ctx, &paint_ctx),
                 CMP_ERR_UNSUPPORTED);

  {
    CMPSemantics semantics;
    CMP_TEST_EXPECT(list.widget.vtable->get_semantics(NULL, &semantics),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(list.widget.vtable->get_semantics(list.widget.ctx, NULL),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(list.widget.vtable->get_semantics(list.widget.ctx, &semantics));
    CMP_TEST_ASSERT(semantics.role == CMP_SEMANTIC_NONE);
  }

  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_POINTER_DOWN;
  handled = CMP_TRUE;
  CMP_TEST_EXPECT(list.widget.vtable->event(NULL, &event, &handled),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(list.widget.vtable->event(list.widget.ctx, NULL, &handled),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(list.widget.vtable->event(list.widget.ctx, &event, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(list.widget.vtable->event(list.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  event.type = CMP_INPUT_POINTER_SCROLL;
  event.data.pointer.scroll_y = 0;
  handled = CMP_TRUE;
  CMP_TEST_OK(list.widget.vtable->event(list.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  event.data.pointer.scroll_y = 5;
  handled = CMP_FALSE;
  CMP_TEST_OK(list.widget.vtable->event(list.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_OK(cmp_list_view_get_scroll(&list, &scroll));
  CMP_TEST_ASSERT(scroll == 20.0f);

  list.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  event.data.pointer.scroll_y = 5;
  handled = CMP_TRUE;
  CMP_TEST_OK(list.widget.vtable->event(list.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  CMP_TEST_OK(cmp_list_view_get_scroll(&list, &scroll));
  CMP_TEST_ASSERT(scroll == 20.0f);
  list.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  list_style = list.style;
  list_style.orientation = CMP_LIST_ORIENTATION_HORIZONTAL;
  list_style.item_extent = 5.0f;
  list_style.spacing = 1.0f;
  list_style.padding.left = 2.0f;
  list_style.padding.right = 2.0f;
  list_style.padding.top = 1.0f;
  list_style.padding.bottom = 1.0f;
  list_style.overscan = 0u;
  CMP_TEST_OK(cmp_list_view_set_style(&list, &list_style));
  CMP_TEST_OK(cmp_list_view_set_item_count(&list, 3));
  CMP_TEST_OK(cmp_list_view_set_scroll(&list, 0.0f));
  bounds.width = 20.0f;
  bounds.height = 10.0f;
  CMP_TEST_OK(list.widget.vtable->layout(list.widget.ctx, bounds));
  CMP_TEST_OK(cmp_list_view_update(&list));
  CMP_TEST_ASSERT(list.slots[0].node.bounds.x == 2.0f);
  CMP_TEST_ASSERT(list.slots[1].node.bounds.x == 8.0f);

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 10.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 5.0f;
  CMP_TEST_OK(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                         height_spec, &measured));
  CMP_TEST_ASSERT(measured.width == 10.0f);
  CMP_TEST_ASSERT(measured.height == 2.0f);

  event.type = CMP_INPUT_POINTER_SCROLL;
  event.data.pointer.scroll_x = 4;
  handled = CMP_FALSE;
  CMP_TEST_OK(list.widget.vtable->event(list.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  {
    CMPSemantics semantics;
    CMPListStyle saved_style;
    CMPRect saved_bounds;
    cmp_usize saved_item_count;

    saved_style = list.style;
    saved_bounds = list.bounds;
    saved_item_count = list.item_count;

    width_spec.mode = CMP_MEASURE_AT_MOST;
    width_spec.size = 10.0f;
    height_spec.mode = 99u;
    height_spec.size = 0.0f;
    CMP_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                               height_spec, &measured),
                   CMP_ERR_INVALID_ARGUMENT);

    height_spec.mode = CMP_MEASURE_AT_MOST;
    height_spec.size = 10.0f;
    CMP_TEST_OK(cmp_list_test_set_fail_point(
        CMP_LIST_TEST_FAIL_LIST_MEASURE_CONTENT_NEGATIVE));
    CMP_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                               height_spec, &measured),
                   CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());

    list.style.spacing = -1.0f;
    CMP_TEST_EXPECT(list.widget.vtable->paint(list.widget.ctx, &paint_ctx),
                   CMP_ERR_RANGE);
    list.style = saved_style;
    list.bounds.width = -1.0f;
    CMP_TEST_EXPECT(list.widget.vtable->paint(list.widget.ctx, &paint_ctx),
                   CMP_ERR_RANGE);
    list.bounds = saved_bounds;

    list.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
    CMP_TEST_OK(list.widget.vtable->get_semantics(list.widget.ctx, &semantics));
    CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_DISABLED) != 0u);
    list.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

    event.type = CMP_INPUT_POINTER_SCROLL;
    list.style.orientation = CMP_LIST_ORIENTATION_VERTICAL;
    event.data.pointer.scroll_y = 3;
    list.style.item_extent = 0.0f;
    CMP_TEST_EXPECT(list.widget.vtable->event(list.widget.ctx, &event, &handled),
                   CMP_ERR_RANGE);
    list.style = saved_style;

    list.bounds.width = -1.0f;
    list.bounds.height = -1.0f;
    CMP_TEST_EXPECT(cmp_list_view_set_item_count(&list, 1), CMP_ERR_RANGE);
    list.bounds = saved_bounds;

    list.bounds.width = -1.0f;
    list.bounds.height = -1.0f;
    CMP_TEST_EXPECT(cmp_list_view_set_style(&list, &saved_style), CMP_ERR_RANGE);
    list.bounds = saved_bounds;
    list.style = saved_style;

    list.bounds.width = -1.0f;
    list.bounds.height = -1.0f;
    CMP_TEST_EXPECT(cmp_list_view_get_content_extent(&list, &content_extent),
                   CMP_ERR_RANGE);
    list.bounds = saved_bounds;

    CMP_TEST_OK(cmp_list_view_set_scroll(&list, -5.0f));
    CMP_TEST_OK(cmp_list_view_get_scroll(&list, &scroll));
    CMP_TEST_ASSERT(scroll == 0.0f);

    list.bounds.width = -1.0f;
    list.bounds.height = -1.0f;
    CMP_TEST_EXPECT(cmp_list_view_set_scroll(&list, 1.0f), CMP_ERR_RANGE);
    list.bounds = saved_bounds;

    list.style.spacing = -1.0f;
    CMP_TEST_EXPECT(cmp_list_view_get_required_slots(&list, &visible_count),
                   CMP_ERR_RANGE);
    list.style = saved_style;

    list.bounds.width = -1.0f;
    list.bounds.height = -1.0f;
    CMP_TEST_EXPECT(cmp_list_view_get_required_slots(&list, &visible_count),
                   CMP_ERR_RANGE);
    list.bounds = saved_bounds;

    list.style.orientation = CMP_LIST_ORIENTATION_HORIZONTAL;
    list.bounds.width = 30.0f;
    CMP_TEST_OK(cmp_list_view_get_required_slots(&list, &visible_count));
    list.style = saved_style;
    list.bounds = saved_bounds;

    CMP_TEST_OK(cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_VISIBLE_STRIDE));
    CMP_TEST_EXPECT(cmp_list_view_get_required_slots(&list, &visible_count),
                   CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());

    list.style.spacing = -1.0f;
    CMP_TEST_EXPECT(cmp_list_view_update(&list), CMP_ERR_RANGE);
    list.style = saved_style;

    list.bounds.width = -1.0f;
    list.bounds.height = -1.0f;
    CMP_TEST_EXPECT(cmp_list_view_update(&list), CMP_ERR_RANGE);
    list.bounds = saved_bounds;

    CMP_TEST_OK(cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_VISIBLE_STRIDE));
    CMP_TEST_EXPECT(cmp_list_view_update(&list), CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());

    list.item_count = 0u;
    CMP_TEST_OK(cmp_list_view_update(&list));

    list.item_count = saved_item_count;
    CMP_TEST_OK(cmp_list_view_set_bind(&list, test_bind, &bind_ctx));
    CMP_TEST_OK(cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_LIST_ITEM_BOUNDS));
    CMP_TEST_EXPECT(cmp_list_view_update(&list), CMP_ERR_IO);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());
    CMP_TEST_OK(
        cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_LIST_RENDER_BOUNDS));
    CMP_TEST_EXPECT(cmp_list_view_update(&list), CMP_ERR_IO);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());

    list.style = saved_style;
    list.bounds = saved_bounds;
    list.item_count = saved_item_count;
  }

  {
    CMPListView list_state;
    CMPListView list_range;
    CMPListStyle state_style;
    TestAllocator state_alloc;
    CMPAllocator state_iface;
    CMPRect state_bounds;

    test_allocator_init(&state_alloc);
    state_iface.ctx = &state_alloc;
    state_iface.alloc = test_alloc;
    state_iface.realloc = test_realloc;
    state_iface.free = test_free;
    CMP_TEST_OK(cmp_list_style_init(&state_style));
    state_style.item_extent = 10.0f;
    state_style.spacing = 0.0f;
    state_style.overscan = 0u;
    state_bounds.x = 0.0f;
    state_bounds.y = 0.0f;
    state_bounds.width = 50.0f;
    state_bounds.height = 25.0f;

    CMP_TEST_OK(
        cmp_list_view_init(&list_state, &state_style, &state_iface, 1, 0));
    list_state.bounds = state_bounds;
    list_state.item_count = 1u;
    CMP_TEST_EXPECT(cmp_list_view_update(&list_state), CMP_ERR_STATE);
    CMP_TEST_OK(list_state.widget.vtable->destroy(list_state.widget.ctx));

    CMP_TEST_OK(
        cmp_list_view_init(&list_range, &state_style, &state_iface, 5, 1));
    list_range.bounds = state_bounds;
    bind_ctx.pool = pool;
    bind_ctx.pool_count = CMP_COUNTOF(pool);
    bind_ctx.next = 0u;
    bind_ctx.calls = 0u;
    bind_ctx.last_index = 0u;
    bind_ctx.fail = 0;
    CMP_TEST_OK(cmp_list_view_set_bind(&list_range, test_bind, &bind_ctx));
    CMP_TEST_EXPECT(cmp_list_view_update(&list_range), CMP_ERR_RANGE);
    CMP_TEST_OK(list_range.widget.vtable->destroy(list_range.widget.ctx));
  }

  {
    TestAllocator free_alloc;
    CMPAllocator free_iface;
    CMPListView list_free;
    CMPListStyle free_style;

    test_allocator_init(&free_alloc);
    free_iface.ctx = &free_alloc;
    free_iface.alloc = test_alloc;
    free_iface.realloc = test_realloc;
    free_iface.free = test_free;
    CMP_TEST_OK(cmp_list_style_init(&free_style));
    CMP_TEST_OK(cmp_list_view_init(&list_free, &free_style, &free_iface, 1, 2));
    free_alloc.fail_free_on = 1u;
    CMP_TEST_EXPECT(list_free.widget.vtable->destroy(list_free.widget.ctx),
                   CMP_ERR_IO);
  }

  {
    TestAllocator free_alloc;
    CMPAllocator free_iface;
    CMPListView list_free;
    CMPListStyle free_style;

    test_allocator_init(&free_alloc);
    free_iface.ctx = &free_alloc;
    free_iface.alloc = test_alloc;
    free_iface.realloc = test_realloc;
    free_iface.free = test_free;
    CMP_TEST_OK(cmp_list_style_init(&free_style));
    CMP_TEST_OK(cmp_list_view_init(&list_free, &free_style, &free_iface, 1, 2));
    free_alloc.fail_free_on = 2u;
    CMP_TEST_EXPECT(list_free.widget.vtable->destroy(list_free.widget.ctx),
                   CMP_ERR_IO);
  }

  CMP_TEST_EXPECT(list.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(list.widget.vtable->destroy(list.widget.ctx));

  CMP_TEST_OK(cmp_grid_style_init(&grid_style));
  CMP_TEST_EXPECT(cmp_grid_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_ASSERT(grid_style.span == CMP_GRID_DEFAULT_SPAN);

  CMP_TEST_EXPECT(cmp_grid_view_set_bind(NULL, NULL, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_set_item_count(NULL, 1), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_set_style(NULL, &grid_style),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_set_scroll(NULL, 0.0f), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_get_scroll(NULL, &scroll),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_get_content_extent(NULL, &content_extent),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_get_required_slots(NULL, &visible_count),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_update(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_grid_view_init(NULL, &grid_style, NULL, 0, 0),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_init(&grid, NULL, NULL, 0, 0),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_grid_view_init(&grid, &grid_style, NULL, 0, 0),
                 CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_FALSE));

  CMP_TEST_OK(cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_RESERVE_NODE_BYTES));
  CMP_TEST_EXPECT(cmp_grid_view_init(&grid, &grid_style, NULL, 0, 2),
                 CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_list_test_clear_fail_points());

  grid_style.span = 0u;
  CMP_TEST_EXPECT(cmp_grid_view_init(&grid, &grid_style, NULL, 0, 0),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_grid_style_init(&grid_style));
  CMP_TEST_EXPECT(cmp_grid_view_init(&grid, &grid_style, &bad_alloc, 0, 0),
                 CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_grid_view_init(&grid, &grid_style, NULL, 5, 0));
  CMP_TEST_OK(cmp_grid_view_reserve(&grid, 6));
  CMP_TEST_EXPECT(cmp_grid_view_reserve(NULL, 1), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_reserve(&grid, overflow_value), CMP_ERR_OVERFLOW);

  grid_style = grid.style;
  grid_style.spacing_x = -1.0f;
  CMP_TEST_EXPECT(cmp_grid_view_set_style(&grid, &grid_style), CMP_ERR_RANGE);
  grid_style.spacing_x = 0.0f;
  grid_style.background_color.b = 2.0f;
  CMP_TEST_EXPECT(cmp_grid_view_set_style(&grid, &grid_style), CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_grid_style_init(&grid_style));

  grid_style = grid.style;
  grid_style.item_width = 10.0f;
  grid_style.item_height = 8.0f;
  grid_style.spacing_x = 2.0f;
  grid_style.spacing_y = 2.0f;
  grid_style.padding.left = 1.0f;
  grid_style.padding.top = 1.0f;
  grid_style.padding.right = 1.0f;
  grid_style.padding.bottom = 1.0f;
  grid_style.overscan = 0u;
  grid_style.span = 2u;
  CMP_TEST_OK(cmp_grid_view_set_style(&grid, &grid_style));
  CMP_TEST_OK(cmp_grid_view_set_bind(&grid, test_bind, &bind_ctx));

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 20.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 25.0f;
  CMP_TEST_EXPECT(
      grid.widget.vtable->measure(NULL, width_spec, height_spec, &measured),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                             height_spec, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  grid_style = grid.style;
  grid.style.span = 0u;
  CMP_TEST_EXPECT(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                             height_spec, &measured),
                 CMP_ERR_RANGE);
  grid.style = grid_style;

  grid.bounds.height = -1.0f;
  CMP_TEST_EXPECT(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                             height_spec, &measured),
                 CMP_ERR_RANGE);
  grid.bounds.height = 0.0f;

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 10.0f;
  CMP_TEST_EXPECT(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                             height_spec, &measured),
                 CMP_ERR_RANGE);

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 5.0f;
  height_spec.mode = 99u;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                             height_spec, &measured),
                 CMP_ERR_INVALID_ARGUMENT);

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 30.0f;
  height_spec.mode = CMP_MEASURE_EXACTLY;
  height_spec.size = 15.0f;
  CMP_TEST_OK(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                         height_spec, &measured));
  CMP_TEST_ASSERT(measured.width == 30.0f);
  CMP_TEST_ASSERT(measured.height == 15.0f);

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 20.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 25.0f;
  CMP_TEST_OK(cmp_list_test_set_fail_point(
      CMP_LIST_TEST_FAIL_GRID_MEASURE_CONTENT_NEGATIVE));
  CMP_TEST_EXPECT(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                             height_spec, &measured),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_list_test_clear_fail_points());

  width_spec.size = 20.0f;
  height_spec.size = 25.0f;
  CMP_TEST_OK(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                         height_spec, &measured));
  CMP_TEST_ASSERT(measured.width == 20.0f);
  CMP_TEST_ASSERT(measured.height == 25.0f);

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_OK(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                         height_spec, &measured));
  CMP_TEST_ASSERT(measured.width == 24.0f);
  CMP_TEST_ASSERT(measured.height == 30.0f);

  bounds.width = -1.0f;
  bounds.height = 20.0f;
  CMP_TEST_EXPECT(grid.widget.vtable->layout(grid.widget.ctx, bounds),
                 CMP_ERR_RANGE);
  bounds.width = 30.0f;
  bounds.height = 20.0f;
  CMP_TEST_EXPECT(grid.widget.vtable->layout(NULL, bounds),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(grid.widget.vtable->layout(grid.widget.ctx, bounds));
  CMP_TEST_OK(cmp_grid_view_set_bind(&grid, NULL, NULL));
  CMP_TEST_EXPECT(cmp_grid_view_update(&grid), CMP_ERR_STATE);
  CMP_TEST_OK(cmp_grid_view_set_bind(&grid, test_bind, &bind_ctx));
  CMP_TEST_OK(cmp_grid_view_update(&grid));
  CMP_TEST_ASSERT(grid.visible_count == 4);
  CMP_TEST_ASSERT(grid.slots[0].index == 0);
  CMP_TEST_ASSERT(grid.slots[1].index == 1);
  CMP_TEST_ASSERT(grid.slots[2].node.bounds.x == 1.0f);
  CMP_TEST_ASSERT(grid.slots[2].node.bounds.y == 11.0f);
  CMP_TEST_OK(cmp_grid_view_get_visible(&grid, &visible, &visible_count));
  CMP_TEST_ASSERT(visible_count == 4);
  CMP_TEST_ASSERT(visible[0] == &grid.slots[0].node);
  CMP_TEST_EXPECT(cmp_grid_view_get_visible(&grid, NULL, &visible_count),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_grid_view_get_visible(&grid, &visible, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_POINTER_DOWN;
  handled = CMP_TRUE;
  CMP_TEST_EXPECT(grid.widget.vtable->event(NULL, &event, &handled),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(grid.widget.vtable->event(grid.widget.ctx, NULL, &handled),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(grid.widget.vtable->event(grid.widget.ctx, &event, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(grid.widget.vtable->event(grid.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  event.type = CMP_INPUT_POINTER_SCROLL;
  event.data.pointer.scroll_y = 0;
  handled = CMP_TRUE;
  CMP_TEST_OK(grid.widget.vtable->event(grid.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  event.data.pointer.scroll_y = 4;
  handled = CMP_FALSE;
  CMP_TEST_OK(grid.widget.vtable->event(grid.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  grid.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  handled = CMP_TRUE;
  CMP_TEST_OK(grid.widget.vtable->event(grid.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  grid.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  grid_style.scroll_axis = CMP_GRID_SCROLL_HORIZONTAL;
  grid_style.span = 2u;
  CMP_TEST_OK(cmp_grid_view_set_style(&grid, &grid_style));
  CMP_TEST_OK(cmp_grid_view_set_scroll(&grid, 3.0f));
  CMP_TEST_OK(cmp_grid_view_get_required_slots(&grid, &visible_count));
  CMP_TEST_ASSERT(visible_count > 0);
  CMP_TEST_OK(cmp_grid_view_update(&grid));

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 30.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 15.0f;
  CMP_TEST_OK(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                         height_spec, &measured));
  CMP_TEST_ASSERT(measured.width == 30.0f);
  CMP_TEST_ASSERT(measured.height == 15.0f);

  event.type = CMP_INPUT_POINTER_SCROLL;
  event.data.pointer.scroll_x = 5;
  handled = CMP_FALSE;
  CMP_TEST_OK(grid.widget.vtable->event(grid.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_OK(cmp_grid_view_get_scroll(&grid, &scroll));
  CMP_TEST_ASSERT(scroll == 6.0f);

  gfx_backend.draw_rect_calls = 0;
  gfx.vtable = &g_test_gfx_vtable;
  grid.style.background_color.a = 1.0f;
  CMP_TEST_EXPECT(grid.widget.vtable->paint(NULL, &paint_ctx),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(grid.widget.vtable->paint(grid.widget.ctx, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  {
    CMPPaintContext bad_ctx;

    bad_ctx = paint_ctx;
    bad_ctx.gfx = NULL;
    CMP_TEST_EXPECT(grid.widget.vtable->paint(grid.widget.ctx, &bad_ctx),
                   CMP_ERR_INVALID_ARGUMENT);
  }
  CMP_TEST_OK(grid.widget.vtable->paint(grid.widget.ctx, &paint_ctx));
  CMP_TEST_ASSERT(gfx_backend.draw_rect_calls == 1);

  gfx.vtable = &g_test_gfx_vtable_no_draw;
  grid.style.background_color.a = 0.0f;
  CMP_TEST_OK(grid.widget.vtable->paint(grid.widget.ctx, &paint_ctx));
  grid.style.background_color.a = 1.0f;
  CMP_TEST_EXPECT(grid.widget.vtable->paint(grid.widget.ctx, &paint_ctx),
                 CMP_ERR_UNSUPPORTED);

  {
    CMPSemantics semantics;
    CMP_TEST_EXPECT(grid.widget.vtable->get_semantics(NULL, &semantics),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(grid.widget.vtable->get_semantics(grid.widget.ctx, NULL),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(grid.widget.vtable->get_semantics(grid.widget.ctx, &semantics));
    CMP_TEST_ASSERT(semantics.role == CMP_SEMANTIC_NONE);
  }

  {
    CMPSemantics semantics;
    CMPGridStyle saved_style;
    CMPRect saved_bounds;
    cmp_usize saved_item_count;

    saved_style = grid.style;
    saved_bounds = grid.bounds;
    saved_item_count = grid.item_count;

    CMP_TEST_OK(cmp_grid_view_set_item_count(&grid, grid.item_count));
    CMP_TEST_OK(cmp_grid_view_get_content_extent(&grid, &content_extent));
    CMP_TEST_OK(
        cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE));
    CMP_TEST_OK(cmp_grid_view_set_scroll(&grid, 0.0f));
    CMP_TEST_OK(cmp_list_test_clear_fail_points());

    event.type = CMP_INPUT_POINTER_SCROLL;
    grid.style.scroll_axis = CMP_GRID_SCROLL_VERTICAL;
    event.data.pointer.scroll_y = 2;
    grid.style.item_height = 0.0f;
    CMP_TEST_EXPECT(grid.widget.vtable->event(grid.widget.ctx, &event, &handled),
                   CMP_ERR_RANGE);
    grid.style = saved_style;

    grid.style.spacing_x = -1.0f;
    CMP_TEST_EXPECT(grid.widget.vtable->paint(grid.widget.ctx, &paint_ctx),
                   CMP_ERR_RANGE);
    grid.style = saved_style;
    grid.bounds.width = -1.0f;
    CMP_TEST_EXPECT(grid.widget.vtable->paint(grid.widget.ctx, &paint_ctx),
                   CMP_ERR_RANGE);
    grid.bounds = saved_bounds;

    grid.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
    CMP_TEST_OK(grid.widget.vtable->get_semantics(grid.widget.ctx, &semantics));
    CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_DISABLED) != 0u);
    grid.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

    grid.bounds.width = -1.0f;
    grid.bounds.height = -1.0f;
    CMP_TEST_EXPECT(cmp_grid_view_set_item_count(&grid, 1), CMP_ERR_RANGE);
    grid.bounds = saved_bounds;

    grid.bounds.width = -1.0f;
    grid.bounds.height = -1.0f;
    CMP_TEST_EXPECT(cmp_grid_view_set_style(&grid, &saved_style), CMP_ERR_RANGE);
    grid.bounds = saved_bounds;
    grid.style = saved_style;

    CMP_TEST_OK(cmp_grid_view_set_scroll(&grid, -5.0f));
    CMP_TEST_OK(cmp_grid_view_get_scroll(&grid, &scroll));
    CMP_TEST_ASSERT(scroll == 0.0f);

    grid.bounds.width = -1.0f;
    grid.bounds.height = -1.0f;
    CMP_TEST_EXPECT(cmp_grid_view_set_scroll(&grid, 1.0f), CMP_ERR_RANGE);
    grid.bounds = saved_bounds;

    grid.bounds.width = -1.0f;
    grid.bounds.height = -1.0f;
    CMP_TEST_EXPECT(cmp_grid_view_get_content_extent(&grid, &content_extent),
                   CMP_ERR_RANGE);
    grid.bounds = saved_bounds;

    grid.style.spacing_x = -1.0f;
    CMP_TEST_EXPECT(cmp_grid_view_get_required_slots(&grid, &visible_count),
                   CMP_ERR_RANGE);
    grid.style = saved_style;

    grid.bounds.width = -1.0f;
    grid.bounds.height = -1.0f;
    CMP_TEST_EXPECT(cmp_grid_view_get_required_slots(&grid, &visible_count),
                   CMP_ERR_RANGE);
    grid.bounds = saved_bounds;

    CMP_TEST_OK(cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_STRIDE));
    CMP_TEST_EXPECT(cmp_grid_view_get_required_slots(&grid, &visible_count),
                   CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());

    grid.style = saved_style;
    grid.bounds = saved_bounds;
    grid.item_count = saved_item_count;
  }

  {
    CMPGridView grid_state;
    CMPGridView grid_range;
    CMPGridView grid_count;
    CMPGridView grid_update;
    CMPGridStyle update_style;
    TestAllocator update_alloc;
    CMPAllocator update_iface;
    TestBindCtx update_bind;
    TestWidget update_pool[8];
    CMPRect update_bounds;

    test_allocator_init(&update_alloc);
    update_iface.ctx = &update_alloc;
    update_iface.alloc = test_alloc;
    update_iface.realloc = test_realloc;
    update_iface.free = test_free;
    CMP_TEST_OK(cmp_grid_style_init(&update_style));
    update_style.item_width = 10.0f;
    update_style.item_height = 10.0f;
    update_style.spacing_x = 0.0f;
    update_style.spacing_y = 0.0f;
    update_style.padding.left = 0.0f;
    update_style.padding.top = 0.0f;
    update_style.padding.right = 0.0f;
    update_style.padding.bottom = 0.0f;
    update_style.span = 2u;
    update_style.overscan = 0u;
    update_bounds.x = 0.0f;
    update_bounds.y = 0.0f;
    update_bounds.width = 30.0f;
    update_bounds.height = 30.0f;

    update_bind.pool = update_pool;
    update_bind.pool_count = CMP_COUNTOF(update_pool);
    update_bind.next = 0u;
    update_bind.calls = 0u;
    update_bind.last_index = 0u;
    update_bind.fail = 0;

    CMP_TEST_OK(
        cmp_grid_view_init(&grid_state, &update_style, &update_iface, 2, 0));
    grid_state.bounds = update_bounds;
    CMP_TEST_OK(cmp_grid_view_set_bind(&grid_state, test_bind, &update_bind));
    CMP_TEST_EXPECT(cmp_grid_view_update(&grid_state), CMP_ERR_STATE);
    CMP_TEST_OK(grid_state.widget.vtable->destroy(grid_state.widget.ctx));

    update_bind.next = 0u;
    update_bind.calls = 0u;
    update_bind.last_index = 0u;
    update_bind.fail = 0;
    CMP_TEST_OK(
        cmp_grid_view_init(&grid_range, &update_style, &update_iface, 6, 1));
    grid_range.bounds = update_bounds;
    CMP_TEST_OK(cmp_grid_view_set_bind(&grid_range, test_bind, &update_bind));
    CMP_TEST_EXPECT(cmp_grid_view_update(&grid_range), CMP_ERR_RANGE);
    CMP_TEST_OK(grid_range.widget.vtable->destroy(grid_range.widget.ctx));

    update_bind.next = 0u;
    update_bind.calls = 0u;
    update_bind.last_index = 0u;
    update_bind.fail = 0;
    CMP_TEST_OK(
        cmp_grid_view_init(&grid_count, &update_style, &update_iface, 0, 2));
    grid_count.bounds = update_bounds;
    CMP_TEST_OK(cmp_grid_view_set_bind(&grid_count, test_bind, &update_bind));
    CMP_TEST_OK(cmp_grid_view_update(&grid_count));
    CMP_TEST_OK(grid_count.widget.vtable->destroy(grid_count.widget.ctx));

    update_bind.next = 0u;
    update_bind.calls = 0u;
    update_bind.last_index = 0u;
    update_bind.fail = 0;
    CMP_TEST_OK(
        cmp_grid_view_init(&grid_update, &update_style, &update_iface, 4, 4));
    grid_update.bounds = update_bounds;
    CMP_TEST_OK(cmp_grid_view_set_bind(&grid_update, test_bind, &update_bind));

    grid_update.style.spacing_x = -1.0f;
    CMP_TEST_EXPECT(cmp_grid_view_update(&grid_update), CMP_ERR_RANGE);
    grid_update.style = update_style;

    grid_update.bounds.height = -1.0f;
    CMP_TEST_EXPECT(cmp_grid_view_update(&grid_update), CMP_ERR_RANGE);
    grid_update.bounds = update_bounds;

    grid_update.style.item_height = 0.0f;
    CMP_TEST_EXPECT(cmp_grid_view_update(&grid_update), CMP_ERR_RANGE);
    grid_update.style.item_height = update_style.item_height;

    CMP_TEST_OK(cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_STRIDE));
    CMP_TEST_EXPECT(cmp_grid_view_update(&grid_update), CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());

    update_bind.fail = 1;
    CMP_TEST_EXPECT(cmp_grid_view_update(&grid_update), CMP_ERR_UNKNOWN);
    update_bind.fail = 0;

    CMP_TEST_OK(
        cmp_grid_view_set_bind(&grid_update, test_bind_missing_widget, NULL));
    grid_update.slots[0].node.widget = NULL;
    CMP_TEST_EXPECT(cmp_grid_view_update(&grid_update), CMP_ERR_STATE);
    CMP_TEST_OK(cmp_grid_view_set_bind(&grid_update, test_bind, &update_bind));

    CMP_TEST_OK(cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_ITEM_BOUNDS));
    CMP_TEST_EXPECT(cmp_grid_view_update(&grid_update), CMP_ERR_IO);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());

    CMP_TEST_OK(
        cmp_list_test_set_fail_point(CMP_LIST_TEST_FAIL_GRID_RENDER_BOUNDS));
    CMP_TEST_EXPECT(cmp_grid_view_update(&grid_update), CMP_ERR_IO);
    CMP_TEST_OK(cmp_list_test_clear_fail_points());

    CMP_TEST_OK(grid_update.widget.vtable->destroy(grid_update.widget.ctx));
  }

  {
    TestAllocator free_alloc;
    CMPAllocator free_iface;
    CMPGridView grid_free;
    CMPGridStyle free_style;

    test_allocator_init(&free_alloc);
    free_iface.ctx = &free_alloc;
    free_iface.alloc = test_alloc;
    free_iface.realloc = test_realloc;
    free_iface.free = test_free;
    CMP_TEST_OK(cmp_grid_style_init(&free_style));
    CMP_TEST_OK(cmp_grid_view_init(&grid_free, &free_style, &free_iface, 1, 2));
    free_alloc.fail_free_on = 1u;
    CMP_TEST_EXPECT(grid_free.widget.vtable->destroy(grid_free.widget.ctx),
                   CMP_ERR_IO);
  }

  {
    TestAllocator free_alloc;
    CMPAllocator free_iface;
    CMPGridView grid_free;
    CMPGridStyle free_style;

    test_allocator_init(&free_alloc);
    free_iface.ctx = &free_alloc;
    free_iface.alloc = test_alloc;
    free_iface.realloc = test_realloc;
    free_iface.free = test_free;
    CMP_TEST_OK(cmp_grid_style_init(&free_style));
    CMP_TEST_OK(cmp_grid_view_init(&grid_free, &free_style, &free_iface, 1, 2));
    free_alloc.fail_free_on = 2u;
    CMP_TEST_EXPECT(grid_free.widget.vtable->destroy(grid_free.widget.ctx),
                   CMP_ERR_IO);
  }

  CMP_TEST_EXPECT(grid.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(grid.widget.vtable->destroy(grid.widget.ctx));

  return 0;
}
