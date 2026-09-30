/**
 * @file md3_elevation.h
 * @brief Material 3 elevation levels, tonal surface tinting, and shadow
 * metrics.
 */

#ifndef MATERIAL3_MD3_ELEVATION_H
#define MATERIAL3_MD3_ELEVATION_H

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
#include "ui_color_space.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Retrieves the elevation depth in dp for a given level (0 to 5).
 *
 * @param level Elevation level (0 through 5).
 * @param out_dp Pointer to receive elevation in dp.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT on bad
 * level/pointer.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_elevation_get_dp(int level,
                                                              float *out_dp);

/**
 * @brief Retrieves the surface tint overlay opacity for a given level (0 to 5).
 *
 * @param level Elevation level (0 through 5).
 * @param out_opacity Pointer to receive the opacity fraction (0.0 to 1.0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT on bad
 * level/pointer.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_elevation_get_tint_opacity(int level, float *out_opacity);

/**
 * @brief Computes the composite elevated surface color using surface tint
 * blending.
 *
 * @param surface_base Base surface color.
 * @param primary_tint The primary color used for surface tinting.
 * @param level Elevation level (0 through 5).
 * @param out_color Pointer to store the blended result color.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_elevation_get_surface_color(
    ui_color_t surface_base, ui_color_t primary_tint, int level,
    ui_color_t *out_color);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_ELEVATION_H */
