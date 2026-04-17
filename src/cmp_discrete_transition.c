/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_discrete_transition {
  int is_active;
};

int cmp_discrete_transition_create(cmp_discrete_transition_t **out_transition) {
  int rc = CMP_SUCCESS;
  struct cmp_discrete_transition *transition = NULL;

  if (!out_transition) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_discrete_transition_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_discrete_transition), (void **)&transition);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_discrete_transition_create: Out of memory\n");
    return rc;
  }

  memset(transition, 0, sizeof(struct cmp_discrete_transition));

  *out_transition = (cmp_discrete_transition_t *)transition;
  return rc;
}

int cmp_discrete_transition_destroy(cmp_discrete_transition_t *transition) {
  int rc = CMP_SUCCESS;
  struct cmp_discrete_transition *internal_transition =
      (struct cmp_discrete_transition *)transition;

  if (!internal_transition) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_discrete_transition_destroy: Invalid argument\n");
    return rc;
  }

  CMP_FREE(internal_transition);
  return rc;
}

int cmp_discrete_transition_evaluate(cmp_discrete_transition_t *transition,
                                     float progress, int *out_is_visible) {
  int rc = CMP_SUCCESS;
  struct cmp_discrete_transition *internal_transition =
      (struct cmp_discrete_transition *)transition;

  if (!internal_transition || !out_is_visible || progress < 0.0f ||
      progress > 1.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_discrete_transition_evaluate: Invalid argument\n");
    return rc;
  }

  if (progress < 0.5f) {
    *out_is_visible = 0; /* Old state / invisible */
  } else {
    *out_is_visible = 1; /* New state / visible */
  }

  return rc;
}
