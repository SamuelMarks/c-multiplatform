/**
 * @file ui_text_layout_hb.h
 * @brief HarfBuzz-backed text layout definitions.
 *
 * @defgroup TextLayoutHb HarfBuzz Text Layout
 * @brief Text layout and shaping using HarfBuzz.
 * @{
 */

#ifndef UI_TEXT_LAYOUT_HB_H
#define UI_TEXT_LAYOUT_HB_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_text_layout.h"
#include "ui_font_manager.h"
/* clang-format on */

/**
 * @brief Initialize the HarfBuzz shaping subsystem if available.
 *        This sets up internal function pointers or static state required
 *        to delegate text shaping to HarfBuzz when requested.
 *
 * @return UI_ERROR_NONE on success, UI_ERROR_UNSUPPORTED if HarfBuzz is not
 * compiled in.
 */
ui_error_t ui_text_layout_hb_init(void);

/**
 * @brief Shapes text using HarfBuzz, outputting into the standard
 * ui_text_layout structure. If HarfBuzz is not available, this returns
 * UI_ERROR_UNSUPPORTED, and the caller should fall back to standard
 * ui_text_layout_shape.
 *
 * @param layout The layout object to populate with shaped glyphs.
 * @param font The font to use for shaping.
 * @param font_size Font size in pixels.
 * @param text The UTF-8 string to shape.
 * @param max_width The maximum width for wrapping.
 * @param direction The base text direction.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_text_layout_shape_with_harfbuzz(struct ui_text_layout *layout,
                                              struct ui_font *font,
                                              float font_size, const char *text,
                                              float max_width,
                                              enum ui_text_direction direction);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TEXT_LAYOUT_HB_H */

/** @} */
