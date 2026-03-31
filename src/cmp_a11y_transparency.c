/* clang-format off */
#include "cmp.h"
/* clang-format on */

struct cmp_a11y_transparency {
  int reduced_transparency_enabled;
};

int cmp_a11y_transparency_create(cmp_a11y_transparency_t **out_trans) {
  cmp_a11y_transparency_t *trans;

  if (!out_trans) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_a11y_transparency_t), (void **)&trans) !=
      CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  trans->reduced_transparency_enabled = 0;
  *out_trans = trans;
  return CMP_SUCCESS;
}

int cmp_a11y_transparency_destroy(cmp_a11y_transparency_t *trans) {
  if (!trans) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(trans);
  return CMP_SUCCESS;
}

int cmp_a11y_transparency_set(cmp_a11y_transparency_t *trans, int enabled) {
  if (!trans) {
    return CMP_ERROR_INVALID_ARG;
  }
  trans->reduced_transparency_enabled = enabled ? 1 : 0;
  return CMP_SUCCESS;
}

int cmp_a11y_transparency_apply(cmp_a11y_transparency_t *trans,
                                float *out_opacity, float fallback_opacity) {
  if (!trans || !out_opacity) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (trans->reduced_transparency_enabled) {
    *out_opacity = fallback_opacity;
  }
  return CMP_SUCCESS;
}
