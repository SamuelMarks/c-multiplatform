#ifndef CMP_CSS_BACKGROUNDS_BORDERS_H
#define CMP_CSS_BACKGROUNDS_BORDERS_H

/* clang-format off */
#include "cmp_css_values.h"
#include "cmp_css_box_model.h"
#include "cmp_css_color.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Placeholder for a CSS Color (to be fully implemented in Section 7).
 */
typedef struct cmp_bg_color_placeholder {
  char *color_string; /**< Null-terminated color string */
} cmp_bg_color_placeholder_t;

/**
 * @brief CSS Background Repeat values.
 */
typedef enum cmp_bg_repeat {
  CMP_BG_REPEAT_REPEAT = 0,
  CMP_BG_REPEAT_SPACE,
  CMP_BG_REPEAT_ROUND,
  CMP_BG_REPEAT_NO_REPEAT
} cmp_bg_repeat_t;

/**
 * @brief CSS Background Attachment values.
 */
typedef enum cmp_bg_attachment {
  CMP_BG_ATTACHMENT_SCROLL = 0,
  CMP_BG_ATTACHMENT_FIXED,
  CMP_BG_ATTACHMENT_LOCAL
} cmp_bg_attachment_t;

/**
 * @brief CSS Background Box values (used for clip and origin).
 */
typedef enum cmp_bg_box {
  CMP_BG_BOX_BORDER_BOX = 0,
  CMP_BG_BOX_PADDING_BOX,
  CMP_BG_BOX_CONTENT_BOX,
  CMP_BG_BOX_TEXT
} cmp_bg_box_t;

/**
 * @brief CSS Background Size.
 */
typedef enum cmp_bg_size_type {
  CMP_BG_SIZE_AUTO = 0,
  CMP_BG_SIZE_COVER,
  CMP_BG_SIZE_CONTAIN,
  CMP_BG_SIZE_EXPLICIT
} cmp_bg_size_type_t;

typedef struct cmp_bg_size {
  cmp_bg_size_type_t type;
  cmp_prop_size_t width;
  cmp_prop_size_t height;
} cmp_bg_size_t;

/**
 * @brief Represents the background properties.
 */
typedef struct cmp_prop_bg_group {
  cmp_prop_color_t color;
  cmp_val_url_t image;
  cmp_bg_repeat_t repeat_x;
  cmp_bg_repeat_t repeat_y;
  cmp_bg_attachment_t attachment;
  cmp_prop_size_t position_x;
  cmp_prop_size_t position_y;
  cmp_bg_size_t size;
  cmp_bg_box_t clip;
  cmp_bg_box_t origin;
} cmp_prop_bg_group_t;

/**
 * @brief CSS Border Style values.
 */
typedef enum cmp_border_style {
  CMP_BORDER_STYLE_NONE = 0,
  CMP_BORDER_STYLE_HIDDEN,
  CMP_BORDER_STYLE_DOTTED,
  CMP_BORDER_STYLE_DASHED,
  CMP_BORDER_STYLE_SOLID,
  CMP_BORDER_STYLE_DOUBLE,
  CMP_BORDER_STYLE_GROOVE,
  CMP_BORDER_STYLE_RIDGE,
  CMP_BORDER_STYLE_INSET,
  CMP_BORDER_STYLE_OUTSET
} cmp_border_style_t;

/**
 * @brief Represents the border properties (width, style, color).
 */
typedef struct cmp_prop_border_group {
  cmp_prop_size_t width;
  cmp_border_style_t style;
  cmp_prop_color_t color;
} cmp_prop_border_group_t;

/**
 * @brief Represents border-radius properties.
 */
typedef struct cmp_prop_border_radius {
  cmp_prop_size_t top_left_x;
  cmp_prop_size_t top_left_y;
  cmp_prop_size_t top_right_x;
  cmp_prop_size_t top_right_y;
  cmp_prop_size_t bottom_right_x;
  cmp_prop_size_t bottom_right_y;
  cmp_prop_size_t bottom_left_x;
  cmp_prop_size_t bottom_left_y;
} cmp_prop_border_radius_t;

/**
 * @brief CSS Border Image Repeat values.
 */
typedef enum cmp_border_image_repeat {
  CMP_BORDER_IMAGE_REPEAT_STRETCH = 0,
  CMP_BORDER_IMAGE_REPEAT_REPEAT,
  CMP_BORDER_IMAGE_REPEAT_ROUND,
  CMP_BORDER_IMAGE_REPEAT_SPACE
} cmp_border_image_repeat_t;

/**
 * @brief Represents the border-image properties.
 */
typedef struct cmp_prop_border_image_group {
  cmp_val_url_t source;
  cmp_prop_size_limits_t slice; /* Reusing size limits as 4 sides */
  cmp_prop_size_limits_t width;
  cmp_prop_size_limits_t outset;
  cmp_border_image_repeat_t repeat_x;
  cmp_border_image_repeat_t repeat_y;
} cmp_prop_border_image_group_t;

/**
 * @brief Represents the box-shadow property.
 */
typedef struct cmp_prop_box_shadow {
  int inset; /* non-zero if inset */
  cmp_prop_size_t offset_x;
  cmp_prop_size_t offset_y;
  cmp_prop_size_t blur_radius;
  cmp_prop_size_t spread_radius;
  cmp_prop_color_t color;
} cmp_prop_box_shadow_t;

/**
 * @brief Represents the outline properties.
 */
typedef struct cmp_prop_outline_group {
  cmp_prop_size_t width;
  cmp_border_style_t style;
  cmp_prop_color_t color;
  cmp_prop_size_t offset;
} cmp_prop_outline_group_t;

/**
 * @brief CSS Box Decoration Break values.
 */
typedef enum cmp_prop_box_decoration_break {
  CMP_BOX_DECORATION_BREAK_SLICE = 0,
  CMP_BOX_DECORATION_BREAK_CLONE
} cmp_prop_box_decoration_break_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes a background group to default/empty values.
 * @param bg The background group to initialize.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_bg_group_init(cmp_prop_bg_group_t *bg);

/**
 * @brief Frees dynamically allocated resources in a background group.
 * @param bg The background group to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_bg_group_free(cmp_prop_bg_group_t *bg);

/**
 * @brief Initializes a border group to default values.
 * @param border The border group to initialize.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_border_group_init(cmp_prop_border_group_t *border);

/**
 * @brief Frees resources in a border group.
 * @param border The border group.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_border_group_free(cmp_prop_border_group_t *border);

/**
 * @brief Initializes a border-radius property to zeroes.
 * @param radius The border-radius property.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_border_radius_init(cmp_prop_border_radius_t *radius);

/**
 * @brief Initializes a border-image group to default values.
 * @param border_img The border-image group.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_border_image_group_init(cmp_prop_border_image_group_t *border_img);

/**
 * @brief Initializes a box-shadow property to default values.
 * @param shadow The box-shadow property.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_box_shadow_init(cmp_prop_box_shadow_t *shadow);

/**
 * @brief Frees resources in a box-shadow property.
 * @param shadow The box-shadow property.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_box_shadow_free(cmp_prop_box_shadow_t *shadow);

/**
 * @brief Initializes an outline group to default values.
 * @param outline The outline group.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_outline_group_init(cmp_prop_outline_group_t *outline);

/**
 * @brief Frees resources in an outline group.
 * @param outline The outline group.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_outline_group_free(cmp_prop_outline_group_t *outline);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_BACKGROUNDS_BORDERS_H */