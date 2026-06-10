/* clang-format off */
#include "cmp_css_masking.h"
#include <string.h>
/* clang-format on */

int cmp_prop_clip_group_init(cmp_prop_clip_group_t *clip) {
  if (clip == NULL) {
    return -1;
  }

  clip->type = CMP_CLIP_PATH_NONE;
  clip->url.url = NULL;
  clip->url.length = 0;
  clip->geometry_box = CMP_GEOMETRY_BOX_BORDER_BOX;
  clip->basic_shape.value = NULL;
  clip->basic_shape.length = 0;
  clip->rule = CMP_CLIP_RULE_NONZERO;

  return 0;
}

int cmp_prop_mask_group_init(cmp_prop_mask_group_t *mask) {
  if (mask == NULL) {
    return -1;
  }

  mask->image.url = NULL;
  mask->image.length = 0;
  mask->mode = CMP_MASK_MODE_MATCH_SOURCE;
  mask->repeat_x = CMP_BG_REPEAT_REPEAT;
  mask->repeat_y = CMP_BG_REPEAT_REPEAT;

  mask->position_x.type = CMP_PROP_SIZE_PERCENT;
  mask->position_x.value.percent.value = 0.0f;

  mask->position_y.type = CMP_PROP_SIZE_PERCENT;
  mask->position_y.value.percent.value = 0.0f;

  mask->clip = CMP_GEOMETRY_BOX_BORDER_BOX;
  mask->clip_no_clip = 0;
  mask->origin = CMP_GEOMETRY_BOX_BORDER_BOX;

  mask->size.type = CMP_BG_SIZE_AUTO;
  mask->size.width.type = CMP_PROP_SIZE_AUTO;
  mask->size.height.type = CMP_PROP_SIZE_AUTO;

  mask->composite = CMP_MASK_COMPOSITE_ADD;

  return 0;
}

int cmp_prop_mask_border_group_init(cmp_prop_mask_border_group_t *mask_border) {
  if (mask_border == NULL) {
    return -1;
  }

  mask_border->type = CMP_MASK_TYPE_LUMINANCE;
  mask_border->source.url = NULL;
  mask_border->source.length = 0;
  mask_border->mode = CMP_MASK_MODE_LUMINANCE;

  mask_border->slice.min_width.type = CMP_PROP_SIZE_AUTO;
  mask_border->slice.min_height.type = CMP_PROP_SIZE_AUTO;
  mask_border->slice.max_width.type = CMP_PROP_SIZE_AUTO;
  mask_border->slice.max_height.type = CMP_PROP_SIZE_AUTO;

  mask_border->width.min_width.type = CMP_PROP_SIZE_AUTO;
  mask_border->width.min_height.type = CMP_PROP_SIZE_AUTO;
  mask_border->width.max_width.type = CMP_PROP_SIZE_AUTO;
  mask_border->width.max_height.type = CMP_PROP_SIZE_AUTO;

  mask_border->outset.min_width.type = CMP_PROP_SIZE_AUTO;
  mask_border->outset.min_height.type = CMP_PROP_SIZE_AUTO;
  mask_border->outset.max_width.type = CMP_PROP_SIZE_AUTO;
  mask_border->outset.max_height.type = CMP_PROP_SIZE_AUTO;

  mask_border->repeat_x = CMP_BORDER_IMAGE_REPEAT_STRETCH;
  mask_border->repeat_y = CMP_BORDER_IMAGE_REPEAT_STRETCH;

  return 0;
}

int cmp_prop_filter_group_init(cmp_prop_filter_group_t *filter) {
  if (filter == NULL) {
    return -1;
  }

  filter->filter.value = NULL;
  filter->filter.length = 0;
  filter->backdrop_filter.value = NULL;
  filter->backdrop_filter.length = 0;

  return 0;
}

int cmp_prop_blend_mode_group_init(cmp_prop_blend_mode_group_t *blend_mode) {
  if (blend_mode == NULL) {
    return -1;
  }

  blend_mode->mix_blend_mode = CMP_BLEND_MODE_NORMAL;
  blend_mode->background_blend_mode = CMP_BLEND_MODE_NORMAL;

  return 0;
}
