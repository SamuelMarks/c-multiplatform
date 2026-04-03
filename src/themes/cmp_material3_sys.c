/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_sys.h"
#include "themes/cmp_material3_color.h"
#include <stddef.h>
/* clang-format on */

int cmp_m3_sys_colors_generate(cmp_color_t seed, int is_dark,
                               cmp_m3_contrast_profile_t contrast,
                               cmp_m3_sys_colors_t *out_sys_colors) {
  float h, c, t;
  if (!out_sys_colors) {
    return CMP_ERROR_INVALID_ARG;
  }

  cmp_m3_srgb_to_hct(&seed, &h, &c, &t);

  if (is_dark) {
    if (contrast == CMP_M3_CONTRAST_HIGH) {
      cmp_m3_hct_to_srgb(h, c, 100.0f, &out_sys_colors->primary);
    } else if (contrast == CMP_M3_CONTRAST_MEDIUM) {
      cmp_m3_hct_to_srgb(h, c, 90.0f, &out_sys_colors->primary);
    } else {
      cmp_m3_hct_to_srgb(h, c, 80.0f, &out_sys_colors->primary);
    }
    cmp_m3_hct_to_srgb(h, c, 20.0f, &out_sys_colors->on_primary);
    cmp_m3_hct_to_srgb(h, c, 30.0f, &out_sys_colors->primary_container);
    cmp_m3_hct_to_srgb(h, c, 90.0f, &out_sys_colors->on_primary_container);

    cmp_m3_hct_to_srgb(h, c / 3.0f, 80.0f, &out_sys_colors->secondary);
    cmp_m3_hct_to_srgb(h, c / 3.0f, 20.0f, &out_sys_colors->on_secondary);
    cmp_m3_hct_to_srgb(h, c / 3.0f, 30.0f,
                       &out_sys_colors->secondary_container);
    cmp_m3_hct_to_srgb(h, c / 3.0f, 90.0f,
                       &out_sys_colors->on_secondary_container);

    cmp_m3_hct_to_srgb(h + 60.0f, c / 2.0f, 80.0f, &out_sys_colors->tertiary);
    cmp_m3_hct_to_srgb(h + 60.0f, c / 2.0f, 20.0f,
                       &out_sys_colors->on_tertiary);
    cmp_m3_hct_to_srgb(h + 60.0f, c / 2.0f, 30.0f,
                       &out_sys_colors->tertiary_container);
    cmp_m3_hct_to_srgb(h + 60.0f, c / 2.0f, 90.0f,
                       &out_sys_colors->on_tertiary_container);

    cmp_m3_hct_to_srgb(25.0f, 84.0f, 80.0f, &out_sys_colors->error);
    cmp_m3_hct_to_srgb(25.0f, 84.0f, 20.0f, &out_sys_colors->on_error);
    cmp_m3_hct_to_srgb(25.0f, 84.0f, 30.0f, &out_sys_colors->error_container);
    cmp_m3_hct_to_srgb(25.0f, 84.0f, 90.0f,
                       &out_sys_colors->on_error_container);

    cmp_m3_hct_to_srgb(h, 4.0f, 6.0f, &out_sys_colors->surface_dim);
    cmp_m3_hct_to_srgb(h, 4.0f, 6.0f, &out_sys_colors->surface);
    cmp_m3_hct_to_srgb(h, 4.0f, 24.0f, &out_sys_colors->surface_bright);

    cmp_m3_hct_to_srgb(h, 4.0f, 4.0f,
                       &out_sys_colors->surface_container_lowest);
    cmp_m3_hct_to_srgb(h, 4.0f, 10.0f, &out_sys_colors->surface_container_low);
    cmp_m3_hct_to_srgb(h, 4.0f, 12.0f, &out_sys_colors->surface_container);
    cmp_m3_hct_to_srgb(h, 4.0f, 17.0f, &out_sys_colors->surface_container_high);
    cmp_m3_hct_to_srgb(h, 4.0f, 22.0f,
                       &out_sys_colors->surface_container_highest);

    cmp_m3_hct_to_srgb(h, 4.0f, 90.0f, &out_sys_colors->on_surface);
    cmp_m3_hct_to_srgb(h, 8.0f, 80.0f, &out_sys_colors->on_surface_variant);
    cmp_m3_hct_to_srgb(h, 8.0f, 60.0f, &out_sys_colors->outline);
    cmp_m3_hct_to_srgb(h, 8.0f, 30.0f, &out_sys_colors->outline_variant);

    cmp_m3_hct_to_srgb(h, 4.0f, 90.0f, &out_sys_colors->inverse_surface);
    cmp_m3_hct_to_srgb(h, 4.0f, 20.0f, &out_sys_colors->inverse_on_surface);
    cmp_m3_hct_to_srgb(h, c, 40.0f, &out_sys_colors->inverse_primary);

    cmp_m3_hct_to_srgb(h, 4.0f, 0.0f, &out_sys_colors->scrim);
    cmp_m3_hct_to_srgb(h, 4.0f, 0.0f, &out_sys_colors->shadow);
    cmp_m3_hct_to_srgb(h, c, 80.0f, &out_sys_colors->surface_tint);

    cmp_m3_hct_to_srgb(h, c, 90.0f, &out_sys_colors->primary_fixed);
    cmp_m3_hct_to_srgb(h, c, 80.0f, &out_sys_colors->primary_fixed_dim);
    cmp_m3_hct_to_srgb(h, c, 10.0f, &out_sys_colors->on_primary_fixed);
    cmp_m3_hct_to_srgb(h, c, 30.0f, &out_sys_colors->on_primary_fixed_variant);

    cmp_m3_hct_to_srgb(h, c / 3.0f, 90.0f, &out_sys_colors->secondary_fixed);
    cmp_m3_hct_to_srgb(h, c / 3.0f, 80.0f,
                       &out_sys_colors->secondary_fixed_dim);
    cmp_m3_hct_to_srgb(h, c / 3.0f, 10.0f, &out_sys_colors->on_secondary_fixed);
    cmp_m3_hct_to_srgb(h, c / 3.0f, 30.0f,
                       &out_sys_colors->on_secondary_fixed_variant);

    cmp_m3_hct_to_srgb(h + 60.0f, c / 2.0f, 90.0f,
                       &out_sys_colors->tertiary_fixed);
    cmp_m3_hct_to_srgb(h + 60.0f, c / 2.0f, 80.0f,
                       &out_sys_colors->tertiary_fixed_dim);
    cmp_m3_hct_to_srgb(h + 60.0f, c / 2.0f, 10.0f,
                       &out_sys_colors->on_tertiary_fixed);
    cmp_m3_hct_to_srgb(h + 60.0f, c / 2.0f, 30.0f,
                       &out_sys_colors->on_tertiary_fixed_variant);

  } else {
    if (contrast == CMP_M3_CONTRAST_HIGH) {
      cmp_m3_hct_to_srgb(h, c, 20.0f, &out_sys_colors->primary);
    } else if (contrast == CMP_M3_CONTRAST_MEDIUM) {
      cmp_m3_hct_to_srgb(h, c, 30.0f, &out_sys_colors->primary);
    } else {
      cmp_m3_hct_to_srgb(h, c, 40.0f, &out_sys_colors->primary);
    }
    cmp_m3_hct_to_srgb(h, c, 100.0f, &out_sys_colors->on_primary);
    cmp_m3_hct_to_srgb(h, c, 90.0f, &out_sys_colors->primary_container);
    cmp_m3_hct_to_srgb(h, c, 10.0f, &out_sys_colors->on_primary_container);

    cmp_m3_hct_to_srgb(h, c / 3.0f, 40.0f, &out_sys_colors->secondary);
    cmp_m3_hct_to_srgb(h, c / 3.0f, 100.0f, &out_sys_colors->on_secondary);
    cmp_m3_hct_to_srgb(h, c / 3.0f, 90.0f,
                       &out_sys_colors->secondary_container);
    cmp_m3_hct_to_srgb(h, c / 3.0f, 10.0f,
                       &out_sys_colors->on_secondary_container);

    cmp_m3_hct_to_srgb(h + 60.0f, c / 2.0f, 40.0f, &out_sys_colors->tertiary);
    cmp_m3_hct_to_srgb(h + 60.0f, c / 2.0f, 100.0f,
                       &out_sys_colors->on_tertiary);
    cmp_m3_hct_to_srgb(h + 60.0f, c / 2.0f, 90.0f,
                       &out_sys_colors->tertiary_container);
    cmp_m3_hct_to_srgb(h + 60.0f, c / 2.0f, 10.0f,
                       &out_sys_colors->on_tertiary_container);

    cmp_m3_hct_to_srgb(25.0f, 84.0f, 40.0f, &out_sys_colors->error);
    cmp_m3_hct_to_srgb(25.0f, 84.0f, 100.0f, &out_sys_colors->on_error);
    cmp_m3_hct_to_srgb(25.0f, 84.0f, 90.0f, &out_sys_colors->error_container);
    cmp_m3_hct_to_srgb(25.0f, 84.0f, 10.0f,
                       &out_sys_colors->on_error_container);

    cmp_m3_hct_to_srgb(h, 4.0f, 87.0f, &out_sys_colors->surface_dim);
    cmp_m3_hct_to_srgb(h, 4.0f, 98.0f, &out_sys_colors->surface);
    cmp_m3_hct_to_srgb(h, 4.0f, 98.0f, &out_sys_colors->surface_bright);

    cmp_m3_hct_to_srgb(h, 4.0f, 100.0f,
                       &out_sys_colors->surface_container_lowest);
    cmp_m3_hct_to_srgb(h, 4.0f, 96.0f, &out_sys_colors->surface_container_low);
    cmp_m3_hct_to_srgb(h, 4.0f, 94.0f, &out_sys_colors->surface_container);
    cmp_m3_hct_to_srgb(h, 4.0f, 92.0f, &out_sys_colors->surface_container_high);
    cmp_m3_hct_to_srgb(h, 4.0f, 90.0f,
                       &out_sys_colors->surface_container_highest);

    cmp_m3_hct_to_srgb(h, 4.0f, 10.0f, &out_sys_colors->on_surface);
    cmp_m3_hct_to_srgb(h, 8.0f, 30.0f, &out_sys_colors->on_surface_variant);
    cmp_m3_hct_to_srgb(h, 8.0f, 50.0f, &out_sys_colors->outline);
    cmp_m3_hct_to_srgb(h, 8.0f, 80.0f, &out_sys_colors->outline_variant);

    cmp_m3_hct_to_srgb(h, 4.0f, 20.0f, &out_sys_colors->inverse_surface);
    cmp_m3_hct_to_srgb(h, 4.0f, 95.0f, &out_sys_colors->inverse_on_surface);
    cmp_m3_hct_to_srgb(h, c, 80.0f, &out_sys_colors->inverse_primary);

    cmp_m3_hct_to_srgb(h, 4.0f, 0.0f, &out_sys_colors->scrim);
    cmp_m3_hct_to_srgb(h, 4.0f, 0.0f, &out_sys_colors->shadow);
    cmp_m3_hct_to_srgb(h, c, 40.0f, &out_sys_colors->surface_tint);

    cmp_m3_hct_to_srgb(h, c, 90.0f, &out_sys_colors->primary_fixed);
    cmp_m3_hct_to_srgb(h, c, 80.0f, &out_sys_colors->primary_fixed_dim);
    cmp_m3_hct_to_srgb(h, c, 10.0f, &out_sys_colors->on_primary_fixed);
    cmp_m3_hct_to_srgb(h, c, 30.0f, &out_sys_colors->on_primary_fixed_variant);

    cmp_m3_hct_to_srgb(h, c / 3.0f, 90.0f, &out_sys_colors->secondary_fixed);
    cmp_m3_hct_to_srgb(h, c / 3.0f, 80.0f,
                       &out_sys_colors->secondary_fixed_dim);
    cmp_m3_hct_to_srgb(h, c / 3.0f, 10.0f, &out_sys_colors->on_secondary_fixed);
    cmp_m3_hct_to_srgb(h, c / 3.0f, 30.0f,
                       &out_sys_colors->on_secondary_fixed_variant);

    cmp_m3_hct_to_srgb(h + 60.0f, c / 2.0f, 90.0f,
                       &out_sys_colors->tertiary_fixed);
    cmp_m3_hct_to_srgb(h + 60.0f, c / 2.0f, 80.0f,
                       &out_sys_colors->tertiary_fixed_dim);
    cmp_m3_hct_to_srgb(h + 60.0f, c / 2.0f, 10.0f,
                       &out_sys_colors->on_tertiary_fixed);
    cmp_m3_hct_to_srgb(h + 60.0f, c / 2.0f, 30.0f,
                       &out_sys_colors->on_tertiary_fixed_variant);
  }

  return CMP_SUCCESS;
}

