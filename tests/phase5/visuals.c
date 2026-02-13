#include "cmpc/cmp_visuals.h"
#include "test_utils.h"

#include <string.h>

#define CMP_VISUALS_TEST_FAIL_NONE 0u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_INIT_RADIUS 1u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_INIT_OPACITY 2u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_START_RADIUS_INIT 3u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_START_OPACITY_INIT 4u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_START_TIMING 5u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_RELEASE_TIMING 6u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_RADIUS_RUNNING 7u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_RADIUS_STEP 8u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_RADIUS_VALUE 9u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_OPACITY_RUNNING 10u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_OPACITY_STEP 11u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_OPACITY_VALUE 12u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_FADE_RUNNING 13u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_STOP_RADIUS 14u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_STOP_OPACITY 15u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_PAINT_RECT 16u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_PAINT_COLOR 17u
#define CMP_VISUALS_TEST_FAIL_SHADOW_NORM 18u

typedef struct TestGfxBackend {
  int draw_rect_calls;
  int push_clip_calls;
  int pop_clip_calls;
  int fail_draw_rect;
  int fail_push_clip;
  int fail_pop_clip;
  CMPRect last_rect;
  CMPRect last_clip;
  CMPScalar last_corner;
  CMPColor last_color;
} TestGfxBackend;

static void test_gfx_backend_init(TestGfxBackend *backend) {
  memset(backend, 0, sizeof(*backend));
  backend->fail_draw_rect = CMP_OK;
  backend->fail_push_clip = CMP_OK;
  backend->fail_pop_clip = CMP_OK;
}

static int test_gfx_draw_rect(void *gfx, const CMPRect *rect, CMPColor color,
                              CMPScalar corner_radius) {
  TestGfxBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestGfxBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_color = color;
  backend->last_corner = corner_radius;
  if (backend->fail_draw_rect != CMP_OK) {
    return backend->fail_draw_rect;
  }
  return CMP_OK;
}

static int test_gfx_push_clip(void *gfx, const CMPRect *rect) {
  TestGfxBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestGfxBackend *)gfx;
  backend->push_clip_calls += 1;
  backend->last_clip = *rect;
  if (backend->fail_push_clip != CMP_OK) {
    return backend->fail_push_clip;
  }
  return CMP_OK;
}

static int test_gfx_pop_clip(void *gfx) {
  TestGfxBackend *backend;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestGfxBackend *)gfx;
  backend->pop_clip_calls += 1;
  if (backend->fail_pop_clip != CMP_OK) {
    return backend->fail_pop_clip;
  }
  return CMP_OK;
}

static const CMPGfxVTable g_test_vtable = {NULL,
                                           NULL,
                                           NULL,
                                           test_gfx_draw_rect,
                                           NULL,
                                           NULL,
                                           test_gfx_push_clip,
                                           test_gfx_pop_clip,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL};

static const CMPGfxVTable g_test_vtable_no_draw = {
    NULL, NULL, NULL, NULL, NULL, NULL, test_gfx_push_clip, test_gfx_pop_clip,
    NULL, NULL, NULL, NULL, NULL};

static const CMPGfxVTable g_test_vtable_no_clip = {
    NULL, NULL, NULL, test_gfx_draw_rect, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL};

