/* clang-format off */
#include "cmp.h"
#include <stdio.h>
/* clang-format on */

int cmp_anim_compose_numerical(float base_value, float anim_value,
                               cmp_anim_compose_op_t op, float *out_value) {
  int rc = CMP_SUCCESS;

  if (!out_value) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_anim_compose_numerical: Invalid argument "
                    "(out_value=NULL)\n");
    return rc;
  }

  if (op == CMP_ANIM_COMPOSE_REPLACE) {
    *out_value = anim_value;
  } else if (op == CMP_ANIM_COMPOSE_ADD) {
    *out_value = base_value + anim_value;
  } else if (op == CMP_ANIM_COMPOSE_ACCUMULATE) {
    /* For simple numerical, accumulate is similar to add, but might have
       type-specific differences in full CSS. Here we treat it as additive for
       floats. */
    *out_value = base_value + anim_value;
  } else {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr,
            "Error in cmp_anim_compose_numerical: Unknown operation %d\n",
            (int)op);
    return rc;
  }

  return rc;
}
