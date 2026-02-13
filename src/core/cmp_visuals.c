#include "cmpc/cmp_visuals.h"

#include <math.h>

#ifdef CMP_TESTING
#define CMP_VISUALS_TEST_FAIL_NONE 0u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_INIT_RADIUS 1u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_INIT_OPACITY 2u /* GCOVR_EXCL_LINE */
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
#define CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_FADE_RUNNING                         \
  13u /* GCOVR_EXCL_LINE                                                       \
       */
#define CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_STOP_RADIUS 14u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_STOP_OPACITY 15u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_PAINT_RECT 16u
#define CMP_VISUALS_TEST_FAIL_RIPPLE_PAINT_COLOR 17u
#define CMP_VISUALS_TEST_FAIL_SHADOW_NORM 18u
#define CMP_VISUALS_TEST_FAIL_SHADOW_INIT 19u

static cmp_u32 g_cmp_visuals_test_fail_point = CMP_VISUALS_TEST_FAIL_NONE;

static CMPBool cmp_visuals_test_fail_point_match(cmp_u32 point) {
  if (g_cmp_visuals_test_fail_point != point) {
    return CMP_FALSE;
  }
  g_cmp_visuals_test_fail_point = CMP_VISUALS_TEST_FAIL_NONE;
  return CMP_TRUE;
}
#endif