static int cmp_near(CMPScalar a, CMPScalar b, CMPScalar tol) {
  CMPScalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

int main(void) {
  CMPRipple ripple;
  CMPShadow shadow;
  CMPColor color;
  CMPBool active;
  CMPBool finished;
  CMPScalar radius;
  CMPRect bounds;
  CMPRect clip;
  CMPRect rect;
  TestGfxBackend backend;
  CMPGfx gfx;

  color.r = 0.0f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;

  CMP_TEST_EXPECT(cmp_visuals_test_validate_rect(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 1.0f;
  CMP_TEST_EXPECT(cmp_visuals_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.width = 1.0f;
  rect.height = -1.0f;
  CMP_TEST_EXPECT(cmp_visuals_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.height = 1.0f;
  CMP_TEST_OK(cmp_visuals_test_validate_rect(&rect));

  CMP_TEST_EXPECT(cmp_visuals_test_validate_color(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  CMP_TEST_EXPECT(cmp_visuals_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 0.0f;
  color.g = 1.2f;
  CMP_TEST_EXPECT(cmp_visuals_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.2f;
  CMP_TEST_EXPECT(cmp_visuals_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = 0.0f;
  color.a = 1.2f;
  CMP_TEST_EXPECT(cmp_visuals_test_validate_color(&color), CMP_ERR_RANGE);
  color.a = 0.0f;
  CMP_TEST_OK(cmp_visuals_test_validate_color(&color));

  CMP_TEST_EXPECT(cmp_visuals_test_validate_gfx(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  test_gfx_backend_init(&backend);
  gfx.ctx = &backend;
  gfx.vtable = NULL;
  gfx.text_vtable = NULL;
  CMP_TEST_EXPECT(cmp_visuals_test_validate_gfx(&gfx),
                  CMP_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_vtable_no_draw;
  CMP_TEST_EXPECT(cmp_visuals_test_validate_gfx(&gfx), CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable;
  CMP_TEST_OK(cmp_visuals_test_validate_gfx(&gfx));

  CMP_TEST_EXPECT(cmp_ripple_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_shadow_init(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_ripple_init(&ripple));
  CMP_TEST_OK(cmp_shadow_init(&shadow));
  CMP_TEST_OK(cmp_visuals_test_set_fail_point(
      CMP_VISUALS_TEST_FAIL_RIPPLE_INIT_RADIUS));
  CMP_TEST_EXPECT(cmp_ripple_init(&ripple), CMP_ERR_IO);
  CMP_TEST_OK(cmp_visuals_test_clear_fail_points());
  CMP_TEST_OK(cmp_visuals_test_set_fail_point(
      CMP_VISUALS_TEST_FAIL_RIPPLE_INIT_OPACITY));
  CMP_TEST_EXPECT(cmp_ripple_init(&ripple), CMP_ERR_IO);
  CMP_TEST_OK(cmp_visuals_test_clear_fail_points());
  CMP_TEST_OK(cmp_ripple_init(&ripple));

  CMP_TEST_EXPECT(cmp_ripple_is_active(NULL, &active),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_ripple_is_active(&ripple, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_ripple_is_active(&ripple, &active));
  CMP_TEST_ASSERT(active == CMP_FALSE);

  CMP_TEST_EXPECT(cmp_ripple_start(NULL, 0.0f, 0.0f, 1.0f, 0.1f, color),
                  CMP_ERR_INVALID_ARGUMENT);
  color.r = 0.2f;
  color.g = 0.3f;
  color.b = 0.4f;
  color.a = 0.5f;
  CMP_TEST_EXPECT(cmp_ripple_start(&ripple, 0.0f, 0.0f, 0.0f, 0.1f, color),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_ripple_start(&ripple, 0.0f, 0.0f, 1.0f, -0.1f, color),
                  CMP_ERR_RANGE);
  color.r = -0.1f;
  CMP_TEST_EXPECT(cmp_ripple_start(&ripple, 0.0f, 0.0f, 1.0f, 0.1f, color),
                  CMP_ERR_RANGE);
  color.r = 0.2f;
  color.g = -0.1f;
  CMP_TEST_EXPECT(cmp_ripple_start(&ripple, 0.0f, 0.0f, 1.0f, 0.1f, color),
                  CMP_ERR_RANGE);
  color.g = 0.3f;
  color.b = -0.1f;
  CMP_TEST_EXPECT(cmp_ripple_start(&ripple, 0.0f, 0.0f, 1.0f, 0.1f, color),
                  CMP_ERR_RANGE);
  color.b = 0.4f;
  color.a = 1.2f;
  CMP_TEST_EXPECT(cmp_ripple_start(&ripple, 0.0f, 0.0f, 1.0f, 0.1f, color),
                  CMP_ERR_RANGE);
  color.a = 0.5f;

  CMP_TEST_OK(cmp_visuals_test_set_fail_point(
      CMP_VISUALS_TEST_FAIL_RIPPLE_START_RADIUS_INIT));
  CMP_TEST_EXPECT(cmp_ripple_start(&ripple, 5.0f, 6.0f, 10.0f, 0.0f, color),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_visuals_test_clear_fail_points());
  CMP_TEST_OK(cmp_visuals_test_set_fail_point(
      CMP_VISUALS_TEST_FAIL_RIPPLE_START_OPACITY_INIT));
  CMP_TEST_EXPECT(cmp_ripple_start(&ripple, 5.0f, 6.0f, 10.0f, 0.0f, color),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_visuals_test_clear_fail_points());
  CMP_TEST_OK(cmp_visuals_test_set_fail_point(
      CMP_VISUALS_TEST_FAIL_RIPPLE_START_TIMING));
  CMP_TEST_EXPECT(cmp_ripple_start(&ripple, 5.0f, 6.0f, 10.0f, 0.0f, color),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_visuals_test_clear_fail_points());

  CMP_TEST_OK(cmp_ripple_start(&ripple, 5.0f, 6.0f, 10.0f, 0.0f, color));
  CMP_TEST_OK(cmp_ripple_is_active(&ripple, &active));
  CMP_TEST_ASSERT(active == CMP_TRUE);

  CMP_TEST_EXPECT(cmp_ripple_release(NULL, 0.1f), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_ripple_release(&ripple, -0.1f), CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_visuals_test_set_fail_point(
      CMP_VISUALS_TEST_FAIL_RIPPLE_RELEASE_TIMING));
  CMP_TEST_EXPECT(cmp_ripple_release(&ripple, 0.1f), CMP_ERR_IO);
  CMP_TEST_OK(cmp_visuals_test_clear_fail_points());

  CMP_TEST_EXPECT(cmp_ripple_step(NULL, 0.1f, &finished),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_ripple_step(&ripple, 0.1f, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_ripple_step(&ripple, -0.1f, &finished), CMP_ERR_RANGE);

  ripple.radius_anim.running = CMP_TRUE;
  ripple.opacity_anim.running = CMP_TRUE;
  ripple.radius_anim.mode = CMP_ANIM_MODE_TIMING;
  ripple.opacity_anim.mode = CMP_ANIM_MODE_TIMING;
  CMP_TEST_OK(cmp_visuals_test_set_fail_point(
      CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_RADIUS_RUNNING));
  CMP_TEST_EXPECT(cmp_ripple_step(&ripple, 0.0f, &finished), CMP_ERR_IO);
  CMP_TEST_OK(cmp_visuals_test_clear_fail_points());
  CMP_TEST_OK(cmp_visuals_test_set_fail_point(
      CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_RADIUS_STEP));
  CMP_TEST_EXPECT(cmp_ripple_step(&ripple, 0.0f, &finished), CMP_ERR_IO);
  CMP_TEST_OK(cmp_visuals_test_clear_fail_points());
  ripple.radius_anim.running = CMP_FALSE;
  CMP_TEST_OK(cmp_visuals_test_set_fail_point(
      CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_RADIUS_VALUE));
  CMP_TEST_EXPECT(cmp_ripple_step(&ripple, 0.0f, &finished), CMP_ERR_IO);
  CMP_TEST_OK(cmp_visuals_test_clear_fail_points());

  ripple.radius_anim.running = CMP_TRUE;
  ripple.opacity_anim.running = CMP_TRUE;
  CMP_TEST_OK(cmp_visuals_test_set_fail_point(
      CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_OPACITY_RUNNING));
  CMP_TEST_EXPECT(cmp_ripple_step(&ripple, 0.0f, &finished), CMP_ERR_IO);
  CMP_TEST_OK(cmp_visuals_test_clear_fail_points());
  CMP_TEST_OK(cmp_visuals_test_set_fail_point(
      CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_OPACITY_STEP));
  CMP_TEST_EXPECT(cmp_ripple_step(&ripple, 0.0f, &finished), CMP_ERR_IO);
  CMP_TEST_OK(cmp_visuals_test_clear_fail_points());
  ripple.opacity_anim.running = CMP_FALSE;
  CMP_TEST_OK(cmp_visuals_test_set_fail_point(
      CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_OPACITY_VALUE));
  CMP_TEST_EXPECT(cmp_ripple_step(&ripple, 0.0f, &finished), CMP_ERR_IO);
  CMP_TEST_OK(cmp_visuals_test_clear_fail_points());

  ripple.state = CMP_RIPPLE_STATE_FADING;
  ripple.opacity_anim.running = CMP_TRUE;
  CMP_TEST_OK(cmp_visuals_test_set_fail_point(
      CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_FADE_RUNNING));
  CMP_TEST_EXPECT(cmp_ripple_step(&ripple, 0.0f, &finished), CMP_ERR_IO);
  CMP_TEST_OK(cmp_visuals_test_clear_fail_points());

  ripple.opacity_anim.running = CMP_FALSE;
  CMP_TEST_OK(cmp_visuals_test_set_fail_point(
      CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_STOP_RADIUS));
  CMP_TEST_EXPECT(cmp_ripple_step(&ripple, 0.0f, &finished), CMP_ERR_IO);
  CMP_TEST_OK(cmp_visuals_test_clear_fail_points());
  CMP_TEST_OK(cmp_visuals_test_set_fail_point(
      CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_STOP_OPACITY));
  CMP_TEST_EXPECT(cmp_ripple_step(&ripple, 0.0f, &finished), CMP_ERR_IO);
  CMP_TEST_OK(cmp_visuals_test_clear_fail_points());
  ripple.state = CMP_RIPPLE_STATE_EXPANDING;

  CMP_TEST_OK(cmp_ripple_step(&ripple, 0.0f, &finished));
  CMP_TEST_ASSERT(ripple.radius == 10.0f);
  CMP_TEST_ASSERT(finished == CMP_FALSE);

  CMP_TEST_OK(cmp_ripple_release(&ripple, 0.0f));
  CMP_TEST_OK(cmp_ripple_step(&ripple, 0.0f, &finished));
  CMP_TEST_ASSERT(finished == CMP_TRUE);
  CMP_TEST_OK(cmp_ripple_is_active(&ripple, &active));
  CMP_TEST_ASSERT(active == CMP_FALSE);
  CMP_TEST_OK(cmp_ripple_step(&ripple, 0.0f, &finished));
  CMP_TEST_ASSERT(finished == CMP_TRUE);

  CMP_TEST_EXPECT(cmp_ripple_release(&ripple, 0.1f), CMP_ERR_STATE);
  ripple.state = CMP_RIPPLE_STATE_FADING;
  CMP_TEST_EXPECT(cmp_ripple_release(&ripple, 0.1f), CMP_ERR_STATE);
  ripple.state = 99;
  CMP_TEST_EXPECT(cmp_ripple_is_active(&ripple, &active),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_ripple_step(&ripple, 0.0f, &finished),
                  CMP_ERR_INVALID_ARGUMENT);
  ripple.state = CMP_RIPPLE_STATE_IDLE;

  CMP_TEST_EXPECT(cmp_ripple_compute_max_radius(NULL, 0.0f, 0.0f, &radius),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_ripple_compute_max_radius(&bounds, 0.0f, 0.0f, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  CMP_TEST_EXPECT(cmp_ripple_compute_max_radius(&bounds, 0.0f, 0.0f, &radius),
                  CMP_ERR_RANGE);
  bounds.width = 10.0f;
  bounds.height = 10.0f;
  CMP_TEST_OK(cmp_ripple_compute_max_radius(&bounds, 5.0f, 5.0f, &radius));
  CMP_TEST_ASSERT(cmp_near(radius, 7.071f, 0.01f));
  CMP_TEST_OK(cmp_ripple_compute_max_radius(&bounds, 0.0f, 0.0f, &radius));
  CMP_TEST_ASSERT(cmp_near(radius, 14.142f, 0.02f));
  bounds.height = 20.0f;
  CMP_TEST_OK(cmp_ripple_compute_max_radius(&bounds, 10.0f, 0.0f, &radius));
  CMP_TEST_ASSERT(cmp_near(radius, 22.360f, 0.02f));
  bounds.height = 10.0f;

  test_gfx_backend_init(&backend);
  gfx.ctx = &backend;
  gfx.vtable = &g_test_vtable;
  gfx.text_vtable = NULL;

  CMP_TEST_EXPECT(cmp_ripple_paint(NULL, &gfx, NULL, 0.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_ripple_paint(&ripple, NULL, NULL, 0.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  gfx.vtable = NULL;
  CMP_TEST_EXPECT(cmp_ripple_paint(&ripple, &gfx, NULL, 0.0f),
                  CMP_ERR_INVALID_ARGUMENT);

  gfx.vtable = &g_test_vtable_no_draw;
  CMP_TEST_EXPECT(cmp_ripple_paint(&ripple, &gfx, NULL, 0.0f),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable;
  CMP_TEST_EXPECT(cmp_ripple_paint(&ripple, &gfx, NULL, -1.0f), CMP_ERR_RANGE);

  ripple.state = CMP_RIPPLE_STATE_IDLE;
  CMP_TEST_OK(cmp_ripple_paint(&ripple, &gfx, NULL, 0.0f));
  CMP_TEST_ASSERT(backend.draw_rect_calls == 0);

  ripple.state = CMP_RIPPLE_STATE_EXPANDING;
  ripple.opacity = 0.0f;
  ripple.radius = 5.0f;
  CMP_TEST_OK(cmp_ripple_paint(&ripple, &gfx, NULL, 0.0f));
  CMP_TEST_ASSERT(backend.draw_rect_calls == 0);

  ripple.opacity = 1.0f;
  ripple.radius = 0.0f;
  CMP_TEST_OK(cmp_ripple_paint(&ripple, &gfx, NULL, 0.0f));
  CMP_TEST_ASSERT(backend.draw_rect_calls == 0);

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

  CMP_TEST_OK(cmp_ripple_paint(&ripple, &gfx, &clip, 2.0f));
  CMP_TEST_ASSERT(backend.draw_rect_calls == 1);
  CMP_TEST_ASSERT(backend.push_clip_calls == 1);
  CMP_TEST_ASSERT(backend.pop_clip_calls == 1);
  CMP_TEST_ASSERT(cmp_near(backend.last_rect.width, 8.0f, 0.0001f));
  CMP_TEST_ASSERT(cmp_near(backend.last_color.a, 0.4f, 0.0001f));

  CMP_TEST_OK(
      cmp_visuals_test_set_fail_point(CMP_VISUALS_TEST_FAIL_RIPPLE_PAINT_RECT));
  CMP_TEST_EXPECT(cmp_ripple_paint(&ripple, &gfx, NULL, 0.0f), CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_visuals_test_clear_fail_points());
  CMP_TEST_OK(cmp_visuals_test_set_fail_point(
      CMP_VISUALS_TEST_FAIL_RIPPLE_PAINT_COLOR));
  CMP_TEST_EXPECT(cmp_ripple_paint(&ripple, &gfx, NULL, 0.0f), CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_visuals_test_clear_fail_points());

  test_gfx_backend_init(&backend);
  gfx.ctx = &backend;
  gfx.vtable = &g_test_vtable;
  CMP_TEST_OK(cmp_ripple_paint(&ripple, &gfx, NULL, 1.0f));
  CMP_TEST_ASSERT(backend.draw_rect_calls == 1);
  CMP_TEST_ASSERT(backend.push_clip_calls == 0);

  ripple.color.a = 1.2f;
  CMP_TEST_EXPECT(cmp_ripple_paint(&ripple, &gfx, NULL, 0.0f), CMP_ERR_RANGE);
  ripple.color.a = 0.8f;

  test_gfx_backend_init(&backend);
  gfx.ctx = &backend;
  gfx.vtable = &g_test_vtable;
  backend.fail_push_clip = CMP_ERR_IO;
  CMP_TEST_EXPECT(cmp_ripple_paint(&ripple, &gfx, &clip, 0.0f), CMP_ERR_IO);
  CMP_TEST_ASSERT(backend.draw_rect_calls == 0);

  test_gfx_backend_init(&backend);
  gfx.ctx = &backend;
  gfx.vtable = &g_test_vtable;
  backend.fail_draw_rect = CMP_ERR_IO;
  CMP_TEST_EXPECT(cmp_ripple_paint(&ripple, &gfx, &clip, 0.0f), CMP_ERR_IO);
  CMP_TEST_ASSERT(backend.pop_clip_calls == 1);

  test_gfx_backend_init(&backend);
  gfx.ctx = &backend;
  gfx.vtable = &g_test_vtable;
  backend.fail_pop_clip = CMP_ERR_IO;
  CMP_TEST_EXPECT(cmp_ripple_paint(&ripple, &gfx, &clip, 0.0f), CMP_ERR_IO);

  gfx.vtable = &g_test_vtable_no_clip;
  CMP_TEST_EXPECT(cmp_ripple_paint(&ripple, &gfx, &clip, 0.0f),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable;
  clip.width = -1.0f;
  CMP_TEST_EXPECT(cmp_ripple_paint(&ripple, &gfx, &clip, 0.0f), CMP_ERR_RANGE);
  clip.width = 10.0f;

  ripple.opacity = 1.1f;
  CMP_TEST_EXPECT(cmp_ripple_paint(&ripple, &gfx, NULL, 0.0f), CMP_ERR_RANGE);
  ripple.opacity = 0.5f;

  ripple.radius = -1.0f;
  CMP_TEST_EXPECT(cmp_ripple_paint(&ripple, &gfx, NULL, 0.0f), CMP_ERR_RANGE);
  ripple.radius = 4.0f;

  ripple.state = 99;
  CMP_TEST_EXPECT(cmp_ripple_paint(&ripple, &gfx, NULL, 0.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  ripple.state = CMP_RIPPLE_STATE_EXPANDING;

  CMP_TEST_EXPECT(cmp_shadow_set(NULL, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1, color),
                  CMP_ERR_INVALID_ARGUMENT);
  color.r = 0.1f;
  color.g = 0.1f;
  color.b = 0.1f;
  color.a = 0.8f;
  color.g = -0.2f;
  CMP_TEST_EXPECT(
      cmp_shadow_set(&shadow, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1, color),
      CMP_ERR_RANGE);
  color.g = 0.1f;
  color.b = -0.2f;
  CMP_TEST_EXPECT(
      cmp_shadow_set(&shadow, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1, color),
      CMP_ERR_RANGE);
  color.b = 0.1f;
  CMP_TEST_EXPECT(
      cmp_shadow_set(&shadow, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1, color),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      cmp_shadow_set(&shadow, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1, color),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      cmp_shadow_set(&shadow, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 1, color),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      cmp_shadow_set(&shadow, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0, color),
      CMP_ERR_RANGE);
  color.a = 1.2f;
  CMP_TEST_EXPECT(
      cmp_shadow_set(&shadow, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1, color),
      CMP_ERR_RANGE);
  color.a = 0.8f;
  CMP_TEST_OK(cmp_shadow_set(&shadow, 2.0f, 3.0f, 6.0f, 2.0f, 4.0f, 3, color));

  rect.x = 10.0f;
  rect.y = 20.0f;
  rect.width = 100.0f;
  rect.height = 40.0f;

  test_gfx_backend_init(&backend);
  gfx.ctx = &backend;
  gfx.vtable = &g_test_vtable;

  CMP_TEST_EXPECT(cmp_shadow_paint(NULL, &gfx, &rect, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_shadow_paint(&shadow, NULL, &rect, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_shadow_paint(&shadow, &gfx, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  gfx.vtable = NULL;
  CMP_TEST_EXPECT(cmp_shadow_paint(&shadow, &gfx, &rect, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_vtable_no_draw;
  CMP_TEST_EXPECT(cmp_shadow_paint(&shadow, &gfx, &rect, NULL),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable;

  CMP_TEST_OK(
      cmp_visuals_test_set_fail_point(CMP_VISUALS_TEST_FAIL_SHADOW_NORM));
  CMP_TEST_EXPECT(cmp_shadow_paint(&shadow, &gfx, &rect, NULL), CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_visuals_test_clear_fail_points());

  rect.width = -1.0f;
  CMP_TEST_EXPECT(cmp_shadow_paint(&shadow, &gfx, &rect, NULL), CMP_ERR_RANGE);
  rect.width = 100.0f;

  shadow.layers = 0;
  CMP_TEST_EXPECT(cmp_shadow_paint(&shadow, &gfx, &rect, NULL), CMP_ERR_RANGE);
  shadow.layers = 3;

  shadow.blur_radius = -1.0f;
  CMP_TEST_EXPECT(cmp_shadow_paint(&shadow, &gfx, &rect, NULL), CMP_ERR_RANGE);
  shadow.blur_radius = 6.0f;

  shadow.color.a = -0.1f;
  CMP_TEST_EXPECT(cmp_shadow_paint(&shadow, &gfx, &rect, NULL), CMP_ERR_RANGE);
  shadow.color.a = 0.8f;

  shadow.color.a = 0.0f;
  CMP_TEST_OK(cmp_shadow_paint(&shadow, &gfx, &rect, NULL));
  CMP_TEST_ASSERT(backend.draw_rect_calls == 0);
  shadow.color.a = 0.8f;

  clip.x = 0.0f;
  clip.y = 0.0f;
  clip.width = 200.0f;
  clip.height = 200.0f;

  test_gfx_backend_init(&backend);
  gfx.ctx = &backend;
  gfx.vtable = &g_test_vtable;
  CMP_TEST_OK(cmp_shadow_paint(&shadow, &gfx, &rect, NULL));
  CMP_TEST_ASSERT(backend.draw_rect_calls == 3);
  CMP_TEST_ASSERT(backend.push_clip_calls == 0);

  test_gfx_backend_init(&backend);
  gfx.ctx = &backend;
  gfx.vtable = &g_test_vtable;
  backend.fail_push_clip = CMP_ERR_IO;
  CMP_TEST_EXPECT(cmp_shadow_paint(&shadow, &gfx, &rect, &clip), CMP_ERR_IO);
  CMP_TEST_ASSERT(backend.draw_rect_calls == 0);

  test_gfx_backend_init(&backend);
  gfx.ctx = &backend;
  gfx.vtable = &g_test_vtable;
  CMP_TEST_OK(cmp_shadow_paint(&shadow, &gfx, &rect, &clip));
  CMP_TEST_ASSERT(backend.draw_rect_calls == 3);
  CMP_TEST_ASSERT(backend.push_clip_calls == 1);
  CMP_TEST_ASSERT(backend.pop_clip_calls == 1);
  CMP_TEST_ASSERT(cmp_near(backend.last_rect.x, 10.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(backend.last_rect.y, 21.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(backend.last_rect.width, 104.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(backend.last_rect.height, 44.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(backend.last_corner, 6.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(backend.last_color.a, 0.4f, 0.0001f));

  test_gfx_backend_init(&backend);
  gfx.ctx = &backend;
  gfx.vtable = &g_test_vtable;
  backend.fail_draw_rect = CMP_ERR_IO;
  CMP_TEST_EXPECT(cmp_shadow_paint(&shadow, &gfx, &rect, &clip), CMP_ERR_IO);
  CMP_TEST_ASSERT(backend.pop_clip_calls == 1);

  test_gfx_backend_init(&backend);
  gfx.ctx = &backend;
  gfx.vtable = &g_test_vtable;
  backend.fail_pop_clip = CMP_ERR_IO;
  CMP_TEST_EXPECT(cmp_shadow_paint(&shadow, &gfx, &rect, &clip), CMP_ERR_IO);

  gfx.vtable = &g_test_vtable_no_clip;
  CMP_TEST_EXPECT(cmp_shadow_paint(&shadow, &gfx, &rect, &clip),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable;
  clip.width = -1.0f;
  CMP_TEST_EXPECT(cmp_shadow_paint(&shadow, &gfx, &rect, &clip), CMP_ERR_RANGE);

  return 0;
}
