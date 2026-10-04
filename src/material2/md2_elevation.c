/**
 * @file md2_elevation.c
 * @brief Implementation of Material Design 2 elevation levels.
 */

/* clang-format off */
#include "material2/md2_elevation.h"
/* clang-format on */

ui_error_t md2_elevation_get_dp(enum md2_elevation_level level, float *out_dp) {
  if (out_dp == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  switch (level) {
  case MD2_ELEVATION_0DP:
    *out_dp = 0.0f;
    break;
  case MD2_ELEVATION_1DP:
    *out_dp = 1.0f;
    break;
  case MD2_ELEVATION_2DP:
    *out_dp = 2.0f;
    break;
  case MD2_ELEVATION_3DP:
    *out_dp = 3.0f;
    break;
  case MD2_ELEVATION_4DP:
    *out_dp = 4.0f;
    break;
  case MD2_ELEVATION_6DP:
    *out_dp = 6.0f;
    break;
  case MD2_ELEVATION_8DP:
    *out_dp = 8.0f;
    break;
  case MD2_ELEVATION_12DP:
    *out_dp = 12.0f;
    break;
  case MD2_ELEVATION_16DP:
    *out_dp = 16.0f;
    break;
  case MD2_ELEVATION_24DP:
    *out_dp = 24.0f;
    break;
  default:
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return UI_ERROR_NONE;
}
