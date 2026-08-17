/**
 * @file ui_elevation.h
 * @brief Material Design-style elevation and shadow management.
 *
 * This header defines enums, structures, and methods for generating drop
 * shadows and tinting surface colors according to elevation levels.
 */

#ifndef UI_ELEVATION_H
#define UI_ELEVATION_H

/* clang-format off */
#include "ui_color_space.h"
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a standardized elevation level (0 to 5 usually).
 */
enum ui_elevation_level {
  UI_ELEVATION_LEVEL_0 = 0, /**< Level 0, no elevation. */
  UI_ELEVATION_LEVEL_1 = 1, /**< Level 1 elevation. */
  UI_ELEVATION_LEVEL_2 = 2, /**< Level 2 elevation. */
  UI_ELEVATION_LEVEL_3 = 3, /**< Level 3 elevation. */
  UI_ELEVATION_LEVEL_4 = 4, /**< Level 4 elevation. */
  UI_ELEVATION_LEVEL_5 = 5  /**< Level 5 elevation. */
};

/**
 * @brief Calculates the tinted surface color based on elevation level.
 *
 * @param surface_color The base surface color (elevation 0).
 * @param tint_color The color used for tinting (usually primary color).
 * @param level The elevation level.
 * @param out_color Pointer to store the resulting tinted color.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_elevation_get_tinted_surface(ui_color_t surface_color,
                                           ui_color_t tint_color,
                                           enum ui_elevation_level level,
                                           ui_color_t *out_color);

/**
 * @brief Represents drop shadow properties.
 */
struct ui_drop_shadow {
  float x_offset;      /**< Horizontal shadow offset. */
  float y_offset;      /**< Vertical shadow offset. */
  float blur_radius;   /**< Blur radius of the shadow. */
  float spread_radius; /**< Spread radius of the shadow. */
  ui_color_t color;    /**< Shadow color. */
};

/**
 * @brief Retrieves standard drop shadow configurations for an elevation level.
 *
 * @param level The elevation level.
 * @param shadow_color The base color for the shadow (alpha is modulated
 * internally).
 * @param out_shadow1 Pointer to store the ambient/key shadow (e.g., umbra).
 * @param out_shadow2 Pointer to store the directional shadow (e.g., penumbra).
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_elevation_get_shadows(enum ui_elevation_level level,
                                    ui_color_t shadow_color,
                                    struct ui_drop_shadow *out_shadow1,
                                    struct ui_drop_shadow *out_shadow2);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_ELEVATION_H */
