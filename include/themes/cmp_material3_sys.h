#ifndef CMP_MATERIAL3_SYS_H
#define CMP_MATERIAL3_SYS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
/* clang-format on */

/**
 * @brief Dynamic Contrast Profiles
 */
typedef enum cmp_m3_contrast_profile {
  CMP_M3_CONTRAST_STANDARD = 0,
  CMP_M3_CONTRAST_MEDIUM = 1,
  CMP_M3_CONTRAST_HIGH = 2
} cmp_m3_contrast_profile_t;

/**
 * @brief Complete System Color Roles for Material 3
 */
typedef struct cmp_m3_sys_colors {
  cmp_color_t primary;
  cmp_color_t on_primary;
  cmp_color_t primary_container;
  cmp_color_t on_primary_container;
  cmp_color_t secondary;
  cmp_color_t on_secondary;
  cmp_color_t secondary_container;
  cmp_color_t on_secondary_container;
  cmp_color_t tertiary;
  cmp_color_t on_tertiary;
  cmp_color_t tertiary_container;
  cmp_color_t on_tertiary_container;
  cmp_color_t error;
  cmp_color_t on_error;
  cmp_color_t error_container;
  cmp_color_t on_error_container;
  cmp_color_t surface_dim;
  cmp_color_t surface;
  cmp_color_t surface_bright;
  cmp_color_t surface_container_lowest;
  cmp_color_t surface_container_low;
  cmp_color_t surface_container;
  cmp_color_t surface_container_high;
  cmp_color_t surface_container_highest;
  cmp_color_t on_surface;
  cmp_color_t on_surface_variant;
  cmp_color_t outline;
  cmp_color_t outline_variant;
  cmp_color_t inverse_surface;
  cmp_color_t inverse_on_surface;
  cmp_color_t inverse_primary;
  cmp_color_t scrim;
  cmp_color_t shadow;
  cmp_color_t surface_tint;
  cmp_color_t primary_fixed;
  cmp_color_t primary_fixed_dim;
  cmp_color_t on_primary_fixed;
  cmp_color_t on_primary_fixed_variant;
  cmp_color_t secondary_fixed;
  cmp_color_t secondary_fixed_dim;
  cmp_color_t on_secondary_fixed;
  cmp_color_t on_secondary_fixed_variant;
  cmp_color_t tertiary_fixed;
  cmp_color_t tertiary_fixed_dim;
  cmp_color_t on_tertiary_fixed;
  cmp_color_t on_tertiary_fixed_variant;
} cmp_m3_sys_colors_t;

/**
 * @brief System Shape Categories
 */
typedef enum cmp_m3_shape_family {
  CMP_M3_SHAPE_NONE,
  CMP_M3_SHAPE_EXTRA_SMALL,
  CMP_M3_SHAPE_SMALL,
  CMP_M3_SHAPE_MEDIUM,
  CMP_M3_SHAPE_LARGE,
  CMP_M3_SHAPE_EXTRA_LARGE,
  CMP_M3_SHAPE_FULL
} cmp_m3_shape_family_t;

/**
 * @brief System Shape Asymmetric Adjustments
 */
typedef struct cmp_m3_shape_modifiers {
  int top_left_override;
  int top_right_override;
  int bottom_left_override;
  int bottom_right_override;
} cmp_m3_shape_modifiers_t;

/**
 * @brief Tonal and Shadow Elevation Levels
 */
typedef enum cmp_m3_elevation_level {
  CMP_M3_ELEVATION_LEVEL_0,
  CMP_M3_ELEVATION_LEVEL_1,
  CMP_M3_ELEVATION_LEVEL_2,
  CMP_M3_ELEVATION_LEVEL_3,
  CMP_M3_ELEVATION_LEVEL_4,
  CMP_M3_ELEVATION_LEVEL_5
} cmp_m3_elevation_level_t;

/**
 * @brief Interactive State Layers
 */
typedef enum cmp_m3_state_layer {
  CMP_M3_STATE_HOVER,
  CMP_M3_STATE_FOCUS,
  CMP_M3_STATE_PRESSED,
  CMP_M3_STATE_DRAGGED,
  CMP_M3_STATE_DISABLED_CONTAINER,
  CMP_M3_STATE_DISABLED_CONTENT
} cmp_m3_state_layer_t;

CMP_API int cmp_m3_sys_colors_generate(cmp_color_t seed, int is_dark,
                                       cmp_m3_contrast_profile_t contrast,
                                       cmp_m3_sys_colors_t *out_sys_colors);

CMP_API int cmp_m3_shape_resolve(cmp_m3_shape_family_t shape,
                                 const cmp_m3_shape_modifiers_t *modifiers,
                                 float *out_tl, float *out_tr, float *out_bl,
                                 float *out_br);

CMP_API int cmp_m3_elevation_resolve(cmp_m3_elevation_level_t level,
                                     float *out_tonal_opacity,
                                     float *out_shadow_y_offset,
                                     float *out_shadow_blur,
                                     float *out_ambient_alpha,
                                     float *out_spot_alpha);

CMP_API int cmp_m3_state_layer_resolve(cmp_m3_state_layer_t state,
                                       float *out_opacity);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_MATERIAL3_SYS_H */