/* clang-format off */
#include "cmp.h"
#include <stdio.h>
/* clang-format on */

struct cmp_a11y_transparency {
  int reduced_transparency_enabled;
};

int cmp_a11y_transparency_create(cmp_a11y_transparency_t **out_trans) {
  int rc = CMP_SUCCESS;
  cmp_a11y_transparency_t *trans = NULL;

  if (!out_trans) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_a11y_transparency_create: Invalid argument "
                    "(out_trans=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_a11y_transparency_t), (void **)&trans);
  if (rc != CMP_SUCCESS) {
    fprintf(stderr, "Error in cmp_a11y_transparency_create: Out of memory\n");
    return rc;
  }

  trans->reduced_transparency_enabled = 0;
  *out_trans = trans;
  return rc;
}

int cmp_a11y_transparency_destroy(cmp_a11y_transparency_t *trans) {
  int rc = CMP_SUCCESS;

  if (!trans) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_a11y_transparency_destroy: Invalid argument "
                    "(trans=NULL)\n");
    return rc;
  }
  CMP_FREE(trans);
  return rc;
}

int cmp_a11y_transparency_set(cmp_a11y_transparency_t *trans, int enabled) {
  int rc = CMP_SUCCESS;

  if (!trans) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(
        stderr,
        "Error in cmp_a11y_transparency_set: Invalid argument (trans=NULL)\n");
    return rc;
  }
  trans->reduced_transparency_enabled = enabled ? 1 : 0;
  return rc;
}

int cmp_a11y_transparency_apply(cmp_a11y_transparency_t *trans,
                                float *out_opacity, float fallback_opacity) {
  int rc = CMP_SUCCESS;

  if (!trans || !out_opacity) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_a11y_transparency_apply: Invalid argument\n");
    return rc;
  }

  if (trans->reduced_transparency_enabled) {
    *out_opacity = fallback_opacity;
  }
  return rc;
}
