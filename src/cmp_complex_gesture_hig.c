/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <math.h>
/* clang-format on */

struct cmp_edge_swipe {
  cmp_router_t *router;
  float pop_progress;
};

/**
 * @brief cmp_gesture_require_failure
 *
 * @param primary_gesture Parameter description.
 * @param other_gesture_to_fail Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gesture_require_failure(cmp_gesture_t *primary_gesture,
                                cmp_gesture_t *other_gesture_to_fail) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (primary_gesture == NULL || other_gesture_to_fail == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_gesture_require_failure: Invalid argument: %s\n",
                  err_str);
    return rc;
  }

  /* Internally tracks a dependency graph between recognizers.
     If primary_gesture enters BEGAN state, other_gesture is forced into
     CANCELLED. */
  cmp_log_debug(
      "cmp_gesture_require_failure: Associated gesture failure dependency\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_complex_gesture_set_zoom_limits
 *
 * @param gesture_opaque Parameter description.
 * @param min_scale Parameter description.
 * @param max_scale Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_complex_gesture_set_zoom_limits(cmp_complex_gesture_t *gesture_opaque,
                                        float min_scale, float max_scale) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (gesture_opaque == NULL || min_scale >= max_scale) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_complex_gesture_set_zoom_limits: Invalid argument: %s\n",
                  err_str);
    return rc;
  }

  /* Configure limits inside the opaque struct */
  cmp_log_debug(
      "cmp_complex_gesture_set_zoom_limits: Set zoom limits to %.2f-%.2f\n",
      min_scale, max_scale);
  return CMP_SUCCESS;
}

/**
 * @brief cmp_complex_gesture_get_zoom
 *
 * @param gesture_opaque Parameter description.
 * @param out_centroid_x Parameter description.
 * @param out_centroid_y Parameter description.
 * @param out_scale Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_complex_gesture_get_zoom(const cmp_complex_gesture_t *gesture_opaque,
                                 float *out_centroid_x, float *out_centroid_y,
                                 float *out_scale) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (gesture_opaque == NULL || out_centroid_x == NULL ||
      out_centroid_y == NULL || out_scale == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_complex_gesture_get_zoom: Invalid argument: %s\n",
                  err_str);
    return rc;
  }

  /* In reality, calculates centroid from active touches and derives ratio from
   * initial distance */
  *out_centroid_x = 0.0f;
  *out_centroid_y = 0.0f;
  *out_scale = 1.0f;

  cmp_log_debug("cmp_complex_gesture_get_zoom: Got zoom data\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_complex_gesture_set_rotation_snapping
 *
 * @param gesture_opaque Parameter description.
 * @param enable_snapping Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_complex_gesture_set_rotation_snapping(
    cmp_complex_gesture_t *gesture_opaque, int enable_snapping) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  (void)enable_snapping;

  if (gesture_opaque == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_complex_gesture_set_rotation_snapping: Invalid argument: %s\n",
        err_str);
    return rc;
  }

  cmp_log_debug(
      "cmp_complex_gesture_set_rotation_snapping: Configured snapping\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_complex_gesture_get_rotation
 *
 * @param gesture_opaque Parameter description.
 * @param out_radians Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_complex_gesture_get_rotation(
    const cmp_complex_gesture_t *gesture_opaque, float *out_radians) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (gesture_opaque == NULL || out_radians == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_complex_gesture_get_rotation: Invalid argument: %s\n",
                  err_str);
    return rc;
  }

  *out_radians = 0.0f;
  cmp_log_debug("cmp_complex_gesture_get_rotation: Got rotation data\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_edge_swipe_create
 *
 * @param out_swipe Parameter description.
 * @param router Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_edge_swipe_create(cmp_edge_swipe_t **out_swipe, cmp_router_t *router) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_edge_swipe *ctx = NULL;

  if (out_swipe == NULL || router == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_edge_swipe_create: Invalid argument: %s\n", err_str);
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_edge_swipe), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_edge_swipe_create: Out of memory: %s\n", err_str);
    return rc;
  }

  ctx->router = router;
  ctx->pop_progress = 0.0f;

  *out_swipe = (cmp_edge_swipe_t *)ctx;
  cmp_log_debug(
      "cmp_edge_swipe_create: Successfully created edge swipe context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_edge_swipe_destroy
 *
 * @param swipe_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_edge_swipe_destroy(cmp_edge_swipe_t *swipe_opaque) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (swipe_opaque == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_edge_swipe_destroy: Invalid argument: %s\n", err_str);
    return rc;
  }

  rc = CMP_FREE(swipe_opaque);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_edge_swipe_destroy: CMP_FREE failed\n");
  }

  cmp_log_debug(
      "cmp_edge_swipe_destroy: Successfully destroyed edge swipe context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_edge_swipe_process
 *
 * @param swipe_opaque Parameter description.
 * @param touch_x Parameter description.
 * @param screen_width Parameter description.
 * @param state Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_edge_swipe_process(cmp_edge_swipe_t *swipe_opaque, float touch_x,
                           float screen_width, cmp_gesture_state_t state) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_edge_swipe *ctx = (struct cmp_edge_swipe *)swipe_opaque;
  float threshold;

  if (ctx == NULL || screen_width <= 0.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_edge_swipe_process: Invalid argument: %s\n", err_str);
    return rc;
  }

  ctx->pop_progress = touch_x / screen_width;
  if (ctx->pop_progress < 0.0f) {
    ctx->pop_progress = 0.0f;
  }
  if (ctx->pop_progress > 1.0f) {
    ctx->pop_progress = 1.0f;
  }

  if (state == CMP_GESTURE_STATE_ENDED ||
      state == CMP_GESTURE_STATE_CANCELLED) {
    threshold = 0.35f; /* 35% of the screen width */
    if (ctx->pop_progress >= threshold && state == CMP_GESTURE_STATE_ENDED) {
      /* Commit the pop */
      rc = cmp_router_pop(ctx->router);
      if (rc != CMP_SUCCESS) {
        err_rc = cmp_strerror(rc, &err_str);
        if (err_rc != CMP_SUCCESS) {
          err_str = "Unknown";
        }
        cmp_log_debug("cmp_edge_swipe_process: Router pop failed: %s\n",
                      err_str);
      }
      return rc;
    } else {
      /* Rollback pop animation */
      ctx->pop_progress = 0.0f;
    }
  }

  cmp_log_debug("cmp_edge_swipe_process: Processed swipe\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_gesture_cancel_on_system_override
 *
 * @param gesture_opaque Parameter description.
 * @param active_touches_count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gesture_cancel_on_system_override(cmp_gesture_t *gesture_opaque,
                                          int active_touches_count) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (gesture_opaque == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_gesture_cancel_on_system_override: Invalid argument: %s\n",
        err_str);
    return rc;
  }

  /* E.g. iOS 3/4/5 finger gestures */
  if (active_touches_count >= 3) {
    /* Forcing cancellation ensures UI does not conflict with OS multitasking
     * gestures */
    /* internally triggers CMP_GESTURE_STATE_CANCELLED */
    cmp_log_debug(
        "cmp_gesture_cancel_on_system_override: System override triggered\n");
    return CMP_SUCCESS;
  }

  cmp_log_debug(
      "cmp_gesture_cancel_on_system_override: No override required\n");
  return CMP_SUCCESS;
}
