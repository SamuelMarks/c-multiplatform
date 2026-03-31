/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_transition {
  double duration_ms;
  double delay_ms;
  double elapsed_ms;
  cmp_transition_behavior_t behavior;
  int is_active;
  int is_finished;
};

int cmp_transition_create(double duration_ms, double delay_ms,
                          cmp_transition_behavior_t behavior,
                          cmp_transition_t **out_transition) {
  struct cmp_transition *transition;

  if (!out_transition || duration_ms < 0.0 || delay_ms < 0.0)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_transition), (void **)&transition) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(transition, 0, sizeof(struct cmp_transition));
  transition->duration_ms = duration_ms;
  transition->delay_ms = delay_ms;
  transition->behavior = behavior;
  transition->is_active = 1;

  *out_transition = (cmp_transition_t *)transition;
  return CMP_SUCCESS;
}

int cmp_transition_destroy(cmp_transition_t *transition) {
  struct cmp_transition *internal_transition =
      (struct cmp_transition *)transition;

  if (!internal_transition)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_transition);
  return CMP_SUCCESS;
}

int cmp_transition_step(cmp_transition_t *transition, double dt_ms,
                        float *out_progress) {
  struct cmp_transition *internal_transition =
      (struct cmp_transition *)transition;
  double active_time;
  float progress;

  if (!internal_transition || !out_progress || dt_ms < 0.0)
    return CMP_ERROR_INVALID_ARG;

  if (!internal_transition->is_active || internal_transition->is_finished) {
    *out_progress = 1.0f;
    return CMP_SUCCESS;
  }

  internal_transition->elapsed_ms += dt_ms;

  if (internal_transition->elapsed_ms < internal_transition->delay_ms) {
    *out_progress = 0.0f;
    return CMP_SUCCESS;
  }

  if (internal_transition->duration_ms == 0.0) {
    internal_transition->is_finished = 1;
    *out_progress = 1.0f;
    return CMP_SUCCESS;
  }

  active_time = internal_transition->elapsed_ms - internal_transition->delay_ms;
  progress = (float)(active_time / internal_transition->duration_ms);

  if (progress >= 1.0f) {
    progress = 1.0f;
    internal_transition->is_finished = 1;
  }

  if (internal_transition->behavior == CMP_TRANSITION_BEHAVIOR_ALLOW_DISCRETE) {
    if (progress < 0.5f) {
      *out_progress = 0.0f;
    } else {
      *out_progress = 1.0f;
    }
  } else {
    *out_progress = progress;
  }

  return CMP_SUCCESS;
}