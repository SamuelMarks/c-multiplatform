/* clang-format off */
#include "cmp_css_writing_modes.h"
/* clang-format on */

int cmp_prop_bidi_group_init(cmp_prop_bidi_group_t *group, cmp_direction_t dir,
                             cmp_unicode_bidi_t bidi) {
  if (!group) {
    return -1;
  }
  group->direction = dir;
  group->unicode_bidi = bidi;
  return 0;
}

int cmp_prop_writing_mode_group_init(
    cmp_prop_writing_mode_group_t *group, cmp_writing_mode_t mode,
    cmp_text_orientation_t orient, cmp_text_combine_upright_type_t combine_type,
    int combine_digits) {
  if (!group) {
    return -1;
  }
  group->writing_mode = mode;
  group->text_orientation = orient;
  group->text_combine_upright_type = combine_type;
  group->text_combine_upright_digits = combine_digits;
  return 0;
}
