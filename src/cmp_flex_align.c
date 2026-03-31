/* clang-format off */
#include "cmp.h"
/* clang-format on */

int cmp_flex_align_evaluate(cmp_flex_align_t align_val, double cross_size,
                            double item_cross_size, double item_baseline,
                            double max_baseline, double *out_position,
                            double *out_cross_size) {
  if (!out_position || !out_cross_size)
    return CMP_ERROR_INVALID_ARG;

  *out_cross_size = item_cross_size;
  *out_position = 0.0;

  if (align_val == CMP_FLEX_ALIGN_START) {
    *out_position = 0.0;
  } else if (align_val == CMP_FLEX_ALIGN_END) {
    *out_position = cross_size - item_cross_size;
  } else if (align_val == CMP_FLEX_ALIGN_CENTER) {
    *out_position = (cross_size - item_cross_size) * 0.5;
  } else if (align_val == CMP_FLEX_ALIGN_STRETCH) {
    *out_position = 0.0;
    *out_cross_size = cross_size;
  } else if (align_val == CMP_FLEX_ALIGN_BASELINE) {
    *out_position = max_baseline - item_baseline;
  } else if (align_val == CMP_FLEX_ALIGN_SPACE_BETWEEN ||
             align_val == CMP_FLEX_ALIGN_SPACE_AROUND) {
    /* Not typically used for individual items but if specified, acts like start
     */
    *out_position = 0.0;
  }

  return CMP_SUCCESS;
}
