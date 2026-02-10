#include "test_utils.h"
#include "m3/m3_visuals.h"

#include <string.h>

#define M3_VISUALS_TEST_FAIL_NONE 0u
#define M3_VISUALS_TEST_FAIL_RIPPLE_INIT_RADIUS 1u
#define M3_VISUALS_TEST_FAIL_RIPPLE_INIT_OPACITY 2u
#define M3_VISUALS_TEST_FAIL_RIPPLE_START_RADIUS_INIT 3u
#define M3_VISUALS_TEST_FAIL_RIPPLE_START_OPACITY_INIT 4u
#define M3_VISUALS_TEST_FAIL_RIPPLE_START_TIMING 5u
#define M3_VISUALS_TEST_FAIL_RIPPLE_RELEASE_TIMING 6u
#define M3_VISUALS_TEST_FAIL_RIPPLE_STEP_RADIUS_RUNNING 7u
#define M3_VISUALS_TEST_FAIL_RIPPLE_STEP_RADIUS_STEP 8u
#define M3_VISUALS_TEST_FAIL_RIPPLE_STEP_RADIUS_VALUE 9u
#define M3_VISUALS_TEST_FAIL_RIPPLE_STEP_OPACITY_RUNNING 10u
#define M3_VISUALS_TEST_FAIL_RIPPLE_STEP_OPACITY_STEP 11u
#define M3_VISUALS_TEST_FAIL_RIPPLE_STEP_OPACITY_VALUE 12u
#define M3_VISUALS_TEST_FAIL_RIPPLE_STEP_FADE_RUNNING 13u
#define M3_VISUALS_TEST_FAIL_RIPPLE_STEP_STOP_RADIUS 14u
#define M3_VISUALS_TEST_FAIL_RIPPLE_STEP_STOP_OPACITY 15u
#define M3_VISUALS_TEST_FAIL_RIPPLE_PAINT_RECT 16u
#define M3_VISUALS_TEST_FAIL_RIPPLE_PAINT_COLOR 17u
#define M3_VISUALS_TEST_FAIL_SHADOW_NORM 18u

typedef struct TestGfxBackend {
    int draw_rect_calls;
    int push_clip_calls;
    int pop_clip_calls;
    int fail_draw_rect;
    int fail_push_clip;
    int fail_pop_clip;
    M3Rect last_rect;
    M3Rect last_clip;
    M3Scalar last_corner;
    M3Color last_color;
} TestGfxBackend;

static void test_gfx_backend_init(TestGfxBackend *backend)
{
    memset(backend, 0, sizeof(*backend));
    backend->fail_draw_rect = M3_OK;
    backend->fail_push_clip = M3_OK;
    backend->fail_pop_clip = M3_OK;
}

