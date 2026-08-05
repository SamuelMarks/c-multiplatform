#ifndef UI_SHAPE_FAMILY_H
#define UI_SHAPE_FAMILY_H

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Standardized shape families for component corners.
 */
enum ui_shape_family {
  UI_SHAPE_FAMILY_NONE,        /**< 0dp */
  UI_SHAPE_FAMILY_EXTRA_SMALL, /**< 4dp */
  UI_SHAPE_FAMILY_SMALL,       /**< 8dp */
  UI_SHAPE_FAMILY_MEDIUM,      /**< 12dp */
  UI_SHAPE_FAMILY_LARGE,       /**< 16dp */
  UI_SHAPE_FAMILY_EXTRA_LARGE, /**< 28dp */
  UI_SHAPE_FAMILY_PILL         /**< Fully rounded (e.g., 9999dp) */
};

/**
 * @brief Retrieves the default corner radius for a given shape family.
 *
 * @param family The shape family.
 * @param out_radius Pointer to store the resulting radius (in logical pixels).
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_shape_family_get_radius(enum ui_shape_family family,
                                      float *out_radius);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SHAPE_FAMILY_H */
