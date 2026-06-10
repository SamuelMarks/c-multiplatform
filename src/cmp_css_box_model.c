/* clang-format off */
#include "cmp_css_box_model.h"
#include <string.h>
/* clang-format on */

int cmp_prop_size_init_auto(cmp_prop_size_t *size) {
  if (!size)
    return -1;
  size->type = CMP_PROP_SIZE_AUTO;
  return 0;
}

int cmp_prop_size_init_phys(cmp_prop_size_t *size, float val,
                            cmp_length_phys_unit_t unit) {
  if (!size)
    return -1;
  size->type = CMP_PROP_SIZE_LENGTH_PHYS;
  size->value.phys.value = val;
  size->value.phys.unit = unit;
  return 0;
}

int cmp_prop_size_init_percent(cmp_prop_size_t *size, float val) {
  if (!size)
    return -1;
  size->type = CMP_PROP_SIZE_PERCENT;
  size->value.percent.value = val;
  return 0;
}

int cmp_prop_size_init_intrinsic(cmp_prop_size_t *size,
                                 cmp_intrinsic_size_t intrinsic) {
  if (!size)
    return -1;
  size->type = CMP_PROP_SIZE_INTRINSIC;
  size->value.intrinsic = intrinsic;
  return 0;
}

int cmp_prop_margin_init_auto(cmp_prop_margin_t *margin) {
  if (!margin)
    return -1;
  margin->type = CMP_PROP_MARGIN_AUTO;
  return 0;
}

int cmp_prop_margin_init_phys(cmp_prop_margin_t *margin, float val,
                              cmp_length_phys_unit_t unit) {
  if (!margin)
    return -1;
  margin->type = CMP_PROP_MARGIN_LENGTH_PHYS;
  margin->value.phys.value = val;
  margin->value.phys.unit = unit;
  return 0;
}

int cmp_prop_padding_init_phys(cmp_prop_padding_t *padding, float val,
                               cmp_length_phys_unit_t unit) {
  if (!padding)
    return -1;
  padding->type = CMP_PROP_PADDING_LENGTH_PHYS;
  padding->value.phys.value = val;
  padding->value.phys.unit = unit;
  return 0;
}

int cmp_prop_aspect_ratio_init_auto(cmp_prop_aspect_ratio_t *ar) {
  if (!ar)
    return -1;
  ar->type = CMP_ASPECT_RATIO_AUTO;
  return 0;
}

int cmp_prop_aspect_ratio_init_ratio(cmp_prop_aspect_ratio_t *ar, float num,
                                     float den) {
  if (!ar)
    return -1;
  if (den == 0.0f)
    return -1; /* Prevent division by zero mathematically */
  ar->type = CMP_ASPECT_RATIO_RATIO;
  ar->ratio.numerator = num;
  ar->ratio.denominator = den;
  return 0;
}