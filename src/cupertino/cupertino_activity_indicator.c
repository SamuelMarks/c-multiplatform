/* clang-format off */
#include "cupertino/cupertino_activity_indicator.h"
#include "cupertino/cupertino_color.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL
cupertino_activity_indicator_init(CupertinoActivityIndicator *indicator) {
  if (indicator == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(indicator, 0, sizeof(*indicator));

  indicator->bounds.x = 0;
  indicator->bounds.y = 0;
  indicator->bounds.width = 20;
  indicator->bounds.height = 20;

  indicator->size = CUPERTINO_ACTIVITY_INDICATOR_SIZE_MEDIUM;
  cupertino_color_get_system(CUPERTINO_COLOR_GRAY, CMP_FALSE,
                             &indicator->color);

  indicator->is_animating = CMP_FALSE;
  indicator->hides_when_stopped = CMP_TRUE;
  indicator->is_dark_mode = CMP_FALSE;
  indicator->animation_time = 0.0;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_activity_indicator_start_animating(
    CupertinoActivityIndicator *indicator) {
  if (indicator == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  indicator->is_animating = CMP_TRUE;
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_activity_indicator_stop_animating(
    CupertinoActivityIndicator *indicator) {
  if (indicator == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  indicator->is_animating = CMP_FALSE;
  /* Often we reset the timer so it starts from the same position next time */
  indicator->animation_time = 0.0;
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_activity_indicator_update(
    CupertinoActivityIndicator *indicator, double delta_time) {
  if (indicator == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (indicator->is_animating) {
    indicator->animation_time += delta_time;
    /* Cycle repeats ~every 1 second. 12 spokes, discrete stepping. */
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_activity_indicator_paint(
    const CupertinoActivityIndicator *indicator, CMPPaintContext *ctx) {
  int i;
  int num_spokes = 12;
  int current_step;
  double period = 1.0;
  CMPScalar center_x;
  CMPScalar center_y;
  CMPScalar spoke_length;
  CMPScalar spoke_width;
  CMPScalar inner_radius;
  CMPColor draw_color;

  if (indicator == NULL || ctx == NULL || ctx->gfx == NULL ||
      ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (!indicator->is_animating && indicator->hides_when_stopped) {
    return CMP_OK; /* Do not render anything */
  }

  center_x = indicator->bounds.x + indicator->bounds.width / 2.0f;
  center_y = indicator->bounds.y + indicator->bounds.height / 2.0f;

  /* Base sizing depending on variant */
  if (indicator->size == CUPERTINO_ACTIVITY_INDICATOR_SIZE_LARGE) {
    inner_radius = 8.0f;
    spoke_length = 8.0f;
    spoke_width = 3.0f;
  } else {
    inner_radius = 4.0f;
    spoke_length = 4.0f;
    spoke_width = 2.0f;
  }

  draw_color = indicator->color;
  if (indicator->is_dark_mode) {
    cupertino_color_get_system(CUPERTINO_COLOR_GRAY, CMP_TRUE, &draw_color);
  }

  /* iOS spinner spins clockwise and has discrete "steps" (ticks), not perfectly
   * smooth rotation */
  /* Modulo division using while loop for C89 compatibility without fmod */
  {
    double time = indicator->animation_time;
    while (time >= period) {
      time -= period;
    }
    current_step = (int)((time / period) * num_spokes);
  }

  for (i = 0; i < num_spokes; i++) {
    CMPRect spoke_rect;
    CMPScalar angle;
    int diff;
    CMPScalar alpha;

    angle = ((CMPScalar)i / (CMPScalar)num_spokes) *
            (CMPScalar)(2.0 * 3.1415926535);

    /* Calculate opacity for this spoke.
     * The "head" of the spinner is the current step. Opacity fades out
     * counter-clockwise. */
    diff = i - current_step;
    if (diff < 0) {
      diff += num_spokes;
    }

    /* 'diff' is 0 for head, goes up to 11 for the tail */
    alpha = 1.0f - ((CMPScalar)diff / (CMPScalar)num_spokes);

    /* The tail shouldn't completely disappear in the default Apple design, base
     * opacity ~0.2 */
    if (alpha < 0.2f) {
      alpha = 0.2f;
    }

    spoke_rect.x = center_x - spoke_width / 2.0f;
    spoke_rect.y = center_y + inner_radius;
    spoke_rect.width = spoke_width;
    spoke_rect.height = spoke_length;

    {
      CMPColor spoke_color = draw_color;
      spoke_color.a *= alpha;

      if (ctx->gfx->vtable->draw_rect) {
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &spoke_rect, spoke_color,
                                    spoke_width / 2.0f);
      }
    }
  }

  return CMP_OK;
}
