/* clang-format off */
#include "cmp_css_positioning.h"
#include <string.h>
/* clang-format on */

int cmp_prop_z_index_init_auto(cmp_prop_z_index_t *z_index) {
  if (!z_index)
    return -1;
  z_index->type = CMP_Z_INDEX_AUTO;
  z_index->value = 0;
  return 0;
}

int cmp_prop_z_index_init_integer(cmp_prop_z_index_t *z_index, int val) {
  if (!z_index)
    return -1;
  z_index->type = CMP_Z_INDEX_INTEGER;
  z_index->value = val;
  return 0;
}

int cmp_prop_inset_init_auto(cmp_prop_inset_t *inset) {
  if (!inset)
    return -1;
  inset->type = CMP_PROP_INSET_AUTO;
  return 0;
}

int cmp_prop_inset_init_phys(cmp_prop_inset_t *inset, float val,
                             cmp_length_phys_unit_t unit) {
  if (!inset)
    return -1;
  inset->type = CMP_PROP_INSET_LENGTH_PHYS;
  inset->value.phys.value = val;
  inset->value.phys.unit = unit;
  return 0;
}

int cmp_prop_inset_init_percent(cmp_prop_inset_t *inset, float val) {
  if (!inset)
    return -1;
  inset->type = CMP_PROP_INSET_PERCENT;
  inset->value.percent.value = val;
  return 0;
}

int cmp_prop_inset_group_init_auto(cmp_prop_inset_group_t *group) {
  int rc;
  if (!group)
    return -1;

  rc = cmp_prop_inset_init_auto(&group->top);
  if (rc != 0)
    return rc;
  rc = cmp_prop_inset_init_auto(&group->right);
  if (rc != 0)
    return rc;
  rc = cmp_prop_inset_init_auto(&group->bottom);
  if (rc != 0)
    return rc;
  rc = cmp_prop_inset_init_auto(&group->left);
  if (rc != 0)
    return rc;

  return 0;
}