static int cmp_visuals_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_visuals_validate_color(const CMPColor *color) {
  if (color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!(color->r >= 0.0f && color->r <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->g >= 0.0f && color->g <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->b >= 0.0f && color->b <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->a >= 0.0f && color->a <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_visuals_validate_gfx(const CMPGfx *gfx) {
  if (gfx == NULL || gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (gfx->vtable->draw_rect == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }
  return CMP_OK;
}

int CMP_CALL cmp_ripple_init(CMPRipple *ripple) {
  int rc;

  if (ripple == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_anim_controller_init(&ripple->radius_anim);
#ifdef CMP_TESTING
  if (cmp_visuals_test_fail_point_match(
          CMP_VISUALS_TEST_FAIL_RIPPLE_INIT_RADIUS)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_anim_controller_init(&ripple->opacity_anim);
#ifdef CMP_TESTING
  if (cmp_visuals_test_fail_point_match(
          CMP_VISUALS_TEST_FAIL_RIPPLE_INIT_OPACITY)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
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
  ripple->state = CMP_RIPPLE_STATE_IDLE;
  return CMP_OK;
}

int CMP_CALL cmp_ripple_start(CMPRipple *ripple, CMPScalar center_x,
                              CMPScalar center_y, CMPScalar max_radius,
                              CMPScalar expand_duration, CMPColor color) {
  int rc;

  if (ripple == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (max_radius <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (expand_duration < 0.0f) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_visuals_validate_color(&color);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_anim_controller_init(&ripple->radius_anim);
#ifdef CMP_TESTING
  if (cmp_visuals_test_fail_point_match(
          CMP_VISUALS_TEST_FAIL_RIPPLE_START_RADIUS_INIT)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_anim_controller_init(&ripple->opacity_anim);
#ifdef CMP_TESTING
  if (cmp_visuals_test_fail_point_match(
          CMP_VISUALS_TEST_FAIL_RIPPLE_START_OPACITY_INIT)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_anim_controller_start_timing(&ripple->radius_anim, 0.0f, max_radius,
                                        expand_duration, CMP_ANIM_EASE_OUT);
#ifdef CMP_TESTING
  if (cmp_visuals_test_fail_point_match(
          CMP_VISUALS_TEST_FAIL_RIPPLE_START_TIMING)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  ripple->center_x = center_x;
  ripple->center_y = center_y;
  ripple->max_radius = max_radius;
  ripple->radius = 0.0f;
  ripple->opacity = 1.0f;
  ripple->opacity_anim.value = 1.0f;
  ripple->opacity_anim.running = CMP_FALSE;
  ripple->opacity_anim.mode = CMP_ANIM_MODE_NONE;
  ripple->color = color;
  ripple->state = CMP_RIPPLE_STATE_EXPANDING;
  return CMP_OK;
}

int CMP_CALL cmp_ripple_release(CMPRipple *ripple, CMPScalar fade_duration) {
  int rc; /* GCOVR_EXCL_LINE */

  if (ripple == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (fade_duration < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (ripple->state == CMP_RIPPLE_STATE_IDLE) {
    return CMP_ERR_STATE;
  }
  if (ripple->state == CMP_RIPPLE_STATE_FADING) {
    return CMP_ERR_STATE;
  }

  rc = cmp_anim_controller_start_timing(&ripple->opacity_anim, ripple->opacity,
                                        0.0f, fade_duration, CMP_ANIM_EASE_OUT);
#ifdef CMP_TESTING
  if (cmp_visuals_test_fail_point_match(
          CMP_VISUALS_TEST_FAIL_RIPPLE_RELEASE_TIMING)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  ripple->state = CMP_RIPPLE_STATE_FADING;
  return CMP_OK;
}

int CMP_CALL cmp_ripple_step(CMPRipple *ripple, CMPScalar dt,
                             CMPBool *out_finished) {
  CMPBool running;
  CMPBool finished;
  CMPScalar value;
  int rc;

  if (ripple == NULL || out_finished == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (dt < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (ripple->state == CMP_RIPPLE_STATE_IDLE) {
    *out_finished = CMP_TRUE;
    return CMP_OK;
  }
  if (ripple->state != CMP_RIPPLE_STATE_EXPANDING &&
      ripple->state != CMP_RIPPLE_STATE_FADING) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_anim_controller_is_running(&ripple->radius_anim, &running);
#ifdef CMP_TESTING
  if (cmp_visuals_test_fail_point_match(
          CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_RADIUS_RUNNING)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }
  if (running) {
    rc = cmp_anim_controller_step(&ripple->radius_anim, dt, &value, &finished);
#ifdef CMP_TESTING
    if (cmp_visuals_test_fail_point_match(
            CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_RADIUS_STEP)) {
      rc = CMP_ERR_IO;
    }
#endif
    if (rc != CMP_OK) {
      return rc;
    }
    ripple->radius = value;
    if (finished) {
      ripple->radius = ripple->max_radius;
    }
  } else {
    rc = cmp_anim_controller_get_value(&ripple->radius_anim, &value);
#ifdef CMP_TESTING
    if (cmp_visuals_test_fail_point_match(
            CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_RADIUS_VALUE)) {
      rc = CMP_ERR_IO;
    }
#endif
    if (rc != CMP_OK) {
      return rc;
    }
    ripple->radius = value;
  }

  rc = cmp_anim_controller_is_running(&ripple->opacity_anim, &running);
#ifdef CMP_TESTING
  if (cmp_visuals_test_fail_point_match(
          CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_OPACITY_RUNNING)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }
  if (running) {
    rc = cmp_anim_controller_step(&ripple->opacity_anim, dt, &value, &finished);
#ifdef CMP_TESTING
    if (cmp_visuals_test_fail_point_match(
            CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_OPACITY_STEP)) {
      rc = CMP_ERR_IO;
    }
#endif
    if (rc != CMP_OK) {
      return rc;
    }
    ripple->opacity = value;
  } else {
    rc = cmp_anim_controller_get_value(&ripple->opacity_anim, &value);
#ifdef CMP_TESTING
    if (cmp_visuals_test_fail_point_match(
            CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_OPACITY_VALUE)) {
      rc = CMP_ERR_IO;
    }
#endif
    if (rc != CMP_OK) {
      return rc;
    }
    ripple->opacity = value;
  }

  if (ripple->state == CMP_RIPPLE_STATE_FADING) {
    rc = cmp_anim_controller_is_running(&ripple->opacity_anim, &running);
#ifdef CMP_TESTING
    if (cmp_visuals_test_fail_point_match(
            CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_FADE_RUNNING)) {
      rc = CMP_ERR_IO;
    }
#endif
    if (rc != CMP_OK) {
      return rc;
    }
    if (!running) {
      ripple->opacity = 0.0f;
      rc = cmp_anim_controller_stop(&ripple->radius_anim);
#ifdef CMP_TESTING
      if (cmp_visuals_test_fail_point_match(
              CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_STOP_RADIUS)) {
        rc = CMP_ERR_IO;
      }
#endif
      if (rc != CMP_OK) {
        return rc;
      }
      rc = cmp_anim_controller_stop(&ripple->opacity_anim);
#ifdef CMP_TESTING
      if (cmp_visuals_test_fail_point_match(
              CMP_VISUALS_TEST_FAIL_RIPPLE_STEP_STOP_OPACITY)) {
        rc = CMP_ERR_IO;
      }
#endif
      if (rc != CMP_OK) {
        return rc;
      }
      ripple->state = CMP_RIPPLE_STATE_IDLE;
    }
  }

  *out_finished =
      (ripple->state == CMP_RIPPLE_STATE_IDLE) ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_ripple_is_active(const CMPRipple *ripple,
                                  CMPBool *out_active) {
  if (ripple == NULL || out_active == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ripple->state != CMP_RIPPLE_STATE_IDLE &&
      ripple->state != CMP_RIPPLE_STATE_EXPANDING &&
      ripple->state != CMP_RIPPLE_STATE_FADING) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_active = (ripple->state == CMP_RIPPLE_STATE_IDLE) ? CMP_FALSE : CMP_TRUE;
  return CMP_OK;
}

int CMP_CALL cmp_ripple_compute_max_radius(const CMPRect *bounds,
                                           CMPScalar center_x,
                                           CMPScalar center_y,
                                           CMPScalar *out_radius) {
  CMPScalar max_radius;
  CMPScalar dx;
  CMPScalar dy;
  CMPScalar dist;
  CMPScalar x0;
  CMPScalar y0;
  CMPScalar x1;
  CMPScalar y1;
  int rc;

  if (bounds == NULL || out_radius == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_visuals_validate_rect(bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  x0 = bounds->x;
  y0 = bounds->y;
  x1 = bounds->x + bounds->width;
  y1 = bounds->y + bounds->height;

  dx = center_x - x0;
  dy = center_y - y0;
  max_radius =
      (CMPScalar)sqrt((double)dx * (double)dx + (double)dy * (double)dy);

  dx = center_x - x1;
  dy = center_y - y0;
  dist = (CMPScalar)sqrt((double)dx * (double)dx + (double)dy * (double)dy);
  if (dist > max_radius) {
    max_radius = dist;
  }

  dx = center_x - x0;
  dy = center_y - y1;
  dist = (CMPScalar)sqrt((double)dx * (double)dx + (double)dy * (double)dy);
  if (dist > max_radius) {
    max_radius = dist;
  }

  dx = center_x - x1;
  dy = center_y - y1;
  dist = (CMPScalar)sqrt((double)dx * (double)dx + (double)dy * (double)dy);
  if (dist > max_radius) {
    max_radius = dist;
  }

  *out_radius = max_radius;
  return CMP_OK;
}

int CMP_CALL cmp_ripple_paint(const CMPRipple *ripple, CMPGfx *gfx,
                              const CMPRect *clip, CMPScalar corner_radius) {
  CMPRect rect;
  CMPColor color;
  int rc;
  int draw_rc;

  if (ripple == NULL || gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (corner_radius < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (ripple->opacity < 0.0f || ripple->opacity > 1.0f) {
    return CMP_ERR_RANGE;
  }
  if (ripple->radius < 0.0f) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_visuals_validate_gfx(gfx);
  if (rc != CMP_OK) {
    return rc;
  }

  if (clip != NULL) {
    rc = cmp_visuals_validate_rect(clip);
    if (rc != CMP_OK) {
      return rc;
    }
    if (gfx->vtable->push_clip == NULL || gfx->vtable->pop_clip == NULL) {
      return CMP_ERR_UNSUPPORTED;
    }
  }

  if (ripple->state == CMP_RIPPLE_STATE_IDLE || ripple->opacity <= 0.0f ||
      ripple->radius <= 0.0f) {
    return CMP_OK;
  }
  if (ripple->state != CMP_RIPPLE_STATE_EXPANDING &&
      ripple->state != CMP_RIPPLE_STATE_FADING) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_visuals_validate_color(&ripple->color);
  if (rc != CMP_OK) {
    return rc;
  }

  rect.x = ripple->center_x - ripple->radius;
  rect.y = ripple->center_y - ripple->radius;
  rect.width = ripple->radius * 2.0f;
  rect.height = ripple->radius * 2.0f;
#ifdef CMP_TESTING
  if (cmp_visuals_test_fail_point_match(
          CMP_VISUALS_TEST_FAIL_RIPPLE_PAINT_RECT)) {
    rect.width = -1.0f;
  }
#endif

  rc = cmp_visuals_validate_rect(&rect);
  if (rc != CMP_OK) {
    return rc;
  }

  color = ripple->color;
  color.a = color.a * ripple->opacity;
#ifdef CMP_TESTING
  if (cmp_visuals_test_fail_point_match(
          CMP_VISUALS_TEST_FAIL_RIPPLE_PAINT_COLOR)) {
    color.a = 2.0f;
  }
#endif

  rc = cmp_visuals_validate_color(&color);
  if (rc != CMP_OK) {
    return rc;
  }

  draw_rc = CMP_OK;
  if (clip != NULL) {
    rc = gfx->vtable->push_clip(gfx->ctx, clip);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = gfx->vtable->draw_rect(gfx->ctx, &rect, color, corner_radius);
  if (rc != CMP_OK) {
    draw_rc = rc;
  }

  if (clip != NULL) {
    rc = gfx->vtable->pop_clip(gfx->ctx);
    if (draw_rc == CMP_OK && rc != CMP_OK) {
      return rc;
    }
    if (draw_rc != CMP_OK) {
      return draw_rc;
    }
  }

  return draw_rc;
}

int CMP_CALL cmp_shadow_init(CMPShadow *shadow) {
  if (shadow == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#ifdef CMP_TESTING
  if (cmp_visuals_test_fail_point_match(CMP_VISUALS_TEST_FAIL_SHADOW_INIT)) {
    return CMP_ERR_IO;
  }
#endif

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
  return CMP_OK;
}

int CMP_CALL cmp_shadow_set(CMPShadow *shadow, CMPScalar offset_x,
                            CMPScalar offset_y, CMPScalar blur_radius,
                            CMPScalar spread, CMPScalar corner_radius,
                            cmp_u32 layers, CMPColor color) {
  int rc;

  if (shadow == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (blur_radius < 0.0f || spread < 0.0f || corner_radius < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (layers == 0) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_visuals_validate_color(&color);
  if (rc != CMP_OK) {
    return rc;
  }

  shadow->offset_x = offset_x;
  shadow->offset_y = offset_y;
  shadow->blur_radius = blur_radius;
  shadow->spread = spread;
  shadow->corner_radius = corner_radius;
  shadow->layers = layers;
  shadow->color = color;
  return CMP_OK;
}

int CMP_CALL cmp_shadow_paint(const CMPShadow *shadow, CMPGfx *gfx,
                              const CMPRect *rect, const CMPRect *clip) {
  CMPRect layer_rect;
  CMPColor layer_color;
  CMPScalar layer_corner;
  CMPScalar expansion;
  CMPScalar t;
  CMPScalar norm;
  cmp_u32 i;
  int rc;
  int draw_rc;

  if (shadow == NULL || gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (shadow->blur_radius < 0.0f || shadow->spread < 0.0f ||
      shadow->corner_radius < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (shadow->layers == 0) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_visuals_validate_color(&shadow->color);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_visuals_validate_gfx(gfx);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_visuals_validate_rect(rect);
  if (rc != CMP_OK) {
    return rc;
  }

  if (clip != NULL) {
    rc = cmp_visuals_validate_rect(clip);
    if (rc != CMP_OK) {
      return rc;
    }
    if (gfx->vtable->push_clip == NULL || gfx->vtable->pop_clip == NULL) {
      return CMP_ERR_UNSUPPORTED;
    }
  }

  if (shadow->color.a <= 0.0f) {
    return CMP_OK;
  }

  norm = (CMPScalar)shadow->layers;
  norm = norm * (norm + 1.0f) / 2.0f;
#ifdef CMP_TESTING
  if (cmp_visuals_test_fail_point_match(CMP_VISUALS_TEST_FAIL_SHADOW_NORM)) {
    norm = 0.0f;
  }
#endif
  if (norm <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  draw_rc = CMP_OK;
  if (clip != NULL) {
    rc = gfx->vtable->push_clip(gfx->ctx, clip);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  for (i = 0; i < shadow->layers; ++i) {
    t = ((CMPScalar)i + 1.0f) / (CMPScalar)shadow->layers;
    expansion = shadow->spread + shadow->blur_radius * (1.0f - t);
    layer_rect.x = rect->x + shadow->offset_x - expansion;
    layer_rect.y = rect->y + shadow->offset_y - expansion;
    layer_rect.width = rect->width + expansion * 2.0f;
    layer_rect.height = rect->height + expansion * 2.0f;
    layer_corner = shadow->corner_radius + expansion;
    layer_color = shadow->color;
    layer_color.a = layer_color.a * (((CMPScalar)i + 1.0f) / norm);

    rc = gfx->vtable->draw_rect(gfx->ctx, &layer_rect, layer_color,
                                layer_corner);
    if (rc != CMP_OK) {
      draw_rc = rc;
      break;
    }
  }

  if (clip != NULL) {
    rc = gfx->vtable->pop_clip(gfx->ctx);
    if (draw_rc == CMP_OK && rc != CMP_OK) {
      return rc;
    }
    if (draw_rc != CMP_OK) {
      return draw_rc;
    }
  }

  return draw_rc;
}

#ifdef CMP_TESTING
int CMP_CALL cmp_visuals_test_set_fail_point(cmp_u32 fail_point) {
  g_cmp_visuals_test_fail_point = fail_point;
  return CMP_OK;
}

int CMP_CALL cmp_visuals_test_clear_fail_points(void) {
  g_cmp_visuals_test_fail_point = CMP_VISUALS_TEST_FAIL_NONE;
  return CMP_OK;
}

int CMP_CALL cmp_visuals_test_validate_rect(const CMPRect *rect) {
  return cmp_visuals_validate_rect(rect);
}

int CMP_CALL cmp_visuals_test_validate_color(const CMPColor *color) {
  return cmp_visuals_validate_color(color);
}

int CMP_CALL cmp_visuals_test_validate_gfx(const CMPGfx *gfx) {
  return cmp_visuals_validate_gfx(gfx);
}
#endif
