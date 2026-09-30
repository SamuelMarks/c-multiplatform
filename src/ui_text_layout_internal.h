/**
 * @file ui_text_layout_internal.h
 * @brief Internal shared layout structure definition for text layout modules.
 */

#ifndef UI_TEXT_LAYOUT_INTERNAL_H
#define UI_TEXT_LAYOUT_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "ui_text_layout.h"
#include <stddef.h>
/* clang-format on */

/**
 * @struct ui_text_layout
 * @brief Internal representation of a text layout.
 */
struct ui_text_layout {
  struct ui_positioned_glyph *glyphs; /**< Array of positioned glyphs. */
  size_t capacity;                    /**< Allocated capacity for glyphs. */
  size_t count;                       /**< Number of active glyphs. */
  float bounds_width;                 /**< Width of the bounds. */
  float bounds_height;                /**< Height of the bounds. */
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TEXT_LAYOUT_INTERNAL_H */
