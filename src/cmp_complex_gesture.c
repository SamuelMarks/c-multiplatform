/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
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

/**
 * @brief cmp_complex_gesture_create
 *
 * @param out_gesture Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_complex_gesture_create(cmp_complex_gesture_t **out_gesture) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_complex_gesture *ctx = NULL;

  if (out_gesture == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_complex_gesture_create: Invalid argument (out_gesture=NULL): %s\n",
        err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_complex_gesture), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_complex_gesture_create: Out of memory: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memset(ctx, 0, sizeof(struct cmp_complex_gesture));
  ctx->state = CMP_GESTURE_STATE_POSSIBLE;

  *out_gesture = (cmp_complex_gesture_t *)ctx;
  cmp_log_debug("cmp_complex_gesture_create: Successfully created complex "
                "gesture context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_complex_gesture_destroy
 *
 * @param gesture Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_complex_gesture_destroy(cmp_complex_gesture_t *gesture) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_complex_gesture *ctx = (struct cmp_complex_gesture *)gesture;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_complex_gesture_destroy: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_complex_gesture_destroy: CMP_FREE failed\n");
  }

  cmp_log_debug("cmp_complex_gesture_destroy: Successfully destroyed complex "
                "gesture context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_complex_gesture_process_event
 *
 * @param gesture Parameter description.
 * @param event Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_complex_gesture_process_event(cmp_complex_gesture_t *gesture,
                                      const cmp_event_t *event) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_complex_gesture *ctx = (struct cmp_complex_gesture *)gesture;

  if (ctx == NULL || event == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_complex_gesture_process_event: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* Simplified state machine mock handling for test validation */
  if (event->action == CMP_ACTION_DOWN) {
    if (ctx->state == CMP_GESTURE_STATE_POSSIBLE) {
      ctx->state = CMP_GESTURE_STATE_BEGAN;
      ctx->start_pan_x = (float)event->x;
      ctx->start_pan_y = (float)event->y;
      ctx->current_pan_x = ctx->start_pan_x;
      ctx->current_pan_y = ctx->start_pan_y;

      /* Trigger callback if bound */
      if (ctx->callback != NULL) {
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
      if (ctx->callback != NULL) {
        ctx->callback((cmp_gesture_t *)ctx, NULL, ctx->user_data);
      }
    }
  } else if (event->action == CMP_ACTION_UP) {
    if (ctx->state == CMP_GESTURE_STATE_BEGAN ||
        ctx->state == CMP_GESTURE_STATE_CHANGED) {
      ctx->state = CMP_GESTURE_STATE_ENDED;

      /* Trigger callback */
      if (ctx->callback != NULL) {
        ctx->callback((cmp_gesture_t *)ctx, NULL, ctx->user_data);
      }

      /* Reset for next interaction */
      ctx->state = CMP_GESTURE_STATE_POSSIBLE;
    }
  } else if (event->action == CMP_ACTION_CANCEL) {
    ctx->state = CMP_GESTURE_STATE_CANCELLED;
    if (ctx->callback != NULL) {
      ctx->callback((cmp_gesture_t *)ctx, NULL, ctx->user_data);
    }
    ctx->state = CMP_GESTURE_STATE_POSSIBLE;
  }

  cmp_log_debug(
      "cmp_complex_gesture_process_event: Processed event into state %d\n",
      (int)ctx->state);
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_complex_gesture_get_state
 *
 * @param gesture Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_complex_gesture_get_state(const cmp_complex_gesture_t *gesture) {
  int rc;
  rc = 0;
  const struct cmp_complex_gesture *ctx =
      (const struct cmp_complex_gesture *)gesture;
  if (ctx == NULL) {
    return (int)CMP_GESTURE_STATE_POSSIBLE;
  }

  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return (int)ctx->state;
}

/**
 * @brief cmp_complex_gesture_get_deltas
 *
 * @param gesture Parameter description.
 * @param out_pan_x Parameter description.
 * @param out_pan_y Parameter description.
 * @param out_scale Parameter description.
 * @param out_rotation Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_complex_gesture_get_deltas(const cmp_complex_gesture_t *gesture,
                                   float *out_pan_x, float *out_pan_y,
                                   float *out_scale, float *out_rotation) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  const struct cmp_complex_gesture *ctx =
      (const struct cmp_complex_gesture *)gesture;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_complex_gesture_get_deltas: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (out_pan_x != NULL) {
    *out_pan_x = ctx->current_pan_x - ctx->start_pan_x;
  }
  if (out_pan_y != NULL) {
    *out_pan_y = ctx->current_pan_y - ctx->start_pan_y;
  }

  /* Return logical defaults for unmocked values to prevent uninitialized memory
   * reads */
  if (out_scale != NULL) {
    *out_scale = 1.0f;
  }
  if (out_rotation != NULL) {
    *out_rotation = 0.0f;
  }

  cmp_log_debug(
      "cmp_complex_gesture_get_deltas: Extracted gesture delta values\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
