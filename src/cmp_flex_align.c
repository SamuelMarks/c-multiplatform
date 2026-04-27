/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
/* clang-format on */

/**
 * @brief cmp_flex_align_evaluate
 *
 * @param align_val Parameter description.
 * @param cross_size Parameter description.
 * @param item_cross_size Parameter description.
 * @param item_baseline Parameter description.
 * @param max_baseline Parameter description.
 * @param out_position Parameter description.
 * @param out_cross_size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_flex_align_evaluate(cmp_flex_align_t align_val, float cross_size,
                            float item_cross_size, float item_baseline,
                            float max_baseline, float *out_position,
                            float *out_cross_size) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (out_position == NULL || out_cross_size == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_flex_align_evaluate: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  *out_cross_size = item_cross_size;
  *out_position = 0.0f;

  if (align_val == CMP_FLEX_ALIGN_START) {
    *out_position = 0.0f;
  } else if (align_val == CMP_FLEX_ALIGN_END) {
    *out_position = cross_size - item_cross_size;
  } else if (align_val == CMP_FLEX_ALIGN_CENTER) {
    *out_position = (cross_size - item_cross_size) * 0.5f;
  } else if (align_val == CMP_FLEX_ALIGN_STRETCH) {
    *out_position = 0.0f;
    *out_cross_size = cross_size;
  } else if (align_val == CMP_FLEX_ALIGN_BASELINE) {
    *out_position = max_baseline - item_baseline;
  } else if (align_val == CMP_FLEX_ALIGN_SPACE_BETWEEN ||
             align_val == CMP_FLEX_ALIGN_SPACE_AROUND) {
    /* Not typically used for individual items but if specified, acts like start
     */
    *out_position = 0.0f;
  }

  cmp_log_debug("cmp_flex_align_evaluate: Evaluated cross position\n");
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
