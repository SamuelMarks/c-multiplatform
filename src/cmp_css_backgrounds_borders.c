/* clang-format off */
#include "cmp_css_backgrounds_borders.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static int cmp_strdup_safe(const char *src, char **dst) {
  size_t len;
  if (!dst)
    return -1;
  if (!src) {
    *dst = NULL;
    return 0;
  }
  len = strlen(src);
  *dst = (char *)malloc(len + 1);
  if (!*dst)
    return -1;
#if defined(_MSC_VER)
  strcpy_s(*dst, len + 1, src);
#else
  strcpy(*dst, src);
#endif
  return 0;
}

int cmp_prop_bg_group_init(cmp_prop_bg_group_t *bg) {
  if (!bg)
    return -1;
  memset(bg, 0, sizeof(cmp_prop_bg_group_t));

  bg->repeat_x = CMP_BG_REPEAT_REPEAT;
  bg->repeat_y = CMP_BG_REPEAT_REPEAT;
  bg->attachment = CMP_BG_ATTACHMENT_SCROLL;
  bg->clip = CMP_BG_BOX_BORDER_BOX;
  bg->origin = CMP_BG_BOX_PADDING_BOX;
  bg->size.type = CMP_BG_SIZE_AUTO;
  if (cmp_prop_size_init_auto(&bg->position_x) != 0)
    return -1;
  if (cmp_prop_size_init_auto(&bg->position_y) != 0)
    return -1;
  return 0;
}

int cmp_prop_bg_group_free(cmp_prop_bg_group_t *bg) {
  if (!bg)
    return -1;
  if (1) {
    (void)cmp_prop_color_free(&bg->color); /* ignore */
  }
  /* Assuming cmp_val_url_t doesn't own its string (as per length/pointer
   * definition in cmp_css_values.h) */
  return 0;
}

int cmp_prop_border_group_init(cmp_prop_border_group_t *border) {
  if (!border)
    return -1;
  memset(border, 0, sizeof(cmp_prop_border_group_t));
  border->style = CMP_BORDER_STYLE_NONE;
  if (cmp_prop_size_init_phys(&border->width, 0.0f, CMP_LENGTH_PX) != 0)
    return -1;
  if (cmp_prop_color_init_transparent(&border->color) != 0)
    return -1;
  return 0;
}

int cmp_prop_border_group_free(cmp_prop_border_group_t *border) {
  if (!border)
    return -1;
  if (1) {
    (void)cmp_prop_color_free(&border->color); /* ignore */

    if (cmp_prop_color_init_transparent(&border->color) != 0)
      return -1;
  }
  return 0;
}

int cmp_prop_border_radius_init(cmp_prop_border_radius_t *radius) {
  if (!radius)
    return -1;
  memset(radius, 0, sizeof(cmp_prop_border_radius_t));
  if (cmp_prop_size_init_phys(&radius->top_left_x, 0.0f, CMP_LENGTH_PX) != 0)
    return -1;
  if (cmp_prop_size_init_phys(&radius->top_left_y, 0.0f, CMP_LENGTH_PX) != 0)
    return -1;
  if (cmp_prop_size_init_phys(&radius->top_right_x, 0.0f, CMP_LENGTH_PX) != 0)
    return -1;
  if (cmp_prop_size_init_phys(&radius->top_right_y, 0.0f, CMP_LENGTH_PX) != 0)
    return -1;
  if (cmp_prop_size_init_phys(&radius->bottom_right_x, 0.0f, CMP_LENGTH_PX) !=
      0)
    return -1;
  if (cmp_prop_size_init_phys(&radius->bottom_right_y, 0.0f, CMP_LENGTH_PX) !=
      0)
    return -1;
  if (cmp_prop_size_init_phys(&radius->bottom_left_x, 0.0f, CMP_LENGTH_PX) != 0)
    return -1;
  if (cmp_prop_size_init_phys(&radius->bottom_left_y, 0.0f, CMP_LENGTH_PX) != 0)
    return -1;
  return 0;
}

int cmp_prop_border_image_group_init(
    cmp_prop_border_image_group_t *border_img) {
  if (!border_img)
    return -1;
  memset(border_img, 0, sizeof(cmp_prop_border_image_group_t));
  border_img->repeat_x = CMP_BORDER_IMAGE_REPEAT_STRETCH;
  border_img->repeat_y = CMP_BORDER_IMAGE_REPEAT_STRETCH;
  return 0;
}

int cmp_prop_box_shadow_init(cmp_prop_box_shadow_t *shadow) {
  if (!shadow)
    return -1;
  memset(shadow, 0, sizeof(cmp_prop_box_shadow_t));
  shadow->inset = 0;
  if (cmp_prop_size_init_phys(&shadow->offset_x, 0.0f, CMP_LENGTH_PX) != 0)
    return -1;
  if (cmp_prop_size_init_phys(&shadow->offset_y, 0.0f, CMP_LENGTH_PX) != 0)
    return -1;
  if (cmp_prop_size_init_phys(&shadow->blur_radius, 0.0f, CMP_LENGTH_PX) != 0)
    return -1;
  if (cmp_prop_size_init_phys(&shadow->spread_radius, 0.0f, CMP_LENGTH_PX) != 0)
    return -1;
  if (cmp_prop_color_init_transparent(&shadow->color) != 0)
    return -1;
  return 0;
}

int cmp_prop_box_shadow_free(cmp_prop_box_shadow_t *shadow) {
  if (!shadow)
    return -1;
  if (1) {
    (void)cmp_prop_color_free(&shadow->color); /* ignore */

    if (cmp_prop_color_init_transparent(&shadow->color) != 0)
      return -1;
  }
  return 0;
}

int cmp_prop_outline_group_init(cmp_prop_outline_group_t *outline) {
  if (!outline)
    return -1;
  memset(outline, 0, sizeof(cmp_prop_outline_group_t));
  if (cmp_prop_size_init_phys(&outline->width, 0.0f, CMP_LENGTH_PX) != 0)
    return -1;
  outline->style = CMP_BORDER_STYLE_NONE;
  if (cmp_prop_size_init_phys(&outline->offset, 0.0f, CMP_LENGTH_PX) != 0)
    return -1;
  if (cmp_prop_color_init_transparent(&outline->color) != 0)
    return -1;
  return 0;
}

int cmp_prop_outline_group_free(cmp_prop_outline_group_t *outline) {
  if (!outline)
    return -1;
  if (1) {
    (void)cmp_prop_color_free(&outline->color); /* ignore */

    if (cmp_prop_color_init_transparent(&outline->color) != 0)
      return -1;
  }
  return 0;
}