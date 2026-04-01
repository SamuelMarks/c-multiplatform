/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_discrete_transition {
  int is_active;
};

int cmp_discrete_transition_create(cmp_discrete_transition_t **out_transition) {
  struct cmp_discrete_transition *transition;

  if (!out_transition)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_discrete_transition),
                 (void **)&transition) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(transition, 0, sizeof(struct cmp_discrete_transition));

  *out_transition = (cmp_discrete_transition_t *)transition;
  return CMP_SUCCESS;
}

int cmp_discrete_transition_destroy(cmp_discrete_transition_t *transition) {
  struct cmp_discrete_transition *internal_transition =
      (struct cmp_discrete_transition *)transition;
  if (!internal_transition)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_transition);
  return CMP_SUCCESS;
}

int cmp_discrete_transition_evaluate(cmp_discrete_transition_t *transition,
                                     float progress, int *out_is_visible) {
  struct cmp_discrete_transition *internal_transition =
      (struct cmp_discrete_transition *)transition;

  if (!internal_transition || !out_is_visible || progress < 0.0f ||
      progress > 1.0f)
    return CMP_ERROR_INVALID_ARG;

  if (progress < 0.5f) {
    *out_is_visible = 0; /* Old state / invisible */
  } else {
    *out_is_visible = 1; /* New state / visible */
  }

  return CMP_SUCCESS;
}
