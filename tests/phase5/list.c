#include "m3/m3_list.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>

#define M3_LIST_TEST_FAIL_NONE 0u
#define M3_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE 1u
#define M3_LIST_TEST_FAIL_VISIBLE_STRIDE 2u
#define M3_LIST_TEST_FAIL_VISIBLE_END 3u
#define M3_LIST_TEST_FAIL_VISIBLE_LAST_BEFORE_FIRST 4u
#define M3_LIST_TEST_FAIL_VISIBLE_LAST_AFTER_OVERSCAN 5u
#define M3_LIST_TEST_FAIL_GRID_LINE_COUNT_ZERO 6u
#define M3_LIST_TEST_FAIL_GRID_STRIDE 7u
#define M3_LIST_TEST_FAIL_GRID_END 8u
#define M3_LIST_TEST_FAIL_GRID_END_NEGATIVE 9u
#define M3_LIST_TEST_FAIL_GRID_LAST_TOO_LARGE 10u
#define M3_LIST_TEST_FAIL_GRID_LAST_BEFORE_FIRST 11u
#define M3_LIST_TEST_FAIL_RESERVE_NODE_BYTES 12u
#define M3_LIST_TEST_FAIL_RESERVE_COPY_BYTES 13u
#define M3_LIST_TEST_FAIL_LIST_ITEM_BOUNDS 14u
#define M3_LIST_TEST_FAIL_LIST_RENDER_BOUNDS 15u
#define M3_LIST_TEST_FAIL_GRID_ITEM_BOUNDS 16u
#define M3_LIST_TEST_FAIL_GRID_RENDER_BOUNDS 17u
#define M3_LIST_TEST_FAIL_LIST_MEASURE_CONTENT_NEGATIVE 18u
#define M3_LIST_TEST_FAIL_GRID_MEASURE_CONTENT_NEGATIVE 19u

typedef struct TestAllocator {
  m3_usize alloc_calls;
  m3_usize realloc_calls;
  m3_usize free_calls;
  m3_usize fail_alloc_on;
  m3_usize fail_free_on;
} TestAllocator;

