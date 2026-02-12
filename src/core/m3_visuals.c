#include "m3/m3_visuals.h"

#include <math.h>

#ifdef M3_TESTING
#define M3_VISUALS_TEST_FAIL_NONE 0u
#define M3_VISUALS_TEST_FAIL_RIPPLE_INIT_RADIUS 1u
#define M3_VISUALS_TEST_FAIL_RIPPLE_INIT_OPACITY 2u /* GCOVR_EXCL_LINE */
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
#define M3_VISUALS_TEST_FAIL_RIPPLE_STEP_FADE_RUNNING 13u /* GCOVR_EXCL_LINE   \
                                                           */
#define M3_VISUALS_TEST_FAIL_RIPPLE_STEP_STOP_RADIUS 14u
#define M3_VISUALS_TEST_FAIL_RIPPLE_STEP_STOP_OPACITY 15u
#define M3_VISUALS_TEST_FAIL_RIPPLE_PAINT_RECT 16u
#define M3_VISUALS_TEST_FAIL_RIPPLE_PAINT_COLOR 17u
#define M3_VISUALS_TEST_FAIL_SHADOW_NORM 18u

static m3_u32 g_m3_visuals_test_fail_point = M3_VISUALS_TEST_FAIL_NONE;

static M3Bool m3_visuals_test_fail_point_match(m3_u32 point) {
  if (g_m3_visuals_test_fail_point != point) {
    return M3_FALSE;
  }
  g_m3_visuals_test_fail_point = M3_VISUALS_TEST_FAIL_NONE;
  return M3_TRUE;
}
#endif