int cmp_m3_shape_resolve(cmp_m3_shape_family_t shape,
                         const cmp_m3_shape_modifiers_t *modifiers,
                         float *out_tl, float *out_tr, float *out_bl,
                         float *out_br) {
  float base_radius = 0.0f;

  if (!out_tl || !out_tr || !out_bl || !out_br) {
    return CMP_ERROR_INVALID_ARG;
  }

  switch (shape) {
  case CMP_M3_SHAPE_NONE:
    base_radius = 0.0f;
    break;
  case CMP_M3_SHAPE_EXTRA_SMALL:
    base_radius = 4.0f;
    break;
  case CMP_M3_SHAPE_SMALL:
    base_radius = 8.0f;
    break;
  case CMP_M3_SHAPE_MEDIUM:
    base_radius = 12.0f;
    break;
  case CMP_M3_SHAPE_LARGE:
    base_radius = 16.0f;
    break;
  case CMP_M3_SHAPE_EXTRA_LARGE:
    base_radius = 28.0f;
    break;
  case CMP_M3_SHAPE_FULL:
    base_radius = 9999.0f;
    break; /* High value for circular */
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  *out_tl = base_radius;
  *out_tr = base_radius;
  *out_bl = base_radius;
  *out_br = base_radius;

  if (modifiers) {
    if (modifiers->top_left_override >= 0)
      *out_tl = (float)modifiers->top_left_override;
    if (modifiers->top_right_override >= 0)
      *out_tr = (float)modifiers->top_right_override;
    if (modifiers->bottom_left_override >= 0)
      *out_bl = (float)modifiers->bottom_left_override;
    if (modifiers->bottom_right_override >= 0)
      *out_br = (float)modifiers->bottom_right_override;
  }

  return CMP_SUCCESS;
}

int cmp_m3_elevation_resolve(cmp_m3_elevation_level_t level,
                             float *out_tonal_opacity,
                             float *out_shadow_y_offset, float *out_shadow_blur,
                             float *out_ambient_alpha, float *out_spot_alpha) {
  if (!out_tonal_opacity || !out_shadow_y_offset || !out_shadow_blur ||
      !out_ambient_alpha || !out_spot_alpha) {
    return CMP_ERROR_INVALID_ARG;
  }

  switch (level) {
  case CMP_M3_ELEVATION_LEVEL_0:
    *out_tonal_opacity = 0.0f;
    *out_shadow_y_offset = 0.0f;
    *out_shadow_blur = 0.0f;
    *out_ambient_alpha = 0.0f;
    *out_spot_alpha = 0.0f;
    break;
  case CMP_M3_ELEVATION_LEVEL_1:
    *out_tonal_opacity = 0.05f;
    *out_shadow_y_offset = 1.0f;
    *out_shadow_blur = 3.0f;
    *out_ambient_alpha = 0.05f;
    *out_spot_alpha = 0.15f;
    break;
  case CMP_M3_ELEVATION_LEVEL_2:
    *out_tonal_opacity = 0.08f;
    *out_shadow_y_offset = 3.0f;
    *out_shadow_blur = 6.0f;
    *out_ambient_alpha = 0.05f;
    *out_spot_alpha = 0.20f;
    break;
  case CMP_M3_ELEVATION_LEVEL_3:
    *out_tonal_opacity = 0.11f;
    *out_shadow_y_offset = 6.0f;
    *out_shadow_blur = 10.0f;
    *out_ambient_alpha = 0.06f;
    *out_spot_alpha = 0.22f;
    break;
  case CMP_M3_ELEVATION_LEVEL_4:
    *out_tonal_opacity = 0.12f;
    *out_shadow_y_offset = 8.0f;
    *out_shadow_blur = 12.0f;
    *out_ambient_alpha = 0.07f;
    *out_spot_alpha = 0.25f;
    break;
  case CMP_M3_ELEVATION_LEVEL_5:
    *out_tonal_opacity = 0.14f;
    *out_shadow_y_offset = 12.0f;
    *out_shadow_blur = 16.0f;
    *out_ambient_alpha = 0.08f;
    *out_spot_alpha = 0.30f;
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}

int cmp_m3_state_layer_resolve(cmp_m3_state_layer_t state, float *out_opacity) {
  if (!out_opacity) {
    return CMP_ERROR_INVALID_ARG;
  }

  switch (state) {
  case CMP_M3_STATE_HOVER:
    *out_opacity = 0.08f;
    break;
  case CMP_M3_STATE_FOCUS:
    *out_opacity = 0.10f;
    break;
  case CMP_M3_STATE_PRESSED:
    *out_opacity = 0.10f;
    break;
  case CMP_M3_STATE_DRAGGED:
    *out_opacity = 0.16f;
    break;
  case CMP_M3_STATE_DISABLED_CONTAINER:
    *out_opacity = 0.12f;
    break;
  case CMP_M3_STATE_DISABLED_CONTENT:
    *out_opacity = 0.38f;
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}