static int test_alloc(void *ctx, m3_usize size, void **out_ptr) {
  TestAllocator *alloc;

  if (ctx == NULL || out_ptr == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (size == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->alloc_calls += 1;
  if (alloc->fail_alloc_on != 0 && alloc->alloc_calls == alloc->fail_alloc_on) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = malloc(size);
  if (*out_ptr == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }
  return M3_OK;
}

static int test_realloc(void *ctx, void *ptr, m3_usize size, void **out_ptr) {
  void *mem;

  if (ctx == NULL || out_ptr == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (size == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  mem = realloc(ptr, size);
  if (mem == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }
  *out_ptr = mem;
  return M3_OK;
}

static int test_free(void *ctx, void *ptr) {
  TestAllocator *alloc;

  if (ctx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->free_calls += 1;
  if (alloc->fail_free_on != 0 && alloc->free_calls == alloc->fail_free_on) {
    return M3_ERR_IO;
  }

  free(ptr);
  return M3_OK;
}

static void test_allocator_init(TestAllocator *alloc) {
  alloc->alloc_calls = 0;
  alloc->realloc_calls = 0;
  alloc->free_calls = 0;
  alloc->fail_alloc_on = 0;
  alloc->fail_free_on = 0;
}

typedef struct TestWidget {
  M3Widget widget;
  M3Rect last_bounds;
} TestWidget;

static int test_widget_measure(void *widget, M3MeasureSpec width,
                               M3MeasureSpec height, M3Size *out_size) {
  M3_UNUSED(widget);
  if (out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  M3_UNUSED(width);
  M3_UNUSED(height);
  out_size->width = 0.0f;
  out_size->height = 0.0f;
  return M3_OK;
}

static int test_widget_layout(void *widget, M3Rect bounds) {
  TestWidget *w;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  w = (TestWidget *)widget;
  w->last_bounds = bounds;
  return M3_OK;
}

static int test_widget_paint(void *widget, M3PaintContext *ctx) {
  M3_UNUSED(widget);
  M3_UNUSED(ctx);
  return M3_OK;
}

static int test_widget_event(void *widget, const M3InputEvent *event,
                             M3Bool *out_handled) {
  if (widget == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_handled = M3_FALSE;
  return M3_OK;
}

static int test_widget_semantics(void *widget, M3Semantics *out_semantics) {
  if (widget == NULL || out_semantics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  out_semantics->role = M3_SEMANTIC_NONE;
  out_semantics->flags = 0;
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return M3_OK;
}

static int test_widget_destroy(void *widget) {
  M3_UNUSED(widget);
  return M3_OK;
}

static const M3WidgetVTable g_test_widget_vtable = {
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
  m3_usize pool_count;
  m3_usize next;
  m3_usize calls;
  m3_usize last_index;
  int fail;
} TestBindCtx;

static int test_bind(void *ctx, M3ListSlot *slot, m3_usize index) {
  TestBindCtx *bind;
  TestWidget *widget;
  M3Rect zero;
  int rc;

  if (ctx == NULL || slot == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  bind = (TestBindCtx *)ctx;
  if (bind->fail) {
    return M3_ERR_UNKNOWN;
  }

  bind->calls += 1;
  bind->last_index = index;

  if (slot->user == NULL) {
    if (bind->next >= bind->pool_count) {
      return M3_ERR_OUT_OF_MEMORY;
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
  rc = m3_render_node_init(&slot->node, &widget->widget, &zero);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int test_bind_missing_widget(void *ctx, M3ListSlot *slot,
                                    m3_usize index) {
  M3_UNUSED(ctx);
  M3_UNUSED(slot);
  M3_UNUSED(index);
  return M3_OK;
}

typedef struct TestGfxBackend {
  int draw_rect_calls;
} TestGfxBackend;

static int test_draw_rect(void *gfx, const M3Rect *rect, M3Color color,
                          M3Scalar radius) {
  TestGfxBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestGfxBackend *)gfx;
  backend->draw_rect_calls += 1;
  M3_UNUSED(color);
  M3_UNUSED(radius);
  return M3_OK;
}

static const M3GfxVTable g_test_gfx_vtable = {NULL, NULL, NULL, test_draw_rect,
                                              NULL, NULL, NULL, NULL,
                                              NULL, NULL, NULL, NULL};

static const M3GfxVTable g_test_gfx_vtable_no_draw = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

int main(void) {
  M3ListStyle list_style;
  M3ListView list;
  M3ListView list_small;
  M3GridStyle grid_style;
  M3GridView grid;
  M3Allocator bad_alloc;
  TestAllocator alloc;
  M3Allocator alloc_iface;
  TestWidget pool[16];
  TestBindCtx bind_ctx;
  M3MeasureSpec width_spec;
  M3MeasureSpec height_spec;
  M3Size measured;
  M3Rect bounds;
  M3RenderNode **visible;
  m3_usize visible_count;
  m3_usize overflow_value;
  M3Scalar content_extent;
  M3Scalar scroll;
  M3Bool handled;
  M3InputEvent event;
  M3PaintContext paint_ctx;
  TestGfxBackend gfx_backend;
  M3Gfx gfx;

  M3_TEST_OK(m3_list_style_init(&list_style));
  M3_TEST_EXPECT(m3_list_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_ASSERT(list_style.orientation == M3_LIST_ORIENTATION_VERTICAL);
  M3_TEST_ASSERT(list_style.spacing == M3_LIST_DEFAULT_SPACING);
  M3_TEST_ASSERT(list_style.item_extent == M3_LIST_DEFAULT_ITEM_EXTENT);
  M3_TEST_ASSERT(list_style.overscan == M3_LIST_DEFAULT_OVERSCAN);

  {
    M3Color color;
    M3LayoutEdges edges;
    M3Rect rect;
    M3MeasureSpec spec;
    M3ListStyle style;
    M3GridStyle grid_style_local;
    M3Scalar extent;
    m3_usize first;
    m3_usize last;
    m3_usize count;
    m3_usize huge_span;
    M3ListView temp_list;
    M3GridView temp_grid;

    M3_TEST_EXPECT(m3_list_test_validate_color(NULL), M3_ERR_INVALID_ARGUMENT);
    color.r = -0.1f;
    color.g = 0.5f;
    color.b = 0.5f;
    color.a = 0.5f;
    M3_TEST_EXPECT(m3_list_test_validate_color(&color), M3_ERR_RANGE);
    color.r = 0.0f;
    color.g = 1.5f;
    M3_TEST_EXPECT(m3_list_test_validate_color(&color), M3_ERR_RANGE);
    color.g = 0.0f;
    color.b = -0.2f;
    M3_TEST_EXPECT(m3_list_test_validate_color(&color), M3_ERR_RANGE);
    color.b = 0.0f;
    color.a = 2.0f;
    M3_TEST_EXPECT(m3_list_test_validate_color(&color), M3_ERR_RANGE);
    color.a = 1.0f;
    M3_TEST_OK(m3_list_test_validate_color(&color));

    M3_TEST_EXPECT(m3_list_test_validate_edges(NULL), M3_ERR_INVALID_ARGUMENT);
    edges.left = -1.0f;
    edges.right = 0.0f;
    edges.top = 0.0f;
    edges.bottom = 0.0f;
    M3_TEST_EXPECT(m3_list_test_validate_edges(&edges), M3_ERR_RANGE);
    edges.left = 0.0f;
    edges.right = -1.0f;
    M3_TEST_EXPECT(m3_list_test_validate_edges(&edges), M3_ERR_RANGE);
    edges.right = 0.0f;
    edges.top = -1.0f;
    M3_TEST_EXPECT(m3_list_test_validate_edges(&edges), M3_ERR_RANGE);
    edges.top = 0.0f;
    edges.bottom = -1.0f;
    M3_TEST_EXPECT(m3_list_test_validate_edges(&edges), M3_ERR_RANGE);
    edges.bottom = 0.0f;
    M3_TEST_OK(m3_list_test_validate_edges(&edges));

    M3_TEST_EXPECT(m3_list_test_validate_rect(NULL), M3_ERR_INVALID_ARGUMENT);
    rect.x = 0.0f;
    rect.y = 0.0f;
    rect.width = -1.0f;
    rect.height = 1.0f;
    M3_TEST_EXPECT(m3_list_test_validate_rect(&rect), M3_ERR_RANGE);
    rect.width = 1.0f;
    rect.height = -1.0f;
    M3_TEST_EXPECT(m3_list_test_validate_rect(&rect), M3_ERR_RANGE);
    rect.height = 1.0f;
    M3_TEST_OK(m3_list_test_validate_rect(&rect));

    spec.mode = 99u;
    spec.size = 1.0f;
    M3_TEST_EXPECT(m3_list_test_validate_measure_spec(spec),
                   M3_ERR_INVALID_ARGUMENT);
    spec.mode = M3_MEASURE_EXACTLY;
    spec.size = -1.0f;
    M3_TEST_EXPECT(m3_list_test_validate_measure_spec(spec), M3_ERR_RANGE);
    spec.mode = M3_MEASURE_AT_MOST;
    spec.size = 1.0f;
    M3_TEST_OK(m3_list_test_validate_measure_spec(spec));

    M3_TEST_OK(m3_list_style_init(&style));
    M3_TEST_EXPECT(m3_list_test_validate_style(NULL), M3_ERR_INVALID_ARGUMENT);
    style.orientation = 99u;
    M3_TEST_EXPECT(m3_list_test_validate_style(&style), M3_ERR_RANGE);
    style.orientation = M3_LIST_ORIENTATION_VERTICAL;
    style.spacing = -1.0f;
    M3_TEST_EXPECT(m3_list_test_validate_style(&style), M3_ERR_RANGE);
    style.spacing = M3_LIST_DEFAULT_SPACING;
    style.item_extent = -1.0f;
    M3_TEST_EXPECT(m3_list_test_validate_style(&style), M3_ERR_RANGE);
    style.item_extent = M3_LIST_DEFAULT_ITEM_EXTENT;
    style.padding.left = -1.0f;
    M3_TEST_EXPECT(m3_list_test_validate_style(&style), M3_ERR_RANGE);
    style.padding.left = 0.0f;
    style.background_color.g = 2.0f;
    M3_TEST_EXPECT(m3_list_test_validate_style(&style), M3_ERR_RANGE);
    style.background_color.g = 0.0f;
    M3_TEST_OK(m3_list_test_validate_style(&style));

    M3_TEST_OK(m3_grid_style_init(&grid_style_local));
    M3_TEST_EXPECT(m3_list_test_validate_grid_style(NULL),
                   M3_ERR_INVALID_ARGUMENT);
    grid_style_local.scroll_axis = 99u;
    M3_TEST_EXPECT(m3_list_test_validate_grid_style(&grid_style_local),
                   M3_ERR_RANGE);
    grid_style_local.scroll_axis = M3_GRID_SCROLL_VERTICAL;
    grid_style_local.span = 0u;
    M3_TEST_EXPECT(m3_list_test_validate_grid_style(&grid_style_local),
                   M3_ERR_RANGE);
    grid_style_local.span = 2u;
    grid_style_local.spacing_x = -1.0f;
    M3_TEST_EXPECT(m3_list_test_validate_grid_style(&grid_style_local),
                   M3_ERR_RANGE);
    grid_style_local.spacing_x = 0.0f;
    grid_style_local.spacing_y = -1.0f;
    M3_TEST_EXPECT(m3_list_test_validate_grid_style(&grid_style_local),
                   M3_ERR_RANGE);
    grid_style_local.spacing_y = 0.0f;
    grid_style_local.item_width = -1.0f;
    M3_TEST_EXPECT(m3_list_test_validate_grid_style(&grid_style_local),
                   M3_ERR_RANGE);
    grid_style_local.item_width = M3_GRID_DEFAULT_ITEM_WIDTH;
    grid_style_local.item_height = -1.0f;
    M3_TEST_EXPECT(m3_list_test_validate_grid_style(&grid_style_local),
                   M3_ERR_RANGE);
    grid_style_local.item_height = M3_GRID_DEFAULT_ITEM_HEIGHT;
    grid_style_local.padding.left = -1.0f;
    M3_TEST_EXPECT(m3_list_test_validate_grid_style(&grid_style_local),
                   M3_ERR_RANGE);
    grid_style_local.padding.left = 0.0f;
    grid_style_local.background_color.a = 2.0f;
    M3_TEST_EXPECT(m3_list_test_validate_grid_style(&grid_style_local),
                   M3_ERR_RANGE);
    grid_style_local.background_color.a = 0.0f;
    M3_TEST_OK(m3_list_test_validate_grid_style(&grid_style_local));

    M3_TEST_EXPECT(
        m3_list_test_compute_content_extent(1u, 1.0f, 1.0f, 0.0f, 0.0f, NULL),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_list_test_compute_content_extent(1u, -1.0f, 1.0f, 0.0f,
                                                       0.0f, &extent),
                   M3_ERR_RANGE);
    M3_TEST_OK(m3_list_test_compute_content_extent(0u, 1.0f, 1.0f, 1.0f, 2.0f,
                                                   &extent));
    M3_TEST_ASSERT(extent > 2.999f && extent < 3.001f);
    M3_TEST_EXPECT(m3_list_test_compute_content_extent(1u, 0.0f, 1.0f, 0.0f,
                                                       0.0f, &extent),
                   M3_ERR_RANGE);
    M3_TEST_OK(m3_list_test_compute_content_extent(2u, 1.0f, 1.0f, 0.0f, 0.0f,
                                                   &extent));
    M3_TEST_OK(
        m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE));
    M3_TEST_EXPECT(m3_list_test_compute_content_extent(2u, 1.0f, 1.0f, 0.0f,
                                                       0.0f, &extent),
                   M3_ERR_RANGE);
    M3_TEST_OK(m3_list_test_clear_fail_points());

    M3_TEST_EXPECT(m3_list_test_compute_visible_range(1u, 1.0f, 0.0f, 0.0f,
                                                      0.0f, 1.0f, 0u, NULL,
                                                      &last, &count),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_list_test_compute_visible_range(
        0u, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0u, &first, &last, &count));
    M3_TEST_ASSERT(count == 0u);
    M3_TEST_OK(m3_list_test_compute_visible_range(
        1u, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0u, &first, &last, &count));
    M3_TEST_EXPECT(m3_list_test_compute_visible_range(1u, 0.0f, 0.0f, 0.0f,
                                                      0.0f, 1.0f, 0u, &first,
                                                      &last, &count),
                   M3_ERR_RANGE);
    M3_TEST_EXPECT(m3_list_test_compute_visible_range(1u, 1.0f, -1.0f, 0.0f,
                                                      0.0f, 1.0f, 0u, &first,
                                                      &last, &count),
                   M3_ERR_RANGE);
    M3_TEST_EXPECT(m3_list_test_compute_visible_range(1u, 1.0f, 0.0f, -1.0f,
                                                      0.0f, 1.0f, 0u, &first,
                                                      &last, &count),
                   M3_ERR_RANGE);
    M3_TEST_OK(m3_list_test_compute_visible_range(
        1u, 1.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0u, &first, &last, &count));
    M3_TEST_OK(m3_list_test_compute_visible_range(
        1u, 1.0f, 0.0f, 0.0f, -10.0f, 5.0f, 0u, &first, &last, &count));
    M3_TEST_OK(m3_list_test_compute_visible_range(
        2u, 1.0f, 0.0f, 0.0f, 0.0f, 0.00005f, 0u, &first, &last, &count));
    M3_TEST_OK(m3_list_test_compute_visible_range(
        1u, 1.0f, 0.0f, 0.0f, 100.0f, 1.0f, 0u, &first, &last, &count));
    M3_TEST_OK(m3_list_test_compute_visible_range(
        2u, 1.0f, 0.0f, 0.0f, 0.0f, 10.0f, 0u, &first, &last, &count));
    M3_TEST_OK(m3_list_test_compute_visible_range(
        3u, 1.0f, 0.0f, 0.0f, 0.0f, 1.1f, 1u, &first, &last, &count));
    M3_TEST_OK(m3_list_test_compute_visible_range(
        3u, 1.0f, 0.0f, 0.0f, 0.0f, 1.1f, 10u, &first, &last, &count));
    M3_TEST_OK(m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_VISIBLE_STRIDE));
    M3_TEST_EXPECT(m3_list_test_compute_visible_range(1u, 1.0f, 0.0f, 0.0f,
                                                      0.0f, 1.0f, 0u, &first,
                                                      &last, &count),
                   M3_ERR_RANGE);
    M3_TEST_OK(m3_list_test_clear_fail_points());
    M3_TEST_OK(m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_VISIBLE_END));
    M3_TEST_OK(m3_list_test_compute_visible_range(
        1u, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0u, &first, &last, &count));
    M3_TEST_ASSERT(count == 0u);
    M3_TEST_OK(m3_list_test_clear_fail_points());
    M3_TEST_OK(m3_list_test_set_fail_point(
        M3_LIST_TEST_FAIL_VISIBLE_LAST_BEFORE_FIRST));
    M3_TEST_OK(m3_list_test_compute_visible_range(
        20u, 1.0f, 0.0f, 0.0f, 10.0f, 1.0f, 0u, &first, &last, &count));
    M3_TEST_ASSERT(count == 0u);
    M3_TEST_OK(m3_list_test_clear_fail_points());
    M3_TEST_OK(m3_list_test_set_fail_point(
        M3_LIST_TEST_FAIL_VISIBLE_LAST_AFTER_OVERSCAN));
    M3_TEST_OK(m3_list_test_compute_visible_range(
        20u, 1.0f, 0.0f, 0.0f, 2.0f, 1.0f, 1u, &first, &last, &count));
    M3_TEST_ASSERT(count == 0u);
    M3_TEST_OK(m3_list_test_clear_fail_points());

    M3_TEST_OK(m3_list_test_set_fail_point(
        M3_LIST_TEST_FAIL_VISIBLE_LAST_BEFORE_FIRST));
    M3_TEST_OK(m3_list_test_compute_visible_range(
        5u, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0u, &first, &last, &count));
    M3_TEST_ASSERT(count == 1u);
    M3_TEST_OK(m3_list_test_clear_fail_points());
    M3_TEST_OK(m3_list_test_set_fail_point(
        M3_LIST_TEST_FAIL_VISIBLE_LAST_AFTER_OVERSCAN));
    M3_TEST_OK(m3_list_test_compute_visible_range(
        5u, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0u, &first, &last, &count));
    M3_TEST_ASSERT(count == 1u);
    M3_TEST_OK(m3_list_test_clear_fail_points());

    memset(&temp_list, 0, sizeof(temp_list));
    temp_list.item_count = 2u;
    M3_TEST_EXPECT(m3_list_test_compute_item_bounds(NULL, 0u, &rect),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_list_test_compute_item_bounds(&temp_list, 2u, &rect),
                   M3_ERR_NOT_FOUND);
    temp_list.style.item_extent = 0.0f;
    temp_list.style.spacing = 0.0f;
    M3_TEST_EXPECT(m3_list_test_compute_item_bounds(&temp_list, 0u, &rect),
                   M3_ERR_RANGE);
    temp_list.style.item_extent = 10.0f;
    temp_list.style.spacing = 1.0f;
    temp_list.bounds.width = 0.0f;
    temp_list.style.padding.left = 2.0f;
    temp_list.style.padding.right = 2.0f;
    M3_TEST_EXPECT(m3_list_test_compute_item_bounds(&temp_list, 0u, &rect),
                   M3_ERR_RANGE);
    temp_list.bounds.width = 20.0f;
    temp_list.bounds.height = 20.0f;
    temp_list.style.padding.left = 0.0f;
    temp_list.style.padding.right = 0.0f;
    temp_list.style.padding.top = 0.0f;
    temp_list.style.padding.bottom = 0.0f;
    temp_list.style.orientation = M3_LIST_ORIENTATION_VERTICAL;
    M3_TEST_OK(m3_list_test_compute_item_bounds(&temp_list, 0u, &rect));

    memset(&temp_grid, 0, sizeof(temp_grid));
    M3_TEST_EXPECT(
        m3_list_test_grid_compute_visible_range(NULL, &first, &last, &count),
        M3_ERR_INVALID_ARGUMENT);
    temp_grid.style = grid_style_local;
    temp_grid.item_count = 0u;
    M3_TEST_OK(m3_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    temp_grid.item_count = 1u;
    temp_grid.style.item_height = -1.0f;
    M3_TEST_EXPECT(m3_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                           &last, &count),
                   M3_ERR_RANGE);
    temp_grid.style.item_height = 10.0f;
    temp_grid.bounds.height = 0.0f;
    M3_TEST_OK(m3_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    temp_grid.bounds.height = 10.0f;
    temp_grid.style.padding.top = 20.0f;
    M3_TEST_OK(m3_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    temp_grid.style.padding.top = 0.0f;
    temp_grid.style.span = 2u;
    temp_grid.item_count = 3u;
    temp_grid.style.spacing_y = 0.0f;
    temp_grid.scroll_offset = 0.0f;
    M3_TEST_OK(m3_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    temp_grid.scroll_offset = 100.0f;
    M3_TEST_OK(m3_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    M3_TEST_OK(
        m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_GRID_LINE_COUNT_ZERO));
    M3_TEST_OK(m3_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    M3_TEST_ASSERT(count == 0u);
    M3_TEST_OK(m3_list_test_clear_fail_points());
    M3_TEST_OK(m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_GRID_STRIDE));
    M3_TEST_EXPECT(m3_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                           &last, &count),
                   M3_ERR_RANGE);
    M3_TEST_OK(m3_list_test_clear_fail_points());
    M3_TEST_OK(m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_GRID_END));
    M3_TEST_OK(m3_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    M3_TEST_ASSERT(count == 0u);
    M3_TEST_OK(m3_list_test_clear_fail_points());
    M3_TEST_OK(
        m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_GRID_END_NEGATIVE));
    M3_TEST_OK(m3_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    M3_TEST_OK(m3_list_test_clear_fail_points());
    M3_TEST_OK(
        m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_GRID_LAST_TOO_LARGE));
    M3_TEST_OK(m3_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    M3_TEST_OK(m3_list_test_clear_fail_points());
    M3_TEST_OK(
        m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_GRID_LAST_BEFORE_FIRST));
    M3_TEST_OK(m3_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    M3_TEST_ASSERT(count == 0u);
    M3_TEST_OK(m3_list_test_clear_fail_points());

    temp_grid.item_count = 4u;
    temp_grid.style.span = 2u;
    temp_grid.style.scroll_axis = M3_GRID_SCROLL_VERTICAL;
    temp_grid.style.item_height = 10.0f;
    temp_grid.style.spacing_y = 0.0f;
    temp_grid.style.padding.top = 0.0f;
    temp_grid.style.padding.bottom = 0.0f;
    temp_grid.bounds.height = 25.0f;
    temp_grid.scroll_offset = 0.0f;
    M3_TEST_OK(
        m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_GRID_LAST_TOO_LARGE));
    M3_TEST_OK(m3_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    M3_TEST_ASSERT(count > 0u);
    M3_TEST_OK(m3_list_test_clear_fail_points());

    temp_grid.item_count = 6u;
    temp_grid.style.span = 2u;
    temp_grid.style.scroll_axis = M3_GRID_SCROLL_VERTICAL;
    temp_grid.style.item_height = 10.0f;
    temp_grid.style.spacing_y = 0.0f;
    temp_grid.style.padding.top = 0.0f;
    temp_grid.style.padding.bottom = 0.0f;
    temp_grid.bounds.height = 5.0f;
    temp_grid.scroll_offset = 15.0f;
    M3_TEST_OK(
        m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_GRID_LAST_BEFORE_FIRST));
    M3_TEST_OK(m3_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    M3_TEST_ASSERT(count == 0u);
    M3_TEST_OK(m3_list_test_clear_fail_points());

    temp_grid.scroll_offset = 0.0f;
    M3_TEST_OK(
        m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_GRID_LAST_BEFORE_FIRST));
    M3_TEST_OK(m3_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    M3_TEST_ASSERT(count > 0u);
    M3_TEST_OK(m3_list_test_clear_fail_points());

    temp_grid.item_count = 6u;
    temp_grid.style.span = 2u;
    temp_grid.style.overscan = 2u;
    temp_grid.bounds.height = 15.0f;
    temp_grid.scroll_offset = 0.0f;
    M3_TEST_OK(m3_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    M3_TEST_ASSERT(count > 0u);

    huge_span = ((m3_usize) ~(m3_usize)0 / 2u) + 2u;
    memset(&temp_grid, 0, sizeof(temp_grid));
    temp_grid.style.scroll_axis = M3_GRID_SCROLL_VERTICAL;
    temp_grid.style.span = huge_span;
    temp_grid.style.item_height = 1.0f;
    temp_grid.style.spacing_y = 0.0f;
    temp_grid.style.padding.top = 0.0f;
    temp_grid.style.padding.bottom = 0.0f;
    temp_grid.item_count = huge_span + 1u;
    temp_grid.bounds.height = 1.0f;
    temp_grid.scroll_offset = 1.2f;
    M3_TEST_OK(m3_list_test_grid_compute_visible_range(&temp_grid, &first,
                                                       &last, &count));
    M3_TEST_ASSERT(count == 0u);

    memset(&temp_grid, 0, sizeof(temp_grid));
    temp_grid.style = grid_style_local;
    temp_grid.item_count = 1u;

    M3_TEST_EXPECT(m3_list_test_grid_compute_item_bounds(NULL, 0u, &rect),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_list_test_grid_compute_item_bounds(&temp_grid, 10u, &rect),
        M3_ERR_NOT_FOUND);
    temp_grid.style.item_width = 0.0f;
    M3_TEST_EXPECT(m3_list_test_grid_compute_item_bounds(&temp_grid, 0u, &rect),
                   M3_ERR_RANGE);
    temp_grid.style.item_width = 10.0f;
    temp_grid.style.item_height = 10.0f;
    M3_TEST_OK(m3_list_test_grid_compute_item_bounds(&temp_grid, 0u, &rect));
  }

  M3_TEST_EXPECT(m3_list_view_init(NULL, &list_style, NULL, 0, 0),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_list_view_init(&list, NULL, NULL, 0, 0),
                 M3_ERR_INVALID_ARGUMENT);

  list_style.orientation = 99u;
  M3_TEST_EXPECT(m3_list_view_init(&list, &list_style, NULL, 0, 0),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_list_style_init(&list_style));

  M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_TRUE));
  M3_TEST_EXPECT(m3_list_view_init(&list, &list_style, NULL, 0, 0),
                 M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_FALSE));

  M3_TEST_EXPECT(m3_list_view_set_bind(NULL, NULL, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_list_view_set_item_count(NULL, 1), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_list_view_set_style(NULL, &list_style),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_list_view_set_scroll(NULL, 0.0f), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_list_view_get_scroll(NULL, &scroll),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_list_view_get_content_extent(NULL, &content_extent),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_list_view_get_required_slots(NULL, &visible_count),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_list_view_update(NULL), M3_ERR_INVALID_ARGUMENT);

  {
    M3ListView metrics_view;
    M3ListStyle metrics_style;

    M3_TEST_EXPECT(m3_list_test_update_metrics(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_list_style_init(&metrics_style));
    M3_TEST_OK(m3_list_view_init(&metrics_view, &metrics_style, NULL, 0, 0));
    metrics_view.bounds.height = -1.0f;
    M3_TEST_EXPECT(m3_list_test_update_metrics(&metrics_view), M3_ERR_RANGE);
    metrics_view.bounds.height = 10.0f;
    metrics_view.scroll_offset = -5.0f;
    M3_TEST_OK(m3_list_test_update_metrics(&metrics_view));
    M3_TEST_ASSERT(metrics_view.scroll_offset == 0.0f);
    metrics_view.scroll_offset = 5.0f;
    M3_TEST_OK(m3_list_test_update_metrics(&metrics_view));
    M3_TEST_OK(metrics_view.widget.vtable->destroy(metrics_view.widget.ctx));
  }

  {
    M3GridView metrics_grid;
    M3GridStyle metrics_grid_style;

    M3_TEST_EXPECT(m3_list_test_grid_update_metrics(NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_grid_style_init(&metrics_grid_style));
    M3_TEST_OK(
        m3_grid_view_init(&metrics_grid, &metrics_grid_style, NULL, 0, 0));
    metrics_grid.style.spacing_y = -1.0f;
    M3_TEST_EXPECT(m3_list_test_grid_update_metrics(&metrics_grid),
                   M3_ERR_RANGE);
    metrics_grid.style.spacing_y = 0.0f;
    metrics_grid.style.span = 0u;
    M3_TEST_EXPECT(m3_list_test_grid_update_metrics(&metrics_grid),
                   M3_ERR_RANGE);
    metrics_grid.style.span = 1u;
    metrics_grid.item_count = 0u;
    metrics_grid.bounds.height = 10.0f;
    M3_TEST_OK(m3_list_test_grid_update_metrics(&metrics_grid));
    metrics_grid.item_count = 2u;
    metrics_grid.style.item_height = 0.0f;
    M3_TEST_EXPECT(m3_list_test_grid_update_metrics(&metrics_grid),
                   M3_ERR_RANGE);
    metrics_grid.style.item_height = metrics_grid_style.item_height;
    metrics_grid.bounds.height = -1.0f;
    M3_TEST_EXPECT(m3_list_test_grid_update_metrics(&metrics_grid),
                   M3_ERR_RANGE);
    metrics_grid.bounds.height = 10.0f;
    metrics_grid.scroll_offset = -5.0f;
    M3_TEST_OK(m3_list_test_grid_update_metrics(&metrics_grid));
    M3_TEST_ASSERT(metrics_grid.scroll_offset == 0.0f);
    metrics_grid.scroll_offset = 50.0f;
    M3_TEST_OK(m3_list_test_grid_update_metrics(&metrics_grid));
    M3_TEST_OK(metrics_grid.widget.vtable->destroy(metrics_grid.widget.ctx));
  }

  {
    M3ListSlot *slots;
    M3RenderNode **nodes;
    m3_usize capacity;

    slots = NULL;
    nodes = NULL;
    capacity = 0;
    M3_TEST_EXPECT(m3_list_test_reserve_slots(NULL, NULL, NULL, NULL, 1),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_list_test_reserve_slots(&slots, &nodes, &capacity, NULL, 1),
        M3_ERR_INVALID_ARGUMENT);
  }

  M3_TEST_EXPECT(m3_list_test_mul_overflow(1, 1, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_list_test_mul_overflow(2, 3, &overflow_value));
  M3_TEST_ASSERT(overflow_value == 6);
  overflow_value = ((m3_usize) ~(m3_usize)0) / (m3_usize)sizeof(M3ListSlot) + 1;
  M3_TEST_EXPECT(m3_list_test_mul_overflow(overflow_value,
                                           (m3_usize)sizeof(M3ListSlot),
                                           &overflow_value),
                 M3_ERR_OVERFLOW);

  bad_alloc.ctx = NULL;
  bad_alloc.alloc = NULL;
  bad_alloc.realloc = NULL;
  bad_alloc.free = NULL;
  M3_TEST_EXPECT(m3_list_view_init(&list, &list_style, &bad_alloc, 0, 0),
                 M3_ERR_INVALID_ARGUMENT);

  test_allocator_init(&alloc);
  alloc.fail_alloc_on = 1;
  alloc_iface.ctx = &alloc;
  alloc_iface.alloc = test_alloc;
  alloc_iface.realloc = test_realloc;
  alloc_iface.free = test_free;
  M3_TEST_EXPECT(m3_list_view_init(&list, &list_style, &alloc_iface, 0, 2),
                 M3_ERR_OUT_OF_MEMORY);

  alloc.fail_alloc_on = 0;
  M3_TEST_OK(m3_list_view_init(&list, &list_style, &alloc_iface, 5, 0));
  M3_TEST_EXPECT(m3_list_view_reserve(NULL, 1), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_list_view_reserve(&list, 4));
  M3_TEST_ASSERT(list.slot_capacity == 4);
  M3_TEST_ASSERT(list.slots != NULL);
  M3_TEST_ASSERT(list.visible_nodes != NULL);
  M3_TEST_OK(m3_list_view_reserve(&list, 2));
  M3_TEST_EXPECT(m3_list_view_reserve(&list, overflow_value), M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_RESERVE_NODE_BYTES));
  M3_TEST_EXPECT(m3_list_view_reserve(&list, 8), M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_list_test_clear_fail_points());
  M3_TEST_OK(m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_RESERVE_COPY_BYTES));
  M3_TEST_EXPECT(m3_list_view_reserve(&list, 8), M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_list_test_clear_fail_points());
  {
    M3ListView list_bad;

    list_bad = list;
    list_bad.allocator.alloc = NULL;
    list_bad.allocator.free = NULL;
    M3_TEST_EXPECT(m3_list_view_reserve(&list_bad, 1), M3_ERR_INVALID_ARGUMENT);
  }
  {
    M3ListView list_fail;
    TestAllocator alloc_fail;
    M3Allocator alloc_iface_fail;

    test_allocator_init(&alloc_fail);
    alloc_fail.fail_alloc_on = 2;
    alloc_iface_fail.ctx = &alloc_fail;
    alloc_iface_fail.alloc = test_alloc;
    alloc_iface_fail.realloc = test_realloc;
    alloc_iface_fail.free = test_free;
    M3_TEST_OK(
        m3_list_view_init(&list_fail, &list_style, &alloc_iface_fail, 0, 0));
    M3_TEST_EXPECT(m3_list_view_reserve(&list_fail, 1), M3_ERR_OUT_OF_MEMORY);
    M3_TEST_OK(list_fail.widget.vtable->destroy(list_fail.widget.ctx));
  }
  {
    M3ListView list_bad;
    memset(&list_bad, 0, sizeof(list_bad));
    M3_TEST_EXPECT(m3_list_view_reserve(&list_bad, 1), M3_ERR_INVALID_ARGUMENT);
  }

  list_style = list.style;
  list_style.padding.left = -1.0f;
  M3_TEST_EXPECT(m3_list_view_set_style(&list, &list_style), M3_ERR_RANGE);
  list_style.padding.left = 0.0f;
  list_style.background_color.r = -0.1f;
  M3_TEST_EXPECT(m3_list_view_set_style(&list, &list_style), M3_ERR_RANGE);
  M3_TEST_OK(m3_list_style_init(&list_style));

  list_style = list.style;
  list_style.item_extent = 10.0f;
  list_style.spacing = 2.0f;
  list_style.padding.left = 1.0f;
  list_style.padding.top = 1.0f;
  list_style.padding.right = 1.0f;
  list_style.padding.bottom = 1.0f;
  list_style.overscan = 1u;
  M3_TEST_OK(m3_list_view_set_style(&list, &list_style));
  M3_TEST_OK(m3_list_view_set_item_count(&list, 5));

  M3_TEST_OK(m3_list_view_set_item_count(&list, 0));
  M3_TEST_OK(m3_list_view_get_content_extent(&list, &content_extent));
  M3_TEST_ASSERT(content_extent == 2.0f);
  M3_TEST_OK(m3_list_view_set_item_count(&list, 5));

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = 5.0f;
  height_spec.mode = M3_MEASURE_AT_MOST;
  height_spec.size = 30.0f;
  M3_TEST_EXPECT(
      list.widget.vtable->measure(NULL, width_spec, height_spec, &measured),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                             height_spec, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  list_style = list.style;
  list.style.orientation = 99u;
  M3_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                             height_spec, &measured),
                 M3_ERR_RANGE);
  list.style = list_style;

  list.bounds.height = -1.0f;
  M3_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                             height_spec, &measured),
                 M3_ERR_RANGE);
  list.bounds.height = 0.0f;

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  height_spec.mode = M3_MEASURE_AT_MOST;
  height_spec.size = 10.0f;
  M3_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                             height_spec, &measured),
                 M3_ERR_RANGE);

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = 5.0f;
  height_spec.mode = M3_MEASURE_AT_MOST;
  height_spec.size = 30.0f;
  M3_TEST_OK(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                         height_spec, &measured));
  M3_TEST_ASSERT(measured.width == 2.0f);
  M3_TEST_ASSERT(measured.height == 30.0f);

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_OK(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                         height_spec, &measured));
  M3_TEST_ASSERT(measured.width == 2.0f);
  M3_TEST_ASSERT(measured.height == 60.0f);

  width_spec.mode = M3_MEASURE_EXACTLY;
  width_spec.size = 100.0f;
  height_spec.mode = M3_MEASURE_EXACTLY;
  height_spec.size = 50.0f;
  M3_TEST_OK(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                         height_spec, &measured));
  M3_TEST_ASSERT(measured.width == 100.0f);
  M3_TEST_ASSERT(measured.height == 50.0f);

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  M3_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                             height_spec, &measured),
                 M3_ERR_INVALID_ARGUMENT);

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = 5.0f;
  height_spec.mode = 99u;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                             height_spec, &measured),
                 M3_ERR_INVALID_ARGUMENT);

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = 10.0f;
  height_spec.mode = M3_MEASURE_AT_MOST;
  height_spec.size = 10.0f;
  M3_TEST_OK(m3_list_test_set_fail_point(
      M3_LIST_TEST_FAIL_LIST_MEASURE_CONTENT_NEGATIVE));
  M3_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                             height_spec, &measured),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_list_test_clear_fail_points());

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  M3_TEST_EXPECT(list.widget.vtable->layout(list.widget.ctx, bounds),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(list.widget.vtable->layout(NULL, bounds),
                 M3_ERR_INVALID_ARGUMENT);

  bounds.width = 50.0f;
  bounds.height = 0.0f;
  M3_TEST_OK(list.widget.vtable->layout(list.widget.ctx, bounds));
  M3_TEST_OK(m3_list_view_get_required_slots(&list, &visible_count));
  M3_TEST_ASSERT(visible_count == 0);

  bounds.height = 25.0f;
  M3_TEST_OK(list.widget.vtable->layout(list.widget.ctx, bounds));
  M3_TEST_ASSERT(list.bounds.width == 50.0f);

  bind_ctx.pool = pool;
  bind_ctx.pool_count = M3_COUNTOF(pool);
  bind_ctx.next = 0;
  bind_ctx.calls = 0;
  bind_ctx.last_index = 0;
  bind_ctx.fail = 0;
  M3_TEST_OK(m3_list_view_set_bind(&list, test_bind, &bind_ctx));

  M3_TEST_OK(m3_list_view_get_required_slots(&list, &visible_count));
  M3_TEST_ASSERT(visible_count == 3);

  M3_TEST_OK(m3_list_view_reserve(&list, 5));
  M3_TEST_OK(m3_list_view_update(&list));
  M3_TEST_ASSERT(list.visible_count == 3);
  M3_TEST_ASSERT(list.visible_first == 0);
  M3_TEST_ASSERT(list.visible_last == 2);
  M3_TEST_ASSERT(list.slots[0].index == 0);
  M3_TEST_ASSERT(list.slots[1].index == 1);
  M3_TEST_ASSERT(list.slots[2].index == 2);
  M3_TEST_ASSERT(list.slots[0].node.bounds.y == 1.0f);
  M3_TEST_ASSERT(list.slots[1].node.bounds.y == 13.0f);

  M3_TEST_OK(m3_list_view_get_visible(&list, &visible, &visible_count));
  M3_TEST_ASSERT(visible_count == 3);
  M3_TEST_ASSERT(visible[0] == &list.slots[0].node);

  M3_TEST_OK(m3_list_view_init(&list_small, &list_style, NULL, 5, 2));
  M3_TEST_OK(m3_list_view_set_bind(&list_small, test_bind, &bind_ctx));
  M3_TEST_OK(list_small.widget.vtable->layout(list_small.widget.ctx, bounds));
  M3_TEST_EXPECT(m3_list_view_update(&list_small), M3_ERR_RANGE);
  M3_TEST_OK(list_small.widget.vtable->destroy(list_small.widget.ctx));

  M3_TEST_OK(m3_list_view_set_scroll(&list, 100.0f));
  M3_TEST_OK(m3_list_view_get_scroll(&list, &scroll));
  M3_TEST_ASSERT(scroll == 35.0f);

  list_style.overscan = 0u;
  M3_TEST_OK(m3_list_view_set_style(&list, &list_style));
  M3_TEST_OK(m3_list_view_set_scroll(&list, 15.0f));
  M3_TEST_OK(m3_list_view_update(&list));
  M3_TEST_ASSERT(list.visible_first == 1);
  M3_TEST_ASSERT(list.visible_last == 3);
  M3_TEST_ASSERT(list.visible_count == 3);

  list_style.item_extent = 0.0f;
  M3_TEST_EXPECT(m3_list_view_set_style(&list, &list_style), M3_ERR_RANGE);
  list.style.item_extent = 0.0f;
  M3_TEST_EXPECT(m3_list_view_update(&list), M3_ERR_RANGE);
  list_style.item_extent = 10.0f;
  M3_TEST_OK(m3_list_view_set_style(&list, &list_style));

  M3_TEST_OK(m3_list_view_set_bind(&list, NULL, NULL));
  M3_TEST_EXPECT(m3_list_view_update(&list), M3_ERR_STATE);
  M3_TEST_OK(m3_list_view_set_bind(&list, test_bind, &bind_ctx));

  bind_ctx.fail = 1;
  M3_TEST_EXPECT(m3_list_view_update(&list), M3_ERR_UNKNOWN);
  bind_ctx.fail = 0;

  M3_TEST_OK(m3_list_view_set_bind(&list, test_bind_missing_widget, NULL));
  list.slots[0].node.widget = NULL;
  M3_TEST_EXPECT(m3_list_view_update(&list), M3_ERR_STATE);
  M3_TEST_OK(m3_list_view_set_bind(&list, test_bind, &bind_ctx));

  M3_TEST_EXPECT(m3_list_view_get_visible(&list, NULL, &visible_count),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_list_view_get_visible(&list, &visible, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  gfx_backend.draw_rect_calls = 0;
  gfx.ctx = &gfx_backend;
  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = NULL;
  paint_ctx.gfx = &gfx;
  paint_ctx.clip = bounds;
  paint_ctx.dpi_scale = 1.0f;
  M3_TEST_EXPECT(list.widget.vtable->paint(NULL, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(list.widget.vtable->paint(list.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  {
    M3PaintContext bad_ctx;

    bad_ctx = paint_ctx;
    bad_ctx.gfx = NULL;
    M3_TEST_EXPECT(list.widget.vtable->paint(list.widget.ctx, &bad_ctx),
                   M3_ERR_INVALID_ARGUMENT);
  }
  list.style.background_color.a = 1.0f;
  M3_TEST_OK(list.widget.vtable->paint(list.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(gfx_backend.draw_rect_calls == 1);

  gfx.vtable = &g_test_gfx_vtable_no_draw;
  list.style.background_color.a = 0.0f;
  M3_TEST_OK(list.widget.vtable->paint(list.widget.ctx, &paint_ctx));

  list.style.background_color.a = 1.0f;
  M3_TEST_EXPECT(list.widget.vtable->paint(list.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);

  {
    M3Semantics semantics;
    M3_TEST_EXPECT(list.widget.vtable->get_semantics(NULL, &semantics),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(list.widget.vtable->get_semantics(list.widget.ctx, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(list.widget.vtable->get_semantics(list.widget.ctx, &semantics));
    M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_NONE);
  }

  memset(&event, 0, sizeof(event));
  event.type = M3_INPUT_POINTER_DOWN;
  handled = M3_TRUE;
  M3_TEST_EXPECT(list.widget.vtable->event(NULL, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(list.widget.vtable->event(list.widget.ctx, NULL, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(list.widget.vtable->event(list.widget.ctx, &event, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(list.widget.vtable->event(list.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  event.type = M3_INPUT_POINTER_SCROLL;
  event.data.pointer.scroll_y = 0;
  handled = M3_TRUE;
  M3_TEST_OK(list.widget.vtable->event(list.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  event.data.pointer.scroll_y = 5;
  handled = M3_FALSE;
  M3_TEST_OK(list.widget.vtable->event(list.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_OK(m3_list_view_get_scroll(&list, &scroll));
  M3_TEST_ASSERT(scroll == 20.0f);

  list.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  event.data.pointer.scroll_y = 5;
  handled = M3_TRUE;
  M3_TEST_OK(list.widget.vtable->event(list.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  M3_TEST_OK(m3_list_view_get_scroll(&list, &scroll));
  M3_TEST_ASSERT(scroll == 20.0f);
  list.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  list_style = list.style;
  list_style.orientation = M3_LIST_ORIENTATION_HORIZONTAL;
  list_style.item_extent = 5.0f;
  list_style.spacing = 1.0f;
  list_style.padding.left = 2.0f;
  list_style.padding.right = 2.0f;
  list_style.padding.top = 1.0f;
  list_style.padding.bottom = 1.0f;
  list_style.overscan = 0u;
  M3_TEST_OK(m3_list_view_set_style(&list, &list_style));
  M3_TEST_OK(m3_list_view_set_item_count(&list, 3));
  M3_TEST_OK(m3_list_view_set_scroll(&list, 0.0f));
  bounds.width = 20.0f;
  bounds.height = 10.0f;
  M3_TEST_OK(list.widget.vtable->layout(list.widget.ctx, bounds));
  M3_TEST_OK(m3_list_view_update(&list));
  M3_TEST_ASSERT(list.slots[0].node.bounds.x == 2.0f);
  M3_TEST_ASSERT(list.slots[1].node.bounds.x == 8.0f);

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = 10.0f;
  height_spec.mode = M3_MEASURE_AT_MOST;
  height_spec.size = 5.0f;
  M3_TEST_OK(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                         height_spec, &measured));
  M3_TEST_ASSERT(measured.width == 10.0f);
  M3_TEST_ASSERT(measured.height == 2.0f);

  event.type = M3_INPUT_POINTER_SCROLL;
  event.data.pointer.scroll_x = 4;
  handled = M3_FALSE;
  M3_TEST_OK(list.widget.vtable->event(list.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  {
    M3Semantics semantics;
    M3ListStyle saved_style;
    M3Rect saved_bounds;
    m3_usize saved_item_count;

    saved_style = list.style;
    saved_bounds = list.bounds;
    saved_item_count = list.item_count;

    width_spec.mode = M3_MEASURE_AT_MOST;
    width_spec.size = 10.0f;
    height_spec.mode = 99u;
    height_spec.size = 0.0f;
    M3_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                               height_spec, &measured),
                   M3_ERR_INVALID_ARGUMENT);

    height_spec.mode = M3_MEASURE_AT_MOST;
    height_spec.size = 10.0f;
    M3_TEST_OK(m3_list_test_set_fail_point(
        M3_LIST_TEST_FAIL_LIST_MEASURE_CONTENT_NEGATIVE));
    M3_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec,
                                               height_spec, &measured),
                   M3_ERR_RANGE);
    M3_TEST_OK(m3_list_test_clear_fail_points());

    list.style.spacing = -1.0f;
    M3_TEST_EXPECT(list.widget.vtable->paint(list.widget.ctx, &paint_ctx),
                   M3_ERR_RANGE);
    list.style = saved_style;
    list.bounds.width = -1.0f;
    M3_TEST_EXPECT(list.widget.vtable->paint(list.widget.ctx, &paint_ctx),
                   M3_ERR_RANGE);
    list.bounds = saved_bounds;

    list.widget.flags |= M3_WIDGET_FLAG_DISABLED;
    M3_TEST_OK(list.widget.vtable->get_semantics(list.widget.ctx, &semantics));
    M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_DISABLED) != 0u);
    list.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

    event.type = M3_INPUT_POINTER_SCROLL;
    list.style.orientation = M3_LIST_ORIENTATION_VERTICAL;
    event.data.pointer.scroll_y = 3;
    list.style.item_extent = 0.0f;
    M3_TEST_EXPECT(list.widget.vtable->event(list.widget.ctx, &event, &handled),
                   M3_ERR_RANGE);
    list.style = saved_style;

    list.bounds.width = -1.0f;
    list.bounds.height = -1.0f;
    M3_TEST_EXPECT(m3_list_view_set_item_count(&list, 1), M3_ERR_RANGE);
    list.bounds = saved_bounds;

    list.bounds.width = -1.0f;
    list.bounds.height = -1.0f;
    M3_TEST_EXPECT(m3_list_view_set_style(&list, &saved_style), M3_ERR_RANGE);
    list.bounds = saved_bounds;
    list.style = saved_style;

    list.bounds.width = -1.0f;
    list.bounds.height = -1.0f;
    M3_TEST_EXPECT(m3_list_view_get_content_extent(&list, &content_extent),
                   M3_ERR_RANGE);
    list.bounds = saved_bounds;

    M3_TEST_OK(m3_list_view_set_scroll(&list, -5.0f));
    M3_TEST_OK(m3_list_view_get_scroll(&list, &scroll));
    M3_TEST_ASSERT(scroll == 0.0f);

    list.bounds.width = -1.0f;
    list.bounds.height = -1.0f;
    M3_TEST_EXPECT(m3_list_view_set_scroll(&list, 1.0f), M3_ERR_RANGE);
    list.bounds = saved_bounds;

    list.style.spacing = -1.0f;
    M3_TEST_EXPECT(m3_list_view_get_required_slots(&list, &visible_count),
                   M3_ERR_RANGE);
    list.style = saved_style;

    list.bounds.width = -1.0f;
    list.bounds.height = -1.0f;
    M3_TEST_EXPECT(m3_list_view_get_required_slots(&list, &visible_count),
                   M3_ERR_RANGE);
    list.bounds = saved_bounds;

    list.style.orientation = M3_LIST_ORIENTATION_HORIZONTAL;
    list.bounds.width = 30.0f;
    M3_TEST_OK(m3_list_view_get_required_slots(&list, &visible_count));
    list.style = saved_style;
    list.bounds = saved_bounds;

    M3_TEST_OK(m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_VISIBLE_STRIDE));
    M3_TEST_EXPECT(m3_list_view_get_required_slots(&list, &visible_count),
                   M3_ERR_RANGE);
    M3_TEST_OK(m3_list_test_clear_fail_points());

    list.style.spacing = -1.0f;
    M3_TEST_EXPECT(m3_list_view_update(&list), M3_ERR_RANGE);
    list.style = saved_style;

    list.bounds.width = -1.0f;
    list.bounds.height = -1.0f;
    M3_TEST_EXPECT(m3_list_view_update(&list), M3_ERR_RANGE);
    list.bounds = saved_bounds;

    M3_TEST_OK(m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_VISIBLE_STRIDE));
    M3_TEST_EXPECT(m3_list_view_update(&list), M3_ERR_RANGE);
    M3_TEST_OK(m3_list_test_clear_fail_points());

    list.item_count = 0u;
    M3_TEST_OK(m3_list_view_update(&list));

    list.item_count = saved_item_count;
    M3_TEST_OK(m3_list_view_set_bind(&list, test_bind, &bind_ctx));
    M3_TEST_OK(m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_LIST_ITEM_BOUNDS));
    M3_TEST_EXPECT(m3_list_view_update(&list), M3_ERR_IO);
    M3_TEST_OK(m3_list_test_clear_fail_points());
    M3_TEST_OK(
        m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_LIST_RENDER_BOUNDS));
    M3_TEST_EXPECT(m3_list_view_update(&list), M3_ERR_IO);
    M3_TEST_OK(m3_list_test_clear_fail_points());

    list.style = saved_style;
    list.bounds = saved_bounds;
    list.item_count = saved_item_count;
  }

  {
    M3ListView list_state;
    M3ListView list_range;
    M3ListStyle state_style;
    TestAllocator state_alloc;
    M3Allocator state_iface;
    M3Rect state_bounds;

    test_allocator_init(&state_alloc);
    state_iface.ctx = &state_alloc;
    state_iface.alloc = test_alloc;
    state_iface.realloc = test_realloc;
    state_iface.free = test_free;
    M3_TEST_OK(m3_list_style_init(&state_style));
    state_style.item_extent = 10.0f;
    state_style.spacing = 0.0f;
    state_style.overscan = 0u;
    state_bounds.x = 0.0f;
    state_bounds.y = 0.0f;
    state_bounds.width = 50.0f;
    state_bounds.height = 25.0f;

    M3_TEST_OK(
        m3_list_view_init(&list_state, &state_style, &state_iface, 1, 0));
    list_state.bounds = state_bounds;
    list_state.item_count = 1u;
    M3_TEST_EXPECT(m3_list_view_update(&list_state), M3_ERR_STATE);
    M3_TEST_OK(list_state.widget.vtable->destroy(list_state.widget.ctx));

    M3_TEST_OK(
        m3_list_view_init(&list_range, &state_style, &state_iface, 5, 1));
    list_range.bounds = state_bounds;
    bind_ctx.pool = pool;
    bind_ctx.pool_count = M3_COUNTOF(pool);
    bind_ctx.next = 0u;
    bind_ctx.calls = 0u;
    bind_ctx.last_index = 0u;
    bind_ctx.fail = 0;
    M3_TEST_OK(m3_list_view_set_bind(&list_range, test_bind, &bind_ctx));
    M3_TEST_EXPECT(m3_list_view_update(&list_range), M3_ERR_RANGE);
    M3_TEST_OK(list_range.widget.vtable->destroy(list_range.widget.ctx));
  }

  {
    TestAllocator free_alloc;
    M3Allocator free_iface;
    M3ListView list_free;
    M3ListStyle free_style;

    test_allocator_init(&free_alloc);
    free_iface.ctx = &free_alloc;
    free_iface.alloc = test_alloc;
    free_iface.realloc = test_realloc;
    free_iface.free = test_free;
    M3_TEST_OK(m3_list_style_init(&free_style));
    M3_TEST_OK(m3_list_view_init(&list_free, &free_style, &free_iface, 1, 2));
    free_alloc.fail_free_on = 1u;
    M3_TEST_EXPECT(list_free.widget.vtable->destroy(list_free.widget.ctx),
                   M3_ERR_IO);
  }

  {
    TestAllocator free_alloc;
    M3Allocator free_iface;
    M3ListView list_free;
    M3ListStyle free_style;

    test_allocator_init(&free_alloc);
    free_iface.ctx = &free_alloc;
    free_iface.alloc = test_alloc;
    free_iface.realloc = test_realloc;
    free_iface.free = test_free;
    M3_TEST_OK(m3_list_style_init(&free_style));
    M3_TEST_OK(m3_list_view_init(&list_free, &free_style, &free_iface, 1, 2));
    free_alloc.fail_free_on = 2u;
    M3_TEST_EXPECT(list_free.widget.vtable->destroy(list_free.widget.ctx),
                   M3_ERR_IO);
  }

  M3_TEST_EXPECT(list.widget.vtable->destroy(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(list.widget.vtable->destroy(list.widget.ctx));

  M3_TEST_OK(m3_grid_style_init(&grid_style));
  M3_TEST_EXPECT(m3_grid_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_ASSERT(grid_style.span == M3_GRID_DEFAULT_SPAN);

  M3_TEST_EXPECT(m3_grid_view_set_bind(NULL, NULL, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_grid_view_set_item_count(NULL, 1), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_grid_view_set_style(NULL, &grid_style),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_grid_view_set_scroll(NULL, 0.0f), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_grid_view_get_scroll(NULL, &scroll),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_grid_view_get_content_extent(NULL, &content_extent),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_grid_view_get_required_slots(NULL, &visible_count),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_grid_view_update(NULL), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_grid_view_init(NULL, &grid_style, NULL, 0, 0),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_grid_view_init(&grid, NULL, NULL, 0, 0),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_TRUE));
  M3_TEST_EXPECT(m3_grid_view_init(&grid, &grid_style, NULL, 0, 0),
                 M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_FALSE));

  M3_TEST_OK(m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_RESERVE_NODE_BYTES));
  M3_TEST_EXPECT(m3_grid_view_init(&grid, &grid_style, NULL, 0, 2),
                 M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_list_test_clear_fail_points());

  grid_style.span = 0u;
  M3_TEST_EXPECT(m3_grid_view_init(&grid, &grid_style, NULL, 0, 0),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_grid_style_init(&grid_style));
  M3_TEST_EXPECT(m3_grid_view_init(&grid, &grid_style, &bad_alloc, 0, 0),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_grid_view_init(&grid, &grid_style, NULL, 5, 0));
  M3_TEST_OK(m3_grid_view_reserve(&grid, 6));
  M3_TEST_EXPECT(m3_grid_view_reserve(NULL, 1), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_grid_view_reserve(&grid, overflow_value), M3_ERR_OVERFLOW);

  grid_style = grid.style;
  grid_style.spacing_x = -1.0f;
  M3_TEST_EXPECT(m3_grid_view_set_style(&grid, &grid_style), M3_ERR_RANGE);
  grid_style.spacing_x = 0.0f;
  grid_style.background_color.b = 2.0f;
  M3_TEST_EXPECT(m3_grid_view_set_style(&grid, &grid_style), M3_ERR_RANGE);
  M3_TEST_OK(m3_grid_style_init(&grid_style));

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
  M3_TEST_OK(m3_grid_view_set_style(&grid, &grid_style));
  M3_TEST_OK(m3_grid_view_set_bind(&grid, test_bind, &bind_ctx));

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = 20.0f;
  height_spec.mode = M3_MEASURE_AT_MOST;
  height_spec.size = 25.0f;
  M3_TEST_EXPECT(
      grid.widget.vtable->measure(NULL, width_spec, height_spec, &measured),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                             height_spec, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  grid_style = grid.style;
  grid.style.span = 0u;
  M3_TEST_EXPECT(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                             height_spec, &measured),
                 M3_ERR_RANGE);
  grid.style = grid_style;

  grid.bounds.height = -1.0f;
  M3_TEST_EXPECT(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                             height_spec, &measured),
                 M3_ERR_RANGE);
  grid.bounds.height = 0.0f;

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  height_spec.mode = M3_MEASURE_AT_MOST;
  height_spec.size = 10.0f;
  M3_TEST_EXPECT(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                             height_spec, &measured),
                 M3_ERR_RANGE);

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = 5.0f;
  height_spec.mode = 99u;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                             height_spec, &measured),
                 M3_ERR_INVALID_ARGUMENT);

  width_spec.mode = M3_MEASURE_EXACTLY;
  width_spec.size = 30.0f;
  height_spec.mode = M3_MEASURE_EXACTLY;
  height_spec.size = 15.0f;
  M3_TEST_OK(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                         height_spec, &measured));
  M3_TEST_ASSERT(measured.width == 30.0f);
  M3_TEST_ASSERT(measured.height == 15.0f);

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = 20.0f;
  height_spec.mode = M3_MEASURE_AT_MOST;
  height_spec.size = 25.0f;
  M3_TEST_OK(m3_list_test_set_fail_point(
      M3_LIST_TEST_FAIL_GRID_MEASURE_CONTENT_NEGATIVE));
  M3_TEST_EXPECT(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                             height_spec, &measured),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_list_test_clear_fail_points());

  width_spec.size = 20.0f;
  height_spec.size = 25.0f;
  M3_TEST_OK(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                         height_spec, &measured));
  M3_TEST_ASSERT(measured.width == 20.0f);
  M3_TEST_ASSERT(measured.height == 25.0f);

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_OK(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                         height_spec, &measured));
  M3_TEST_ASSERT(measured.width == 24.0f);
  M3_TEST_ASSERT(measured.height == 30.0f);

  bounds.width = -1.0f;
  bounds.height = 20.0f;
  M3_TEST_EXPECT(grid.widget.vtable->layout(grid.widget.ctx, bounds),
                 M3_ERR_RANGE);
  bounds.width = 30.0f;
  bounds.height = 20.0f;
  M3_TEST_EXPECT(grid.widget.vtable->layout(NULL, bounds),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(grid.widget.vtable->layout(grid.widget.ctx, bounds));
  M3_TEST_OK(m3_grid_view_set_bind(&grid, NULL, NULL));
  M3_TEST_EXPECT(m3_grid_view_update(&grid), M3_ERR_STATE);
  M3_TEST_OK(m3_grid_view_set_bind(&grid, test_bind, &bind_ctx));
  M3_TEST_OK(m3_grid_view_update(&grid));
  M3_TEST_ASSERT(grid.visible_count == 4);
  M3_TEST_ASSERT(grid.slots[0].index == 0);
  M3_TEST_ASSERT(grid.slots[1].index == 1);
  M3_TEST_ASSERT(grid.slots[2].node.bounds.x == 1.0f);
  M3_TEST_ASSERT(grid.slots[2].node.bounds.y == 11.0f);
  M3_TEST_OK(m3_grid_view_get_visible(&grid, &visible, &visible_count));
  M3_TEST_ASSERT(visible_count == 4);
  M3_TEST_ASSERT(visible[0] == &grid.slots[0].node);
  M3_TEST_EXPECT(m3_grid_view_get_visible(&grid, NULL, &visible_count),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_grid_view_get_visible(&grid, &visible, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  memset(&event, 0, sizeof(event));
  event.type = M3_INPUT_POINTER_DOWN;
  handled = M3_TRUE;
  M3_TEST_EXPECT(grid.widget.vtable->event(NULL, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(grid.widget.vtable->event(grid.widget.ctx, NULL, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(grid.widget.vtable->event(grid.widget.ctx, &event, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(grid.widget.vtable->event(grid.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  event.type = M3_INPUT_POINTER_SCROLL;
  event.data.pointer.scroll_y = 0;
  handled = M3_TRUE;
  M3_TEST_OK(grid.widget.vtable->event(grid.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  event.data.pointer.scroll_y = 4;
  handled = M3_FALSE;
  M3_TEST_OK(grid.widget.vtable->event(grid.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  grid.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  handled = M3_TRUE;
  M3_TEST_OK(grid.widget.vtable->event(grid.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  grid.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  grid_style.scroll_axis = M3_GRID_SCROLL_HORIZONTAL;
  grid_style.span = 2u;
  M3_TEST_OK(m3_grid_view_set_style(&grid, &grid_style));
  M3_TEST_OK(m3_grid_view_set_scroll(&grid, 3.0f));
  M3_TEST_OK(m3_grid_view_get_required_slots(&grid, &visible_count));
  M3_TEST_ASSERT(visible_count > 0);
  M3_TEST_OK(m3_grid_view_update(&grid));

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = 30.0f;
  height_spec.mode = M3_MEASURE_AT_MOST;
  height_spec.size = 15.0f;
  M3_TEST_OK(grid.widget.vtable->measure(grid.widget.ctx, width_spec,
                                         height_spec, &measured));
  M3_TEST_ASSERT(measured.width == 30.0f);
  M3_TEST_ASSERT(measured.height == 15.0f);

  event.type = M3_INPUT_POINTER_SCROLL;
  event.data.pointer.scroll_x = 5;
  handled = M3_FALSE;
  M3_TEST_OK(grid.widget.vtable->event(grid.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_OK(m3_grid_view_get_scroll(&grid, &scroll));
  M3_TEST_ASSERT(scroll == 6.0f);

  gfx_backend.draw_rect_calls = 0;
  gfx.vtable = &g_test_gfx_vtable;
  grid.style.background_color.a = 1.0f;
  M3_TEST_EXPECT(grid.widget.vtable->paint(NULL, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(grid.widget.vtable->paint(grid.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  {
    M3PaintContext bad_ctx;

    bad_ctx = paint_ctx;
    bad_ctx.gfx = NULL;
    M3_TEST_EXPECT(grid.widget.vtable->paint(grid.widget.ctx, &bad_ctx),
                   M3_ERR_INVALID_ARGUMENT);
  }
  M3_TEST_OK(grid.widget.vtable->paint(grid.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(gfx_backend.draw_rect_calls == 1);

  gfx.vtable = &g_test_gfx_vtable_no_draw;
  grid.style.background_color.a = 0.0f;
  M3_TEST_OK(grid.widget.vtable->paint(grid.widget.ctx, &paint_ctx));
  grid.style.background_color.a = 1.0f;
  M3_TEST_EXPECT(grid.widget.vtable->paint(grid.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);

  {
    M3Semantics semantics;
    M3_TEST_EXPECT(grid.widget.vtable->get_semantics(NULL, &semantics),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(grid.widget.vtable->get_semantics(grid.widget.ctx, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(grid.widget.vtable->get_semantics(grid.widget.ctx, &semantics));
    M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_NONE);
  }

  {
    M3Semantics semantics;
    M3GridStyle saved_style;
    M3Rect saved_bounds;
    m3_usize saved_item_count;

    saved_style = grid.style;
    saved_bounds = grid.bounds;
    saved_item_count = grid.item_count;

    M3_TEST_OK(m3_grid_view_set_item_count(&grid, grid.item_count));
    M3_TEST_OK(m3_grid_view_get_content_extent(&grid, &content_extent));
    M3_TEST_OK(
        m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE));
    M3_TEST_OK(m3_grid_view_set_scroll(&grid, 0.0f));
    M3_TEST_OK(m3_list_test_clear_fail_points());

    event.type = M3_INPUT_POINTER_SCROLL;
    grid.style.scroll_axis = M3_GRID_SCROLL_VERTICAL;
    event.data.pointer.scroll_y = 2;
    grid.style.item_height = 0.0f;
    M3_TEST_EXPECT(grid.widget.vtable->event(grid.widget.ctx, &event, &handled),
                   M3_ERR_RANGE);
    grid.style = saved_style;

    grid.style.spacing_x = -1.0f;
    M3_TEST_EXPECT(grid.widget.vtable->paint(grid.widget.ctx, &paint_ctx),
                   M3_ERR_RANGE);
    grid.style = saved_style;
    grid.bounds.width = -1.0f;
    M3_TEST_EXPECT(grid.widget.vtable->paint(grid.widget.ctx, &paint_ctx),
                   M3_ERR_RANGE);
    grid.bounds = saved_bounds;

    grid.widget.flags |= M3_WIDGET_FLAG_DISABLED;
    M3_TEST_OK(grid.widget.vtable->get_semantics(grid.widget.ctx, &semantics));
    M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_DISABLED) != 0u);
    grid.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

    grid.bounds.width = -1.0f;
    grid.bounds.height = -1.0f;
    M3_TEST_EXPECT(m3_grid_view_set_item_count(&grid, 1), M3_ERR_RANGE);
    grid.bounds = saved_bounds;

    grid.bounds.width = -1.0f;
    grid.bounds.height = -1.0f;
    M3_TEST_EXPECT(m3_grid_view_set_style(&grid, &saved_style), M3_ERR_RANGE);
    grid.bounds = saved_bounds;
    grid.style = saved_style;

    M3_TEST_OK(m3_grid_view_set_scroll(&grid, -5.0f));
    M3_TEST_OK(m3_grid_view_get_scroll(&grid, &scroll));
    M3_TEST_ASSERT(scroll == 0.0f);

    grid.bounds.width = -1.0f;
    grid.bounds.height = -1.0f;
    M3_TEST_EXPECT(m3_grid_view_set_scroll(&grid, 1.0f), M3_ERR_RANGE);
    grid.bounds = saved_bounds;

    grid.bounds.width = -1.0f;
    grid.bounds.height = -1.0f;
    M3_TEST_EXPECT(m3_grid_view_get_content_extent(&grid, &content_extent),
                   M3_ERR_RANGE);
    grid.bounds = saved_bounds;

    grid.style.spacing_x = -1.0f;
    M3_TEST_EXPECT(m3_grid_view_get_required_slots(&grid, &visible_count),
                   M3_ERR_RANGE);
    grid.style = saved_style;

    grid.bounds.width = -1.0f;
    grid.bounds.height = -1.0f;
    M3_TEST_EXPECT(m3_grid_view_get_required_slots(&grid, &visible_count),
                   M3_ERR_RANGE);
    grid.bounds = saved_bounds;

    M3_TEST_OK(m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_GRID_STRIDE));
    M3_TEST_EXPECT(m3_grid_view_get_required_slots(&grid, &visible_count),
                   M3_ERR_RANGE);
    M3_TEST_OK(m3_list_test_clear_fail_points());

    grid.style = saved_style;
    grid.bounds = saved_bounds;
    grid.item_count = saved_item_count;
  }

  {
    M3GridView grid_state;
    M3GridView grid_range;
    M3GridView grid_count;
    M3GridView grid_update;
    M3GridStyle update_style;
    TestAllocator update_alloc;
    M3Allocator update_iface;
    TestBindCtx update_bind;
    TestWidget update_pool[8];
    M3Rect update_bounds;

    test_allocator_init(&update_alloc);
    update_iface.ctx = &update_alloc;
    update_iface.alloc = test_alloc;
    update_iface.realloc = test_realloc;
    update_iface.free = test_free;
    M3_TEST_OK(m3_grid_style_init(&update_style));
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
    update_bind.pool_count = M3_COUNTOF(update_pool);
    update_bind.next = 0u;
    update_bind.calls = 0u;
    update_bind.last_index = 0u;
    update_bind.fail = 0;

    M3_TEST_OK(
        m3_grid_view_init(&grid_state, &update_style, &update_iface, 2, 0));
    grid_state.bounds = update_bounds;
    M3_TEST_OK(m3_grid_view_set_bind(&grid_state, test_bind, &update_bind));
    M3_TEST_EXPECT(m3_grid_view_update(&grid_state), M3_ERR_STATE);
    M3_TEST_OK(grid_state.widget.vtable->destroy(grid_state.widget.ctx));

    update_bind.next = 0u;
    update_bind.calls = 0u;
    update_bind.last_index = 0u;
    update_bind.fail = 0;
    M3_TEST_OK(
        m3_grid_view_init(&grid_range, &update_style, &update_iface, 6, 1));
    grid_range.bounds = update_bounds;
    M3_TEST_OK(m3_grid_view_set_bind(&grid_range, test_bind, &update_bind));
    M3_TEST_EXPECT(m3_grid_view_update(&grid_range), M3_ERR_RANGE);
    M3_TEST_OK(grid_range.widget.vtable->destroy(grid_range.widget.ctx));

    update_bind.next = 0u;
    update_bind.calls = 0u;
    update_bind.last_index = 0u;
    update_bind.fail = 0;
    M3_TEST_OK(
        m3_grid_view_init(&grid_count, &update_style, &update_iface, 0, 2));
    grid_count.bounds = update_bounds;
    M3_TEST_OK(m3_grid_view_set_bind(&grid_count, test_bind, &update_bind));
    M3_TEST_OK(m3_grid_view_update(&grid_count));
    M3_TEST_OK(grid_count.widget.vtable->destroy(grid_count.widget.ctx));

    update_bind.next = 0u;
    update_bind.calls = 0u;
    update_bind.last_index = 0u;
    update_bind.fail = 0;
    M3_TEST_OK(
        m3_grid_view_init(&grid_update, &update_style, &update_iface, 4, 4));
    grid_update.bounds = update_bounds;
    M3_TEST_OK(m3_grid_view_set_bind(&grid_update, test_bind, &update_bind));

    grid_update.style.spacing_x = -1.0f;
    M3_TEST_EXPECT(m3_grid_view_update(&grid_update), M3_ERR_RANGE);
    grid_update.style = update_style;

    grid_update.bounds.height = -1.0f;
    M3_TEST_EXPECT(m3_grid_view_update(&grid_update), M3_ERR_RANGE);
    grid_update.bounds = update_bounds;

    grid_update.style.item_height = 0.0f;
    M3_TEST_EXPECT(m3_grid_view_update(&grid_update), M3_ERR_RANGE);
    grid_update.style.item_height = update_style.item_height;

    M3_TEST_OK(m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_GRID_STRIDE));
    M3_TEST_EXPECT(m3_grid_view_update(&grid_update), M3_ERR_RANGE);
    M3_TEST_OK(m3_list_test_clear_fail_points());

    update_bind.fail = 1;
    M3_TEST_EXPECT(m3_grid_view_update(&grid_update), M3_ERR_UNKNOWN);
    update_bind.fail = 0;

    M3_TEST_OK(
        m3_grid_view_set_bind(&grid_update, test_bind_missing_widget, NULL));
    grid_update.slots[0].node.widget = NULL;
    M3_TEST_EXPECT(m3_grid_view_update(&grid_update), M3_ERR_STATE);
    M3_TEST_OK(m3_grid_view_set_bind(&grid_update, test_bind, &update_bind));

    M3_TEST_OK(m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_GRID_ITEM_BOUNDS));
    M3_TEST_EXPECT(m3_grid_view_update(&grid_update), M3_ERR_IO);
    M3_TEST_OK(m3_list_test_clear_fail_points());

    M3_TEST_OK(
        m3_list_test_set_fail_point(M3_LIST_TEST_FAIL_GRID_RENDER_BOUNDS));
    M3_TEST_EXPECT(m3_grid_view_update(&grid_update), M3_ERR_IO);
    M3_TEST_OK(m3_list_test_clear_fail_points());

    M3_TEST_OK(grid_update.widget.vtable->destroy(grid_update.widget.ctx));
  }

  {
    TestAllocator free_alloc;
    M3Allocator free_iface;
    M3GridView grid_free;
    M3GridStyle free_style;

    test_allocator_init(&free_alloc);
    free_iface.ctx = &free_alloc;
    free_iface.alloc = test_alloc;
    free_iface.realloc = test_realloc;
    free_iface.free = test_free;
    M3_TEST_OK(m3_grid_style_init(&free_style));
    M3_TEST_OK(m3_grid_view_init(&grid_free, &free_style, &free_iface, 1, 2));
    free_alloc.fail_free_on = 1u;
    M3_TEST_EXPECT(grid_free.widget.vtable->destroy(grid_free.widget.ctx),
                   M3_ERR_IO);
  }

  {
    TestAllocator free_alloc;
    M3Allocator free_iface;
    M3GridView grid_free;
    M3GridStyle free_style;

    test_allocator_init(&free_alloc);
    free_iface.ctx = &free_alloc;
    free_iface.alloc = test_alloc;
    free_iface.realloc = test_realloc;
    free_iface.free = test_free;
    M3_TEST_OK(m3_grid_style_init(&free_style));
    M3_TEST_OK(m3_grid_view_init(&grid_free, &free_style, &free_iface, 1, 2));
    free_alloc.fail_free_on = 2u;
    M3_TEST_EXPECT(grid_free.widget.vtable->destroy(grid_free.widget.ctx),
                   M3_ERR_IO);
  }

  M3_TEST_EXPECT(grid.widget.vtable->destroy(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(grid.widget.vtable->destroy(grid.widget.ctx));

  return 0;
}
