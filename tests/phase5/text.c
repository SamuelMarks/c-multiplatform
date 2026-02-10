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

    *out_width = (M3Scalar)(utf8_len * 10u);
    *out_height = 20.0f;
    *out_baseline = 15.0f;
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
    M3Gfx gfx;
    M3TextStyle style;
    M3TextMetrics metrics;
    M3TextWidget widget;
    M3PaintContext paint_ctx;
    M3MeasureSpec width_spec;
    M3MeasureSpec height_spec;
    M3Size size;
    M3Rect bounds;
    M3Semantics semantics;
    M3Handle font;
    M3Bool handled;
    int rc;

    test_text_backend_init(&backend);
    gfx.ctx = &backend;
    gfx.vtable = NULL;
    gfx.text_vtable = &g_test_text_vtable;

    M3_TEST_EXPECT(m3_text_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_text_style_init(&style));
    M3_TEST_ASSERT(style.size_px == 14);
    M3_TEST_ASSERT(style.weight == 400);
    M3_TEST_ASSERT(style.italic == M3_FALSE);

    M3_TEST_EXPECT(m3_text_backend_from_gfx(NULL, &text_backend), M3_ERR_INVALID_ARGUMENT);
    gfx.text_vtable = NULL;
    M3_TEST_EXPECT(m3_text_backend_from_gfx(&gfx, &text_backend), M3_ERR_UNSUPPORTED);
    gfx.text_vtable = &g_test_text_vtable;
    M3_TEST_OK(m3_text_backend_from_gfx(&gfx, &text_backend));

    M3_TEST_EXPECT(m3_text_font_create(NULL, &style, &font), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_font_create(&text_backend, NULL, &font), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_font_create(&text_backend, &style, NULL), M3_ERR_INVALID_ARGUMENT);

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

    M3_TEST_EXPECT(m3_text_font_destroy(NULL, font), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_text_font_destroy(&text_backend, font));
    M3_TEST_ASSERT(backend.destroy_calls == 1);

    M3_TEST_EXPECT(m3_text_measure_utf8(NULL, font, "abc", 3, &metrics), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_measure_utf8(&text_backend, font, NULL, 3, &metrics), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_measure_utf8(&text_backend, font, "abc", 3, NULL), M3_ERR_INVALID_ARGUMENT);

    backend.fail_measure = 1;
    M3_TEST_EXPECT(m3_text_measure_utf8(&text_backend, font, "abc", 3, &metrics), M3_ERR_IO);
    backend.fail_measure = 0;

    M3_TEST_OK(m3_text_measure_utf8(&text_backend, font, "abc", 3, &metrics));
    M3_TEST_ASSERT(m3_near(metrics.width, 30.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(metrics.height, 20.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(metrics.baseline, 15.0f, 0.001f));

    M3_TEST_OK(m3_text_font_metrics(&text_backend, font, &metrics));
    M3_TEST_ASSERT(m3_near(metrics.width, 0.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(metrics.height, 20.0f, 0.001f));

    M3_TEST_OK(m3_text_measure_cstr(&text_backend, font, "", &metrics));
    M3_TEST_ASSERT(m3_near(metrics.width, 0.0f, 0.001f));

    M3_TEST_EXPECT(m3_text_widget_init(NULL, &text_backend, &style, "Hi", 2), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_widget_init(&widget, NULL, &style, "Hi", 2), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_widget_init(&widget, &text_backend, NULL, "Hi", 2), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_text_widget_init(&widget, &text_backend, &style, NULL, 2), M3_ERR_INVALID_ARGUMENT);

    text_backend.vtable = &g_test_text_vtable_no_draw;
    M3_TEST_EXPECT(m3_text_widget_init(&widget, &text_backend, &style, "Hi", 2), M3_ERR_UNSUPPORTED);
    text_backend.vtable = &g_test_text_vtable_no_measure;
    M3_TEST_EXPECT(m3_text_widget_init(&widget, &text_backend, &style, "Hi", 2), M3_ERR_UNSUPPORTED);
    text_backend.vtable = &g_test_text_vtable;

    M3_TEST_OK(m3_text_widget_init(&widget, &text_backend, &style, "Hi", 2));

    width_spec.mode = M3_MEASURE_AT_MOST;
    width_spec.size = 10.0f;
    height_spec.mode = M3_MEASURE_UNSPECIFIED;
    height_spec.size = 0.0f;
    M3_TEST_OK(widget.widget.vtable->measure(widget.widget.ctx, width_spec, height_spec, &size));
    M3_TEST_ASSERT(m3_near(size.width, 10.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(size.height, 20.0f, 0.001f));

    M3_TEST_OK(m3_text_widget_set_text(&widget, "Hello", 5));
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

    M3_TEST_OK(widget.widget.vtable->event(widget.widget.ctx, (const M3InputEvent *)&bounds, &handled));
    M3_TEST_ASSERT(handled == M3_FALSE);

    rc = widget.widget.vtable->destroy(widget.widget.ctx);
    M3_TEST_OK(rc);
    M3_TEST_ASSERT(backend.destroy_calls >= 1);

    return 0;
}
