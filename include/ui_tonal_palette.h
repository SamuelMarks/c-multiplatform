#ifndef UI_TONAL_PALETTE_H
#define UI_TONAL_PALETTE_H

/* clang-format off */
#include "ui_color_space.h"
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a tonal palette (a range of shades from 0 to 100).
 * Standard stops are usually 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 95, 99,
 * 100.
 */
struct ui_tonal_palette {
  float hue;
  float chroma;
};

/**
 * @brief Creates a tonal palette from a key color.
 *
 * @param argb The key color to seed the palette.
 * @param out_palette The resulting tonal palette structure.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_tonal_palette_from_color(ui_color_t argb,
                                       struct ui_tonal_palette *out_palette);

/**
 * @brief Gets a specific shade from the tonal palette.
 *
 * @param palette The tonal palette.
 * @param tone The tone value (0.0 to 100.0). 0 is black, 100 is white.
 * @param out_color The resulting ARGB color.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_tonal_palette_get_tone(const struct ui_tonal_palette *palette,
                                     float tone, ui_color_t *out_color);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TONAL_PALETTE_H */
