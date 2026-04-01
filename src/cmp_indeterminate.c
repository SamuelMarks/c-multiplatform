/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_indeterminate {
  int is_indeterminate;
};

int cmp_indeterminate_create(cmp_indeterminate_t **out_state) {
  struct cmp_indeterminate *state;

  if (!out_state)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_indeterminate), (void **)&state) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(state, 0, sizeof(struct cmp_indeterminate));

  *out_state = (cmp_indeterminate_t *)state;
  return CMP_SUCCESS;
}

int cmp_indeterminate_destroy(cmp_indeterminate_t *state) {
  struct cmp_indeterminate *internal_state = (struct cmp_indeterminate *)state;
  if (!internal_state)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_state);
  return CMP_SUCCESS;
}

int cmp_indeterminate_set(cmp_indeterminate_t *state, int is_indeterminate) {
  struct cmp_indeterminate *internal_state = (struct cmp_indeterminate *)state;

  if (!internal_state)
    return CMP_ERROR_INVALID_ARG;

  internal_state->is_indeterminate = is_indeterminate;

  return CMP_SUCCESS;
}
