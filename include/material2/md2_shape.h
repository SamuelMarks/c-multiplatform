/**
 * @file md2_shape.h
 * @brief Material Design 2 shape system and corner radiuses.
 */

#ifndef MATERIAL2_MD2_SHAPE_H
#define MATERIAL2_MD2_SHAPE_H

/* clang-format off */
#include "ui_error.h"
#include "ui_export.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum md2_shape_category
 * @brief Material Design 2 shape categories.
 */
enum md2_shape_category {
  MD2_SHAPE_CATEGORY_SMALL_COMPONENT = 0,
  MD2_SHAPE_CATEGORY_MEDIUM_COMPONENT = 1,
  MD2_SHAPE_CATEGORY_LARGE_COMPONENT = 2
};

/**
 * @brief Retrieves the corner radius in dp for a given Material Design 2 shape
 * category.
 *
 * @param category Shape category token.
 * @param out_radius_dp Pointer receiving the float dp radius value.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_shape_get_corner_radius(
    enum md2_shape_category category, float *out_radius_dp);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL2_MD2_SHAPE_H */
