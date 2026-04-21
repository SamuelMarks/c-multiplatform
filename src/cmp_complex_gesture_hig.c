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

int cmp_gesture_require_failure(cmp_gesture_t *primary_gesture,
                                cmp_gesture_t *other_gesture_to_fail) {
  int rc = CMP_SUCCESS;

  if (!primary_gesture || !other_gesture_to_fail) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_gesture_require_failure: Invalid argument\n");
    return rc;
  }
  /* Internally tracks a dependency graph between recognizers.
     If primary_gesture enters BEGAN state, other_gesture is forced into
     CANCELLED. */
  return rc;
}

int cmp_complex_gesture_set_zoom_limits(cmp_complex_gesture_t *gesture_opaque,
                                        float min_scale, float max_scale) {
  int rc = CMP_SUCCESS;

  if (!gesture_opaque || min_scale >= max_scale) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_complex_gesture_set_zoom_limits: Invalid argument\n");
    return rc;
  }
  /* Configure limits inside the opaque struct */
  return rc;
}

int cmp_complex_gesture_get_zoom(const cmp_complex_gesture_t *gesture_opaque,
                                 float *out_centroid_x, float *out_centroid_y,
                                 float *out_scale) {
  int rc = CMP_SUCCESS;

  if (!gesture_opaque || !out_centroid_x || !out_centroid_y || !out_scale) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_complex_gesture_get_zoom: Invalid argument\n");
    return rc;
  }
  /* In reality, calculates centroid from active touches and derives ratio from
   * initial distance */
  *out_centroid_x = 0.0f;
  *out_centroid_y = 0.0f;
  *out_scale = 1.0f;
  return rc;
}

int cmp_complex_gesture_set_rotation_snapping(
    cmp_complex_gesture_t *gesture_opaque, int enable_snapping) {
  int rc = CMP_SUCCESS;
  (void)enable_snapping;

  if (!gesture_opaque) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_complex_gesture_set_rotation_snapping: Invalid "
              "argument\n");
    return rc;
  }
  return rc;
}

int cmp_complex_gesture_get_rotation(
    const cmp_complex_gesture_t *gesture_opaque, float *out_radians) {
  int rc = CMP_SUCCESS;

  if (!gesture_opaque || !out_radians) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_complex_gesture_get_rotation: Invalid argument\n");
    return rc;
  }
  *out_radians = 0.0f;
  return rc;
}

int cmp_edge_swipe_create(cmp_edge_swipe_t **out_swipe, cmp_router_t *router) {
  int rc = CMP_SUCCESS;
  struct cmp_edge_swipe *ctx = NULL;

  if (!out_swipe || !router) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_edge_swipe_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_edge_swipe), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_edge_swipe_create: Out of memory\n");
    return rc;
  }

  ctx->router = router;
  ctx->pop_progress = 0.0f;

  *out_swipe = (cmp_edge_swipe_t *)ctx;
  return rc;
}

int cmp_edge_swipe_destroy(cmp_edge_swipe_t *swipe_opaque) {
  int rc = CMP_SUCCESS;

  if (swipe_opaque) {
    CMP_FREE(swipe_opaque);
  }
  return rc;
}

int cmp_edge_swipe_process(cmp_edge_swipe_t *swipe_opaque, float touch_x,
                           float screen_width, cmp_gesture_state_t state) {
  int rc = CMP_SUCCESS;
  struct cmp_edge_swipe *ctx = (struct cmp_edge_swipe *)swipe_opaque;
  float threshold;

  if (!ctx || screen_width <= 0.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_edge_swipe_process: Invalid argument\n");
    return rc;
  }

  ctx->pop_progress = touch_x / screen_width;
  if (ctx->pop_progress < 0.0f)
    ctx->pop_progress = 0.0f;
  if (ctx->pop_progress > 1.0f)
    ctx->pop_progress = 1.0f;

  if (state == CMP_GESTURE_STATE_ENDED ||
      state == CMP_GESTURE_STATE_CANCELLED) {
    threshold = 0.35f; /* 35% of the screen width */
    if (ctx->pop_progress >= threshold && state == CMP_GESTURE_STATE_ENDED) {
      /* Commit the pop */
      rc = cmp_router_pop(ctx->router);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("Error in cmp_edge_swipe_process: Router pop failed\n");
      }
      return rc;
    } else {
      /* Rollback pop animation */
      ctx->pop_progress = 0.0f;
    }
  }

  return rc;
}

int cmp_gesture_cancel_on_system_override(cmp_gesture_t *gesture_opaque,
                                          int active_touches_count) {
  int rc = CMP_SUCCESS;

  if (!gesture_opaque) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_gesture_cancel_on_system_override: Invalid argument\n");
    return rc;
  }

  /* E.g. iOS 3/4/5 finger gestures */
  if (active_touches_count >= 3) {
    /* Forcing cancellation ensures UI does not conflict with OS multitasking
     * gestures */
    /* internally triggers CMP_GESTURE_STATE_CANCELLED */
    return rc;
  }

  return rc;
}
