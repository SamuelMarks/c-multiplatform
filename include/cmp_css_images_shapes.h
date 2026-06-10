#ifndef CMP_CSS_IMAGES_SHAPES_H
#define CMP_CSS_IMAGES_SHAPES_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_images_shapes.h
 * @brief CSS Images, Objects & Shapes implementation.
 */

/**
 * @brief Represents object-fit property.
 */
typedef enum cmp_object_fit {
  CMP_OBJECT_FIT_FILL = 0,
  CMP_OBJECT_FIT_CONTAIN,
  CMP_OBJECT_FIT_COVER,
  CMP_OBJECT_FIT_NONE,
  CMP_OBJECT_FIT_SCALE_DOWN
} cmp_object_fit_t;

/**
 * @brief Represents object-fit and object-position properties.
 */
typedef struct cmp_prop_object_fit_group {
  cmp_object_fit_t object_fit;
  char *object_position; /**< String representation of position */
} cmp_prop_object_fit_group_t;

/**
 * @brief Represents image-rendering property.
 */
typedef enum cmp_image_rendering {
  CMP_IMAGE_RENDERING_AUTO = 0,
  CMP_IMAGE_RENDERING_CRISP_EDGES,
  CMP_IMAGE_RENDERING_PIXELATED
} cmp_image_rendering_t;

/**
 * @brief Represents image-rendering.
 */
typedef struct cmp_prop_image_rendering {
  cmp_image_rendering_t rendering;
} cmp_prop_image_rendering_t;

/**
 * @brief Represents image-resolution property.
 */
typedef struct cmp_prop_image_resolution {
  char *resolution; /**< String representation of resolution */
} cmp_prop_image_resolution_t;

/**
 * @brief Represents CSS Shapes properties group.
 */
typedef struct cmp_prop_shape_group {
  char *shape_outside;         /**< String representation of shape */
  char *shape_margin;          /**< String representation of margin */
  char *shape_image_threshold; /**< String representation of threshold */
} cmp_prop_shape_group_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes object-fit group.
 * @param group The group to initialize.
 * @param fit object-fit value.
 * @param pos object-position string.
 * @return 0 on success.
 */
int cmp_prop_object_fit_group_init(cmp_prop_object_fit_group_t *group,
                                   cmp_object_fit_t fit, const char *pos);

/**
 * @brief Frees object-fit group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_object_fit_group_free(cmp_prop_object_fit_group_t *group);

/**
 * @brief Initializes image-rendering property.
 * @param prop The property to initialize.
 * @param rendering rendering value.
 * @return 0 on success.
 */
int cmp_prop_image_rendering_init(cmp_prop_image_rendering_t *prop,
                                  cmp_image_rendering_t rendering);

/**
 * @brief Initializes image-resolution property.
 * @param prop The property to initialize.
 * @param resolution resolution string.
 * @return 0 on success.
 */
int cmp_prop_image_resolution_init(cmp_prop_image_resolution_t *prop,
                                   const char *resolution);

/**
 * @brief Frees image-resolution property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_image_resolution_free(cmp_prop_image_resolution_t *prop);

/**
 * @brief Initializes shape group.
 * @param group The group to initialize.
 * @param outside shape-outside string.
 * @param margin shape-margin string.
 * @param threshold shape-image-threshold string.
 * @return 0 on success.
 */
int cmp_prop_shape_group_init(cmp_prop_shape_group_t *group,
                              const char *outside, const char *margin,
                              const char *threshold);

/**
 * @brief Frees shape group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_shape_group_free(cmp_prop_shape_group_t *group);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_IMAGES_SHAPES_H */
