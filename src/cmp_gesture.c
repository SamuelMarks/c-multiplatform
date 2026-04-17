/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_gesture {
  cmp_gesture_state_t state;
  cmp_gesture_cb_t callback;
  void *user_data;
};

int cmp_gesture_create(cmp_gesture_t **out_gesture) {
  int rc = CMP_SUCCESS;
  struct cmp_gesture *ctx = NULL;

  if (!out_gesture) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_gesture_create: Invalid argument (out_gesture=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_gesture), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_gesture_create: Out of memory\n");
    return rc;
  }

  memset(ctx, 0, sizeof(struct cmp_gesture));
  ctx->state = CMP_GESTURE_STATE_POSSIBLE;

  *out_gesture = (cmp_gesture_t *)ctx;
  return rc;
}

int cmp_gesture_destroy(cmp_gesture_t *gesture) {
  int rc = CMP_SUCCESS;
  struct cmp_gesture *ctx = (struct cmp_gesture *)gesture;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_gesture_destroy: Invalid argument (gesture=NULL)\n");
    return rc;
  }

  CMP_FREE(ctx);
  return rc;
}

int cmp_gesture_set_callback(cmp_gesture_t *gesture, cmp_gesture_cb_t callback,
                             void *user_data) {
  int rc = CMP_SUCCESS;
  struct cmp_gesture *ctx = (struct cmp_gesture *)gesture;

  if (!ctx || !callback) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_gesture_set_callback: Invalid argument\n");
    return rc;
  }

  ctx->callback = callback;
  ctx->user_data = user_data;

  return rc;
}

int cmp_gesture_get_state(const cmp_gesture_t *gesture) {
  const struct cmp_gesture *ctx = (const struct cmp_gesture *)gesture;
  if (!ctx)
    return (int)CMP_GESTURE_STATE_POSSIBLE;

  return (int)ctx->state;
}

int cmp_gesture_process_event(cmp_gesture_t *gesture,
                              const cmp_event_t *event) {
  int rc = CMP_SUCCESS;
  struct cmp_gesture *ctx = (struct cmp_gesture *)gesture;

  if (!ctx || !event) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_gesture_process_event: Invalid argument\n");
    return rc;
  }

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

  return rc;
}
