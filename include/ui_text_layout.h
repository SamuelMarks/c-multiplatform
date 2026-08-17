#ifndef UI_TEXT_LAYOUT_H
#define UI_TEXT_LAYOUT_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_font_manager.h"
#include <stddef.h>
/* clang-format on */

/**
 * @struct ui_text_layout
 * @brief Opaque handle for a text layout manager.
 */
struct ui_text_layout;

/**
 * @struct ui_positioned_glyph
 * @brief Represents a single positioned glyph.
 */
struct ui_positioned_glyph {
  /** @brief The unicode codepoint of the glyph. */
  int codepoint;
  /** @brief The X coordinate of the glyph. */
  float x;
  /** @brief The Y coordinate of the glyph. */
  float y;
  /** @brief The horizontal advance for the next glyph. */
  float advance;
};

/**
 * @enum ui_text_direction
 * @brief Text layout direction.
 */
enum ui_text_direction {
  /** @brief Left-to-Right layout direction. */
  UI_TEXT_DIRECTION_LTR,
  /** @brief Right-to-Left layout direction. */
  UI_TEXT_DIRECTION_RTL
};

/**
 * @brief Creates a text layout object.
 *
 * @param out_layout Pointer to receive the text layout object.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_text_layout_create(struct ui_text_layout **out_layout);

/**
 * @brief Destroys a text layout object.
 *
 * @param layout The layout object to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_text_layout_destroy(struct ui_text_layout *layout);

/**
 * @brief Shapes text into positioned glyphs, handling BiDi (stubbed), kerning,
 * and wrapping.
 *
 * @param layout The layout object.
 * @param font The font to use.
 * @param font_size Font size in pixels.
 * @param text The UTF-8 string to shape.
 * @param max_width The maximum width for wrapping (0 or negative for no
 * wrapping).
 * @param direction The base text direction.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_text_layout_shape(struct ui_text_layout *layout,
                                struct ui_font *font, float font_size,
                                const char *text, float max_width,
                                enum ui_text_direction direction);

/**
 * @brief Retrieves the positioned glyphs.
 *
 * @param layout The layout object.
 * @param out_glyphs Pointer to receive an array of positioned glyphs.
 * @param out_count Pointer to receive the number of glyphs in the array.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_text_layout_get_glyphs(struct ui_text_layout *layout,
                          const struct ui_positioned_glyph **out_glyphs,
                          size_t *out_count);

/**
 * @brief Retrieves the computed bounding box of the shaped text.
 *
 * @param layout The layout object.
 * @param out_width Pointer to receive the width of the bounding box.
 * @param out_height Pointer to receive the height of the bounding box.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_text_layout_get_bounds(struct ui_text_layout *layout,
                                     float *out_width, float *out_height);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TEXT_LAYOUT_H */
