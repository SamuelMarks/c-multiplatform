/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include "cmp_css_masking.h"
/* clang-format on */

int main(void) {
  cmp_prop_clip_group_t clip;
  cmp_prop_mask_group_t mask;
  cmp_prop_mask_border_group_t mask_border;
  cmp_prop_filter_group_t filter;
  cmp_prop_blend_mode_group_t blend;

  printf("Testing CSS Masking, Clipping & Filters...\n");

  /* Test Clip Initialization */
  if (cmp_prop_clip_group_init(NULL) != -1) {
    printf("Failed to handle NULL in cmp_prop_clip_group_init.\n");
    return EXIT_FAILURE;
  }
  if (cmp_prop_clip_group_init(&clip) != 0) {
    printf("Failed to init clip group.\n");
    return EXIT_FAILURE;
  }
  if (clip.type != CMP_CLIP_PATH_NONE ||
      clip.geometry_box != CMP_GEOMETRY_BOX_BORDER_BOX) {
    printf("Clip group default values incorrect.\n");
    return EXIT_FAILURE;
  }

  /* Test Mask Initialization */
  if (cmp_prop_mask_group_init(NULL) != -1) {
    printf("Failed to handle NULL in cmp_prop_mask_group_init.\n");
    return EXIT_FAILURE;
  }
  if (cmp_prop_mask_group_init(&mask) != 0) {
    printf("Failed to init mask group.\n");
    return EXIT_FAILURE;
  }
  if (mask.mode != CMP_MASK_MODE_MATCH_SOURCE ||
      mask.composite != CMP_MASK_COMPOSITE_ADD) {
    printf("Mask group default values incorrect.\n");
    return EXIT_FAILURE;
  }

  /* Test Mask Border Initialization */
  if (cmp_prop_mask_border_group_init(NULL) != -1) {
    printf("Failed to handle NULL in cmp_prop_mask_border_group_init.\n");
    return EXIT_FAILURE;
  }
  if (cmp_prop_mask_border_group_init(&mask_border) != 0) {
    printf("Failed to init mask border group.\n");
    return EXIT_FAILURE;
  }
  if (mask_border.type != CMP_MASK_TYPE_LUMINANCE ||
      mask_border.repeat_x != CMP_BORDER_IMAGE_REPEAT_STRETCH) {
    printf("Mask border group default values incorrect.\n");
    return EXIT_FAILURE;
  }

  /* Test Filter Initialization */
  if (cmp_prop_filter_group_init(NULL) != -1) {
    printf("Failed to handle NULL in cmp_prop_filter_group_init.\n");
    return EXIT_FAILURE;
  }
  if (cmp_prop_filter_group_init(&filter) != 0) {
    printf("Failed to init filter group.\n");
    return EXIT_FAILURE;
  }
  if (filter.filter.value != NULL) {
    printf("Filter group default values incorrect.\n");
    return EXIT_FAILURE;
  }

  /* Test Blend Mode Initialization */
  if (cmp_prop_blend_mode_group_init(NULL) != -1) {
    printf("Failed to handle NULL in cmp_prop_blend_mode_group_init.\n");
    return EXIT_FAILURE;
  }
  if (cmp_prop_blend_mode_group_init(&blend) != 0) {
    printf("Failed to init blend mode group.\n");
    return EXIT_FAILURE;
  }
  if (blend.mix_blend_mode != CMP_BLEND_MODE_NORMAL) {
    printf("Blend mode group default values incorrect.\n");
    return EXIT_FAILURE;
  }

  printf("All CSS Masking, Clipping & Filters tests passed!\n");
  return EXIT_SUCCESS;
}
