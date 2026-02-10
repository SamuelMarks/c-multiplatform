#include "test_utils.h"
#include "m3/m3_navigation.h"

#include <string.h>

typedef struct TestNavBackend {
    int create_calls;
    int destroy_calls;
    int measure_calls;
    int draw_calls;
    int draw_rect_calls;
    int fail_create;
    int fail_destroy;
    int fail_measure;
    int fail_draw;
    int fail_draw_rect;
    M3Handle last_font;
    M3Rect last_rect;
    M3Color last_rect_color;
    M3Scalar last_corner;
    M3Scalar last_text_x;
    M3Scalar last_text_y;
    m3_usize last_text_len;
    M3Color last_text_color;
} TestNavBackend;

typedef struct TestSelectState {
    int calls;
    m3_usize last_index;
    int fail_index;
} TestSelectState;

static int test_backend_init(TestNavBackend *backend)
{
    if (backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    memset(backend, 0, sizeof(*backend));
    return M3_OK;
}

static int test_text_create_font(void *text, const char *utf8_family, m3_i32 size_px, m3_i32 weight, M3Bool italic,
    M3Handle *out_font)
{
    TestNavBackend *backend;

    if (text == NULL || utf8_family == NULL || out_font == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (size_px <= 0) {
        return M3_ERR_RANGE;
    }
    if (weight < 100 || weight > 900) {
        return M3_ERR_RANGE;
    }
    if (italic != M3_FALSE && italic != M3_TRUE) {
        return M3_ERR_RANGE;
    }

    backend = (TestNavBackend *)text;
    backend->create_calls += 1;
    if (backend->fail_create) {
        return M3_ERR_IO;
    }

    out_font->id = 1u;
    out_font->generation = 1u;
    backend->last_font = *out_font;
    return M3_OK;
}

static int test_text_destroy_font(void *text, M3Handle font)
{
    TestNavBackend *backend;

    if (text == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (font.id == 0u && font.generation == 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (TestNavBackend *)text;
    backend->destroy_calls += 1;
    if (backend->fail_destroy) {
        return M3_ERR_IO;
    }
    return M3_OK;
}

static int test_text_measure_text(void *text, M3Handle font, const char *utf8, m3_usize utf8_len, M3Scalar *out_width,
    M3Scalar *out_height, M3Scalar *out_baseline)
{
    TestNavBackend *backend;

    if (text == NULL || out_width == NULL || out_height == NULL || out_baseline == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8 == NULL && utf8_len != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (font.id == 0u && font.generation == 0u) {
        return M3_ERR_STATE;
    }

    backend = (TestNavBackend *)text;
    backend->measure_calls += 1;
    if (backend->fail_measure) {
        return M3_ERR_IO;
    }

    *out_width = (M3Scalar)(utf8_len * 10u);
    *out_height = 20.0f;
    *out_baseline = 15.0f;
    return M3_OK;
}

static int test_text_draw_text(void *text, M3Handle font, const char *utf8, m3_usize utf8_len, M3Scalar x, M3Scalar y,
    M3Color color)
{
    TestNavBackend *backend;

    if (text == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8 == NULL && utf8_len != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (font.id == 0u && font.generation == 0u) {
        return M3_ERR_STATE;
    }

    backend = (TestNavBackend *)text;
    backend->draw_calls += 1;
    if (backend->fail_draw) {
        return M3_ERR_IO;
    }

    backend->last_text_x = x;
    backend->last_text_y = y;
    backend->last_text_len = utf8_len;
    backend->last_text_color = color;
    return M3_OK;
}

static int test_gfx_draw_rect(void *gfx, const M3Rect *rect, M3Color color, M3Scalar corner_radius)
{
    TestNavBackend *backend;

    if (gfx == NULL || rect == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (TestNavBackend *)gfx;
    backend->draw_rect_calls += 1;
    backend->last_rect = *rect;
    backend->last_rect_color = color;
    backend->last_corner = corner_radius;
    if (backend->fail_draw_rect) {
        return M3_ERR_IO;
    }
    return M3_OK;
}

static const M3TextVTable g_test_text_vtable = {
    test_text_create_font,
    test_text_destroy_font,
    test_text_measure_text,
    test_text_draw_text
};

static const M3TextVTable g_test_text_vtable_no_draw = {
    test_text_create_font,
    test_text_destroy_font,
    test_text_measure_text,
    NULL
};

static const M3TextVTable g_test_text_vtable_no_measure = {
    test_text_create_font,
    test_text_destroy_font,
    NULL,
    test_text_draw_text
};

static const M3GfxVTable g_test_gfx_vtable = {
    NULL,
    NULL,
    NULL,
    test_gfx_draw_rect,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static int m3_near(M3Scalar a, M3Scalar b, M3Scalar tol)
{
    M3Scalar diff;

    diff = a - b;
    if (diff < 0.0f) {
        diff = -diff;
    }
    return (diff <= tol) ? 1 : 0;
}

static int test_style_init(void)
{
    M3NavigationStyle style;

    M3_TEST_EXPECT(m3_navigation_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_navigation_style_init(&style));
    M3_TEST_ASSERT(style.mode == M3_NAV_MODE_AUTO);
    M3_TEST_ASSERT(m3_near(style.bar_height, M3_NAV_DEFAULT_BAR_HEIGHT, 0.001f));
    M3_TEST_ASSERT(m3_near(style.rail_width, M3_NAV_DEFAULT_RAIL_WIDTH, 0.001f));
    M3_TEST_ASSERT(m3_near(style.drawer_width, M3_NAV_DEFAULT_DRAWER_WIDTH, 0.001f));
    M3_TEST_ASSERT(m3_near(style.item_height, M3_NAV_DEFAULT_ITEM_HEIGHT, 0.001f));
    M3_TEST_ASSERT(m3_near(style.item_min_width, M3_NAV_DEFAULT_ITEM_MIN_WIDTH, 0.001f));
    M3_TEST_ASSERT(m3_near(style.item_spacing, M3_NAV_DEFAULT_ITEM_SPACING, 0.001f));
    M3_TEST_ASSERT(m3_near(style.indicator_thickness, M3_NAV_DEFAULT_INDICATOR_THICKNESS, 0.001f));
    M3_TEST_ASSERT(m3_near(style.indicator_corner, M3_NAV_DEFAULT_INDICATOR_CORNER, 0.001f));
    M3_TEST_ASSERT(m3_near(style.breakpoint_rail, M3_NAV_DEFAULT_BREAKPOINT_RAIL, 0.001f));
    M3_TEST_ASSERT(m3_near(style.breakpoint_drawer, M3_NAV_DEFAULT_BREAKPOINT_DRAWER, 0.001f));
    M3_TEST_ASSERT(style.text_style.size_px == 14);
    M3_TEST_ASSERT(style.text_style.weight == 400);
    M3_TEST_ASSERT(style.text_style.italic == M3_FALSE);
    M3_TEST_ASSERT(style.text_style.utf8_family == NULL);
    M3_TEST_ASSERT(m3_near(style.background_color.a, 0.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(style.selected_text_color.a, 1.0f, 0.001f));
    return M3_OK;
}

static int test_init_validation(void)
{
    TestNavBackend backend_state;
    M3TextBackend backend;
    M3NavigationStyle style;
    M3NavigationStyle bad_style;
    M3Navigation nav;
    M3NavigationItem items[2];
    M3NavigationItem bad_items[1];
    M3TextBackend bad_backend;

    M3_TEST_OK(test_backend_init(&backend_state));
    backend.ctx = &backend_state;
    backend.vtable = &g_test_text_vtable;

    M3_TEST_OK(m3_navigation_style_init(&style));
    style.text_style.utf8_family = "Sans";

    items[0].utf8_label = "Home";
    items[0].utf8_len = 4;
    items[1].utf8_label = "Settings";
    items[1].utf8_len = 8;

    M3_TEST_EXPECT(m3_navigation_init(NULL, &backend, &style, items, 2, 0), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_navigation_init(&nav, NULL, &style, items, 2, 0), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_navigation_init(&nav, &backend, NULL, items, 2, 0), M3_ERR_INVALID_ARGUMENT);

    bad_backend = backend;
    bad_backend.vtable = &g_test_text_vtable_no_draw;
    M3_TEST_EXPECT(m3_navigation_init(&nav, &bad_backend, &style, items, 2, 0), M3_ERR_UNSUPPORTED);

    bad_style = style;
    bad_style.item_height = -1.0f;
    M3_TEST_EXPECT(m3_navigation_init(&nav, &backend, &bad_style, items, 2, 0), M3_ERR_RANGE);

    M3_TEST_EXPECT(m3_navigation_init(&nav, &backend, &style, NULL, 1, 0), M3_ERR_INVALID_ARGUMENT);

    bad_items[0].utf8_label = NULL;
    bad_items[0].utf8_len = 1;
    M3_TEST_EXPECT(m3_navigation_init(&nav, &backend, &style, bad_items, 1, 0), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(m3_navigation_init(&nav, &backend, &style, items, 2, 5), M3_ERR_RANGE);

    M3_TEST_OK(m3_navigation_init(&nav, &backend, &style, items, 2, 0));
    M3_TEST_ASSERT(nav.widget.vtable != NULL);
    M3_TEST_ASSERT((nav.widget.flags & M3_WIDGET_FLAG_FOCUSABLE) != 0u);
    M3_TEST_ASSERT(nav.selected_index == 0u);

    M3_TEST_OK(nav.widget.vtable->destroy(&nav));
    return M3_OK;
}

static int test_setters_and_getters(void)
{
    TestNavBackend backend_state;
    M3TextBackend backend;
    M3NavigationStyle style;
    M3Navigation nav;
    M3NavigationItem items[2];
    M3NavigationItem new_items[1];
    m3_usize selected;
    m3_u32 mode;

    M3_TEST_OK(test_backend_init(&backend_state));
    backend.ctx = &backend_state;
    backend.vtable = &g_test_text_vtable;

    M3_TEST_OK(m3_navigation_style_init(&style));
    style.text_style.utf8_family = "Sans";

    items[0].utf8_label = "One";
    items[0].utf8_len = 3;
    items[1].utf8_label = "Two";
    items[1].utf8_len = 3;

    M3_TEST_OK(m3_navigation_init(&nav, &backend, &style, items, 2, 0));

    M3_TEST_EXPECT(m3_navigation_get_selected(NULL, &selected), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_navigation_get_selected(&nav, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_navigation_get_selected(&nav, &selected));
    M3_TEST_ASSERT(selected == 0u);

    M3_TEST_EXPECT(m3_navigation_set_selected(NULL, 0), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_navigation_set_selected(&nav, 5), M3_ERR_RANGE);
    M3_TEST_OK(m3_navigation_set_selected(&nav, M3_NAV_INVALID_INDEX));
    M3_TEST_OK(m3_navigation_get_selected(&nav, &selected));
    M3_TEST_ASSERT(selected == M3_NAV_INVALID_INDEX);
    M3_TEST_OK(m3_navigation_set_selected(&nav, 1));

    M3_TEST_EXPECT(m3_navigation_set_on_select(NULL, NULL, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_navigation_set_on_select(&nav, NULL, NULL));

    M3_TEST_EXPECT(m3_navigation_set_items(NULL, NULL, 0), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_navigation_set_items(&nav, NULL, 0));
    M3_TEST_ASSERT(nav.item_count == 0u);
    M3_TEST_ASSERT(nav.selected_index == M3_NAV_INVALID_INDEX);

    new_items[0].utf8_label = "Only";
    new_items[0].utf8_len = 4;
    nav.selected_index = 2;
    nav.pressed_index = 2;
    M3_TEST_OK(m3_navigation_set_items(&nav, new_items, 1));
    M3_TEST_ASSERT(nav.selected_index == M3_NAV_INVALID_INDEX);
    M3_TEST_ASSERT(nav.pressed_index == M3_NAV_INVALID_INDEX);

    nav.bounds.width = -1.0f;
    M3_TEST_EXPECT(m3_navigation_get_mode(&nav, &mode), M3_ERR_RANGE);

    M3_TEST_OK(nav.widget.vtable->destroy(&nav));
    return M3_OK;
}

static int test_measure_layout_mode(void)
{
    TestNavBackend backend_state;
    M3TextBackend backend;
    M3NavigationStyle style;
    M3Navigation nav;
    M3NavigationItem items[2];
    M3MeasureSpec bad_spec;
    M3MeasureSpec size_spec;
    M3Size size;
    M3Rect bounds;
    m3_u32 mode;

    M3_TEST_OK(test_backend_init(&backend_state));
    backend.ctx = &backend_state;
    backend.vtable = &g_test_text_vtable;

    M3_TEST_OK(m3_navigation_style_init(&style));
    style.text_style.utf8_family = "Sans";
    style.item_min_width = 50.0f;
    style.item_spacing = 10.0f;
    style.bar_height = 60.0f;
    style.padding.left = 5.0f;
    style.padding.right = 5.0f;
    style.padding.top = 2.0f;
    style.padding.bottom = 2.0f;

    items[0].utf8_label = "A";
    items[0].utf8_len = 1;
    items[1].utf8_label = "B";
    items[1].utf8_len = 1;

    M3_TEST_OK(m3_navigation_init(&nav, &backend, &style, items, 2, 0));

    bad_spec.mode = 99u;
    bad_spec.size = 0.0f;
    size_spec.mode = M3_MEASURE_EXACTLY;
    size_spec.size = 10.0f;
    M3_TEST_EXPECT(nav.widget.vtable->measure(&nav, bad_spec, size_spec, &size), M3_ERR_INVALID_ARGUMENT);

    bad_spec.mode = M3_MEASURE_EXACTLY;
    bad_spec.size = -5.0f;
    M3_TEST_EXPECT(nav.widget.vtable->measure(&nav, bad_spec, size_spec, &size), M3_ERR_RANGE);

    style.mode = M3_NAV_MODE_BAR;
    M3_TEST_OK(m3_navigation_set_style(&nav, &style));
    size_spec.mode = M3_MEASURE_UNSPECIFIED;
    size_spec.size = 0.0f;
    M3_TEST_OK(nav.widget.vtable->measure(&nav, size_spec, size_spec, &size));
    M3_TEST_ASSERT(m3_near(size.width, 120.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(size.height, 60.0f, 0.001f));

    style.mode = M3_NAV_MODE_RAIL;
    style.rail_width = 72.0f;
    style.item_height = 40.0f;
    style.item_spacing = 5.0f;
    style.padding.top = 4.0f;
    style.padding.bottom = 4.0f;
    M3_TEST_OK(m3_navigation_set_style(&nav, &style));
    M3_TEST_OK(nav.widget.vtable->measure(&nav, size_spec, size_spec, &size));
    M3_TEST_ASSERT(m3_near(size.width, 72.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(size.height, 93.0f, 0.001f));

    style.mode = M3_NAV_MODE_DRAWER;
    style.drawer_width = 300.0f;
    M3_TEST_OK(m3_navigation_set_style(&nav, &style));
    M3_TEST_OK(nav.widget.vtable->measure(&nav, size_spec, size_spec, &size));
    M3_TEST_ASSERT(m3_near(size.width, 300.0f, 0.001f));

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = -1.0f;
    bounds.height = 10.0f;
    M3_TEST_EXPECT(nav.widget.vtable->layout(&nav, bounds), M3_ERR_RANGE);

    style.mode = M3_NAV_MODE_AUTO;
    style.breakpoint_rail = 100.0f;
    style.breakpoint_drawer = 200.0f;
    M3_TEST_OK(m3_navigation_set_style(&nav, &style));

    bounds.width = 80.0f;
    bounds.height = 60.0f;
    M3_TEST_OK(nav.widget.vtable->layout(&nav, bounds));
    M3_TEST_OK(m3_navigation_get_mode(&nav, &mode));
    M3_TEST_ASSERT(mode == M3_NAV_MODE_BAR);

    bounds.width = 150.0f;
    bounds.height = 60.0f;
    M3_TEST_OK(nav.widget.vtable->layout(&nav, bounds));
    M3_TEST_OK(m3_navigation_get_mode(&nav, &mode));
    M3_TEST_ASSERT(mode == M3_NAV_MODE_RAIL);

    bounds.width = 250.0f;
    bounds.height = 60.0f;
    M3_TEST_OK(nav.widget.vtable->layout(&nav, bounds));
    M3_TEST_OK(m3_navigation_get_mode(&nav, &mode));
    M3_TEST_ASSERT(mode == M3_NAV_MODE_DRAWER);

    style.mode = M3_NAV_MODE_BAR;
    M3_TEST_OK(m3_navigation_set_style(&nav, &style));
    M3_TEST_OK(m3_navigation_get_mode(&nav, &mode));
    M3_TEST_ASSERT(mode == M3_NAV_MODE_BAR);

    M3_TEST_OK(nav.widget.vtable->destroy(&nav));
    return M3_OK;
}

static int test_on_select(void *ctx, M3Navigation *nav, m3_usize index)
{
    TestSelectState *state;

    if (ctx == NULL || nav == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    state = (TestSelectState *)ctx;
    state->calls += 1;
    state->last_index = index;
    if (state->fail_index >= 0 && (m3_usize)state->fail_index == index) {
        return M3_ERR_IO;
    }
    return M3_OK;
}

static int test_event_selection(void)
{
    TestNavBackend backend_state;
    TestSelectState select_state;
    M3TextBackend backend;
    M3NavigationStyle style;
    M3Navigation nav;
    M3NavigationItem items[3];
    M3Rect bounds;
    M3InputEvent event;
    M3Bool handled;

    M3_TEST_OK(test_backend_init(&backend_state));
    backend.ctx = &backend_state;
    backend.vtable = &g_test_text_vtable;

    M3_TEST_OK(m3_navigation_style_init(&style));
    style.text_style.utf8_family = "Sans";
    style.mode = M3_NAV_MODE_BAR;
    style.item_spacing = 0.0f;
    style.padding.left = 0.0f;
    style.padding.right = 0.0f;
    style.padding.top = 0.0f;
    style.padding.bottom = 0.0f;

    items[0].utf8_label = "A";
    items[0].utf8_len = 1;
    items[1].utf8_label = "B";
    items[1].utf8_len = 1;
    items[2].utf8_label = "C";
    items[2].utf8_len = 1;

    M3_TEST_OK(m3_navigation_init(&nav, &backend, &style, items, 3, 0));

    select_state.calls = 0;
    select_state.last_index = M3_NAV_INVALID_INDEX;
    select_state.fail_index = -1;
    M3_TEST_OK(m3_navigation_set_on_select(&nav, test_on_select, &select_state));

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = 300.0f;
    bounds.height = 60.0f;
    M3_TEST_OK(nav.widget.vtable->layout(&nav, bounds));

    memset(&event, 0, sizeof(event));
    event.type = M3_INPUT_POINTER_DOWN;
    event.data.pointer.x = 10;
    event.data.pointer.y = 10;
    M3_TEST_OK(nav.widget.vtable->event(&nav, &event, &handled));
    M3_TEST_ASSERT(handled == M3_TRUE);
    M3_TEST_ASSERT(nav.pressed_index == 0u);

    M3_TEST_EXPECT(nav.widget.vtable->event(&nav, &event, &handled), M3_ERR_STATE);

    event.type = M3_INPUT_POINTER_UP;
    M3_TEST_OK(nav.widget.vtable->event(&nav, &event, &handled));
    M3_TEST_ASSERT(handled == M3_TRUE);
    M3_TEST_ASSERT(nav.selected_index == 0u);
    M3_TEST_ASSERT(select_state.calls == 1);
    M3_TEST_ASSERT(select_state.last_index == 0u);

    event.type = M3_INPUT_POINTER_DOWN;
    event.data.pointer.x = 150;
    event.data.pointer.y = 10;
    M3_TEST_OK(nav.widget.vtable->event(&nav, &event, &handled));
    M3_TEST_ASSERT(handled == M3_TRUE);

    event.type = M3_INPUT_POINTER_UP;
    event.data.pointer.x = 290;
    event.data.pointer.y = 10;
    M3_TEST_OK(nav.widget.vtable->event(&nav, &event, &handled));
    M3_TEST_ASSERT(handled == M3_TRUE);
    M3_TEST_ASSERT(nav.selected_index == 0u);

    select_state.fail_index = 1;
    event.type = M3_INPUT_POINTER_DOWN;
    event.data.pointer.x = 150;
    event.data.pointer.y = 10;
    M3_TEST_OK(nav.widget.vtable->event(&nav, &event, &handled));
    event.type = M3_INPUT_POINTER_UP;
    M3_TEST_EXPECT(nav.widget.vtable->event(&nav, &event, &handled), M3_ERR_IO);
    M3_TEST_ASSERT(nav.selected_index == 0u);

    nav.widget.flags |= M3_WIDGET_FLAG_DISABLED;
    event.type = M3_INPUT_POINTER_DOWN;
    M3_TEST_OK(nav.widget.vtable->event(&nav, &event, &handled));
    M3_TEST_ASSERT(handled == M3_FALSE);
    nav.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

    event.type = M3_INPUT_KEY_DOWN;
    M3_TEST_OK(nav.widget.vtable->event(&nav, &event, &handled));
    M3_TEST_ASSERT(handled == M3_FALSE);

    M3_TEST_OK(nav.widget.vtable->destroy(&nav));
    return M3_OK;
}

static int test_paint(void)
{
    TestNavBackend backend_state;
    M3TextBackend backend;
    M3NavigationStyle style;
    M3Navigation nav;
    M3NavigationItem items[2];
    M3PaintContext paint_ctx;
    M3Gfx gfx;
    M3Rect bounds;
    M3Rect clip;
    int rc;

    M3_TEST_OK(test_backend_init(&backend_state));
    backend.ctx = &backend_state;
    backend.vtable = &g_test_text_vtable;

    M3_TEST_OK(m3_navigation_style_init(&style));
    style.text_style.utf8_family = "Sans";
    style.mode = M3_NAV_MODE_BAR;
    style.background_color.a = 1.0f;
    style.indicator_thickness = 2.0f;
    style.selected_text_color.r = 1.0f;
    style.selected_text_color.g = 0.0f;
    style.selected_text_color.b = 0.0f;
    style.selected_text_color.a = 1.0f;

    items[0].utf8_label = "X";
    items[0].utf8_len = 1;
    items[1].utf8_label = "Y";
    items[1].utf8_len = 1;

    M3_TEST_OK(m3_navigation_init(&nav, &backend, &style, items, 2, 1));

    gfx.ctx = &backend_state;
    gfx.vtable = &g_test_gfx_vtable;
    gfx.text_vtable = &g_test_text_vtable;

    clip.x = 0.0f;
    clip.y = 0.0f;
    clip.width = 200.0f;
    clip.height = 80.0f;
    paint_ctx.gfx = &gfx;
    paint_ctx.clip = clip;
    paint_ctx.dpi_scale = 1.0f;

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = 200.0f;
    bounds.height = 80.0f;
    M3_TEST_OK(nav.widget.vtable->layout(&nav, bounds));

    rc = nav.widget.vtable->paint(&nav, &paint_ctx);
    M3_TEST_OK(rc);
    M3_TEST_ASSERT(backend_state.draw_rect_calls == 2);
    M3_TEST_ASSERT(backend_state.draw_calls == 2);
    M3_TEST_ASSERT(m3_near(backend_state.last_text_color.r, 1.0f, 0.001f));

    style.background_color.a = 0.0f;
    M3_TEST_OK(m3_navigation_set_style(&nav, &style));
    backend_state.draw_rect_calls = 0;
    backend_state.draw_calls = 0;
    M3_TEST_OK(nav.widget.vtable->paint(&nav, &paint_ctx));
    M3_TEST_ASSERT(backend_state.draw_rect_calls == 1);
    M3_TEST_ASSERT(backend_state.draw_calls == 2);

    nav.bounds.width = -1.0f;
    M3_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), M3_ERR_RANGE);
    nav.bounds.width = bounds.width;

    gfx.vtable = NULL;
    M3_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), M3_ERR_INVALID_ARGUMENT);
    gfx.vtable = &g_test_gfx_vtable;

    gfx.vtable = &g_test_gfx_vtable;
    gfx.text_vtable = NULL;
    M3_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), M3_ERR_UNSUPPORTED);
    gfx.text_vtable = &g_test_text_vtable_no_draw;
    M3_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), M3_ERR_UNSUPPORTED);
    gfx.text_vtable = &g_test_text_vtable;

    nav.text_backend.vtable = &g_test_text_vtable_no_measure;
    M3_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), M3_ERR_UNSUPPORTED);
    nav.text_backend.vtable = &g_test_text_vtable;

    backend_state.fail_measure = 1;
    M3_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), M3_ERR_IO);
    backend_state.fail_measure = 0;

    backend_state.fail_draw = 1;
    M3_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), M3_ERR_IO);
    backend_state.fail_draw = 0;

    backend_state.fail_draw_rect = 1;
    M3_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), M3_ERR_IO);
    backend_state.fail_draw_rect = 0;

    nav.style.indicator_thickness = -1.0f;
    M3_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), M3_ERR_RANGE);

    M3_TEST_OK(nav.widget.vtable->destroy(&nav));
    return M3_OK;
}