static int m3_visuals_validate_rect(const M3Rect *rect) {
  if (rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_visuals_validate_color(const M3Color *color) {
  if (color == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!(color->r >= 0.0f && color->r <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->g >= 0.0f && color->g <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->b >= 0.0f && color->b <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->a >= 0.0f && color->a <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_visuals_validate_gfx(const M3Gfx *gfx) {
  if (gfx == NULL || gfx->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (gfx->vtable->draw_rect == NULL) {
    return M3_ERR_UNSUPPORTED;
  }
  return M3_OK;
}

int M3_CALL m3_ripple_init(M3Ripple *ripple) {
  int rc;

  if (ripple == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_anim_controller_init(&ripple->radius_anim);
#ifdef M3_TESTING
  if (m3_visuals_test_fail_point_match(
          M3_VISUALS_TEST_FAIL_RIPPLE_INIT_RADIUS)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_anim_controller_init(&ripple->opacity_anim);
#ifdef M3_TESTING
  if (m3_visuals_test_fail_point_match(
          M3_VISUALS_TEST_FAIL_RIPPLE_INIT_OPACITY)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  ripple->center_x = 0.0f;
  ripple->center_y = 0.0f;
  ripple->max_radius = 0.0f;
  ripple->radius = 0.0f;
  ripple->opacity = 0.0f;
  ripple->color.r = 0.0f;
  ripple->color.g = 0.0f;
  ripple->color.b = 0.0f;
  ripple->color.a = 0.0f;
  ripple->state = M3_RIPPLE_STATE_IDLE;
  return M3_OK;
}

int M3_CALL m3_ripple_start(M3Ripple *ripple, M3Scalar center_x,
                            M3Scalar center_y, M3Scalar max_radius,
                            M3Scalar expand_duration, M3Color color) {
  int rc;

  if (ripple == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (max_radius <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (expand_duration < 0.0f) {
    return M3_ERR_RANGE;
  }

  rc = m3_visuals_validate_color(&color);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_anim_controller_init(&ripple->radius_anim);
#ifdef M3_TESTING
  if (m3_visuals_test_fail_point_match(
          M3_VISUALS_TEST_FAIL_RIPPLE_START_RADIUS_INIT)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_anim_controller_init(&ripple->opacity_anim);
#ifdef M3_TESTING
  if (m3_visuals_test_fail_point_match(
          M3_VISUALS_TEST_FAIL_RIPPLE_START_OPACITY_INIT)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_anim_controller_start_timing(&ripple->radius_anim, 0.0f, max_radius,
                                       expand_duration, M3_ANIM_EASE_OUT);
#ifdef M3_TESTING
  if (m3_visuals_test_fail_point_match(
          M3_VISUALS_TEST_FAIL_RIPPLE_START_TIMING)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  ripple->center_x = center_x;
  ripple->center_y = center_y;
  ripple->max_radius = max_radius;
  ripple->radius = 0.0f;
  ripple->opacity = 1.0f;
  ripple->opacity_anim.value = 1.0f;
  ripple->opacity_anim.running = M3_FALSE;
  ripple->opacity_anim.mode = M3_ANIM_MODE_NONE;
  ripple->color = color;
  ripple->state = M3_RIPPLE_STATE_EXPANDING;
  return M3_OK;
}

int M3_CALL m3_ripple_release(M3Ripple *ripple, M3Scalar fade_duration) {
  int rc; /* GCOVR_EXCL_LINE */

  if (ripple == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (fade_duration < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (ripple->state == M3_RIPPLE_STATE_IDLE) {
    return M3_ERR_STATE;
  }
  if (ripple->state == M3_RIPPLE_STATE_FADING) {
    return M3_ERR_STATE;
  }

  rc = m3_anim_controller_start_timing(&ripple->opacity_anim, ripple->opacity,
                                       0.0f, fade_duration, M3_ANIM_EASE_OUT);
#ifdef M3_TESTING
  if (m3_visuals_test_fail_point_match(
          M3_VISUALS_TEST_FAIL_RIPPLE_RELEASE_TIMING)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  ripple->state = M3_RIPPLE_STATE_FADING;
  return M3_OK;
}

int M3_CALL m3_ripple_step(M3Ripple *ripple, M3Scalar dt,
                           M3Bool *out_finished) {
  M3Bool running;
  M3Bool finished;
  M3Scalar value;
  int rc;

  if (ripple == NULL || out_finished == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (dt < 0.0f) {
    return M3_ERR_RANGE;
  }

  if (ripple->state == M3_RIPPLE_STATE_IDLE) {
    *out_finished = M3_TRUE;
    return M3_OK;
  }
  if (ripple->state != M3_RIPPLE_STATE_EXPANDING &&
      ripple->state != M3_RIPPLE_STATE_FADING) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_anim_controller_is_running(&ripple->radius_anim, &running);
#ifdef M3_TESTING
  if (m3_visuals_test_fail_point_match(
          M3_VISUALS_TEST_FAIL_RIPPLE_STEP_RADIUS_RUNNING)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }
  if (running) {
    rc = m3_anim_controller_step(&ripple->radius_anim, dt, &value, &finished);
#ifdef M3_TESTING
    if (m3_visuals_test_fail_point_match(
            M3_VISUALS_TEST_FAIL_RIPPLE_STEP_RADIUS_STEP)) {
      rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }
    ripple->radius = value;
    if (finished) {
      ripple->radius = ripple->max_radius;
    }
  } else {
    rc = m3_anim_controller_get_value(&ripple->radius_anim, &value);
#ifdef M3_TESTING
    if (m3_visuals_test_fail_point_match(
            M3_VISUALS_TEST_FAIL_RIPPLE_STEP_RADIUS_VALUE)) {
      rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }
    ripple->radius = value;
  }

  rc = m3_anim_controller_is_running(&ripple->opacity_anim, &running);
#ifdef M3_TESTING
  if (m3_visuals_test_fail_point_match(
          M3_VISUALS_TEST_FAIL_RIPPLE_STEP_OPACITY_RUNNING)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }
  if (running) {
    rc = m3_anim_controller_step(&ripple->opacity_anim, dt, &value, &finished);
#ifdef M3_TESTING
    if (m3_visuals_test_fail_point_match(
            M3_VISUALS_TEST_FAIL_RIPPLE_STEP_OPACITY_STEP)) {
      rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }
    ripple->opacity = value;
  } else {
    rc = m3_anim_controller_get_value(&ripple->opacity_anim, &value);
#ifdef M3_TESTING
    if (m3_visuals_test_fail_point_match(
            M3_VISUALS_TEST_FAIL_RIPPLE_STEP_OPACITY_VALUE)) {
      rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }
    ripple->opacity = value;
  }

  if (ripple->state == M3_RIPPLE_STATE_FADING) {
    rc = m3_anim_controller_is_running(&ripple->opacity_anim, &running);
#ifdef M3_TESTING
    if (m3_visuals_test_fail_point_match(
            M3_VISUALS_TEST_FAIL_RIPPLE_STEP_FADE_RUNNING)) {
      rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }
    if (!running) {
      ripple->opacity = 0.0f;
      rc = m3_anim_controller_stop(&ripple->radius_anim);
#ifdef M3_TESTING
      if (m3_visuals_test_fail_point_match(
              M3_VISUALS_TEST_FAIL_RIPPLE_STEP_STOP_RADIUS)) {
        rc = M3_ERR_IO;
      }
#endif
      if (rc != M3_OK) {
        return rc;
      }
      rc = m3_anim_controller_stop(&ripple->opacity_anim);
#ifdef M3_TESTING
      if (m3_visuals_test_fail_point_match(
              M3_VISUALS_TEST_FAIL_RIPPLE_STEP_STOP_OPACITY)) {
        rc = M3_ERR_IO;
      }
#endif
      if (rc != M3_OK) {
        return rc;
      }
      ripple->state = M3_RIPPLE_STATE_IDLE;
    }
  }

  *out_finished = (ripple->state == M3_RIPPLE_STATE_IDLE) ? M3_TRUE : M3_FALSE;
  return M3_OK;
}

int M3_CALL m3_ripple_is_active(const M3Ripple *ripple, M3Bool *out_active) {
  if (ripple == NULL || out_active == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ripple->state != M3_RIPPLE_STATE_IDLE &&
      ripple->state != M3_RIPPLE_STATE_EXPANDING &&
      ripple->state != M3_RIPPLE_STATE_FADING) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_active = (ripple->state == M3_RIPPLE_STATE_IDLE) ? M3_FALSE : M3_TRUE;
  return M3_OK;
}

int M3_CALL m3_ripple_compute_max_radius(const M3Rect *bounds,
                                         M3Scalar center_x, M3Scalar center_y,
                                         M3Scalar *out_radius) {
  M3Scalar max_radius;
  M3Scalar dx;
  M3Scalar dy;
  M3Scalar dist;
  M3Scalar x0;
  M3Scalar y0;
  M3Scalar x1;
  M3Scalar y1;
  int rc;

  if (bounds == NULL || out_radius == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_visuals_validate_rect(bounds);
  if (rc != M3_OK) {
    return rc;
  }

  x0 = bounds->x;
  y0 = bounds->y;
  x1 = bounds->x + bounds->width;
  y1 = bounds->y + bounds->height;

  dx = center_x - x0;
  dy = center_y - y0;
  max_radius =
      (M3Scalar)sqrt((double)dx * (double)dx + (double)dy * (double)dy);

  dx = center_x - x1;
  dy = center_y - y0;
  dist = (M3Scalar)sqrt((double)dx * (double)dx + (double)dy * (double)dy);
  if (dist > max_radius) {
    max_radius = dist;
  }

  dx = center_x - x0;
  dy = center_y - y1;
  dist = (M3Scalar)sqrt((double)dx * (double)dx + (double)dy * (double)dy);
  if (dist > max_radius) {
    max_radius = dist;
  }

  dx = center_x - x1;
  dy = center_y - y1;
  dist = (M3Scalar)sqrt((double)dx * (double)dx + (double)dy * (double)dy);
  if (dist > max_radius) {
    max_radius = dist;
  }

  *out_radius = max_radius;
  return M3_OK;
}

int M3_CALL m3_ripple_paint(const M3Ripple *ripple, M3Gfx *gfx,
                            const M3Rect *clip, M3Scalar corner_radius) {
  M3Rect rect;
  M3Color color;
  int rc;
  int draw_rc;

  if (ripple == NULL || gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (corner_radius < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (ripple->opacity < 0.0f || ripple->opacity > 1.0f) {
    return M3_ERR_RANGE;
  }
  if (ripple->radius < 0.0f) {
    return M3_ERR_RANGE;
  }

  rc = m3_visuals_validate_gfx(gfx);
  if (rc != M3_OK) {
    return rc;
  }

  if (clip != NULL) {
    rc = m3_visuals_validate_rect(clip);
    if (rc != M3_OK) {
      return rc;
    }
    if (gfx->vtable->push_clip == NULL || gfx->vtable->pop_clip == NULL) {
      return M3_ERR_UNSUPPORTED;
    }
  }

  if (ripple->state == M3_RIPPLE_STATE_IDLE || ripple->opacity <= 0.0f ||
      ripple->radius <= 0.0f) {
    return M3_OK;
  }
  if (ripple->state != M3_RIPPLE_STATE_EXPANDING &&
      ripple->state != M3_RIPPLE_STATE_FADING) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_visuals_validate_color(&ripple->color);
  if (rc != M3_OK) {
    return rc;
  }

  rect.x = ripple->center_x - ripple->radius;
  rect.y = ripple->center_y - ripple->radius;
  rect.width = ripple->radius * 2.0f;
  rect.height = ripple->radius * 2.0f;
#ifdef M3_TESTING
  if (m3_visuals_test_fail_point_match(
          M3_VISUALS_TEST_FAIL_RIPPLE_PAINT_RECT)) {
    rect.width = -1.0f;
  }
#endif

  rc = m3_visuals_validate_rect(&rect);
  if (rc != M3_OK) {
    return rc;
  }

  color = ripple->color;
  color.a = color.a * ripple->opacity;
#ifdef M3_TESTING
  if (m3_visuals_test_fail_point_match(
          M3_VISUALS_TEST_FAIL_RIPPLE_PAINT_COLOR)) {
    color.a = 2.0f;
  }
#endif

  rc = m3_visuals_validate_color(&color);
  if (rc != M3_OK) {
    return rc;
  }

  draw_rc = M3_OK;
  if (clip != NULL) {
    rc = gfx->vtable->push_clip(gfx->ctx, clip);
    if (rc != M3_OK) {
      return rc;
    }
  }

  rc = gfx->vtable->draw_rect(gfx->ctx, &rect, color, corner_radius);
  if (rc != M3_OK) {
    draw_rc = rc;
  }

  if (clip != NULL) {
    rc = gfx->vtable->pop_clip(gfx->ctx);
    if (draw_rc == M3_OK && rc != M3_OK) {
      return rc;
    }
    if (draw_rc != M3_OK) {
      return draw_rc;
    }
  }

  return draw_rc;
}

int M3_CALL m3_shadow_init(M3Shadow *shadow) {
  if (shadow == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  shadow->offset_x = 0.0f;
  shadow->offset_y = 0.0f;
  shadow->blur_radius = 0.0f;
  shadow->spread = 0.0f;
  shadow->corner_radius = 0.0f;
  shadow->layers = 1;
  shadow->color.r = 0.0f;
  shadow->color.g = 0.0f;
  shadow->color.b = 0.0f;
  shadow->color.a = 0.0f;
  return M3_OK;
}

int M3_CALL m3_shadow_set(M3Shadow *shadow, M3Scalar offset_x,
                          M3Scalar offset_y, M3Scalar blur_radius,
                          M3Scalar spread, M3Scalar corner_radius,
                          m3_u32 layers, M3Color color) {
  int rc;

  if (shadow == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (blur_radius < 0.0f || spread < 0.0f || corner_radius < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (layers == 0) {
    return M3_ERR_RANGE;
  }

  rc = m3_visuals_validate_color(&color);
  if (rc != M3_OK) {
    return rc;
  }

  shadow->offset_x = offset_x;
  shadow->offset_y = offset_y;
  shadow->blur_radius = blur_radius;
  shadow->spread = spread;
  shadow->corner_radius = corner_radius;
  shadow->layers = layers;
  shadow->color = color;
  return M3_OK;
}

int M3_CALL m3_shadow_paint(const M3Shadow *shadow, M3Gfx *gfx,
                            const M3Rect *rect, const M3Rect *clip) {
  M3Rect layer_rect;
  M3Color layer_color;
  M3Scalar layer_corner;
  M3Scalar expansion;
  M3Scalar t;
  M3Scalar norm;
  m3_u32 i;
  int rc;
  int draw_rc;

  if (shadow == NULL || gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (shadow->blur_radius < 0.0f || shadow->spread < 0.0f ||
      shadow->corner_radius < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (shadow->layers == 0) {
    return M3_ERR_RANGE;
  }

  rc = m3_visuals_validate_color(&shadow->color);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_visuals_validate_gfx(gfx);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_visuals_validate_rect(rect);
  if (rc != M3_OK) {
    return rc;
  }

  if (clip != NULL) {
    rc = m3_visuals_validate_rect(clip);
    if (rc != M3_OK) {
      return rc;
    }
    if (gfx->vtable->push_clip == NULL || gfx->vtable->pop_clip == NULL) {
      return M3_ERR_UNSUPPORTED;
    }
  }

  if (shadow->color.a <= 0.0f) {
    return M3_OK;
  }

  norm = (M3Scalar)shadow->layers;
  norm = norm * (norm + 1.0f) / 2.0f;
#ifdef M3_TESTING
  if (m3_visuals_test_fail_point_match(M3_VISUALS_TEST_FAIL_SHADOW_NORM)) {
    norm = 0.0f;
  }
#endif
  if (norm <= 0.0f) {
    return M3_ERR_RANGE;
  }

  draw_rc = M3_OK;
  if (clip != NULL) {
    rc = gfx->vtable->push_clip(gfx->ctx, clip);
    if (rc != M3_OK) {
      return rc;
    }
  }

  for (i = 0; i < shadow->layers; ++i) {
    t = ((M3Scalar)i + 1.0f) / (M3Scalar)shadow->layers;
    expansion = shadow->spread + shadow->blur_radius * (1.0f - t);
    layer_rect.x = rect->x + shadow->offset_x - expansion;
    layer_rect.y = rect->y + shadow->offset_y - expansion;
    layer_rect.width = rect->width + expansion * 2.0f;
    layer_rect.height = rect->height + expansion * 2.0f;
    layer_corner = shadow->corner_radius + expansion;
    layer_color = shadow->color;
    layer_color.a = layer_color.a * (((M3Scalar)i + 1.0f) / norm);

    rc = gfx->vtable->draw_rect(gfx->ctx, &layer_rect, layer_color,
                                layer_corner);
    if (rc != M3_OK) {
      draw_rc = rc;
      break;
    }
  }

  if (clip != NULL) {
    rc = gfx->vtable->pop_clip(gfx->ctx);
    if (draw_rc == M3_OK && rc != M3_OK) {
      return rc;
    }
    if (draw_rc != M3_OK) {
      return draw_rc;
    }
  }

  return draw_rc;
}

#ifdef M3_TESTING
int M3_CALL m3_visuals_test_set_fail_point(m3_u32 fail_point) {
  g_m3_visuals_test_fail_point = fail_point;
  return M3_OK;
}

int M3_CALL m3_visuals_test_clear_fail_points(void) {
  g_m3_visuals_test_fail_point = M3_VISUALS_TEST_FAIL_NONE;
  return M3_OK;
}

int M3_CALL m3_visuals_test_validate_rect(const M3Rect *rect) {
  return m3_visuals_validate_rect(rect);
}

int M3_CALL m3_visuals_test_validate_color(const M3Color *color) {
  return m3_visuals_validate_color(color);
}

int M3_CALL m3_visuals_test_validate_gfx(const M3Gfx *gfx) {
  return m3_visuals_validate_gfx(gfx);
}
#endif
