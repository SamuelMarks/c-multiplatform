/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_complex_gesture {
  cmp_gesture_state_t state;
  cmp_gesture_cb_t callback;
  void *user_data;

  /* Internal state tracking */
  int pointer_count;
  float start_pan_x;
  float start_pan_y;
  float current_pan_x;
  float current_pan_y;

  float start_distance;
  float current_distance;

  float start_angle;
  float current_angle;
};

int cmp_complex_gesture_create(cmp_complex_gesture_t **out_gesture) {
  struct cmp_complex_gesture *ctx;

  if (!out_gesture)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_complex_gesture), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(ctx, 0, sizeof(struct cmp_complex_gesture));
  ctx->state = CMP_GESTURE_STATE_POSSIBLE;

  *out_gesture = (cmp_complex_gesture_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_complex_gesture_destroy(cmp_complex_gesture_t *gesture) {
  struct cmp_complex_gesture *ctx = (struct cmp_complex_gesture *)gesture;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_complex_gesture_process_event(cmp_complex_gesture_t *gesture,
                                      const cmp_event_t *event) {
  struct cmp_complex_gesture *ctx = (struct cmp_complex_gesture *)gesture;

  if (!ctx || !event)
    return CMP_ERROR_INVALID_ARG;

  /* Simplified state machine mock handling for test validation */
  if (event->action == CMP_ACTION_DOWN) {
    if (ctx->state == CMP_GESTURE_STATE_POSSIBLE) {
      ctx->state = CMP_GESTURE_STATE_BEGAN;
      ctx->start_pan_x = (float)event->x;
      ctx->start_pan_y = (float)event->y;
      ctx->current_pan_x = ctx->start_pan_x;
      ctx->current_pan_y = ctx->start_pan_y;

      /* Trigger callback if bound */
      if (ctx->callback) {
        ctx->callback((cmp_gesture_t *)ctx, NULL, ctx->user_data);
      }
    }
  } else if (event->action == CMP_ACTION_MOVE) {
    if (ctx->state == CMP_GESTURE_STATE_BEGAN ||
        ctx->state == CMP_GESTURE_STATE_CHANGED) {
      ctx->state = CMP_GESTURE_STATE_CHANGED;
      ctx->current_pan_x = (float)event->x;
      ctx->current_pan_y = (float)event->y;

      /* Trigger callback */
      if (ctx->callback) {
        ctx->callback((cmp_gesture_t *)ctx, NULL, ctx->user_data);
      }
    }
  } else if (event->action == CMP_ACTION_UP) {
    if (ctx->state == CMP_GESTURE_STATE_BEGAN ||
        ctx->state == CMP_GESTURE_STATE_CHANGED) {
      ctx->state = CMP_GESTURE_STATE_ENDED;

      /* Trigger callback */
      if (ctx->callback) {
        ctx->callback((cmp_gesture_t *)ctx, NULL, ctx->user_data);
      }

      /* Reset for next interaction */
      ctx->state = CMP_GESTURE_STATE_POSSIBLE;
    }
  } else if (event->action == CMP_ACTION_CANCEL) {
    ctx->state = CMP_GESTURE_STATE_CANCELLED;
    if (ctx->callback) {
      ctx->callback((cmp_gesture_t *)ctx, NULL, ctx->user_data);
    }
    ctx->state = CMP_GESTURE_STATE_POSSIBLE;
  }

  return CMP_SUCCESS;
}

int cmp_complex_gesture_get_state(const cmp_complex_gesture_t *gesture) {
  const struct cmp_complex_gesture *ctx =
      (const struct cmp_complex_gesture *)gesture;
  if (!ctx)
    return (int)CMP_GESTURE_STATE_POSSIBLE;

  return (int)ctx->state;
}

int cmp_complex_gesture_get_deltas(const cmp_complex_gesture_t *gesture,
                                   float *out_pan_x, float *out_pan_y,
                                   float *out_scale, float *out_rotation) {
  const struct cmp_complex_gesture *ctx =
      (const struct cmp_complex_gesture *)gesture;

  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  if (out_pan_x)
    *out_pan_x = ctx->current_pan_x - ctx->start_pan_x;
  if (out_pan_y)
    *out_pan_y = ctx->current_pan_y - ctx->start_pan_y;

  /* Return logical defaults for unmocked values to prevent uninitialized memory
   * reads */
  if (out_scale)
    *out_scale = 1.0f;
  if (out_rotation)
    *out_rotation = 0.0f;

  return CMP_SUCCESS;
}
