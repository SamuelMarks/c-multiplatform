/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_gesture {
  cmp_gesture_state_t state;
  cmp_gesture_cb_t callback;
  void *user_data;
};

int cmp_gesture_create(cmp_gesture_t **out_gesture) {
  struct cmp_gesture *ctx;

  if (!out_gesture)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_gesture), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(ctx, 0, sizeof(struct cmp_gesture));
  ctx->state = CMP_GESTURE_STATE_POSSIBLE;

  *out_gesture = (cmp_gesture_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_gesture_destroy(cmp_gesture_t *gesture) {
  struct cmp_gesture *ctx = (struct cmp_gesture *)gesture;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_gesture_set_callback(cmp_gesture_t *gesture, cmp_gesture_cb_t callback,
                             void *user_data) {
  struct cmp_gesture *ctx = (struct cmp_gesture *)gesture;
  if (!ctx || !callback)
    return CMP_ERROR_INVALID_ARG;

  ctx->callback = callback;
  ctx->user_data = user_data;

  return CMP_SUCCESS;
}

cmp_gesture_state_t cmp_gesture_get_state(const cmp_gesture_t *gesture) {
  const struct cmp_gesture *ctx = (const struct cmp_gesture *)gesture;
  if (!ctx)
    return CMP_GESTURE_STATE_POSSIBLE;

  return ctx->state;
}

int cmp_gesture_process_event(cmp_gesture_t *gesture,
                              const cmp_event_t *event) {
  struct cmp_gesture *ctx = (struct cmp_gesture *)gesture;

  if (!ctx || !event)
    return CMP_ERROR_INVALID_ARG;

  /* Mock deterministic state progression based on standard action flow */
  if (event->action == CMP_ACTION_DOWN) {
    if (ctx->state == CMP_GESTURE_STATE_POSSIBLE) {
      ctx->state = CMP_GESTURE_STATE_BEGAN;
      if (ctx->callback)
        ctx->callback(gesture, NULL, ctx->user_data);
    }
  } else if (event->action == CMP_ACTION_MOVE) {
    if (ctx->state == CMP_GESTURE_STATE_BEGAN ||
        ctx->state == CMP_GESTURE_STATE_CHANGED) {
      ctx->state = CMP_GESTURE_STATE_CHANGED;
      if (ctx->callback)
        ctx->callback(gesture, NULL, ctx->user_data);
    }
  } else if (event->action == CMP_ACTION_UP) {
    if (ctx->state == CMP_GESTURE_STATE_BEGAN ||
        ctx->state == CMP_GESTURE_STATE_CHANGED) {
      ctx->state = CMP_GESTURE_STATE_ENDED;
      if (ctx->callback)
        ctx->callback(gesture, NULL, ctx->user_data);
      ctx->state = CMP_GESTURE_STATE_POSSIBLE; /* Loop back to idle */
    }
  } else if (event->action == CMP_ACTION_CANCEL) {
    ctx->state = CMP_GESTURE_STATE_CANCELLED;
    if (ctx->callback)
      ctx->callback(gesture, NULL, ctx->user_data);
    ctx->state = CMP_GESTURE_STATE_POSSIBLE; /* Loop back to idle */
  }

  return CMP_SUCCESS;
}