static int test_gfx_draw_rect(void *gfx, const M3Rect *rect, M3Color color, M3Scalar corner_radius)
{
    TestGfxBackend *backend;

    if (gfx == NULL || rect == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (TestGfxBackend *)gfx;
    backend->draw_rect_calls += 1;
    backend->last_rect = *rect;
    backend->last_color = color;
    backend->last_corner = corner_radius;
    if (backend->fail_draw_rect != M3_OK) {
        return backend->fail_draw_rect;
    }
    return M3_OK;
}

static int test_gfx_push_clip(void *gfx, const M3Rect *rect)
{
    TestGfxBackend *backend;

    if (gfx == NULL || rect == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (TestGfxBackend *)gfx;
    backend->push_clip_calls += 1;
    backend->last_clip = *rect;
    if (backend->fail_push_clip != M3_OK) {
        return backend->fail_push_clip;
    }
    return M3_OK;
}

static int test_gfx_pop_clip(void *gfx)
{
    TestGfxBackend *backend;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (TestGfxBackend *)gfx;
    backend->pop_clip_calls += 1;
    if (backend->fail_pop_clip != M3_OK) {
        return backend->fail_pop_clip;
    }
    return M3_OK;
}

static const M3GfxVTable g_test_vtable = {
    NULL,
    NULL,
    NULL,
    test_gfx_draw_rect,
    NULL,
    test_gfx_push_clip,
    test_gfx_pop_clip,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static const M3GfxVTable g_test_vtable_no_draw = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    test_gfx_push_clip,
    test_gfx_pop_clip,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static const M3GfxVTable g_test_vtable_no_clip = {
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

int main(void)
{
    M3Ripple ripple;
    M3Shadow shadow;
    M3Color color;
    M3Bool active;
    M3Bool finished;
    M3Scalar radius;
    M3Rect bounds;
    M3Rect clip;
    M3Rect rect;
    TestGfxBackend backend;
    M3Gfx gfx;

    color.r = 0.0f;
    color.g = 0.0f;
    color.b = 0.0f;
    color.a = 0.0f;

    M3_TEST_EXPECT(m3_visuals_test_validate_rect(NULL), M3_ERR_INVALID_ARGUMENT);
    rect.x = 0.0f;
    rect.y = 0.0f;
    rect.width = -1.0f;
    rect.height = 1.0f;
    M3_TEST_EXPECT(m3_visuals_test_validate_rect(&rect), M3_ERR_RANGE);
    rect.width = 1.0f;
    rect.height = -1.0f;
    M3_TEST_EXPECT(m3_visuals_test_validate_rect(&rect), M3_ERR_RANGE);
    rect.height = 1.0f;
    M3_TEST_OK(m3_visuals_test_validate_rect(&rect));

    M3_TEST_EXPECT(m3_visuals_test_validate_color(NULL), M3_ERR_INVALID_ARGUMENT);
    color.r = -0.1f;
    M3_TEST_EXPECT(m3_visuals_test_validate_color(&color), M3_ERR_RANGE);
    color.r = 0.0f;
    color.g = 1.2f;
    M3_TEST_EXPECT(m3_visuals_test_validate_color(&color), M3_ERR_RANGE);
    color.g = 0.0f;
    color.b = 1.2f;
    M3_TEST_EXPECT(m3_visuals_test_validate_color(&color), M3_ERR_RANGE);
    color.b = 0.0f;
    color.a = 1.2f;
    M3_TEST_EXPECT(m3_visuals_test_validate_color(&color), M3_ERR_RANGE);
    color.a = 0.0f;
    M3_TEST_OK(m3_visuals_test_validate_color(&color));

    M3_TEST_EXPECT(m3_visuals_test_validate_gfx(NULL), M3_ERR_INVALID_ARGUMENT);
    test_gfx_backend_init(&backend);
    gfx.ctx = &backend;
    gfx.vtable = NULL;
    gfx.text_vtable = NULL;
    M3_TEST_EXPECT(m3_visuals_test_validate_gfx(&gfx), M3_ERR_INVALID_ARGUMENT);
    gfx.vtable = &g_test_vtable_no_draw;
    M3_TEST_EXPECT(m3_visuals_test_validate_gfx(&gfx), M3_ERR_UNSUPPORTED);
    gfx.vtable = &g_test_vtable;
    M3_TEST_OK(m3_visuals_test_validate_gfx(&gfx));

    M3_TEST_EXPECT(m3_ripple_init(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_shadow_init(NULL), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_ripple_init(&ripple));
    M3_TEST_OK(m3_shadow_init(&shadow));
    M3_TEST_OK(m3_visuals_test_set_fail_point(M3_VISUALS_TEST_FAIL_RIPPLE_INIT_RADIUS));
    M3_TEST_EXPECT(m3_ripple_init(&ripple), M3_ERR_IO);
    M3_TEST_OK(m3_visuals_test_clear_fail_points());
    M3_TEST_OK(m3_visuals_test_set_fail_point(M3_VISUALS_TEST_FAIL_RIPPLE_INIT_OPACITY));
    M3_TEST_EXPECT(m3_ripple_init(&ripple), M3_ERR_IO);
    M3_TEST_OK(m3_visuals_test_clear_fail_points());
    M3_TEST_OK(m3_ripple_init(&ripple));

    M3_TEST_EXPECT(m3_ripple_is_active(NULL, &active), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_ripple_is_active(&ripple, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_ripple_is_active(&ripple, &active));
    M3_TEST_ASSERT(active == M3_FALSE);

    M3_TEST_EXPECT(m3_ripple_start(NULL, 0.0f, 0.0f, 1.0f, 0.1f, color), M3_ERR_INVALID_ARGUMENT);
    color.r = 0.2f;
    color.g = 0.3f;
    color.b = 0.4f;
    color.a = 0.5f;
    M3_TEST_EXPECT(m3_ripple_start(&ripple, 0.0f, 0.0f, 0.0f, 0.1f, color), M3_ERR_RANGE);
    M3_TEST_EXPECT(m3_ripple_start(&ripple, 0.0f, 0.0f, 1.0f, -0.1f, color), M3_ERR_RANGE);
    color.r = -0.1f;
    M3_TEST_EXPECT(m3_ripple_start(&ripple, 0.0f, 0.0f, 1.0f, 0.1f, color), M3_ERR_RANGE);
    color.r = 0.2f;
    color.g = -0.1f;
    M3_TEST_EXPECT(m3_ripple_start(&ripple, 0.0f, 0.0f, 1.0f, 0.1f, color), M3_ERR_RANGE);
    color.g = 0.3f;
    color.b = -0.1f;
    M3_TEST_EXPECT(m3_ripple_start(&ripple, 0.0f, 0.0f, 1.0f, 0.1f, color), M3_ERR_RANGE);
    color.b = 0.4f;
    color.a = 1.2f;
    M3_TEST_EXPECT(m3_ripple_start(&ripple, 0.0f, 0.0f, 1.0f, 0.1f, color), M3_ERR_RANGE);
    color.a = 0.5f;

    M3_TEST_OK(m3_visuals_test_set_fail_point(M3_VISUALS_TEST_FAIL_RIPPLE_START_RADIUS_INIT));
    M3_TEST_EXPECT(m3_ripple_start(&ripple, 5.0f, 6.0f, 10.0f, 0.0f, color), M3_ERR_IO);
    M3_TEST_OK(m3_visuals_test_clear_fail_points());
    M3_TEST_OK(m3_visuals_test_set_fail_point(M3_VISUALS_TEST_FAIL_RIPPLE_START_OPACITY_INIT));
    M3_TEST_EXPECT(m3_ripple_start(&ripple, 5.0f, 6.0f, 10.0f, 0.0f, color), M3_ERR_IO);
    M3_TEST_OK(m3_visuals_test_clear_fail_points());
    M3_TEST_OK(m3_visuals_test_set_fail_point(M3_VISUALS_TEST_FAIL_RIPPLE_START_TIMING));
    M3_TEST_EXPECT(m3_ripple_start(&ripple, 5.0f, 6.0f, 10.0f, 0.0f, color), M3_ERR_IO);
    M3_TEST_OK(m3_visuals_test_clear_fail_points());

    M3_TEST_OK(m3_ripple_start(&ripple, 5.0f, 6.0f, 10.0f, 0.0f, color));
    M3_TEST_OK(m3_ripple_is_active(&ripple, &active));
    M3_TEST_ASSERT(active == M3_TRUE);

    M3_TEST_EXPECT(m3_ripple_release(NULL, 0.1f), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_ripple_release(&ripple, -0.1f), M3_ERR_RANGE);
    M3_TEST_OK(m3_visuals_test_set_fail_point(M3_VISUALS_TEST_FAIL_RIPPLE_RELEASE_TIMING));
    M3_TEST_EXPECT(m3_ripple_release(&ripple, 0.1f), M3_ERR_IO);
    M3_TEST_OK(m3_visuals_test_clear_fail_points());

    M3_TEST_EXPECT(m3_ripple_step(NULL, 0.1f, &finished), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_ripple_step(&ripple, 0.1f, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_ripple_step(&ripple, -0.1f, &finished), M3_ERR_RANGE);

    ripple.radius_anim.running = M3_TRUE;
    ripple.opacity_anim.running = M3_TRUE;
    ripple.radius_anim.mode = M3_ANIM_MODE_TIMING;
    ripple.opacity_anim.mode = M3_ANIM_MODE_TIMING;
    M3_TEST_OK(m3_visuals_test_set_fail_point(M3_VISUALS_TEST_FAIL_RIPPLE_STEP_RADIUS_RUNNING));
    M3_TEST_EXPECT(m3_ripple_step(&ripple, 0.0f, &finished), M3_ERR_IO);
    M3_TEST_OK(m3_visuals_test_clear_fail_points());
    M3_TEST_OK(m3_visuals_test_set_fail_point(M3_VISUALS_TEST_FAIL_RIPPLE_STEP_RADIUS_STEP));
    M3_TEST_EXPECT(m3_ripple_step(&ripple, 0.0f, &finished), M3_ERR_IO);
    M3_TEST_OK(m3_visuals_test_clear_fail_points());
    ripple.radius_anim.running = M3_FALSE;
    M3_TEST_OK(m3_visuals_test_set_fail_point(M3_VISUALS_TEST_FAIL_RIPPLE_STEP_RADIUS_VALUE));
    M3_TEST_EXPECT(m3_ripple_step(&ripple, 0.0f, &finished), M3_ERR_IO);
    M3_TEST_OK(m3_visuals_test_clear_fail_points());

    ripple.radius_anim.running = M3_TRUE;
    ripple.opacity_anim.running = M3_TRUE;
    M3_TEST_OK(m3_visuals_test_set_fail_point(M3_VISUALS_TEST_FAIL_RIPPLE_STEP_OPACITY_RUNNING));
    M3_TEST_EXPECT(m3_ripple_step(&ripple, 0.0f, &finished), M3_ERR_IO);
    M3_TEST_OK(m3_visuals_test_clear_fail_points());
    M3_TEST_OK(m3_visuals_test_set_fail_point(M3_VISUALS_TEST_FAIL_RIPPLE_STEP_OPACITY_STEP));
    M3_TEST_EXPECT(m3_ripple_step(&ripple, 0.0f, &finished), M3_ERR_IO);
    M3_TEST_OK(m3_visuals_test_clear_fail_points());
    ripple.opacity_anim.running = M3_FALSE;
    M3_TEST_OK(m3_visuals_test_set_fail_point(M3_VISUALS_TEST_FAIL_RIPPLE_STEP_OPACITY_VALUE));
    M3_TEST_EXPECT(m3_ripple_step(&ripple, 0.0f, &finished), M3_ERR_IO);
    M3_TEST_OK(m3_visuals_test_clear_fail_points());

    ripple.state = M3_RIPPLE_STATE_FADING;
    ripple.opacity_anim.running = M3_TRUE;
    M3_TEST_OK(m3_visuals_test_set_fail_point(M3_VISUALS_TEST_FAIL_RIPPLE_STEP_FADE_RUNNING));
    M3_TEST_EXPECT(m3_ripple_step(&ripple, 0.0f, &finished), M3_ERR_IO);
    M3_TEST_OK(m3_visuals_test_clear_fail_points());

    ripple.opacity_anim.running = M3_FALSE;
    M3_TEST_OK(m3_visuals_test_set_fail_point(M3_VISUALS_TEST_FAIL_RIPPLE_STEP_STOP_RADIUS));
    M3_TEST_EXPECT(m3_ripple_step(&ripple, 0.0f, &finished), M3_ERR_IO);
    M3_TEST_OK(m3_visuals_test_clear_fail_points());
    M3_TEST_OK(m3_visuals_test_set_fail_point(M3_VISUALS_TEST_FAIL_RIPPLE_STEP_STOP_OPACITY));
    M3_TEST_EXPECT(m3_ripple_step(&ripple, 0.0f, &finished), M3_ERR_IO);
    M3_TEST_OK(m3_visuals_test_clear_fail_points());
    ripple.state = M3_RIPPLE_STATE_EXPANDING;

    M3_TEST_OK(m3_ripple_step(&ripple, 0.0f, &finished));
    M3_TEST_ASSERT(ripple.radius == 10.0f);
    M3_TEST_ASSERT(finished == M3_FALSE);

    M3_TEST_OK(m3_ripple_release(&ripple, 0.0f));
    M3_TEST_OK(m3_ripple_step(&ripple, 0.0f, &finished));
    M3_TEST_ASSERT(finished == M3_TRUE);
    M3_TEST_OK(m3_ripple_is_active(&ripple, &active));
    M3_TEST_ASSERT(active == M3_FALSE);
    M3_TEST_OK(m3_ripple_step(&ripple, 0.0f, &finished));
    M3_TEST_ASSERT(finished == M3_TRUE);

    M3_TEST_EXPECT(m3_ripple_release(&ripple, 0.1f), M3_ERR_STATE);
    ripple.state = M3_RIPPLE_STATE_FADING;
    M3_TEST_EXPECT(m3_ripple_release(&ripple, 0.1f), M3_ERR_STATE);
    ripple.state = 99;
    M3_TEST_EXPECT(m3_ripple_is_active(&ripple, &active), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_ripple_step(&ripple, 0.0f, &finished), M3_ERR_INVALID_ARGUMENT);
    ripple.state = M3_RIPPLE_STATE_IDLE;

    M3_TEST_EXPECT(m3_ripple_compute_max_radius(NULL, 0.0f, 0.0f, &radius), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_ripple_compute_max_radius(&bounds, 0.0f, 0.0f, NULL), M3_ERR_INVALID_ARGUMENT);
    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = -1.0f;
    bounds.height = 10.0f;
    M3_TEST_EXPECT(m3_ripple_compute_max_radius(&bounds, 0.0f, 0.0f, &radius), M3_ERR_RANGE);
    bounds.width = 10.0f;
    bounds.height = 10.0f;
    M3_TEST_OK(m3_ripple_compute_max_radius(&bounds, 5.0f, 5.0f, &radius));
    M3_TEST_ASSERT(m3_near(radius, 7.071f, 0.01f));
    M3_TEST_OK(m3_ripple_compute_max_radius(&bounds, 0.0f, 0.0f, &radius));
    M3_TEST_ASSERT(m3_near(radius, 14.142f, 0.02f));
    bounds.height = 20.0f;
    M3_TEST_OK(m3_ripple_compute_max_radius(&bounds, 10.0f, 0.0f, &radius));
    M3_TEST_ASSERT(m3_near(radius, 22.360f, 0.02f));
    bounds.height = 10.0f;

    test_gfx_backend_init(&backend);
    gfx.ctx = &backend;
    gfx.vtable = &g_test_vtable;
    gfx.text_vtable = NULL;

    M3_TEST_EXPECT(m3_ripple_paint(NULL, &gfx, NULL, 0.0f), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_ripple_paint(&ripple, NULL, NULL, 0.0f), M3_ERR_INVALID_ARGUMENT);
    gfx.vtable = NULL;
    M3_TEST_EXPECT(m3_ripple_paint(&ripple, &gfx, NULL, 0.0f), M3_ERR_INVALID_ARGUMENT);

    gfx.vtable = &g_test_vtable_no_draw;
    M3_TEST_EXPECT(m3_ripple_paint(&ripple, &gfx, NULL, 0.0f), M3_ERR_UNSUPPORTED);
    gfx.vtable = &g_test_vtable;
    M3_TEST_EXPECT(m3_ripple_paint(&ripple, &gfx, NULL, -1.0f), M3_ERR_RANGE);

    ripple.state = M3_RIPPLE_STATE_IDLE;
    M3_TEST_OK(m3_ripple_paint(&ripple, &gfx, NULL, 0.0f));
    M3_TEST_ASSERT(backend.draw_rect_calls == 0);

    ripple.state = M3_RIPPLE_STATE_EXPANDING;
    ripple.opacity = 0.0f;
    ripple.radius = 5.0f;
    M3_TEST_OK(m3_ripple_paint(&ripple, &gfx, NULL, 0.0f));
    M3_TEST_ASSERT(backend.draw_rect_calls == 0);

    ripple.opacity = 1.0f;
    ripple.radius = 0.0f;
    M3_TEST_OK(m3_ripple_paint(&ripple, &gfx, NULL, 0.0f));
    M3_TEST_ASSERT(backend.draw_rect_calls == 0);

    ripple.opacity = 0.5f;
    ripple.radius = 4.0f;
    ripple.color.r = 0.1f;
    ripple.color.g = 0.2f;
    ripple.color.b = 0.3f;
    ripple.color.a = 0.8f;
    clip.x = 0.0f;
    clip.y = 0.0f;
    clip.width = 10.0f;
    clip.height = 10.0f;

    M3_TEST_OK(m3_ripple_paint(&ripple, &gfx, &clip, 2.0f));
    M3_TEST_ASSERT(backend.draw_rect_calls == 1);
    M3_TEST_ASSERT(backend.push_clip_calls == 1);
    M3_TEST_ASSERT(backend.pop_clip_calls == 1);
    M3_TEST_ASSERT(m3_near(backend.last_rect.width, 8.0f, 0.0001f));
    M3_TEST_ASSERT(m3_near(backend.last_color.a, 0.4f, 0.0001f));

    M3_TEST_OK(m3_visuals_test_set_fail_point(M3_VISUALS_TEST_FAIL_RIPPLE_PAINT_RECT));
    M3_TEST_EXPECT(m3_ripple_paint(&ripple, &gfx, NULL, 0.0f), M3_ERR_RANGE);
    M3_TEST_OK(m3_visuals_test_clear_fail_points());
    M3_TEST_OK(m3_visuals_test_set_fail_point(M3_VISUALS_TEST_FAIL_RIPPLE_PAINT_COLOR));
    M3_TEST_EXPECT(m3_ripple_paint(&ripple, &gfx, NULL, 0.0f), M3_ERR_RANGE);
    M3_TEST_OK(m3_visuals_test_clear_fail_points());

    test_gfx_backend_init(&backend);
    gfx.ctx = &backend;
    gfx.vtable = &g_test_vtable;
    M3_TEST_OK(m3_ripple_paint(&ripple, &gfx, NULL, 1.0f));
    M3_TEST_ASSERT(backend.draw_rect_calls == 1);
    M3_TEST_ASSERT(backend.push_clip_calls == 0);

    ripple.color.a = 1.2f;
    M3_TEST_EXPECT(m3_ripple_paint(&ripple, &gfx, NULL, 0.0f), M3_ERR_RANGE);
    ripple.color.a = 0.8f;

    test_gfx_backend_init(&backend);
    gfx.ctx = &backend;
    gfx.vtable = &g_test_vtable;
    backend.fail_push_clip = M3_ERR_IO;
    M3_TEST_EXPECT(m3_ripple_paint(&ripple, &gfx, &clip, 0.0f), M3_ERR_IO);
    M3_TEST_ASSERT(backend.draw_rect_calls == 0);

    test_gfx_backend_init(&backend);
    gfx.ctx = &backend;
    gfx.vtable = &g_test_vtable;
    backend.fail_draw_rect = M3_ERR_IO;
    M3_TEST_EXPECT(m3_ripple_paint(&ripple, &gfx, &clip, 0.0f), M3_ERR_IO);
    M3_TEST_ASSERT(backend.pop_clip_calls == 1);

    test_gfx_backend_init(&backend);
    gfx.ctx = &backend;
    gfx.vtable = &g_test_vtable;
    backend.fail_pop_clip = M3_ERR_IO;
    M3_TEST_EXPECT(m3_ripple_paint(&ripple, &gfx, &clip, 0.0f), M3_ERR_IO);

    gfx.vtable = &g_test_vtable_no_clip;
    M3_TEST_EXPECT(m3_ripple_paint(&ripple, &gfx, &clip, 0.0f), M3_ERR_UNSUPPORTED);
    gfx.vtable = &g_test_vtable;
    clip.width = -1.0f;
    M3_TEST_EXPECT(m3_ripple_paint(&ripple, &gfx, &clip, 0.0f), M3_ERR_RANGE);
    clip.width = 10.0f;

    ripple.opacity = 1.1f;
    M3_TEST_EXPECT(m3_ripple_paint(&ripple, &gfx, NULL, 0.0f), M3_ERR_RANGE);
    ripple.opacity = 0.5f;

    ripple.radius = -1.0f;
    M3_TEST_EXPECT(m3_ripple_paint(&ripple, &gfx, NULL, 0.0f), M3_ERR_RANGE);
    ripple.radius = 4.0f;

    ripple.state = 99;
    M3_TEST_EXPECT(m3_ripple_paint(&ripple, &gfx, NULL, 0.0f), M3_ERR_INVALID_ARGUMENT);
    ripple.state = M3_RIPPLE_STATE_EXPANDING;

    M3_TEST_EXPECT(m3_shadow_set(NULL, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1, color), M3_ERR_INVALID_ARGUMENT);
    color.r = 0.1f;
    color.g = 0.1f;
    color.b = 0.1f;
    color.a = 0.8f;
    color.g = -0.2f;
    M3_TEST_EXPECT(m3_shadow_set(&shadow, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1, color), M3_ERR_RANGE);
    color.g = 0.1f;
    color.b = -0.2f;
    M3_TEST_EXPECT(m3_shadow_set(&shadow, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1, color), M3_ERR_RANGE);
    color.b = 0.1f;
    M3_TEST_EXPECT(m3_shadow_set(&shadow, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1, color), M3_ERR_RANGE);
    M3_TEST_EXPECT(m3_shadow_set(&shadow, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1, color), M3_ERR_RANGE);
    M3_TEST_EXPECT(m3_shadow_set(&shadow, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 1, color), M3_ERR_RANGE);
    M3_TEST_EXPECT(m3_shadow_set(&shadow, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0, color), M3_ERR_RANGE);
    color.a = 1.2f;
    M3_TEST_EXPECT(m3_shadow_set(&shadow, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1, color), M3_ERR_RANGE);
    color.a = 0.8f;
    M3_TEST_OK(m3_shadow_set(&shadow, 2.0f, 3.0f, 6.0f, 2.0f, 4.0f, 3, color));

    rect.x = 10.0f;
    rect.y = 20.0f;
    rect.width = 100.0f;
    rect.height = 40.0f;

    test_gfx_backend_init(&backend);
    gfx.ctx = &backend;
    gfx.vtable = &g_test_vtable;

    M3_TEST_EXPECT(m3_shadow_paint(NULL, &gfx, &rect, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_shadow_paint(&shadow, NULL, &rect, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_shadow_paint(&shadow, &gfx, NULL, NULL), M3_ERR_INVALID_ARGUMENT);

    gfx.vtable = NULL;
    M3_TEST_EXPECT(m3_shadow_paint(&shadow, &gfx, &rect, NULL), M3_ERR_INVALID_ARGUMENT);
    gfx.vtable = &g_test_vtable_no_draw;
    M3_TEST_EXPECT(m3_shadow_paint(&shadow, &gfx, &rect, NULL), M3_ERR_UNSUPPORTED);
    gfx.vtable = &g_test_vtable;

    M3_TEST_OK(m3_visuals_test_set_fail_point(M3_VISUALS_TEST_FAIL_SHADOW_NORM));
    M3_TEST_EXPECT(m3_shadow_paint(&shadow, &gfx, &rect, NULL), M3_ERR_RANGE);
    M3_TEST_OK(m3_visuals_test_clear_fail_points());

    rect.width = -1.0f;
    M3_TEST_EXPECT(m3_shadow_paint(&shadow, &gfx, &rect, NULL), M3_ERR_RANGE);
    rect.width = 100.0f;

    shadow.layers = 0;
    M3_TEST_EXPECT(m3_shadow_paint(&shadow, &gfx, &rect, NULL), M3_ERR_RANGE);
    shadow.layers = 3;

    shadow.blur_radius = -1.0f;
    M3_TEST_EXPECT(m3_shadow_paint(&shadow, &gfx, &rect, NULL), M3_ERR_RANGE);
    shadow.blur_radius = 6.0f;

    shadow.color.a = -0.1f;
    M3_TEST_EXPECT(m3_shadow_paint(&shadow, &gfx, &rect, NULL), M3_ERR_RANGE);
    shadow.color.a = 0.8f;

    shadow.color.a = 0.0f;
    M3_TEST_OK(m3_shadow_paint(&shadow, &gfx, &rect, NULL));
    M3_TEST_ASSERT(backend.draw_rect_calls == 0);
    shadow.color.a = 0.8f;

    clip.x = 0.0f;
    clip.y = 0.0f;
    clip.width = 200.0f;
    clip.height = 200.0f;

    test_gfx_backend_init(&backend);
    gfx.ctx = &backend;
    gfx.vtable = &g_test_vtable;
    M3_TEST_OK(m3_shadow_paint(&shadow, &gfx, &rect, NULL));
    M3_TEST_ASSERT(backend.draw_rect_calls == 3);
    M3_TEST_ASSERT(backend.push_clip_calls == 0);

    test_gfx_backend_init(&backend);
    gfx.ctx = &backend;
    gfx.vtable = &g_test_vtable;
    backend.fail_push_clip = M3_ERR_IO;
    M3_TEST_EXPECT(m3_shadow_paint(&shadow, &gfx, &rect, &clip), M3_ERR_IO);
    M3_TEST_ASSERT(backend.draw_rect_calls == 0);

    test_gfx_backend_init(&backend);
    gfx.ctx = &backend;
    gfx.vtable = &g_test_vtable;
    M3_TEST_OK(m3_shadow_paint(&shadow, &gfx, &rect, &clip));
    M3_TEST_ASSERT(backend.draw_rect_calls == 3);
    M3_TEST_ASSERT(backend.push_clip_calls == 1);
    M3_TEST_ASSERT(backend.pop_clip_calls == 1);
    M3_TEST_ASSERT(m3_near(backend.last_rect.x, 10.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(backend.last_rect.y, 21.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(backend.last_rect.width, 104.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(backend.last_rect.height, 44.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(backend.last_corner, 6.0f, 0.001f));
    M3_TEST_ASSERT(m3_near(backend.last_color.a, 0.4f, 0.0001f));

    test_gfx_backend_init(&backend);
    gfx.ctx = &backend;
    gfx.vtable = &g_test_vtable;
    backend.fail_draw_rect = M3_ERR_IO;
    M3_TEST_EXPECT(m3_shadow_paint(&shadow, &gfx, &rect, &clip), M3_ERR_IO);
    M3_TEST_ASSERT(backend.pop_clip_calls == 1);

    test_gfx_backend_init(&backend);
    gfx.ctx = &backend;
    gfx.vtable = &g_test_vtable;
    backend.fail_pop_clip = M3_ERR_IO;
    M3_TEST_EXPECT(m3_shadow_paint(&shadow, &gfx, &rect, &clip), M3_ERR_IO);

    gfx.vtable = &g_test_vtable_no_clip;
    M3_TEST_EXPECT(m3_shadow_paint(&shadow, &gfx, &rect, &clip), M3_ERR_UNSUPPORTED);
    gfx.vtable = &g_test_vtable;
    clip.width = -1.0f;
    M3_TEST_EXPECT(m3_shadow_paint(&shadow, &gfx, &rect, &clip), M3_ERR_RANGE);

    return 0;
}
