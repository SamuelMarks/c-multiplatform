/* clang-format off */
#include "cmp_css_tables.h"
#include <stddef.h>
/* clang-format on */

int cmp_prop_table_border_spacing_val_init_phys(
    cmp_prop_table_border_spacing_val_t *val, float len,
    cmp_length_phys_unit_t unit) {
  if (val == NULL) {
    return -1;
  }
  val->type = CMP_TABLE_BORDER_SPACING_LENGTH_PHYS;
  val->value.phys.value = len;
  val->value.phys.unit = unit;
  return 0;
}

int cmp_prop_table_border_spacing_val_init_rel(
    cmp_prop_table_border_spacing_val_t *val, float len,
    cmp_length_rel_unit_t unit) {
  if (val == NULL) {
    return -1;
  }
  val->type = CMP_TABLE_BORDER_SPACING_LENGTH_REL;
  val->value.rel.value = len;
  val->value.rel.unit = unit;
  return 0;
}
