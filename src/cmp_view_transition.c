/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_view_transition {
  int is_active;
};

int cmp_view_transition_create(cmp_view_transition_t **out_transition) {
  struct cmp_view_transition *transition;

  if (!out_transition)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_view_transition), (void **)&transition) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(transition, 0, sizeof(struct cmp_view_transition));

  *out_transition = (cmp_view_transition_t *)transition;
  return CMP_SUCCESS;
}

int cmp_view_transition_destroy(cmp_view_transition_t *transition) {
  struct cmp_view_transition *internal_transition =
      (struct cmp_view_transition *)transition;
  if (!internal_transition)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_transition);
  return CMP_SUCCESS;
}

int cmp_view_transition_start(cmp_view_transition_t *transition) {
  struct cmp_view_transition *internal_transition =
      (struct cmp_view_transition *)transition;

  if (!internal_transition)
    return CMP_ERROR_INVALID_ARG;

  internal_transition->is_active = 1;
  return CMP_SUCCESS;
}
