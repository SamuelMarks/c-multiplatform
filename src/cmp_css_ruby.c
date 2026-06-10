/* clang-format off */
#include "cmp_css_ruby.h"
/* clang-format on */

int cmp_layout_ruby_init(cmp_layout_ruby_t *layout, cmp_layout_ruby_tag_t tag) {
  if (!layout) {
    return -1;
  }
  layout->tag_type = tag;
  return 0;
}

int cmp_prop_ruby_align_group_init(cmp_prop_ruby_align_group_t *group,
                                   cmp_ruby_position_t position,
                                   cmp_ruby_align_t align) {
  if (!group) {
    return -1;
  }
  group->position = position;
  group->align = align;
  return 0;
}

int cmp_prop_ruby_overhang_init(cmp_prop_ruby_overhang_t *prop,
                                cmp_ruby_overhang_t overhang) {
  if (!prop) {
    return -1;
  }
  prop->overhang = overhang;
  return 0;
}
