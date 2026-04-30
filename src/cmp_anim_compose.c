/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdio.h>
/* clang-format on */

/**
 * @brief cmp_anim_compose_numerical
 *
 * @param base_value Parameter description.
 * @param anim_value Parameter description.
 * @param op Parameter description.
 * @param out_value Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_anim_compose_numerical(float base_value, float anim_value,
                               cmp_anim_compose_op_t op, float *out_value) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (out_value == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_anim_compose_numerical: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  if (op == CMP_ANIM_COMPOSE_REPLACE) {
    *out_value = anim_value;
    cmp_log_debug(
        "cmp_anim_compose_numerical: Replaced base (%.2f) with anim (%.2f)\n",
        base_value, anim_value);
  } else if (op == CMP_ANIM_COMPOSE_ADD) {
    *out_value = base_value + anim_value;
    cmp_log_debug("cmp_anim_compose_numerical: Added base (%.2f) and anim "
                  "(%.2f) -> %.2f\n",
                  base_value, anim_value, *out_value);
  } else if (op == CMP_ANIM_COMPOSE_ACCUMULATE) {
    /* For simple numerical, accumulate is similar to add, but might have
       type-specific differences in full CSS. Here we treat it as additive for
       floats. */
    *out_value = base_value + anim_value;
    cmp_log_debug("cmp_anim_compose_numerical: Accumulated base (%.2f) and "
                  "anim (%.2f) -> %.2f\n",
                  base_value, anim_value, *out_value);
  } else {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_anim_compose_numerical: Unknown operation %d: %s\n",
                  (int)op, err_str);

    return rc;
  }

  return rc;
}
