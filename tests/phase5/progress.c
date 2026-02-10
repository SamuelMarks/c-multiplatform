#include "test_utils.h"
#include "m3/m3_progress.h"

#include <string.h>

typedef struct TestProgressBackend {
    int draw_rect_calls;
    int draw_line_calls;
    int fail_draw_rect;
    int fail_draw_line;
    M3Rect last_rect;
    M3Color last_rect_color;
    M3Scalar last_corner;
    M3Scalar last_line_x0;
    M3Scalar last_line_y0;
    M3Scalar last_line_x1;
    M3Scalar last_line_y1;
    M3Color last_line_color;
    M3Scalar last_line_thickness;
} TestProgressBackend;

typedef struct SliderCounter {
    int calls;
    M3Scalar last_value;
    int fail;
} SliderCounter;

static int test_backend_init(TestProgressBackend *backend)
{
    if (backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    memset(backend, 0, sizeof(*backend));
    backend->fail_draw_rect = M3_OK;
    backend->fail_draw_line = M3_OK;
    return M3_OK;
}

static int test_gfx_draw_rect(void *gfx, const M3Rect *rect, M3Color color, M3Scalar corner_radius)
{
    TestProgressBackend *backend;

    if (gfx == NULL || rect == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (TestProgressBackend *)gfx;
    backend->draw_rect_calls += 1;
    backend->last_rect = *rect;
    backend->last_rect_color = color;
    backend->last_corner = corner_radius;
    if (backend->fail_draw_rect != M3_OK) {
        return backend->fail_draw_rect;
    }
    return M3_OK;
}

static int test_gfx_draw_line(void *gfx, M3Scalar x0, M3Scalar y0, M3Scalar x1, M3Scalar y1, M3Color color,
    M3Scalar thickness)
{
    TestProgressBackend *backend;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (TestProgressBackend *)gfx;
    backend->draw_line_calls += 1;
    backend->last_line_x0 = x0;
    backend->last_line_y0 = y0;
    backend->last_line_x1 = x1;
    backend->last_line_y1 = y1;
    backend->last_line_color = color;
    backend->last_line_thickness = thickness;
    if (backend->fail_draw_line != M3_OK) {
        return backend->fail_draw_line;
    }
    return M3_OK;
}

static const M3GfxVTable g_test_vtable = {
    NULL,
    NULL,
    NULL,
    test_gfx_draw_rect,
    test_gfx_draw_line,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static const M3GfxVTable g_test_vtable_no_rect = {
    NULL,
    NULL,
    NULL,
    NULL,
    test_gfx_draw_line,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static const M3GfxVTable g_test_vtable_no_line = {
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

static int init_pointer_event(M3InputEvent *event, m3_u32 type, m3_i32 x, m3_i32 y)
{
    if (event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    memset(event, 0, sizeof(*event));
    event->type = type;
    event->data.pointer.x = x;
    event->data.pointer.y = y;
    return M3_OK;
}

static int test_slider_on_change(void *ctx, struct M3Slider *slider, M3Scalar value)
{
    SliderCounter *counter;

    if (ctx == NULL || slider == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    counter = (SliderCounter *)ctx;
    counter->calls += 1;
    counter->last_value = value;
    if (counter->fail) {
        return M3_ERR_IO;
    }
    return M3_OK;
}

int main(void)
{
    TestProgressBackend backend;
    M3Gfx gfx;
    M3PaintContext paint_ctx;
    M3LinearProgressStyle linear_style;
    M3LinearProgressStyle bad_linear_style;
    M3LinearProgress linear;
    M3CircularProgressStyle circular_style;
    M3CircularProgressStyle bad_circular_style;
    M3CircularProgress circular;
    M3SliderStyle slider_style;
    M3SliderStyle bad_slider_style;
    M3Slider slider;
    M3MeasureSpec width_spec;
    M3MeasureSpec height_spec;
    M3Size size;
    M3Rect bounds;
    M3Semantics semantics;
    M3Bool handled;
    M3Scalar value;
    SliderCounter counter;
    M3InputEvent event;

    M3_TEST_OK(test_backend_init(&backend));
    gfx.ctx = &backend;
    gfx.vtable = &g_test_vtable;
    gfx.text_vtable = NULL;

    paint_ctx.gfx = &gfx;
    paint_ctx.dpi_scale = 1.0f;
    paint_ctx.clip.x = 0.0f;
    paint_ctx.clip.y = 0.0f;
    paint_ctx.clip.width = 200.0f;
    paint_ctx.clip.height = 200.0f;

    M3_TEST_EXPECT(m3_linear_progress_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_linear_progress_style_init(&linear_style));
    M3_TEST_ASSERT(m3_near(linear_style.height, M3_LINEAR_PROGRESS_DEFAULT_HEIGHT, 0.001f));

    M3_TEST_EXPECT(m3_linear_progress_init(NULL, &linear_style, 0.5f), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_linear_progress_init(&linear, NULL, 0.5f), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_linear_progress_init(&linear, &linear_style, -0.1f), M3_ERR_RANGE);

    bad_linear_style = linear_style;
    bad_linear_style.height = 0.0f;
    M3_TEST_EXPECT(m3_linear_progress_init(&linear, &bad_linear_style, 0.5f), M3_ERR_RANGE);

    M3_TEST_OK(m3_linear_progress_init(&linear, &linear_style, 0.25f));
    M3_TEST_ASSERT(linear.widget.ctx == &linear);
    M3_TEST_ASSERT(linear.widget.vtable != NULL);

    M3_TEST_EXPECT(m3_linear_progress_set_value(NULL, 0.5f), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_linear_progress_set_value(&linear, 1.5f), M3_ERR_RANGE);
    M3_TEST_OK(m3_linear_progress_set_value(&linear, 0.75f));
    M3_TEST_OK(m3_linear_progress_get_value(&linear, &value));
    M3_TEST_ASSERT(m3_near(value, 0.75f, 0.001f));
    M3_TEST_EXPECT(m3_linear_progress_get_value(NULL, &value), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_linear_progress_get_value(&linear, NULL), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(m3_linear_progress_set_label(NULL, "A", 1), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_linear_progress_set_label(&linear, NULL, 1), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_linear_progress_set_label(&linear, "Loading", 7));

    bad_linear_style = linear_style;
    bad_linear_style.track_color.r = -1.0f;
    M3_TEST_EXPECT(m3_linear_progress_set_style(&linear, &bad_linear_style), M3_ERR_RANGE);
    M3_TEST_OK(m3_linear_progress_set_style(&linear, &linear_style));

    width_spec.mode = 99u;
    width_spec.size = 0.0f;
    height_spec.mode = M3_MEASURE_UNSPECIFIED;
    height_spec.size = 0.0f;
    M3_TEST_EXPECT(linear.widget.vtable->measure(linear.widget.ctx, width_spec, height_spec, &size),
        M3_ERR_INVALID_ARGUMENT);

    width_spec.mode = M3_MEASURE_AT_MOST;
    width_spec.size = -1.0f;
    M3_TEST_EXPECT(linear.widget.vtable->measure(linear.widget.ctx, width_spec, height_spec, &size), M3_ERR_RANGE);

    width_spec.mode = M3_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec.mode = M3_MEASURE_UNSPECIFIED;
    height_spec.size = 0.0f;
    M3_TEST_OK(linear.widget.vtable->measure(linear.widget.ctx, width_spec, height_spec, &size));
    M3_TEST_ASSERT(m3_near(size.width, linear_style.min_width, 0.001f));
    M3_TEST_ASSERT(m3_near(size.height, linear_style.height, 0.001f));

    width_spec.mode = M3_MEASURE_AT_MOST;
    width_spec.size = 10.0f;
    height_spec.mode = M3_MEASURE_EXACTLY;
    height_spec.size = 12.0f;
    M3_TEST_OK(linear.widget.vtable->measure(linear.widget.ctx, width_spec, height_spec, &size));
    M3_TEST_ASSERT(m3_near(size.width, 10.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(size.height, 12.0f, 0.001f));

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = -1.0f;
    bounds.height = 10.0f;
    M3_TEST_EXPECT(linear.widget.vtable->layout(linear.widget.ctx, bounds), M3_ERR_RANGE);

    bounds.width = 120.0f;
    bounds.height = 8.0f;
    M3_TEST_OK(linear.widget.vtable->layout(linear.widget.ctx, bounds));

    M3_TEST_OK(test_backend_init(&backend));
    M3_TEST_EXPECT(linear.widget.vtable->paint(NULL, &paint_ctx), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, NULL), M3_ERR_INVALID_ARGUMENT);

    gfx.vtable = NULL;
    M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx), M3_ERR_INVALID_ARGUMENT);
    gfx.vtable = &g_test_vtable_no_rect;
    M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx), M3_ERR_UNSUPPORTED);
    gfx.vtable = &g_test_vtable;

    linear.value = 2.0f;
    M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx), M3_ERR_RANGE);
    linear.value = 0.5f;

    linear.bounds.width = -1.0f;
    M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx), M3_ERR_RANGE);
    linear.bounds = bounds;

    bad_linear_style = linear_style;
    bad_linear_style.height = 0.0f;
    linear.style = bad_linear_style;
    M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx), M3_ERR_RANGE);
    linear.style = linear_style;

    M3_TEST_OK(test_backend_init(&backend));
    backend.fail_draw_rect = M3_ERR_IO;
    M3_TEST_EXPECT(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx), M3_ERR_IO);

    M3_TEST_OK(test_backend_init(&backend));
    linear.value = 0.0f;
    M3_TEST_OK(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx));
    M3_TEST_ASSERT(backend.draw_rect_calls == 1);

    M3_TEST_OK(test_backend_init(&backend));
    linear.value = 0.5f;
    M3_TEST_OK(linear.widget.vtable->paint(linear.widget.ctx, &paint_ctx));
    M3_TEST_ASSERT(backend.draw_rect_calls == 2);

    linear.widget.flags = M3_WIDGET_FLAG_DISABLED;
    M3_TEST_OK(linear.widget.vtable->get_semantics(linear.widget.ctx, &semantics));
    M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_NONE);
    M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_DISABLED) != 0u);
    linear.widget.flags = 0u;

    M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
    M3_TEST_OK(linear.widget.vtable->event(linear.widget.ctx, &event, &handled));
    M3_TEST_ASSERT(handled == M3_FALSE);

    M3_TEST_OK(linear.widget.vtable->destroy(linear.widget.ctx));
    M3_TEST_ASSERT(linear.widget.vtable == NULL);

    M3_TEST_EXPECT(m3_circular_progress_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_circular_progress_style_init(&circular_style));
    M3_TEST_ASSERT(circular_style.segments == M3_CIRCULAR_PROGRESS_DEFAULT_SEGMENTS);

    M3_TEST_EXPECT(m3_circular_progress_init(NULL, &circular_style, 0.5f), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_circular_progress_init(&circular, NULL, 0.5f), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_circular_progress_init(&circular, &circular_style, 1.5f), M3_ERR_RANGE);

    bad_circular_style = circular_style;
    bad_circular_style.segments = 2u;
    M3_TEST_EXPECT(m3_circular_progress_init(&circular, &bad_circular_style, 0.5f), M3_ERR_RANGE);

    M3_TEST_OK(m3_circular_progress_init(&circular, &circular_style, 0.25f));
    M3_TEST_ASSERT(circular.widget.ctx == &circular);
    M3_TEST_ASSERT(circular.widget.vtable != NULL);

    M3_TEST_EXPECT(m3_circular_progress_set_value(NULL, 0.5f), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_circular_progress_set_value(&circular, -0.5f), M3_ERR_RANGE);
    M3_TEST_OK(m3_circular_progress_set_value(&circular, 0.75f));
    M3_TEST_OK(m3_circular_progress_get_value(&circular, &value));
    M3_TEST_ASSERT(m3_near(value, 0.75f, 0.001f));

    M3_TEST_EXPECT(m3_circular_progress_get_value(NULL, &value), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_circular_progress_get_value(&circular, NULL), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(m3_circular_progress_set_label(NULL, "B", 1), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_circular_progress_set_label(&circular, NULL, 1), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_circular_progress_set_label(&circular, "Sync", 4));

    bad_circular_style = circular_style;
    bad_circular_style.track_color.a = 2.0f;
    M3_TEST_EXPECT(m3_circular_progress_set_style(&circular, &bad_circular_style), M3_ERR_RANGE);
    M3_TEST_OK(m3_circular_progress_set_style(&circular, &circular_style));

    width_spec.mode = 99u;
    width_spec.size = 0.0f;
    height_spec.mode = M3_MEASURE_UNSPECIFIED;
    height_spec.size = 0.0f;
    M3_TEST_EXPECT(circular.widget.vtable->measure(circular.widget.ctx, width_spec, height_spec, &size),
        M3_ERR_INVALID_ARGUMENT);

    width_spec.mode = M3_MEASURE_AT_MOST;
    width_spec.size = -1.0f;
    M3_TEST_EXPECT(circular.widget.vtable->measure(circular.widget.ctx, width_spec, height_spec, &size), M3_ERR_RANGE);

    width_spec.mode = M3_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec.mode = M3_MEASURE_UNSPECIFIED;
    height_spec.size = 0.0f;
    M3_TEST_OK(circular.widget.vtable->measure(circular.widget.ctx, width_spec, height_spec, &size));
    M3_TEST_ASSERT(m3_near(size.width, circular_style.diameter, 0.001f));
    M3_TEST_ASSERT(m3_near(size.height, circular_style.diameter, 0.001f));

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = -1.0f;
    bounds.height = 10.0f;
    M3_TEST_EXPECT(circular.widget.vtable->layout(circular.widget.ctx, bounds), M3_ERR_RANGE);

    bounds.width = 60.0f;
    bounds.height = 60.0f;
    M3_TEST_OK(circular.widget.vtable->layout(circular.widget.ctx, bounds));

    M3_TEST_OK(test_backend_init(&backend));
    M3_TEST_EXPECT(circular.widget.vtable->paint(NULL, &paint_ctx), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(circular.widget.vtable->paint(circular.widget.ctx, NULL), M3_ERR_INVALID_ARGUMENT);

    gfx.vtable = &g_test_vtable_no_line;
    M3_TEST_EXPECT(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx), M3_ERR_UNSUPPORTED);
    gfx.vtable = &g_test_vtable;

    circular.value = 2.0f;
    M3_TEST_EXPECT(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx), M3_ERR_RANGE);
    circular.value = 0.25f;

    circular.bounds.width = -1.0f;
    M3_TEST_EXPECT(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx), M3_ERR_RANGE);
    circular.bounds = bounds;

    bad_circular_style = circular_style;
    bad_circular_style.thickness = bad_circular_style.diameter;
    circular.style = bad_circular_style;
    M3_TEST_EXPECT(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx), M3_ERR_RANGE);
    circular.style = circular_style;

    M3_TEST_OK(test_backend_init(&backend));
    backend.fail_draw_line = M3_ERR_IO;
    M3_TEST_EXPECT(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx), M3_ERR_IO);

    M3_TEST_OK(test_backend_init(&backend));
    circular.value = 0.0f;
    M3_TEST_OK(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx));
    M3_TEST_ASSERT(backend.draw_line_calls == (int)circular.style.segments);

    M3_TEST_OK(test_backend_init(&backend));
    circular.value = 0.5f;
    M3_TEST_OK(circular.widget.vtable->paint(circular.widget.ctx, &paint_ctx));
    M3_TEST_ASSERT(backend.draw_line_calls > (int)circular.style.segments);

    circular.widget.flags = M3_WIDGET_FLAG_DISABLED;
    M3_TEST_OK(circular.widget.vtable->get_semantics(circular.widget.ctx, &semantics));
    M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_DISABLED) != 0u);
    circular.widget.flags = 0u;

    M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
    M3_TEST_OK(circular.widget.vtable->event(circular.widget.ctx, &event, &handled));
    M3_TEST_ASSERT(handled == M3_FALSE);

    M3_TEST_OK(circular.widget.vtable->destroy(circular.widget.ctx));
    M3_TEST_ASSERT(circular.widget.vtable == NULL);

    M3_TEST_EXPECT(m3_slider_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_slider_style_init(&slider_style));
    M3_TEST_ASSERT(m3_near(slider_style.track_length, M3_SLIDER_DEFAULT_LENGTH, 0.001f));

    M3_TEST_EXPECT(m3_slider_init(NULL, &slider_style, 0.0f, 1.0f, 0.5f), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_slider_init(&slider, NULL, 0.0f, 1.0f, 0.5f), M3_ERR_INVALID_ARGUMENT);

    bad_slider_style = slider_style;
    bad_slider_style.thumb_radius = 0.0f;
    M3_TEST_EXPECT(m3_slider_init(&slider, &bad_slider_style, 0.0f, 1.0f, 0.5f), M3_ERR_RANGE);
    M3_TEST_EXPECT(m3_slider_init(&slider, &slider_style, 1.0f, 0.0f, 0.5f), M3_ERR_RANGE);
    M3_TEST_EXPECT(m3_slider_init(&slider, &slider_style, 0.0f, 1.0f, 2.0f), M3_ERR_RANGE);

    M3_TEST_OK(m3_slider_init(&slider, &slider_style, 0.0f, 1.0f, 0.25f));
    M3_TEST_ASSERT(slider.widget.ctx == &slider);
    M3_TEST_ASSERT(slider.widget.vtable != NULL);
    M3_TEST_ASSERT((slider.widget.flags & M3_WIDGET_FLAG_FOCUSABLE) != 0u);

    M3_TEST_EXPECT(m3_slider_set_value(NULL, 0.5f), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_slider_set_value(&slider, 2.0f), M3_ERR_RANGE);
    M3_TEST_EXPECT(m3_slider_get_value(NULL, &value), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_slider_get_value(&slider, NULL), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_slider_set_step(&slider, 0.25f));
    M3_TEST_OK(m3_slider_set_value(&slider, 0.4f));
    M3_TEST_OK(m3_slider_get_value(&slider, &value));
    M3_TEST_ASSERT(m3_near(value, 0.5f, 0.001f));

    M3_TEST_EXPECT(m3_slider_set_step(&slider, -1.0f), M3_ERR_RANGE);
    M3_TEST_EXPECT(m3_slider_set_step(&slider, 2.0f), M3_ERR_RANGE);

    M3_TEST_EXPECT(m3_slider_set_range(NULL, 0.0f, 1.0f), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_slider_set_range(&slider, 1.0f, 0.0f), M3_ERR_RANGE);
    M3_TEST_EXPECT(m3_slider_set_range(&slider, 0.0f, 0.1f), M3_ERR_RANGE);
    M3_TEST_OK(m3_slider_set_step(&slider, 0.0f));
    M3_TEST_OK(m3_slider_set_range(&slider, 0.0f, 1.0f));

    M3_TEST_EXPECT(m3_slider_set_style(NULL, &slider_style), M3_ERR_INVALID_ARGUMENT);
    bad_slider_style = slider_style;
    bad_slider_style.track_color.a = 2.0f;
    M3_TEST_EXPECT(m3_slider_set_style(&slider, &bad_slider_style), M3_ERR_RANGE);
    M3_TEST_OK(m3_slider_set_style(&slider, &slider_style));

    M3_TEST_EXPECT(m3_slider_set_label(NULL, "S", 1), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_slider_set_label(&slider, NULL, 1), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_slider_set_label(&slider, "Volume", 6));

    M3_TEST_EXPECT(m3_slider_set_on_change(NULL, test_slider_on_change, NULL), M3_ERR_INVALID_ARGUMENT);
    counter.calls = 0;
    counter.last_value = 0.0f;
    counter.fail = 0;
    M3_TEST_OK(m3_slider_set_on_change(&slider, test_slider_on_change, &counter));

    width_spec.mode = 99u;
    width_spec.size = 0.0f;
    height_spec.mode = M3_MEASURE_UNSPECIFIED;
    height_spec.size = 0.0f;
    M3_TEST_EXPECT(slider.widget.vtable->measure(slider.widget.ctx, width_spec, height_spec, &size),
        M3_ERR_INVALID_ARGUMENT);

    width_spec.mode = M3_MEASURE_AT_MOST;
    width_spec.size = -1.0f;
    M3_TEST_EXPECT(slider.widget.vtable->measure(slider.widget.ctx, width_spec, height_spec, &size), M3_ERR_RANGE);

    width_spec.mode = M3_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec.mode = M3_MEASURE_UNSPECIFIED;
    height_spec.size = 0.0f;
    M3_TEST_OK(slider.widget.vtable->measure(slider.widget.ctx, width_spec, height_spec, &size));
    M3_TEST_ASSERT(m3_near(size.width, slider_style.track_length, 0.001f));
    M3_TEST_ASSERT(m3_near(size.height, slider_style.thumb_radius * 2.0f, 0.001f));

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = -1.0f;
    bounds.height = 20.0f;
    M3_TEST_EXPECT(slider.widget.vtable->layout(slider.widget.ctx, bounds), M3_ERR_RANGE);

    bounds.width = 100.0f;
    bounds.height = 20.0f;
    M3_TEST_OK(slider.widget.vtable->layout(slider.widget.ctx, bounds));

    M3_TEST_OK(test_backend_init(&backend));
    M3_TEST_EXPECT(slider.widget.vtable->paint(NULL, &paint_ctx), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, NULL), M3_ERR_INVALID_ARGUMENT);

    gfx.vtable = &g_test_vtable_no_rect;
    M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx), M3_ERR_UNSUPPORTED);
    gfx.vtable = &g_test_vtable;

    slider.bounds.width = 0.0f;
    M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx), M3_ERR_RANGE);
    slider.bounds = bounds;

    slider.max_value = slider.min_value;
    M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx), M3_ERR_RANGE);
    slider.max_value = 1.0f;

    M3_TEST_OK(test_backend_init(&backend));
    backend.fail_draw_rect = M3_ERR_IO;
    M3_TEST_EXPECT(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx), M3_ERR_IO);

    M3_TEST_OK(test_backend_init(&backend));
    slider.value = 0.5f;
    M3_TEST_OK(slider.widget.vtable->paint(slider.widget.ctx, &paint_ctx));
    M3_TEST_ASSERT(backend.draw_rect_calls == 3);

    slider.widget.flags |= M3_WIDGET_FLAG_DISABLED;
    M3_TEST_OK(slider.widget.vtable->get_semantics(slider.widget.ctx, &semantics));
    M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_SLIDER);
    M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_DISABLED) != 0u);
    slider.widget.flags = M3_WIDGET_FLAG_FOCUSABLE;

    counter.calls = 0;
    counter.fail = 0;
    M3_TEST_OK(m3_slider_set_on_change(&slider, test_slider_on_change, &counter));

    M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 50, 10));
    M3_TEST_OK(slider.widget.vtable->event(slider.widget.ctx, &event, &handled));
    M3_TEST_ASSERT(handled == M3_TRUE);
    M3_TEST_ASSERT(counter.calls == 1);
    M3_TEST_ASSERT(m3_near(counter.last_value, 0.5f, 0.01f));

    M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_MOVE, 80, 10));
    M3_TEST_OK(slider.widget.vtable->event(slider.widget.ctx, &event, &handled));
    M3_TEST_ASSERT(handled == M3_TRUE);
    M3_TEST_ASSERT(counter.calls == 2);
    M3_TEST_ASSERT(m3_near(counter.last_value, 0.8f, 0.02f));

    M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 80, 10));
    M3_TEST_OK(slider.widget.vtable->event(slider.widget.ctx, &event, &handled));
    M3_TEST_ASSERT(handled == M3_TRUE);

    slider.pressed = M3_TRUE;
    M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
    M3_TEST_EXPECT(slider.widget.vtable->event(slider.widget.ctx, &event, &handled), M3_ERR_STATE);
    slider.pressed = M3_FALSE;

    counter.fail = 1;
    slider.value = 0.2f;
    M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 90, 10));
    M3_TEST_EXPECT(slider.widget.vtable->event(slider.widget.ctx, &event, &handled), M3_ERR_IO);
    M3_TEST_ASSERT(m3_near(slider.value, 0.2f, 0.001f));
    M3_TEST_ASSERT(slider.pressed == M3_FALSE);

    M3_TEST_OK(slider.widget.vtable->destroy(slider.widget.ctx));
    M3_TEST_ASSERT(slider.widget.vtable == NULL);

    return 0;
}
