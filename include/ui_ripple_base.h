/**
 * @file ui_ripple_base.h
 * @brief Touch ripple effect generation and state tracking.
 */

#ifndef UI_RIPPLE_BASE_H
#define UI_RIPPLE_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Configuration for a ripple effect.
 */
struct ui_ripple_config {
  float color[4];    /**< RGBA color of the ripple */
  float duration_ms; /**< Duration in milliseconds */
  float max_radius;  /**< Maximum radius (or 0 for auto) */
  int center_origin; /**< If non-zero, ripple starts from center instead of
                        pointer */
};

/**
 * @brief Represents the state of an active ripple.
 */
struct ui_ripple_state {
  float center_x;   /**< X coordinate of origin */
  float center_y;   /**< Y coordinate of origin */
  float radius;     /**< Current radius */
  float opacity;    /**< Current opacity */
  float elapsed_ms; /**< Elapsed time in milliseconds */
  int active;       /**< Non-zero if currently animating */
};

/**
 * @brief Initializes a ripple configuration with default values.
 *
 * @param out_config Pointer to the config struct to initialize.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_ripple_config_init(struct ui_ripple_config *out_config);

/**
 * @brief Starts a new ripple effect.
 *
 * @param config The configuration to use.
 * @param x The pointer X coordinate.
 * @param y The pointer Y coordinate.
 * @param out_state Pointer to the state struct to update.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_ripple_start(const struct ui_ripple_config *config, float x,
                           float y, struct ui_ripple_state *out_state);

/**
 * @brief Updates an active ripple's state over time.
 *
 * @param config The configuration used for the ripple.
 * @param delta_ms The elapsed time since last update in milliseconds.
 * @param state Pointer to the state struct to update.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_ripple_update(const struct ui_ripple_config *config,
                            float delta_ms, struct ui_ripple_state *state);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_RIPPLE_BASE_H */
