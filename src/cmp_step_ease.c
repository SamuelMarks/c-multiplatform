/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

struct cmp_step_ease {
  int steps;
  cmp_step_position_t position;
};

int cmp_step_ease_create(int steps, cmp_step_position_t position,
                         cmp_step_ease_t **out_step) {
  struct cmp_step_ease *step_ease;

  if (!out_step || steps <= 0)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_step_ease), (void **)&step_ease) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  step_ease->steps = steps;
  step_ease->position = position;

  *out_step = (cmp_step_ease_t *)step_ease;
  return CMP_SUCCESS;
}

int cmp_step_ease_destroy(cmp_step_ease_t *step) {
  struct cmp_step_ease *internal_step = (struct cmp_step_ease *)step;
  if (!internal_step)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_step);
  return CMP_SUCCESS;
}

int cmp_step_ease_evaluate(cmp_step_ease_t *step, float t, float *out_value) {
  struct cmp_step_ease *s = (struct cmp_step_ease *)step;
  int current_step;

  if (!s || !out_value || t < 0.0f || t > 1.0f)
    return CMP_ERROR_INVALID_ARG;

  if (t == 1.0f) {
    *out_value = 1.0f;
    return CMP_SUCCESS;
  }

  current_step = (int)(t * s->steps);

  if (s->position == CMP_STEP_POSITION_START) {
    current_step += 1;
  }

  if (current_step > s->steps) {
    current_step = s->steps;
  }

  *out_value = (float)current_step / (float)s->steps;

  return CMP_SUCCESS;
}
