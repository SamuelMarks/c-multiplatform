/**
 * @file md2_elevation.h
 * @brief Material Design 2 elevation definitions.
 */

#ifndef MATERIAL2_MD2_ELEVATION_H
#define MATERIAL2_MD2_ELEVATION_H

/* clang-format off */
#include "ui_error.h"
#include "ui_export.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum md2_elevation_level
 * @brief Standard Material Design 2 elevation levels in dp.
 */
enum md2_elevation_level {
  MD2_ELEVATION_0DP = 0,
  MD2_ELEVATION_1DP = 1,
  MD2_ELEVATION_2DP = 2,
  MD2_ELEVATION_3DP = 3,
  MD2_ELEVATION_4DP = 4,
  MD2_ELEVATION_6DP = 6,
  MD2_ELEVATION_8DP = 8,
  MD2_ELEVATION_12DP = 12,
  MD2_ELEVATION_16DP = 16,
  MD2_ELEVATION_24DP = 24
};

/**
 * @brief Retrieves the float dp value for a given Material Design 2 elevation
 * level.
 *
 * @param level Elevation level token.
 * @param out_dp Pointer receiving the float dp value.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_elevation_get_dp(enum md2_elevation_level level, float *out_dp);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL2_MD2_ELEVATION_H */
