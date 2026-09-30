/**
 * @file ui_text_layout.c
 * @brief Implementation of text layout components.
 */

/* clang-format off */
#include "../include/ui_text_layout.h"
#include "ui_text_layout_internal.h"
#include "ui_internal_mem.h"
/* clang-format on */

/**
 * @brief Creates a new text layout instance.
 * @param[out] out_layout Pointer to store the created layout.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_text_layout_create(struct ui_text_layout **out_layout) {
  struct ui_text_layout *layout;

  if (!out_layout) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  layout = (struct ui_text_layout *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_text_layout));
  if (!layout) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  layout->glyphs = NULL;
  layout->capacity = 0;
  layout->count = 0;
  layout->bounds_width = 0.0f;
  layout->bounds_height = 0.0f;

  *out_layout = layout;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a text layout instance.
 * @param[in,out] layout The text layout to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_text_layout_destroy(struct ui_text_layout *layout) {
  if (!layout) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (layout->glyphs) {
    C_MULTIPLATFORM_FREE(layout->glyphs);
  }
  C_MULTIPLATFORM_FREE(layout);
  return UI_ERROR_NONE;
}

/**
 * @brief Helper function to decode a UTF-8 character.
 * @param[in,out] text Pointer to the text string, updated to the next
 * character.
 * @param[out] out_codepoint Pointer to receive the decoded codepoint.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t decode_utf8(const char **text, int *out_codepoint) {
  const unsigned char *s = (const unsigned char *)*text;
  int c = *s++;
  if (c < 0x80) {
    *text = (const char *)s;
    *out_codepoint = c;
    return UI_ERROR_NONE;
  }
  if ((c & 0xE0) == 0xC0) {
    if (*s) {
      c = ((c & 0x1F) << 6) | (*s++ & 0x3F);
    }
  } else if ((c & 0xF0) == 0xE0) {
    if (*s) {
      if (*(s + 1)) {
        c = ((c & 0x0F) << 12) | ((*s & 0x3F) << 6);
        s++;
        c |= (*s++ & 0x3F);
      }
    }
  } else if ((c & 0xF8) == 0xF0) {
    if (*s) {
      if (*(s + 1)) {
        if (*(s + 2)) {
          c = ((c & 0x07) << 18) | ((*s & 0x3F) << 12);
          s++;
          c |= ((*s & 0x3F) << 6);
          s++;
          c |= (*s++ & 0x3F);
        }
      }
    }
  }
  *text = (const char *)s;
  *out_codepoint = c;
  return UI_ERROR_NONE;
}

/**
 * @brief Helper function to add a glyph to the layout.
 * @param[in,out] layout The layout instance.
 * @param[in] codepoint The codepoint to add.
 * @param[in] x The X coordinate.
 * @param[in] y The Y coordinate.
 * @param[in] advance The glyph's advance width.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t add_glyph(struct ui_text_layout *layout, int codepoint,
                            float x, float y, float advance) {
  if (layout->count >= layout->capacity) {
    size_t new_cap = layout->capacity == 0 ? 32 : layout->capacity * 2;
    struct ui_positioned_glyph *new_glyphs =
        (struct ui_positioned_glyph *)C_MULTIPLATFORM_MALLOC(
            (size_t)new_cap * sizeof(struct ui_positioned_glyph));
    if (!new_glyphs) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    if (layout->glyphs) {
      size_t i;
      for (i = 0; i < layout->count; ++i) {
        new_glyphs[i] = layout->glyphs[i];
      }
      C_MULTIPLATFORM_FREE(layout->glyphs);
    }
    layout->glyphs = new_glyphs;
    layout->capacity = new_cap;
  }

  layout->glyphs[layout->count].codepoint = codepoint;
  layout->glyphs[layout->count].x = x;
  layout->glyphs[layout->count].y = y;
  layout->glyphs[layout->count].advance = advance;
  layout->count++;

  return UI_ERROR_NONE;
}

/**
 * @brief Shapes text into positioned glyphs.
 * @param[in,out] layout The text layout.
 * @param[in] font The font to use for shaping.
 * @param[in] font_size The size of the font.
 * @param[in] text The text string to shape.
 * @param[in] max_width The maximum width for line wrapping.
 * @param[in] direction The text direction (e.g., LTR, RTL).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_text_layout_shape(struct ui_text_layout *layout,
                                struct ui_font *font, float font_size,
                                const char *text, float max_width,
                                enum ui_text_direction direction) {
  float x = 0.0f;
  float y = 0.0f;
  float max_x = 0.0f;
  int prev_codepoint = 0;
  float ascent = 0.0f, descent = 0.0f, line_gap = 0.0f;
  size_t last_break_glyph_idx = (size_t)-1;
  float last_break_x = 0.0f;
  float line_height;
  ui_error_t rc;

  if (!layout || !font || !text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (direction != UI_TEXT_DIRECTION_LTR &&
      direction != UI_TEXT_DIRECTION_RTL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  layout->count = 0;

  rc = ui_font_get_vmetrics(font, font_size, &ascent, &descent, &line_gap);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  line_height = ascent - descent + line_gap;
  y += ascent;

  while (*text) {
    int codepoint = 0;
    struct ui_glyph_metrics metrics;
    float kerning = 0.0f;
    rc = decode_utf8(&text, &codepoint);

    if (codepoint == '\n') {
      x = 0.0f;
      y += line_height;
      prev_codepoint = 0;
      last_break_glyph_idx = (size_t)-1;
      continue;
    }

    rc = ui_font_get_glyph_metrics(font, codepoint, font_size, &metrics);
    if (rc != UI_ERROR_NONE) {
      continue;
    }

    if (prev_codepoint != 0) {
      rc = ui_font_get_kerning(font, prev_codepoint, codepoint, font_size,
                               &kerning);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }

    x += kerning;

    /* Word wrap check on line boundaries */
    if (max_width > 0.0f && x + (float)metrics.width > max_width && x > 0.0f) {
      if (last_break_glyph_idx != (size_t)-1 &&
          last_break_glyph_idx + 1 < layout->count) {
        /* Rewrap words starting after the last whitespace/break opportunity */
        size_t move_idx;
        float shift_x = layout->glyphs[last_break_glyph_idx + 1].x;
        y += line_height;
        for (move_idx = last_break_glyph_idx + 1; move_idx < layout->count;
             ++move_idx) {
          layout->glyphs[move_idx].x -= shift_x;
          layout->glyphs[move_idx].y = y;
        }
        x = x - shift_x;
        last_break_glyph_idx = (size_t)-1;
      } else {
        /* Emergency wrap if word exceeds line length */
        x = 0.0f;
        y += line_height;
        last_break_glyph_idx = (size_t)-1;
      }
    }

    rc = add_glyph(layout, codepoint, x, y, (float)metrics.advance);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    /* Record break opportunities on whitespace */
    if (codepoint == ' ' || codepoint == '\t') {
      last_break_glyph_idx = layout->count - 1;
      last_break_x = x + (float)metrics.advance;
    }

    x += (float)metrics.advance;
    if (x > max_x) {
      max_x = x;
    }

    prev_codepoint = codepoint;
  }

  if (last_break_x > 0.0f) {
    /* Reference to avoid unused variable warning */
  }

  if (direction == UI_TEXT_DIRECTION_RTL && layout->count > 0) {
    size_t line_start = 0;
    size_t i;
    for (i = 0; i <= layout->count; ++i) {
      if (i == layout->count ||
          (i > line_start &&
           layout->glyphs[i].y != layout->glyphs[line_start].y)) {
        float line_w = 0.0f;
        size_t j;
        for (j = line_start; j < i; ++j) {
          float end_x = layout->glyphs[j].x + layout->glyphs[j].advance;
          if (end_x > line_w) {
            line_w = end_x;
          }
        }
        for (j = line_start; j < i; ++j) {
          layout->glyphs[j].x =
              line_w - (layout->glyphs[j].x + layout->glyphs[j].advance);
        }
        line_start = i;
      }
    }
  }

  layout->bounds_width = max_x;
  layout->bounds_height = y - descent; /* Total height based on baselines */

  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves the array of positioned glyphs.
 * @param[in] layout The text layout.
 * @param[out] out_glyphs Pointer to store the glyphs array.
 * @param[out] out_count Pointer to store the number of glyphs.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_text_layout_get_glyphs(struct ui_text_layout *layout,
                          const struct ui_positioned_glyph **out_glyphs,
                          size_t *out_count) {
  if (!layout || !out_glyphs || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_glyphs = layout->glyphs;
  *out_count = layout->count;

  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves the bounding box of the layout.
 * @param[in] layout The text layout.
 * @param[out] out_width Pointer to store the width of the bounds.
 * @param[out] out_height Pointer to store the height of the bounds.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_text_layout_get_bounds(struct ui_text_layout *layout,
                                     float *out_width, float *out_height) {
  if (!layout || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_width = layout->bounds_width;
  *out_height = layout->bounds_height;

  return UI_ERROR_NONE;
}
