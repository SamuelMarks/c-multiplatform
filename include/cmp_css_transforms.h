#ifndef CMP_CSS_TRANSFORMS_H
#define CMP_CSS_TRANSFORMS_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_transforms.h
 * @brief CSS Transforms & Motion Path (Level 1 & 2) implementation.
 */

/**
 * @brief Represents backface-visibility values.
 */
typedef enum cmp_backface_visibility {
  CMP_BACKFACE_VISIBILITY_VISIBLE = 0,
  CMP_BACKFACE_VISIBILITY_HIDDEN
} cmp_backface_visibility_t;

/**
 * @brief Represents transform-style values.
 */
typedef enum cmp_transform_style {
  CMP_TRANSFORM_STYLE_FLAT = 0,
  CMP_TRANSFORM_STYLE_PRESERVE_3D
} cmp_transform_style_t;

/**
 * @brief Represents transform, transform-origin, transform-style, perspective,
 * perspective-origin, backface-visibility.
 */
typedef struct cmp_prop_transform_group {
  char *transform;        /**< transform (e.g., "scale(2)") */
  char *transform_origin; /**< transform-origin (e.g., "50% 50%") */
  cmp_transform_style_t transform_style; /**< transform-style */
  char *perspective;                     /**< perspective (e.g., "1000px") */
  char *perspective_origin; /**< perspective-origin (e.g., "50% 50%") */
  cmp_backface_visibility_t backface; /**< backface-visibility */
} cmp_prop_transform_group_t;

/**
 * @brief Represents independent transform properties (translate, rotate,
 * scale).
 */
typedef struct cmp_prop_independent_transform {
  char *translate; /**< translate (e.g., "50px 100px") */
  char *rotate;    /**< rotate (e.g., "1turn") */
  char *scale;     /**< scale (e.g., "2 0.5") */
} cmp_prop_independent_transform_t;

/**
 * @brief Types of transform functions.
 */
typedef enum cmp_transform_func_type {
  CMP_TRANSFORM_FUNC_MATRIX = 0,
  CMP_TRANSFORM_FUNC_MATRIX3D,
  CMP_TRANSFORM_FUNC_TRANSLATE3D,
  CMP_TRANSFORM_FUNC_ROTATE3D,
  CMP_TRANSFORM_FUNC_SCALE3D
} cmp_transform_func_type_t;

/**
 * @brief Represents a parsed matrix(), matrix3d(), or 3d function
 * representation.
 */
typedef struct cmp_transform_funcs {
  cmp_transform_func_type_t type;
  char *args; /**< String representation of arguments (e.g., "1, 0, 0, 1, 0, 0")
               */
} cmp_transform_funcs_t;

/**
 * @brief Represents offset-path, offset-distance, offset-position,
 * offset-anchor, offset-rotate.
 */
typedef struct cmp_prop_motion_path_group {
  char *path;     /**< offset-path (e.g., "path('M 0 0 L 100 100')") */
  char *distance; /**< offset-distance (e.g., "50%") */
  char *position; /**< offset-position (e.g., "auto") */
  char *anchor;   /**< offset-anchor (e.g., "center") */
  char *rotate;   /**< offset-rotate (e.g., "auto 90deg") */
} cmp_prop_motion_path_group_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes a transform property group.
 * @param group The transform group to initialize.
 * @param transform transform string (can be NULL).
 * @param origin transform-origin string (can be NULL).
 * @param style transform-style enum.
 * @param perspective perspective string (can be NULL).
 * @param perspective_origin perspective-origin string (can be NULL).
 * @param backface backface-visibility enum.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_transform_group_init(cmp_prop_transform_group_t *group,
                                  const char *transform, const char *origin,
                                  cmp_transform_style_t style,
                                  const char *perspective,
                                  const char *perspective_origin,
                                  cmp_backface_visibility_t backface);

/**
 * @brief Frees resources in a transform property group.
 * @param group The group to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_transform_group_free(cmp_prop_transform_group_t *group);

/**
 * @brief Initializes independent transform properties.
 * @param group The independent transform group to initialize.
 * @param translate translate string (can be NULL).
 * @param rotate rotate string (can be NULL).
 * @param scale scale string (can be NULL).
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_independent_transform_init(cmp_prop_independent_transform_t *group,
                                        const char *translate,
                                        const char *rotate, const char *scale);

/**
 * @brief Frees resources in an independent transform group.
 * @param group The group to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_independent_transform_free(
    cmp_prop_independent_transform_t *group);

/**
 * @brief Initializes a transform function representation.
 * @param func The function structure.
 * @param type The type of function (matrix, matrix3d, translate3d, rotate3d,
 * scale3d).
 * @param args The arguments inside the function.
 * @return 0 on success, non-zero on error.
 */
int cmp_transform_funcs_init(cmp_transform_funcs_t *func,
                             cmp_transform_func_type_t type, const char *args);

/**
 * @brief Frees resources in a transform function.
 * @param func The function to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_transform_funcs_free(cmp_transform_funcs_t *func);

/**
 * @brief Initializes a motion path group.
 * @param group The motion path group to initialize.
 * @param path offset-path string (can be NULL).
 * @param distance offset-distance string (can be NULL).
 * @param position offset-position string (can be NULL).
 * @param anchor offset-anchor string (can be NULL).
 * @param rotate offset-rotate string (can be NULL).
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_motion_path_group_init(cmp_prop_motion_path_group_t *group,
                                    const char *path, const char *distance,
                                    const char *position, const char *anchor,
                                    const char *rotate);

/**
 * @brief Frees resources in a motion path group.
 * @param group The group to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_motion_path_group_free(cmp_prop_motion_path_group_t *group);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_TRANSFORMS_H */