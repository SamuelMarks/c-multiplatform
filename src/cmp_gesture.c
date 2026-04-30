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

/**
 * @brief cmp_gesture_create
 *
 * @param out_gesture Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gesture_create(cmp_gesture_t **out_gesture) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_gesture *ctx = NULL;

  if (out_gesture == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_gesture_create: Invalid argument (out_gesture=NULL): %s\n",
        err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_gesture), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_gesture_create: Out of memory: %s\n", err_str);

    return rc;
  }

  memset(ctx, 0, sizeof(struct cmp_gesture));
  ctx->state = CMP_GESTURE_STATE_POSSIBLE;

  *out_gesture = (cmp_gesture_t *)ctx;
  cmp_log_debug("cmp_gesture_create: Successfully created gesture context\n");
  return rc;
}

/**
 * @brief cmp_gesture_destroy
 *
 * @param gesture Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gesture_destroy(cmp_gesture_t *gesture) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_gesture *ctx = (struct cmp_gesture *)gesture;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_gesture_destroy: Invalid argument: %s\n", err_str);

    return rc;
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_gesture_destroy: CMP_FREE failed\n");
  }

  cmp_log_debug(
      "cmp_gesture_destroy: Successfully destroyed gesture context\n");
  return rc;
}

/**
 * @brief cmp_gesture_set_callback
 *
 * @param gesture Parameter description.
 * @param callback Parameter description.
 * @param user_data Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gesture_set_callback(cmp_gesture_t *gesture, cmp_gesture_cb_t callback,
                             void *user_data) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_gesture *ctx = (struct cmp_gesture *)gesture;

  if (ctx == NULL || callback == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_gesture_set_callback: Invalid argument: %s\n", err_str);

    return rc;
  }

  ctx->callback = callback;
  ctx->user_data = user_data;

  cmp_log_debug("cmp_gesture_set_callback: Registered callback\n");

  return rc;
}

/**
 * @brief cmp_gesture_get_state
 *
 * @param gesture Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gesture_get_state(const cmp_gesture_t *gesture) {
  int rc = CMP_SUCCESS;
  const struct cmp_gesture *ctx = (const struct cmp_gesture *)gesture;
  if (ctx == NULL) {
    return (int)CMP_GESTURE_STATE_POSSIBLE;
  }

  rc = (int)ctx->state;
  return rc;
}

/**
 * @brief cmp_gesture_process_event
 *
 * @param gesture Parameter description.
 * @param event Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gesture_process_event(cmp_gesture_t *gesture,
                              const cmp_event_t *event) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_gesture *ctx = (struct cmp_gesture *)gesture;

  if (ctx == NULL || event == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_gesture_process_event: Invalid argument: %s\n", err_str);

    return rc;
  }

  /* Mock deterministic state progression based on standard action flow */
  if (event->action == CMP_ACTION_DOWN) {
    if (ctx->state == CMP_GESTURE_STATE_POSSIBLE) {
      ctx->state = CMP_GESTURE_STATE_BEGAN;
      if (ctx->callback != NULL) {
        ctx->callback(gesture, NULL, ctx->user_data);
      }
    }
  } else if (event->action == CMP_ACTION_MOVE) {
    if (ctx->state == CMP_GESTURE_STATE_BEGAN ||
        ctx->state == CMP_GESTURE_STATE_CHANGED) {
      ctx->state = CMP_GESTURE_STATE_CHANGED;
      if (ctx->callback != NULL) {
        ctx->callback(gesture, NULL, ctx->user_data);
      }
    }
  } else if (event->action == CMP_ACTION_UP) {
    if (ctx->state == CMP_GESTURE_STATE_BEGAN ||
        ctx->state == CMP_GESTURE_STATE_CHANGED) {
      ctx->state = CMP_GESTURE_STATE_ENDED;
      if (ctx->callback != NULL) {
        ctx->callback(gesture, NULL, ctx->user_data);
      }
      ctx->state = CMP_GESTURE_STATE_POSSIBLE; /* Loop back to idle */
    }
  } else if (event->action == CMP_ACTION_CANCEL) {
    ctx->state = CMP_GESTURE_STATE_CANCELLED;
    if (ctx->callback != NULL) {
      ctx->callback(gesture, NULL, ctx->user_data);
    }
    ctx->state = CMP_GESTURE_STATE_POSSIBLE; /* Loop back to idle */
  }

  cmp_log_debug("cmp_gesture_process_event: Processed event into state %d\n",
                (int)ctx->state);

  return rc;
}
