/**
 * @file md3_shape.h
 * @brief Material 3 baseline shape scale and Expressive shape definitions.
 */

#ifndef MATERIAL3_MD3_SHAPE_H
#define MATERIAL3_MD3_SHAPE_H

/* clang-format off */
#include "ui_design_tokens.h"
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum md3_shape_scale
 * @brief Corner radius scale in Material 3.
 */
enum md3_shape_scale {
  MD3_SHAPE_NONE = 0,    /**< 0dp */
  MD3_SHAPE_EXTRA_SMALL, /**< 4dp */
  MD3_SHAPE_SMALL,       /**< 8dp */
  MD3_SHAPE_MEDIUM,      /**< 12dp */
  MD3_SHAPE_LARGE,       /**< 16dp */
  MD3_SHAPE_EXTRA_LARGE, /**< 28dp */
  MD3_SHAPE_FULL,        /**< 9999dp (pill / circle) */
  MD3_SHAPE_SCALE_COUNT
};

/**
 * @brief Retrieves the corner radius in dp for a given shape scale.
 *
 * @param scale The shape scale.
 * @param out_radius_dp Pointer to receive the radius in dp.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT on bad
 * scale/pointer.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_shape_get_corner_radius(enum md3_shape_scale scale, float *out_radius_dp);

/**
 * @brief Injects shape tokens into a design token dictionary.
 *
 * @param is_expressive Non-zero to inject expressive shape tokens.
 * @param dict Pointer to the design token dictionary.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_shape_apply_tokens(int is_expressive, struct ui_design_token_dict *dict);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_SHAPE_H */
