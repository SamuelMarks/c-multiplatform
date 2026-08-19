/**
 * @file ui_state_layer.h
 * @brief Interactive state layer calculations.
 *
 * @defgroup StateLayer State Layer
 * @brief Utilities for calculating interaction state layer overlays (hover,
 * focus, pressed).
 * @{
 */

#ifndef UI_STATE_LAYER_H
#define UI_STATE_LAYER_H

/* clang-format off */
#include "ui_color_space.h"
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Standard interaction state layers.
 */
enum ui_state_layer_type {
  UI_STATE_LAYER_HOVER,   /**< Hover state layer. */
  UI_STATE_LAYER_FOCUS,   /**< Focus state layer. */
  UI_STATE_LAYER_PRESSED, /**< Pressed state layer. */
  UI_STATE_LAYER_DRAGGED  /**< Dragged state layer. */
};

/**
 * @brief Calculates the blended color for a state layer overlay.
 *
 * @param base_color The underlying surface color.
 * @param on_color The color to use for the state overlay (usually the
 * on-surface color).
 * @param state The interaction state.
 * @param out_color The resulting blended color.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_state_layer_get_color(ui_color_t base_color, ui_color_t on_color,
                                    enum ui_state_layer_type state,
                                    ui_color_t *out_color);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_STATE_LAYER_H */

/** @} */
