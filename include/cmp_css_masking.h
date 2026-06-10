#ifndef CMP_CSS_MASKING_H
#define CMP_CSS_MASKING_H

/* clang-format off */
#include "cmp_css_values.h"
#include "cmp_css_box_model.h"
#include "cmp_css_backgrounds_borders.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_masking.h
 * @brief CSS Masking, Clipping, and Filters properties.
 */

/**
 * @brief CSS clip-rule values.
 */
typedef enum cmp_clip_rule {
  CMP_CLIP_RULE_NONZERO = 0,
  CMP_CLIP_RULE_EVENODD
} cmp_clip_rule_t;

/**
 * @brief CSS Geometry Box values.
 */
typedef enum cmp_geometry_box {
  CMP_GEOMETRY_BOX_BORDER_BOX = 0,
  CMP_GEOMETRY_BOX_PADDING_BOX,
  CMP_GEOMETRY_BOX_CONTENT_BOX,
  CMP_GEOMETRY_BOX_MARGIN_BOX,
  CMP_GEOMETRY_BOX_FILL_BOX,
  CMP_GEOMETRY_BOX_STROKE_BOX,
  CMP_GEOMETRY_BOX_VIEW_BOX
} cmp_geometry_box_t;

/**
 * @brief Type of clip-path property.
 */
typedef enum cmp_clip_path_type {
  CMP_CLIP_PATH_NONE = 0,
  CMP_CLIP_PATH_URL,
  CMP_CLIP_PATH_BASIC_SHAPE,
  CMP_CLIP_PATH_GEOMETRY_BOX
} cmp_clip_path_type_t;

/**
 * @brief Represents the clip-path property.
 */
typedef struct cmp_prop_clip_group {
  cmp_clip_path_type_t type;
  cmp_val_url_t url;
  cmp_geometry_box_t geometry_box;
  cmp_val_string_t basic_shape; /**< Placeholder for basic shape functions */
  cmp_clip_rule_t rule;
} cmp_prop_clip_group_t;

/**
 * @brief CSS mask-mode values.
 */
typedef enum cmp_mask_mode {
  CMP_MASK_MODE_MATCH_SOURCE = 0,
  CMP_MASK_MODE_LUMINANCE,
  CMP_MASK_MODE_ALPHA
} cmp_mask_mode_t;

/**
 * @brief CSS mask-composite values.
 */
typedef enum cmp_mask_composite {
  CMP_MASK_COMPOSITE_ADD = 0,
  CMP_MASK_COMPOSITE_SUBTRACT,
  CMP_MASK_COMPOSITE_INTERSECT,
  CMP_MASK_COMPOSITE_EXCLUDE
} cmp_mask_composite_t;

/**
 * @brief Represents the mask properties.
 */
typedef struct cmp_prop_mask_group {
  cmp_val_url_t image;
  cmp_mask_mode_t mode;
  cmp_bg_repeat_t repeat_x;
  cmp_bg_repeat_t repeat_y;
  cmp_prop_size_t position_x;
  cmp_prop_size_t position_y;
  cmp_geometry_box_t clip;
  int clip_no_clip; /**< 1 if no-clip, 0 if geometry_box is used */
  cmp_geometry_box_t origin;
  cmp_bg_size_t size;
  cmp_mask_composite_t composite;
} cmp_prop_mask_group_t;

/**
 * @brief CSS mask-type values.
 */
typedef enum cmp_mask_type {
  CMP_MASK_TYPE_LUMINANCE = 0,
  CMP_MASK_TYPE_ALPHA
} cmp_mask_type_t;

/**
 * @brief Represents the mask-border properties.
 */
typedef struct cmp_prop_mask_border_group {
  cmp_mask_type_t type;
  cmp_val_url_t source;
  cmp_mask_mode_t mode;
  cmp_prop_size_limits_t slice;
  cmp_prop_size_limits_t width;
  cmp_prop_size_limits_t outset;
  cmp_border_image_repeat_t repeat_x;
  cmp_border_image_repeat_t repeat_y;
} cmp_prop_mask_border_group_t;

/**
 * @brief Represents the filter and backdrop-filter properties.
 */
typedef struct cmp_prop_filter_group {
  cmp_val_string_t
      filter; /**< Simplified representation for filter-function-list */
  cmp_val_string_t backdrop_filter; /**< Simplified representation */
} cmp_prop_filter_group_t;

/**
 * @brief CSS Blend Mode values.
 */
typedef enum cmp_blend_mode {
  CMP_BLEND_MODE_NORMAL = 0,
  CMP_BLEND_MODE_MULTIPLY,
  CMP_BLEND_MODE_SCREEN,
  CMP_BLEND_MODE_OVERLAY,
  CMP_BLEND_MODE_DARKEN,
  CMP_BLEND_MODE_LIGHTEN,
  CMP_BLEND_MODE_COLOR_DODGE,
  CMP_BLEND_MODE_COLOR_BURN,
  CMP_BLEND_MODE_HARD_LIGHT,
  CMP_BLEND_MODE_SOFT_LIGHT,
  CMP_BLEND_MODE_DIFFERENCE,
  CMP_BLEND_MODE_EXCLUSION,
  CMP_BLEND_MODE_HUE,
  CMP_BLEND_MODE_SATURATION,
  CMP_BLEND_MODE_COLOR,
  CMP_BLEND_MODE_LUMINOSITY,
  CMP_BLEND_MODE_PLUS_LIGHTER,
  CMP_BLEND_MODE_PLUS_DARKER
} cmp_blend_mode_t;

/**
 * @brief Represents the blend mode properties.
 */
typedef struct cmp_prop_blend_mode_group {
  cmp_blend_mode_t mix_blend_mode;
  cmp_blend_mode_t background_blend_mode;
} cmp_prop_blend_mode_group_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes a clip group to default/none values.
 * @param clip The clip group to initialize.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_clip_group_init(cmp_prop_clip_group_t *clip);

/**
 * @brief Initializes a mask group to default values.
 * @param mask The mask group to initialize.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_mask_group_init(cmp_prop_mask_group_t *mask);

/**
 * @brief Initializes a mask border group to default values.
 * @param mask_border The mask border group to initialize.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_mask_border_group_init(cmp_prop_mask_border_group_t *mask_border);

/**
 * @brief Initializes a filter group to default values.
 * @param filter The filter group to initialize.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_filter_group_init(cmp_prop_filter_group_t *filter);

/**
 * @brief Initializes a blend mode group to default values.
 * @param blend_mode The blend mode group to initialize.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_blend_mode_group_init(cmp_prop_blend_mode_group_t *blend_mode);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_MASKING_H */
