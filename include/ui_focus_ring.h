#ifndef UI_FOCUS_RING_H
#define UI_FOCUS_RING_H

/* clang-format off */
#include "ui_color_space.h"
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents advanced focus ring properties.
 */
struct ui_focus_ring {
  float offset;      /**< Distance from the component border (often negative or
                        positive) */
  float width;       /**< Thickness of the focus ring */
  ui_color_t color;  /**< Color of the focus ring */
  float inner_width; /**< Optional inner ring thickness */
  ui_color_t inner_color; /**< Optional inner ring color */
};

/**
 * @brief Retrieves standard focus ring properties.
 *
 * @param theme_primary_color The primary color from the theme.
 * @param out_ring Pointer to store the focus ring properties.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_focus_ring_get_standard(ui_color_t theme_primary_color,
                                      struct ui_focus_ring *out_ring);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_FOCUS_RING_H */
