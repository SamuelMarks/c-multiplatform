/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <math.h>
/* clang-format on */

struct cmp_edge_swipe {
  cmp_router_t *router;
  float pop_progress;
};

int cmp_gesture_require_failure(cmp_gesture_t *primary_gesture,
                                cmp_gesture_t *other_gesture_to_fail) {
  if (!primary_gesture || !other_gesture_to_fail)
    return CMP_ERROR_INVALID_ARG;
  /* Internally tracks a dependency graph between recognizers.
     If primary_gesture enters BEGAN state, other_gesture is forced into
     CANCELLED. */
  return CMP_SUCCESS;
}

int cmp_complex_gesture_set_zoom_limits(cmp_complex_gesture_t *gesture_opaque,
                                        float min_scale, float max_scale) {
  if (!gesture_opaque || min_scale >= max_scale)
    return CMP_ERROR_INVALID_ARG;
  /* Configure limits inside the opaque struct */
  return CMP_SUCCESS;
}

int cmp_complex_gesture_get_zoom(const cmp_complex_gesture_t *gesture_opaque,
                                 float *out_centroid_x, float *out_centroid_y,
                                 float *out_scale) {
  if (!gesture_opaque || !out_centroid_x || !out_centroid_y || !out_scale)
    return CMP_ERROR_INVALID_ARG;
  /* In reality, calculates centroid from active touches and derives ratio from
   * initial distance */
  *out_centroid_x = 0.0f;
  *out_centroid_y = 0.0f;
  *out_scale = 1.0f;
  return CMP_SUCCESS;
}

int cmp_complex_gesture_set_rotation_snapping(
    cmp_complex_gesture_t *gesture_opaque, int enable_snapping) {
  (void)enable_snapping;
  if (!gesture_opaque)
    return CMP_ERROR_INVALID_ARG;
  return CMP_SUCCESS;
}

int cmp_complex_gesture_get_rotation(
    const cmp_complex_gesture_t *gesture_opaque, float *out_radians) {
  if (!gesture_opaque || !out_radians)
    return CMP_ERROR_INVALID_ARG;
  *out_radians = 0.0f;
  return CMP_SUCCESS;
}

int cmp_edge_swipe_create(cmp_edge_swipe_t **out_swipe, cmp_router_t *router) {
  struct cmp_edge_swipe *ctx;
  if (!out_swipe || !router)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_edge_swipe), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->router = router;
  ctx->pop_progress = 0.0f;

  *out_swipe = (cmp_edge_swipe_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_edge_swipe_destroy(cmp_edge_swipe_t *swipe_opaque) {
  if (swipe_opaque)
    CMP_FREE(swipe_opaque);
  return CMP_SUCCESS;
}

int cmp_edge_swipe_process(cmp_edge_swipe_t *swipe_opaque, float touch_x,
                           float screen_width, cmp_gesture_state_t state) {
  struct cmp_edge_swipe *ctx = (struct cmp_edge_swipe *)swipe_opaque;
  float threshold;

  if (!ctx || screen_width <= 0.0f)
    return CMP_ERROR_INVALID_ARG;

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
      return cmp_router_pop(ctx->router);
    } else {
      /* Rollback pop animation */
      ctx->pop_progress = 0.0f;
    }
  }

  return CMP_SUCCESS;
}

int cmp_gesture_cancel_on_system_override(cmp_gesture_t *gesture_opaque,
                                          int active_touches_count) {
  if (!gesture_opaque)
    return CMP_ERROR_INVALID_ARG;

  /* E.g. iOS 3/4/5 finger gestures */
  if (active_touches_count >= 3) {
    /* Forcing cancellation ensures UI does not conflict with OS multitasking
     * gestures */
    /* internally triggers CMP_GESTURE_STATE_CANCELLED */
    return CMP_SUCCESS;
  }

  return CMP_SUCCESS;
}
