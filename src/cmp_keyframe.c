/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_keyframe {
  float stops[100];
  int count;
  double duration_ms;
  double elapsed_ms;
  cmp_animation_fill_mode_t fill_mode;
};

/**
 * @brief cmp_keyframe_create
 *
 * @param out_keyframe Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_keyframe_create(cmp_keyframe_t **out_keyframe) {
  int rc = CMP_SUCCESS;
  struct cmp_keyframe *keyframe = NULL;

  if (!out_keyframe) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_keyframe_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_keyframe), (void **)&keyframe);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_keyframe_create: Out of memory\n");
    return rc;
  }

  memset(keyframe, 0, sizeof(struct cmp_keyframe));

  *out_keyframe = (cmp_keyframe_t *)keyframe;
  return rc;
}

/**
 * @brief cmp_keyframe_destroy
 *
 * @param keyframe Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_keyframe_destroy(cmp_keyframe_t *keyframe) {
  int rc = CMP_SUCCESS;
  struct cmp_keyframe *internal_keyframe = (struct cmp_keyframe *)keyframe;

  if (!internal_keyframe) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_keyframe_destroy: Invalid argument\n");
    return rc;
  }

  rc = CMP_FREE(internal_keyframe);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_keyframe_destroy: CMP_FREE failed\n");
  }
  return rc;
}

/**
 * @brief cmp_keyframe_add_stop
 *
 * @param keyframe Parameter description.
 * @param percentage Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_keyframe_add_stop(cmp_keyframe_t *keyframe, float percentage) {
  int rc = CMP_SUCCESS;
  struct cmp_keyframe *internal_keyframe = (struct cmp_keyframe *)keyframe;

  if (!internal_keyframe || percentage < 0.0f || percentage > 1.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_keyframe_add_stop: Invalid argument\n");
    return rc;
  }

  if (internal_keyframe->count >= 100) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_keyframe_add_stop: Maximum stops reached\n");
    return rc;
  }

  internal_keyframe->stops[internal_keyframe->count++] = percentage;
  return rc;
}

/**
 * @brief cmp_keyframe_step
 *
 * @param keyframe Parameter description.
 * @param dt_ms Parameter description.
 * @param play_state Parameter description.
 * @param out_current_percentage Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_keyframe_step(cmp_keyframe_t *keyframe, double dt_ms,
                      cmp_animation_play_state_t play_state,
                      float *out_current_percentage) {
  int rc = CMP_SUCCESS;
  struct cmp_keyframe *internal_keyframe = (struct cmp_keyframe *)keyframe;
  float progress;

  if (!internal_keyframe || !out_current_percentage || dt_ms < 0.0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_keyframe_step: Invalid argument\n");
    return rc;
  }

  if (internal_keyframe->duration_ms <= 0.0) {
    *out_current_percentage = 1.0f;
    return rc;
  }

  if (play_state == CMP_ANIMATION_PLAY_STATE_RUNNING) {
    internal_keyframe->elapsed_ms += dt_ms;
  }

  progress =
      (float)(internal_keyframe->elapsed_ms / internal_keyframe->duration_ms);

  if (progress >= 1.0f) {
    if (internal_keyframe->fill_mode == CMP_ANIMATION_FILL_MODE_FORWARDS ||
        internal_keyframe->fill_mode == CMP_ANIMATION_FILL_MODE_BOTH) {
      *out_current_percentage = 1.0f;
    } else {
      *out_current_percentage =
          0.0f; /* Reset to start if not filling forwards */
    }
  } else if (progress <= 0.0f) {
    if (internal_keyframe->fill_mode == CMP_ANIMATION_FILL_MODE_BACKWARDS ||
        internal_keyframe->fill_mode == CMP_ANIMATION_FILL_MODE_BOTH) {
      *out_current_percentage = 0.0f;
    } else {
      *out_current_percentage = 0.0f;
    }
  } else {
    *out_current_percentage = progress;
  }

  return rc;
}
