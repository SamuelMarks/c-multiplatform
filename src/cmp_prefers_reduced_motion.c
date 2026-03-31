/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

struct cmp_prefers_reduced_motion {
  int is_enabled;
};

int cmp_prefers_reduced_motion_create(cmp_prefers_reduced_motion_t **out_rm) {
  struct cmp_prefers_reduced_motion *rm;

  if (!out_rm)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_prefers_reduced_motion), (void **)&rm) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  rm->is_enabled = 0; /* Default to off (full motion) */

  *out_rm = (cmp_prefers_reduced_motion_t *)rm;
  return CMP_SUCCESS;
}

int cmp_prefers_reduced_motion_destroy(cmp_prefers_reduced_motion_t *rm) {
  struct cmp_prefers_reduced_motion *r =
      (struct cmp_prefers_reduced_motion *)rm;

  if (!r)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(r);
  return CMP_SUCCESS;
}

int cmp_prefers_reduced_motion_set(cmp_prefers_reduced_motion_t *rm,
                                   int enabled) {
  struct cmp_prefers_reduced_motion *r =
      (struct cmp_prefers_reduced_motion *)rm;

  if (!r)
    return CMP_ERROR_INVALID_ARG;

  r->is_enabled = enabled;
  return CMP_SUCCESS;
}

int cmp_prefers_reduced_motion_apply(cmp_prefers_reduced_motion_t *rm,
                                     float *duration_ms) {
  struct cmp_prefers_reduced_motion *r =
      (struct cmp_prefers_reduced_motion *)rm;

  if (!r || !duration_ms)
    return CMP_ERROR_INVALID_ARG;

  if (r->is_enabled) {
    *duration_ms = 0.0f; /* Instantly complete animations */
  }

  return CMP_SUCCESS;
}
