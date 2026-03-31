/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

struct cmp_dynamic_type {
  float scale_factor;
};

int cmp_dynamic_type_create(cmp_dynamic_type_t **out_dyn_type) {
  struct cmp_dynamic_type *dyn;

  if (!out_dyn_type)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_dynamic_type), (void **)&dyn) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  dyn->scale_factor = 1.0f; /* Normal default */

  *out_dyn_type = (cmp_dynamic_type_t *)dyn;
  return CMP_SUCCESS;
}

int cmp_dynamic_type_destroy(cmp_dynamic_type_t *dyn_type) {
  struct cmp_dynamic_type *d = (struct cmp_dynamic_type *)dyn_type;

  if (!d)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(d);
  return CMP_SUCCESS;
}

int cmp_dynamic_type_set_scale(cmp_dynamic_type_t *dyn_type, float scale) {
  struct cmp_dynamic_type *d = (struct cmp_dynamic_type *)dyn_type;

  if (!d || scale <= 0.0f)
    return CMP_ERROR_INVALID_ARG;

  d->scale_factor = scale;
  return CMP_SUCCESS;
}

int cmp_dynamic_type_apply(cmp_dynamic_type_t *dyn_type,
                           void *root_style_tree) {
  struct cmp_dynamic_type *d = (struct cmp_dynamic_type *)dyn_type;

  if (!d || !root_style_tree)
    return CMP_ERROR_INVALID_ARG;

  /* In a real implementation, this would recursively traverse the style tree
   * and multiply all 'rem' and raw 'px' font-size assignments by
   * d->scale_factor, then trigger a layout dirty pass.
   */
  (void)root_style_tree; /* unused for stub */

  return CMP_SUCCESS;
}