static int test_destroy(void)
{
    TestNavBackend backend_state;
    M3TextBackend backend;
    M3NavigationStyle style;
    M3Navigation nav;
    M3NavigationItem items[1];

    M3_TEST_OK(test_backend_init(&backend_state));
    backend.ctx = &backend_state;
    backend.vtable = &g_test_text_vtable;

    M3_TEST_OK(m3_navigation_style_init(&style));
    style.text_style.utf8_family = "Sans";

    items[0].utf8_label = "Z";
    items[0].utf8_len = 1;

    M3_TEST_OK(m3_navigation_init(&nav, &backend, &style, items, 1, 0));
    M3_TEST_OK(nav.widget.vtable->destroy(&nav));

    M3_TEST_OK(m3_navigation_init(&nav, &backend, &style, items, 1, 0));
    backend_state.fail_destroy = 1;
    M3_TEST_EXPECT(nav.widget.vtable->destroy(&nav), M3_ERR_IO);
    return M3_OK;
}

int main(void)
{
    M3_TEST_OK(test_style_init());
    M3_TEST_OK(test_init_validation());
    M3_TEST_OK(test_setters_and_getters());
    M3_TEST_OK(test_measure_layout_mode());
    M3_TEST_OK(test_event_selection());
    M3_TEST_OK(test_paint());
    M3_TEST_OK(test_destroy());
    return 0;
}
