/* clang-format off */
#include "../include/ui_text_layout.h"
#include "ui_internal_mem.h"
/* clang-format on */

struct ui_text_layout {
  struct ui_positioned_glyph *glyphs;
  size_t capacity;
  size_t count;
  float bounds_width;
  float bounds_height;
};

enum ui_error ui_text_layout_create(struct ui_text_layout **out_layout) {
  struct ui_text_layout *layout;

  if (!out_layout) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  layout = (struct ui_text_layout *)UI_MALLOC(sizeof(struct ui_text_layout));
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

enum ui_error ui_text_layout_destroy(struct ui_text_layout *layout) {
  if (!layout) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (layout->glyphs) {
    UI_FREE(layout->glyphs);
  }
  UI_FREE(layout);
  return UI_ERROR_NONE;
}

static enum ui_error decode_utf8(const char **text, int *out_codepoint) {
  const unsigned char *s = (const unsigned char *)*text;
  int c = *s++;
  *out_codepoint = 0;
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
    if (*s && *(s + 1)) {
      c = ((c & 0x0F) << 12) | ((*s & 0x3F) << 6);
      s++;
      c |= (*s++ & 0x3F);
    }
  } else if ((c & 0xF8) == 0xF0) {
    if (*s && *(s + 1) && *(s + 2)) {
      c = ((c & 0x07) << 18) | ((*s & 0x3F) << 12);
      s++;
      c |= ((*s & 0x3F) << 6);
      s++;
      c |= (*s++ & 0x3F);
    }
  }
  *text = (const char *)s;
  *out_codepoint = c;
  return UI_ERROR_NONE;
}

static enum ui_error add_glyph(struct ui_text_layout *layout, int codepoint,
                               float x, float y, float advance) {
  if (layout->count >= layout->capacity) {
    size_t new_cap = layout->capacity == 0 ? 32 : layout->capacity * 2;
    struct ui_positioned_glyph *new_glyphs =
        (struct ui_positioned_glyph *)UI_MALLOC(
            new_cap * sizeof(struct ui_positioned_glyph));
    if (!new_glyphs) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    if (layout->glyphs) {
      size_t i;
      for (i = 0; i < layout->count; ++i) {
        new_glyphs[i] = layout->glyphs[i];
      }
      UI_FREE(layout->glyphs);
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

enum ui_error ui_text_layout_shape(struct ui_text_layout *layout,
                                   struct ui_font *font, float font_size,
                                   const char *text, float max_width,
                                   enum ui_text_direction direction) {
  float x = 0.0f;
  float y = 0.0f;
  float max_x = 0.0f;
  int prev_codepoint = 0;
  float ascent = 0.0f, descent = 0.0f, line_gap = 0.0f;
  enum ui_error rc;

  (void)direction; /* BiDi stub: currently only processes left-to-right */

  if (!layout || !font || !text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  layout->count = 0; /* Reset state */

  rc = ui_font_get_vmetrics(font, font_size, &ascent, &descent, &line_gap);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  y += ascent;

  while (*text) {
    int codepoint = 0;
    struct ui_glyph_metrics metrics;
    float kerning = 0.0f;
    (void)decode_utf8(&text, &codepoint);

    if (codepoint == '\n') {
      x = 0.0f;
      y += (ascent - descent + line_gap);
      prev_codepoint = 0;
      continue;
    }

    rc = ui_font_get_glyph_metrics(font, codepoint, font_size, &metrics);
    if (rc != UI_ERROR_NONE) {
      continue;
    }

    if (prev_codepoint != 0) {
      ui_font_get_kerning(font, prev_codepoint, codepoint, font_size, &kerning);
    }

    x += kerning;

    /* Word wrap logic (simplified per-character wrap for stub) */
    if (max_width > 0.0f && x + (float)metrics.width > max_width && x > 0.0f) {
      x = 0.0f;
      y += (ascent - descent + line_gap);
    }

    rc = add_glyph(layout, codepoint, x, y, (float)metrics.advance);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    x += (float)metrics.advance;
    if (x > max_x) {
      max_x = x;
    }

    prev_codepoint = codepoint;
  }

  layout->bounds_width = max_x;
  layout->bounds_height = y - descent; /* Total height based on baselines */

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
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

enum ui_error ui_text_layout_get_bounds(struct ui_text_layout *layout,
                                        float *out_width, float *out_height) {
  if (!layout || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_width = layout->bounds_width;
  *out_height = layout->bounds_height;

  return UI_ERROR_NONE;
}
