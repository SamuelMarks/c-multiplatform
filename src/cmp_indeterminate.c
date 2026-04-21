/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_indeterminate {
  int is_indeterminate;
};

int cmp_indeterminate_create(cmp_indeterminate_t **out_state) {
  int rc = CMP_SUCCESS;
  struct cmp_indeterminate *state = NULL;

  if (!out_state) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_indeterminate_create: Invalid argument "
              "(out_state=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_indeterminate), (void **)&state);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_indeterminate_create: Out of memory\n");
    return rc;
  }

  memset(state, 0, sizeof(struct cmp_indeterminate));

  *out_state = (cmp_indeterminate_t *)state;
  return rc;
}

int cmp_indeterminate_destroy(cmp_indeterminate_t *state) {
  int rc = CMP_SUCCESS;
  struct cmp_indeterminate *internal_state = (struct cmp_indeterminate *)state;

  if (!internal_state) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_indeterminate_destroy: Invalid argument (state=NULL)\n");
    return rc;
  }

  CMP_FREE(internal_state);
  return rc;
}

int cmp_indeterminate_set(cmp_indeterminate_t *state, int is_indeterminate) {
  int rc = CMP_SUCCESS;
  struct cmp_indeterminate *internal_state = (struct cmp_indeterminate *)state;

  if (!internal_state) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_indeterminate_set: Invalid argument\n");
    return rc;
  }

  internal_state->is_indeterminate = is_indeterminate;

  return rc;
}
