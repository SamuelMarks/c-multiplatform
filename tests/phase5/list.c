#include "test_utils.h"
#include "m3/m3_list.h"

#include <stdlib.h>
#include <string.h>

typedef struct TestAllocator {
    m3_usize alloc_calls;
    m3_usize realloc_calls;
    m3_usize free_calls;
    m3_usize fail_alloc_on;
} TestAllocator;

static int test_alloc(void *ctx, m3_usize size, void **out_ptr)
{
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

static int test_realloc(void *ctx, void *ptr, m3_usize size, void **out_ptr)
{
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

static int test_free(void *ctx, void *ptr)
{
    if (ctx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    free(ptr);
    return M3_OK;
}

static void test_allocator_init(TestAllocator *alloc)
{
    alloc->alloc_calls = 0;
    alloc->realloc_calls = 0;
    alloc->free_calls = 0;
    alloc->fail_alloc_on = 0;
}

typedef struct TestWidget {
    M3Widget widget;
    M3Rect last_bounds;
} TestWidget;

static int test_widget_measure(void *widget, M3MeasureSpec width, M3MeasureSpec height, M3Size *out_size)
{
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

static int test_widget_layout(void *widget, M3Rect bounds)
{
    TestWidget *w;

    if (widget == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    w = (TestWidget *)widget;
    w->last_bounds = bounds;
    return M3_OK;
}

static int test_widget_paint(void *widget, M3PaintContext *ctx)
{
    M3_UNUSED(widget);
    M3_UNUSED(ctx);
    return M3_OK;
}

static int test_widget_event(void *widget, const M3InputEvent *event, M3Bool *out_handled)
{
    if (widget == NULL || event == NULL || out_handled == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_handled = M3_FALSE;
    return M3_OK;
}

static int test_widget_semantics(void *widget, M3Semantics *out_semantics)
{
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

static int test_widget_destroy(void *widget)
{
    M3_UNUSED(widget);
    return M3_OK;
}

static const M3WidgetVTable g_test_widget_vtable = {
    test_widget_measure,
    test_widget_layout,
    test_widget_paint,
    test_widget_event,
    test_widget_semantics,
    test_widget_destroy
};

static void test_widget_init(TestWidget *widget)
{
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

static int test_bind(void *ctx, M3ListSlot *slot, m3_usize index)
{
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

static int test_bind_missing_widget(void *ctx, M3ListSlot *slot, m3_usize index)
{
    M3_UNUSED(ctx);
    M3_UNUSED(slot);
    M3_UNUSED(index);
    return M3_OK;
}

typedef struct TestGfxBackend {
    int draw_rect_calls;
} TestGfxBackend;

static int test_draw_rect(void *gfx, const M3Rect *rect, M3Color color, M3Scalar radius)
{
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

static const M3GfxVTable g_test_gfx_vtable = {
    NULL,
    NULL,
    NULL,
    test_draw_rect,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static const M3GfxVTable g_test_gfx_vtable_no_draw = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

int main(void)
{
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
    M3_TEST_ASSERT(list_style.orientation == M3_LIST_ORIENTATION_VERTICAL);
    M3_TEST_ASSERT(list_style.spacing == M3_LIST_DEFAULT_SPACING);
    M3_TEST_ASSERT(list_style.item_extent == M3_LIST_DEFAULT_ITEM_EXTENT);
    M3_TEST_ASSERT(list_style.overscan == M3_LIST_DEFAULT_OVERSCAN);

    M3_TEST_EXPECT(m3_list_view_init(NULL, &list_style, NULL, 0, 0), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_list_view_init(&list, NULL, NULL, 0, 0), M3_ERR_INVALID_ARGUMENT);

    list_style.orientation = 99u;
    M3_TEST_EXPECT(m3_list_view_init(&list, &list_style, NULL, 0, 0), M3_ERR_RANGE);
    M3_TEST_OK(m3_list_style_init(&list_style));

    M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_TRUE));
    M3_TEST_EXPECT(m3_list_view_init(&list, &list_style, NULL, 0, 0), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_FALSE));

    M3_TEST_EXPECT(m3_list_view_set_bind(NULL, NULL, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_list_view_set_item_count(NULL, 1), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_list_view_set_style(NULL, &list_style), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_list_view_set_scroll(NULL, 0.0f), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_list_view_get_scroll(NULL, &scroll), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_list_view_get_content_extent(NULL, &content_extent), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_list_view_get_required_slots(NULL, &visible_count), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_list_view_update(NULL), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(m3_list_test_mul_overflow(1, 1, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_list_test_mul_overflow(2, 3, &overflow_value));
    M3_TEST_ASSERT(overflow_value == 6);
    overflow_value = ((m3_usize)~(m3_usize)0) / (m3_usize)sizeof(M3ListSlot) + 1;
    M3_TEST_EXPECT(m3_list_test_mul_overflow(overflow_value, (m3_usize)sizeof(M3ListSlot), &overflow_value), M3_ERR_OVERFLOW);

    bad_alloc.ctx = NULL;
    bad_alloc.alloc = NULL;
    bad_alloc.realloc = NULL;
    bad_alloc.free = NULL;
    M3_TEST_EXPECT(m3_list_view_init(&list, &list_style, &bad_alloc, 0, 0), M3_ERR_INVALID_ARGUMENT);

    test_allocator_init(&alloc);
    alloc.fail_alloc_on = 1;
    alloc_iface.ctx = &alloc;
    alloc_iface.alloc = test_alloc;
    alloc_iface.realloc = test_realloc;
    alloc_iface.free = test_free;
    M3_TEST_EXPECT(m3_list_view_init(&list, &list_style, &alloc_iface, 0, 2), M3_ERR_OUT_OF_MEMORY);

    alloc.fail_alloc_on = 0;
    M3_TEST_OK(m3_list_view_init(&list, &list_style, &alloc_iface, 5, 0));
    M3_TEST_EXPECT(m3_list_view_reserve(NULL, 1), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_list_view_reserve(&list, 4));
    M3_TEST_ASSERT(list.slot_capacity == 4);
    M3_TEST_ASSERT(list.slots != NULL);
    M3_TEST_ASSERT(list.visible_nodes != NULL);
    M3_TEST_OK(m3_list_view_reserve(&list, 2));
    M3_TEST_EXPECT(m3_list_view_reserve(&list, overflow_value), M3_ERR_OVERFLOW);
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
    width_spec.size = -1.0f;
    height_spec.mode = M3_MEASURE_AT_MOST;
    height_spec.size = 10.0f;
    M3_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec, height_spec, &measured), M3_ERR_RANGE);

    width_spec.mode = M3_MEASURE_AT_MOST;
    width_spec.size = 5.0f;
    height_spec.mode = M3_MEASURE_AT_MOST;
    height_spec.size = 30.0f;
    M3_TEST_OK(list.widget.vtable->measure(list.widget.ctx, width_spec, height_spec, &measured));
    M3_TEST_ASSERT(measured.width == 2.0f);
    M3_TEST_ASSERT(measured.height == 30.0f);

    width_spec.mode = M3_MEASURE_EXACTLY;
    width_spec.size = 100.0f;
    height_spec.mode = M3_MEASURE_EXACTLY;
    height_spec.size = 50.0f;
    M3_TEST_OK(list.widget.vtable->measure(list.widget.ctx, width_spec, height_spec, &measured));
    M3_TEST_ASSERT(measured.width == 100.0f);
    M3_TEST_ASSERT(measured.height == 50.0f);

    width_spec.mode = 99u;
    width_spec.size = 0.0f;
    M3_TEST_EXPECT(list.widget.vtable->measure(list.widget.ctx, width_spec, height_spec, &measured), M3_ERR_INVALID_ARGUMENT);

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = -1.0f;
    bounds.height = 10.0f;
    M3_TEST_EXPECT(list.widget.vtable->layout(list.widget.ctx, bounds), M3_ERR_RANGE);

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

    M3_TEST_EXPECT(m3_list_view_get_visible(&list, NULL, &visible_count), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_list_view_get_visible(&list, &visible, NULL), M3_ERR_INVALID_ARGUMENT);

    gfx_backend.draw_rect_calls = 0;
    gfx.ctx = &gfx_backend;
    gfx.vtable = &g_test_gfx_vtable;
    gfx.text_vtable = NULL;
    paint_ctx.gfx = &gfx;
    paint_ctx.clip = bounds;
    paint_ctx.dpi_scale = 1.0f;
    list.style.background_color.a = 1.0f;
    M3_TEST_OK(list.widget.vtable->paint(list.widget.ctx, &paint_ctx));
    M3_TEST_ASSERT(gfx_backend.draw_rect_calls == 1);

    gfx.vtable = &g_test_gfx_vtable_no_draw;
    list.style.background_color.a = 0.0f;
    M3_TEST_OK(list.widget.vtable->paint(list.widget.ctx, &paint_ctx));

    list.style.background_color.a = 1.0f;
    M3_TEST_EXPECT(list.widget.vtable->paint(list.widget.ctx, &paint_ctx), M3_ERR_UNSUPPORTED);

    {
        M3Semantics semantics;
        M3_TEST_OK(list.widget.vtable->get_semantics(list.widget.ctx, &semantics));
        M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_NONE);
    }

    memset(&event, 0, sizeof(event));
    event.type = M3_INPUT_POINTER_DOWN;
    handled = M3_TRUE;
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
    M3_TEST_OK(list.widget.vtable->measure(list.widget.ctx, width_spec, height_spec, &measured));
    M3_TEST_ASSERT(measured.width == 10.0f);
    M3_TEST_ASSERT(measured.height == 2.0f);

    event.type = M3_INPUT_POINTER_SCROLL;
    event.data.pointer.scroll_x = 4;
    handled = M3_FALSE;
    M3_TEST_OK(list.widget.vtable->event(list.widget.ctx, &event, &handled));
    M3_TEST_ASSERT(handled == M3_TRUE);

    M3_TEST_OK(list.widget.vtable->destroy(list.widget.ctx));

    M3_TEST_OK(m3_grid_style_init(&grid_style));
    M3_TEST_ASSERT(grid_style.span == M3_GRID_DEFAULT_SPAN);

    M3_TEST_EXPECT(m3_grid_view_set_bind(NULL, NULL, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_grid_view_set_item_count(NULL, 1), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_grid_view_set_style(NULL, &grid_style), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_grid_view_set_scroll(NULL, 0.0f), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_grid_view_get_scroll(NULL, &scroll), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_grid_view_get_content_extent(NULL, &content_extent), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_grid_view_get_required_slots(NULL, &visible_count), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_grid_view_update(NULL), M3_ERR_INVALID_ARGUMENT);

    grid_style.span = 0u;
    M3_TEST_EXPECT(m3_grid_view_init(&grid, &grid_style, NULL, 0, 0), M3_ERR_RANGE);
    M3_TEST_OK(m3_grid_style_init(&grid_style));
    M3_TEST_EXPECT(m3_grid_view_init(&grid, &grid_style, &bad_alloc, 0, 0), M3_ERR_INVALID_ARGUMENT);

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
    width_spec.size = -1.0f;
    height_spec.mode = M3_MEASURE_AT_MOST;
    height_spec.size = 10.0f;
    M3_TEST_EXPECT(grid.widget.vtable->measure(grid.widget.ctx, width_spec, height_spec, &measured), M3_ERR_RANGE);

    width_spec.size = 20.0f;
    height_spec.size = 25.0f;
    M3_TEST_OK(grid.widget.vtable->measure(grid.widget.ctx, width_spec, height_spec, &measured));
    M3_TEST_ASSERT(measured.width == 20.0f);
    M3_TEST_ASSERT(measured.height == 25.0f);

    bounds.width = 30.0f;
    bounds.height = 20.0f;
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
    M3_TEST_EXPECT(m3_grid_view_get_visible(&grid, NULL, &visible_count), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_grid_view_get_visible(&grid, &visible, NULL), M3_ERR_INVALID_ARGUMENT);

    memset(&event, 0, sizeof(event));
    event.type = M3_INPUT_POINTER_DOWN;
    handled = M3_TRUE;
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
    M3_TEST_OK(grid.widget.vtable->measure(grid.widget.ctx, width_spec, height_spec, &measured));
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
    M3_TEST_OK(grid.widget.vtable->paint(grid.widget.ctx, &paint_ctx));
    M3_TEST_ASSERT(gfx_backend.draw_rect_calls == 1);

    gfx.vtable = &g_test_gfx_vtable_no_draw;
    grid.style.background_color.a = 0.0f;
    M3_TEST_OK(grid.widget.vtable->paint(grid.widget.ctx, &paint_ctx));
    grid.style.background_color.a = 1.0f;
    M3_TEST_EXPECT(grid.widget.vtable->paint(grid.widget.ctx, &paint_ctx), M3_ERR_UNSUPPORTED);

    {
        M3Semantics semantics;
        M3_TEST_OK(grid.widget.vtable->get_semantics(grid.widget.ctx, &semantics));
        M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_NONE);
    }

    M3_TEST_OK(grid.widget.vtable->destroy(grid.widget.ctx));

    return 0;
}
