#include "test_utils.h"
#include "m3/m3_text.h"

#include <string.h>

typedef struct TestTextBackend {
    int create_calls;
    int destroy_calls;
    int measure_calls;
    int draw_calls;
    int fail_create;
    int fail_measure;
    int fail_draw;
    int negative_metrics;
    M3Handle last_font;
    m3_i32 last_size;
    m3_i32 last_weight;
    M3Bool last_italic;
    char last_family[64];
    M3Scalar last_x;
    M3Scalar last_y;
    m3_usize last_len;
    M3Color last_color;
} TestTextBackend;

static void test_text_backend_init(TestTextBackend *backend)
{
    memset(backend, 0, sizeof(*backend));
}

static int test_text_create_font(void *text, const char *utf8_family, m3_i32 size_px, m3_i32 weight, M3Bool italic, M3Handle *out_font)
{
    TestTextBackend *backend;

    if (text == NULL || utf8_family == NULL || out_font == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (size_px <= 0) {
        return M3_ERR_RANGE;
    }
    if (weight < 100 || weight > 900) {
        return M3_ERR_RANGE;
    }

    backend = (TestTextBackend *)text;
    backend->create_calls += 1;
    if (backend->fail_create) {
        return M3_ERR_IO;
    }

    backend->last_size = size_px;
    backend->last_weight = weight;
    backend->last_italic = italic;
    strncpy(backend->last_family, utf8_family, sizeof(backend->last_family) - 1);
    backend->last_family[sizeof(backend->last_family) - 1] = '\0';

    out_font->id = 1u;
    out_font->generation = 1u;
    backend->last_font = *out_font;
    return M3_OK;
}

static int test_text_destroy_font(void *text, M3Handle font)
{
    TestTextBackend *backend;

    if (text == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (font.id == 0u && font.generation == 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (TestTextBackend *)text;
    backend->destroy_calls += 1;
    return M3_OK;
}

static int test_text_measure_text(void *text, M3Handle font, const char *utf8, m3_usize utf8_len, M3Scalar *out_width,
                                  M3Scalar *out_height, M3Scalar *out_baseline)
{
    TestTextBackend *backend;

    if (text == NULL || out_width == NULL || out_height == NULL || out_baseline == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8 == NULL && utf8_len != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (font.id == 0u && font.generation == 0u) {
        return M3_ERR_STATE;
    }

    backend = (TestTextBackend *)text;
    backend->measure_calls += 1;
    if (backend->fail_measure) {
        return M3_ERR_IO;
    }

    if (backend->negative_metrics) {
        *out_width = -1.0f;
        *out_height = 20.0f;
        *out_baseline = 15.0f;
    } else {
        *out_width = (M3Scalar)(utf8_len * 10u);
        *out_height = 20.0f;
        *out_baseline = 15.0f;
    }
    return M3_OK;
}

static int test_text_draw_text(void *text, M3Handle font, const char *utf8, m3_usize utf8_len, M3Scalar x, M3Scalar y, M3Color color)
{
    TestTextBackend *backend;

    if (text == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8 == NULL && utf8_len != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (font.id == 0u && font.generation == 0u) {
        return M3_ERR_STATE;
    }

    backend = (TestTextBackend *)text;
    backend->draw_calls += 1;
    if (backend->fail_draw) {
        return M3_ERR_IO;
    }

    backend->last_x = x;
    backend->last_y = y;
    backend->last_len = utf8_len;
    backend->last_color = color;
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

static const M3TextVTable g_test_text_vtable_no_destroy = {
    test_text_create_font,
    NULL,
    test_text_measure_text,
    test_text_draw_text
};

static const M3TextVTable g_test_text_vtable_no_create = {
    NULL,
    test_text_destroy_font,
    test_text_measure_text,
    test_text_draw_text
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

int main(void)
{
    TestTextBackend backend;
    M3TextBackend text_backend;
    M3TextBackend alt_backend;
    M3TextBackend invalid_backend;
    M3Gfx gfx;
    M3TextStyle style;
    M3TextStyle alt_style;
    M3TextMetrics metrics;
    M3TextWidget widget;
    M3TextWidget temp_widget;
    M3PaintContext paint_ctx;
    M3MeasureSpec width_spec;
    M3MeasureSpec height_spec;
    M3Size size;
    M3Rect bounds;
    M3Semantics semantics;
    M3Handle font;
    M3Handle saved_font;
    M3Color color;
    m3_usize cstr_len;
    const M3WidgetVTable *text_vtable;
    M3InputEvent event;
    M3Bool handled;
    int rc;

    test_text_backend_init(&backend);
    memset(&event, 0, sizeof(event));
    gfx.ctx = &backend;
    gfx.vtable = NULL;
    gfx.text_vtable = &g_test_text_vtable;

    M3_TEST_EXPECT(m3_text_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_text_style_init(&style));
    M3_TEST_ASSERT(style.size_px == 14);
    M3_TEST_ASSERT(style.weight == 400);
    M3_TEST_ASSERT(style.italic == M3_FALSE);

    M3_TEST_EXPECT(m3_text_test_validate_color(NULL), M3_ERR_INVALID_ARGUMENT);
    color = style.color;
    color.r = -0.1f;
    M3_TEST_EXPECT(m3_text_test_validate_color(&color), M3_ERR_RANGE);
    color.r = 0.0f;
    color.g = 1.2f;
    M3_TEST_EXPECT(m3_text_test_validate_color(&color), M3_ERR_RANGE);
    color.g = 0.0f;
    color.b = 1.1f;
    M3_TEST_EXPECT(m3_text_test_validate_color(&color), M3_ERR_RANGE);
    color.b = 0.0f;
    color.a = -0.2f;
    M3_TEST_EXPECT(m3_text_test_validate_color(&color), M3_ERR_RANGE);
    color.a = 1.0f;
    M3_TEST_OK(m3_text_test_validate_color(&color));

    M3_TEST_EXPECT(m3_text_test_validate_style(NULL), M3_ERR_INVALID_ARGUMENT);
    alt_style = style;
    alt_style.utf8_family = NULL;
    M3_TEST_EXPECT(m3_text_test_validate_style(&alt_style), M3_ERR_INVALID_ARGUMENT);
    alt_style.utf8_family = "Test";
    alt_style.size_px = 0;
    M3_TEST_EXPECT(m3_text_test_validate_style(&alt_style), M3_ERR_RANGE);
    alt_style.size_px = 16;
    alt_style.weight = 99;
    M3_TEST_EXPECT(m3_text_test_validate_style(&alt_style), M3_ERR_RANGE);
    alt_style.weight = 400;
    alt_style.italic = 2;
    M3_TEST_EXPECT(m3_text_test_validate_style(&alt_style), M3_ERR_RANGE);
    alt_style.italic = M3_FALSE;
    alt_style.color.a = 1.5f;
    M3_TEST_EXPECT(m3_text_test_validate_style(&alt_style), M3_ERR_RANGE);
    alt_style.color.a = 1.0f;
    M3_TEST_OK(m3_text_test_validate_style(&alt_style));

    M3_TEST_EXPECT(m3_text_test_validate_backend(NULL), M3_ERR_INVALID_ARGUMENT);
    alt_backend.ctx = &backend;
    alt_backend.vtable = NULL;
    M3_TEST_EXPECT(m3_text_test_validate_backend(&alt_backend), M3_ERR_INVALID_ARGUMENT);
    alt_backend.vtable = &g_test_text_vtable;
    M3_TEST_OK(m3_text_test_validate_backend(&alt_backend));

    width_spec.mode = 99u;
    width_spec.size = 0.0f;
    M3_TEST_EXPECT(m3_text_test_validate_measure_spec(width_spec), M3_ERR_INVALID_ARGUMENT);
    width_spec.mode = M3_MEASURE_EXACTLY;
    width_spec.size = -1.0f;
    M3_TEST_EXPECT(m3_text_test_validate_measure_spec(width_spec), M3_ERR_RANGE);
    width_spec.mode = M3_MEASURE_UNSPECIFIED;
    width_spec.size = -1.0f;
    M3_TEST_OK(m3_text_test_validate_measure_spec(width_spec));

    M3_TEST_EXPECT(m3_text_test_validate_rect(NULL), M3_ERR_INVALID_ARGUMENT);
    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = -1.0f;
    bounds.height = 5.0f;
    M3_TEST_EXPECT(m3_text_test_validate_rect(&bounds), M3_ERR_RANGE);
    bounds.width = 1.0f;
    bounds.height = -2.0f;
    M3_TEST_EXPECT(m3_text_test_validate_rect(&bounds), M3_ERR_RANGE);
    bounds.height = 2.0f;
    M3_TEST_OK(m3_text_test_validate_rect(&bounds));

    M3_TEST_EXPECT(m3_text_backend_from_gfx(NULL, &text_backend), M3_ERR_INVALID_ARGUMENT);
    gfx.text_vtable = NULL;
    M3_TEST_EXPECT(m3_text_backend_from_gfx(&gfx, &text_backend), M3_ERR_UNSUPPORTED);
    gfx.text_vtable = &g_test_text_vtable;
    M3_TEST_OK(m3_text_backend_from_gfx(&gfx, &text_backend));

    M3_TEST_EXPECT(m3_text_font_create(NULL, &style, &font), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_font_create(&text_backend, NULL, &font), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_font_create(&text_backend, &style, NULL), M3_ERR_INVALID_ARGUMENT);
    memset(&invalid_backend, 0, sizeof(invalid_backend));
    M3_TEST_EXPECT(m3_text_font_create(&invalid_backend, &style, &font), M3_ERR_INVALID_ARGUMENT);

    text_backend.vtable = &g_test_text_vtable_no_create;
    M3_TEST_EXPECT(m3_text_font_create(&text_backend, &style, &font), M3_ERR_UNSUPPORTED);
    text_backend.vtable = &g_test_text_vtable;

    style.utf8_family = NULL;
    M3_TEST_EXPECT(m3_text_font_create(&text_backend, &style, &font), M3_ERR_INVALID_ARGUMENT);
    style.utf8_family = "Test";
    style.size_px = 0;
    M3_TEST_EXPECT(m3_text_font_create(&text_backend, &style, &font), M3_ERR_RANGE);
    style.size_px = 16;
    style.weight = 50;
    M3_TEST_EXPECT(m3_text_font_create(&text_backend, &style, &font), M3_ERR_RANGE);
    style.weight = 400;
    style.color.a = 1.2f;
    M3_TEST_EXPECT(m3_text_font_create(&text_backend, &style, &font), M3_ERR_RANGE);
    style.color.a = 1.0f;
    M3_TEST_OK(m3_text_font_create(&text_backend, &style, &font));
    M3_TEST_ASSERT(backend.create_calls == 1);
    saved_font = font;

    M3_TEST_EXPECT(m3_text_font_destroy(NULL, font), M3_ERR_INVALID_ARGUMENT);
    text_backend.vtable = &g_test_text_vtable_no_destroy;
    M3_TEST_EXPECT(m3_text_font_destroy(&text_backend, saved_font), M3_ERR_UNSUPPORTED);
    text_backend.vtable = &g_test_text_vtable;
    font.id = 0u;
    font.generation = 0u;
    M3_TEST_OK(m3_text_font_destroy(&text_backend, font));
    font = saved_font;

    M3_TEST_EXPECT(m3_text_measure_utf8(NULL, font, "abc", 3, &metrics), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_measure_utf8(&invalid_backend, font, "abc", 3, &metrics), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_measure_utf8(&text_backend, font, NULL, 3, &metrics), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_measure_utf8(&text_backend, font, "abc", 3, NULL), M3_ERR_INVALID_ARGUMENT);

    text_backend.vtable = &g_test_text_vtable_no_measure;
    M3_TEST_EXPECT(m3_text_measure_utf8(&text_backend, font, "abc", 3, &metrics), M3_ERR_UNSUPPORTED);
    text_backend.vtable = &g_test_text_vtable;

    backend.fail_measure = 1;
    M3_TEST_EXPECT(m3_text_measure_utf8(&text_backend, font, "abc", 3, &metrics), M3_ERR_IO);
    backend.fail_measure = 0;

    backend.negative_metrics = 1;
    M3_TEST_EXPECT(m3_text_measure_utf8(&text_backend, font, "abc", 3, &metrics), M3_ERR_RANGE);
    backend.negative_metrics = 0;

    M3_TEST_OK(m3_text_measure_utf8(&text_backend, font, "abc", 3, &metrics));
    M3_TEST_ASSERT(m3_near(metrics.width, 30.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(metrics.height, 20.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(metrics.baseline, 15.0f, 0.001f));

    M3_TEST_EXPECT(m3_text_measure_cstr(NULL, font, "abc", &metrics), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_measure_cstr(&text_backend, font, NULL, &metrics), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_text_test_set_cstr_limit(2));
    M3_TEST_EXPECT(m3_text_measure_cstr(&text_backend, font, "abcd", &metrics), M3_ERR_OVERFLOW);
    M3_TEST_OK(m3_text_test_set_cstr_limit(0));

    M3_TEST_OK(m3_text_font_metrics(&text_backend, font, &metrics));
    M3_TEST_ASSERT(m3_near(metrics.width, 0.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(metrics.height, 20.0f, 0.001f));

    M3_TEST_OK(m3_text_measure_cstr(&text_backend, font, "", &metrics));
    M3_TEST_ASSERT(m3_near(metrics.width, 0.0f, 0.001f));

    M3_TEST_EXPECT(m3_text_test_cstrlen(NULL, &cstr_len), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_test_cstrlen("", NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_text_test_set_cstr_limit(2));
    M3_TEST_EXPECT(m3_text_test_cstrlen("abc", &cstr_len), M3_ERR_OVERFLOW);
    M3_TEST_OK(m3_text_test_set_cstr_limit(0));
    M3_TEST_OK(m3_text_test_cstrlen("abc", &cstr_len));
    M3_TEST_ASSERT(cstr_len == 3u);

    M3_TEST_OK(m3_text_font_destroy(&text_backend, font));
    M3_TEST_ASSERT(backend.destroy_calls == 1);

    M3_TEST_EXPECT(m3_text_widget_init(NULL, &text_backend, &style, "Hi", 2), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_widget_init(&widget, NULL, &style, "Hi", 2), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_widget_init(&widget, &text_backend, NULL, "Hi", 2), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_widget_init(&widget, &text_backend, &style, NULL, 2), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_widget_init(&widget, &invalid_backend, &style, "Hi", 2), M3_ERR_INVALID_ARGUMENT);

    text_backend.vtable = &g_test_text_vtable_no_create;
    M3_TEST_EXPECT(m3_text_widget_init(&widget, &text_backend, &style, "Hi", 2), M3_ERR_UNSUPPORTED);
    text_backend.vtable = &g_test_text_vtable_no_draw;
    M3_TEST_EXPECT(m3_text_widget_init(&widget, &text_backend, &style, "Hi", 2), M3_ERR_UNSUPPORTED);
    text_backend.vtable = &g_test_text_vtable_no_measure;
    M3_TEST_EXPECT(m3_text_widget_init(&widget, &text_backend, &style, "Hi", 2), M3_ERR_UNSUPPORTED);
    text_backend.vtable = &g_test_text_vtable_no_destroy;
    M3_TEST_EXPECT(m3_text_widget_init(&widget, &text_backend, &style, "Hi", 2), M3_ERR_UNSUPPORTED);
    text_backend.vtable = &g_test_text_vtable;

    alt_style = style;
    alt_style.size_px = 0;
    M3_TEST_EXPECT(m3_text_widget_init(&widget, &text_backend, &alt_style, "Hi", 2), M3_ERR_RANGE);
    backend.fail_create = 1;
    M3_TEST_EXPECT(m3_text_widget_init(&widget, &text_backend, &style, "Hi", 2), M3_ERR_IO);
    backend.fail_create = 0;

    M3_TEST_OK(m3_text_widget_init(&widget, &text_backend, &style, "Hi", 2));
    text_vtable = widget.widget.vtable;
    M3_TEST_ASSERT(text_vtable != NULL);

    temp_widget = widget;
    temp_widget.metrics_valid = M3_TRUE;
    M3_TEST_OK(m3_text_test_metrics_update(&temp_widget));
    M3_TEST_EXPECT(m3_text_test_metrics_update(NULL), M3_ERR_INVALID_ARGUMENT);

    temp_widget = widget;
    temp_widget.metrics_valid = M3_FALSE;
    backend.fail_measure = 1;
    M3_TEST_EXPECT(m3_text_test_metrics_update(&temp_widget), M3_ERR_IO);
    backend.fail_measure = 0;

    width_spec.mode = M3_MEASURE_AT_MOST;
    width_spec.size = 10.0f;
    height_spec.mode = M3_MEASURE_UNSPECIFIED;
    height_spec.size = 0.0f;
    M3_TEST_OK(widget.widget.vtable->measure(widget.widget.ctx, width_spec, height_spec, &size));
    M3_TEST_ASSERT(m3_near(size.width, 10.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(size.height, 20.0f, 0.001f));

    M3_TEST_OK(m3_text_widget_set_text(&widget, "Hello", 5));
    M3_TEST_EXPECT(m3_text_widget_set_text(NULL, "Hi", 2), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_widget_set_text(&widget, NULL, 2), M3_ERR_INVALID_ARGUMENT);
    width_spec.mode = M3_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec.mode = M3_MEASURE_EXACTLY;
    height_spec.size = 12.0f;
    M3_TEST_OK(widget.widget.vtable->measure(widget.widget.ctx, width_spec, height_spec, &size));
    M3_TEST_ASSERT(m3_near(size.width, 50.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(size.height, 12.0f, 0.001f));
    M3_TEST_OK(m3_text_widget_set_text(&widget, NULL, 0));
    width_spec.mode = M3_MEASURE_EXACTLY;
    width_spec.size = 42.0f;
    height_spec.mode = M3_MEASURE_AT_MOST;
    height_spec.size = 10.0f;
    M3_TEST_OK(widget.widget.vtable->measure(widget.widget.ctx, width_spec, height_spec, &size));
    M3_TEST_ASSERT(m3_near(size.width, 42.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(size.height, 10.0f, 0.001f));

    bounds.x = 5.0f;
    bounds.y = 7.0f;
    bounds.width = 100.0f;
    bounds.height = 40.0f;
    M3_TEST_OK(widget.widget.vtable->layout(widget.widget.ctx, bounds));

    paint_ctx.gfx = &gfx;
    paint_ctx.clip = bounds;
    paint_ctx.dpi_scale = 1.0f;
    M3_TEST_OK(widget.widget.vtable->paint(widget.widget.ctx, &paint_ctx));
    M3_TEST_ASSERT(backend.draw_calls == 1);
    M3_TEST_ASSERT(m3_near(backend.last_x, 5.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(backend.last_y, 22.0f, 0.001f));

    M3_TEST_OK(widget.widget.vtable->get_semantics(widget.widget.ctx, &semantics));
    M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_TEXT);
    M3_TEST_ASSERT(semantics.utf8_label == widget.utf8);

    M3_TEST_EXPECT(widget.widget.vtable->measure(NULL, width_spec, height_spec, &size), M3_ERR_INVALID_ARGUMENT);
    width_spec.mode = 42u;
    width_spec.size = 10.0f;
    M3_TEST_EXPECT(widget.widget.vtable->measure(widget.widget.ctx, width_spec, height_spec, &size), M3_ERR_INVALID_ARGUMENT);
    width_spec.mode = M3_MEASURE_EXACTLY;
    width_spec.size = 10.0f;
    height_spec.mode = M3_MEASURE_AT_MOST;
    height_spec.size = -5.0f;
    M3_TEST_EXPECT(widget.widget.vtable->measure(widget.widget.ctx, width_spec, height_spec, &size), M3_ERR_RANGE);
    height_spec.mode = M3_MEASURE_AT_MOST;
    height_spec.size = 10.0f;
    widget.metrics_valid = M3_FALSE;
    backend.fail_measure = 1;
    M3_TEST_EXPECT(widget.widget.vtable->measure(widget.widget.ctx, width_spec, height_spec, &size), M3_ERR_IO);
    backend.fail_measure = 0;

    bounds.width = -1.0f;
    M3_TEST_EXPECT(widget.widget.vtable->layout(widget.widget.ctx, bounds), M3_ERR_RANGE);
    bounds.width = 100.0f;
    M3_TEST_EXPECT(widget.widget.vtable->layout(NULL, bounds), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(widget.widget.vtable->layout(widget.widget.ctx, bounds));

    M3_TEST_EXPECT(widget.widget.vtable->paint(NULL, &paint_ctx), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(widget.widget.vtable->paint(widget.widget.ctx, NULL), M3_ERR_INVALID_ARGUMENT);
    paint_ctx.gfx = NULL;
    M3_TEST_EXPECT(widget.widget.vtable->paint(widget.widget.ctx, &paint_ctx), M3_ERR_INVALID_ARGUMENT);
    paint_ctx.gfx = &gfx;
    gfx.text_vtable = &g_test_text_vtable_no_draw;
    M3_TEST_EXPECT(widget.widget.vtable->paint(widget.widget.ctx, &paint_ctx), M3_ERR_UNSUPPORTED);
    gfx.text_vtable = &g_test_text_vtable;
    backend.fail_measure = 1;
    M3_TEST_EXPECT(widget.widget.vtable->paint(widget.widget.ctx, &paint_ctx), M3_ERR_IO);
    backend.fail_measure = 0;
    backend.fail_draw = 1;
    M3_TEST_EXPECT(widget.widget.vtable->paint(widget.widget.ctx, &paint_ctx), M3_ERR_IO);
    backend.fail_draw = 0;

    M3_TEST_EXPECT(widget.widget.vtable->event(NULL, &event, &handled), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(widget.widget.vtable->event(widget.widget.ctx, NULL, &handled), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(widget.widget.vtable->event(widget.widget.ctx, &event, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(widget.widget.vtable->event(widget.widget.ctx, &event, &handled));
    M3_TEST_ASSERT(handled == M3_FALSE);

    M3_TEST_EXPECT(widget.widget.vtable->get_semantics(NULL, &semantics), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(widget.widget.vtable->get_semantics(widget.widget.ctx, NULL), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(widget.widget.vtable->destroy(NULL), M3_ERR_INVALID_ARGUMENT);
    temp_widget = widget;
    temp_widget.backend.vtable = &g_test_text_vtable_no_destroy;
    temp_widget.owns_font = M3_TRUE;
    temp_widget.font.id = 1u;
    temp_widget.font.generation = 1u;
    rc = text_vtable->destroy(&temp_widget);
    M3_TEST_EXPECT(rc, M3_ERR_UNSUPPORTED);
    M3_TEST_ASSERT(temp_widget.font.id == 0u);

    temp_widget = widget;
    temp_widget.owns_font = M3_FALSE;
    temp_widget.font.id = 1u;
    temp_widget.font.generation = 1u;
    M3_TEST_OK(text_vtable->destroy(&temp_widget));

    alt_style = style;
    alt_style.weight = 50;
    M3_TEST_EXPECT(m3_text_widget_set_style(&widget, &alt_style), M3_ERR_RANGE);
    M3_TEST_EXPECT(m3_text_widget_set_style(NULL, &style), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_widget_set_style(&widget, NULL), M3_ERR_INVALID_ARGUMENT);
    backend.fail_create = 1;
    M3_TEST_EXPECT(m3_text_widget_set_style(&widget, &style), M3_ERR_IO);
    backend.fail_create = 0;
    M3_TEST_OK(m3_text_widget_set_style(&widget, &style));

    M3_TEST_EXPECT(m3_text_widget_get_metrics(NULL, &metrics), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_widget_get_metrics(&widget, NULL), M3_ERR_INVALID_ARGUMENT);
    backend.fail_measure = 1;
    M3_TEST_EXPECT(m3_text_widget_get_metrics(&widget, &metrics), M3_ERR_IO);
    backend.fail_measure = 0;
    M3_TEST_OK(m3_text_widget_get_metrics(&widget, &metrics));

    rc = widget.widget.vtable->destroy(widget.widget.ctx);
    M3_TEST_OK(rc);
    M3_TEST_ASSERT(backend.destroy_calls >= 1);

    return 0;
}
