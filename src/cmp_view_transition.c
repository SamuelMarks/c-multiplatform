/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_view_transition {
  int is_active;
};

/**
 * @brief cmp_view_transition_create
 *
 * @param out_transition Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_view_transition_create(cmp_view_transition_t **out_transition) {
  int rc;
  rc = 0;
  struct cmp_view_transition *transition;

  if (!out_transition)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_view_transition), (void **)&transition) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(transition, 0, sizeof(struct cmp_view_transition));

  *out_transition = (cmp_view_transition_t *)transition;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_view_transition_destroy
 *
 * @param transition Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_view_transition_destroy(cmp_view_transition_t *transition) {
  int rc;
  rc = 0;
  struct cmp_view_transition *internal_transition =
      (struct cmp_view_transition *)transition;
  if (!internal_transition)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_transition);
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_view_transition_start
 *
 * @param transition Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_view_transition_start(cmp_view_transition_t *transition) {
  int rc;
  rc = 0;
  struct cmp_view_transition *internal_transition =
      (struct cmp_view_transition *)transition;

  if (!internal_transition)
    return CMP_ERROR_INVALID_ARG;

  internal_transition->is_active = 1;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
