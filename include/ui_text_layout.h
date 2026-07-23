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

struct ui_text_layout;

/**
 * @brief Represents a single positioned glyph.
 */
struct ui_positioned_glyph {
  int codepoint;
  float x;
  float y;
  float advance;
};

/**
 * @brief Text layout direction.
 */
enum ui_text_direction { UI_TEXT_DIRECTION_LTR, UI_TEXT_DIRECTION_RTL };

/**
 * @brief Creates a text layout object.
 */
enum ui_error ui_text_layout_create(struct ui_text_layout **out_layout);

/**
 * @brief Destroys a text layout object.
 */
enum ui_error ui_text_layout_destroy(struct ui_text_layout *layout);

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
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_text_layout_shape(struct ui_text_layout *layout,
                                   struct ui_font *font, float font_size,
                                   const char *text, float max_width,
                                   enum ui_text_direction direction);

/**
 * @brief Retrieves the positioned glyphs.
 */
enum ui_error
ui_text_layout_get_glyphs(struct ui_text_layout *layout,
                          const struct ui_positioned_glyph **out_glyphs,
                          size_t *out_count);

/**
 * @brief Retrieves the computed bounding box of the shaped text.
 */
enum ui_error ui_text_layout_get_bounds(struct ui_text_layout *layout,
                                        float *out_width, float *out_height);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TEXT_LAYOUT_H */
