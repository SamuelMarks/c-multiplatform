/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

struct cmp_prefers_reduced_motion {
  int is_enabled;
};

/**
 * @brief cmp_prefers_reduced_motion_create
 *
 * @param out_rm Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_prefers_reduced_motion_create(cmp_prefers_reduced_motion_t **out_rm) {
  int rc;
  rc = 0;
  struct cmp_prefers_reduced_motion *rm;

  if (!out_rm)
    return CMP_ERROR_INVALID_ARG;

  rc = CMP_MALLOC(sizeof(struct cmp_prefers_reduced_motion), (void **)&rm);
  if (rc != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  rm->is_enabled = 0; /* Default to off (full motion) */

  *out_rm = (cmp_prefers_reduced_motion_t *)rm;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_prefers_reduced_motion_destroy
 *
 * @param rm Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_prefers_reduced_motion_destroy(cmp_prefers_reduced_motion_t *rm) {
  int rc;
  rc = 0;
  struct cmp_prefers_reduced_motion *r =
      (struct cmp_prefers_reduced_motion *)rm;

  if (!r)
    return CMP_ERROR_INVALID_ARG;

  rc = CMP_FREE(r);
  if (rc != CMP_SUCCESS) {
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_prefers_reduced_motion_set
 *
 * @param rm Parameter description.
 * @param enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_prefers_reduced_motion_set(cmp_prefers_reduced_motion_t *rm,
                                   int enabled) {
  int rc;
  rc = 0;
  struct cmp_prefers_reduced_motion *r =
      (struct cmp_prefers_reduced_motion *)rm;

  if (!r)
    return CMP_ERROR_INVALID_ARG;

  r->is_enabled = enabled;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_prefers_reduced_motion_apply
 *
 * @param rm Parameter description.
 * @param duration_ms Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_prefers_reduced_motion_apply(cmp_prefers_reduced_motion_t *rm,
                                     float *duration_ms) {
  int rc;
  rc = 0;
  struct cmp_prefers_reduced_motion *r =
      (struct cmp_prefers_reduced_motion *)rm;

  if (!r || !duration_ms)
    return CMP_ERROR_INVALID_ARG;

  if (r->is_enabled) {
    *duration_ms = 0.0f; /* Instantly complete animations */
  }
  return CMP_SUCCESS;
}
