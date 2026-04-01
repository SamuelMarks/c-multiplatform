/* clang-format off */
#include "cmp.h"
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

int cmp_keyframe_create(cmp_keyframe_t **out_keyframe) {
  struct cmp_keyframe *keyframe;

  if (!out_keyframe)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_keyframe), (void **)&keyframe) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(keyframe, 0, sizeof(struct cmp_keyframe));

  *out_keyframe = (cmp_keyframe_t *)keyframe;
  return CMP_SUCCESS;
}

int cmp_keyframe_destroy(cmp_keyframe_t *keyframe) {
  struct cmp_keyframe *internal_keyframe = (struct cmp_keyframe *)keyframe;

  if (!internal_keyframe)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_keyframe);
  return CMP_SUCCESS;
}

int cmp_keyframe_add_stop(cmp_keyframe_t *keyframe, float percentage) {
  struct cmp_keyframe *internal_keyframe = (struct cmp_keyframe *)keyframe;

  if (!internal_keyframe || percentage < 0.0f || percentage > 1.0f)
    return CMP_ERROR_INVALID_ARG;

  if (internal_keyframe->count >= 100)
    return CMP_ERROR_OOM;

  internal_keyframe->stops[internal_keyframe->count++] = percentage;
  return CMP_SUCCESS;
}

int cmp_keyframe_step(cmp_keyframe_t *keyframe, double dt_ms,
                      cmp_animation_play_state_t play_state,
                      float *out_current_percentage) {
  struct cmp_keyframe *internal_keyframe = (struct cmp_keyframe *)keyframe;
  float progress;

  if (!internal_keyframe || !out_current_percentage || dt_ms < 0.0)
    return CMP_ERROR_INVALID_ARG;

  if (internal_keyframe->duration_ms <= 0.0) {
    *out_current_percentage = 1.0f;
    return CMP_SUCCESS;
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

  return CMP_SUCCESS;
}
