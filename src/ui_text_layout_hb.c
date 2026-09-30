/**
 * @file ui_text_layout_hb.c
 * @brief ui_text_layout_hb.c implementation.
 */
/* clang-format off */
#include "../include/ui_text_layout_hb.h"
#include "ui_internal_mem.h"

#ifdef UI_USE_HARFBUZZ
#include <hb.h>
#endif
/* clang-format on */

/**
 * @brief ui_text_layout_hb_init.
 * @return Return value.
 */
ui_error_t ui_text_layout_hb_init(void) {
#ifdef UI_USE_HARFBUZZ
  return UI_ERROR_NONE;
#else
  return UI_ERROR_UNSUPPORTED;
#endif
}

/* \brief ui_text_layout_shape_with_harfbuzz
 */
ui_error_t ui_text_layout_shape_with_harfbuzz(
    struct ui_text_layout *layout, struct ui_font *font, float font_size,
    const char *text, float max_width, enum ui_text_direction direction) {
#ifdef UI_USE_HARFBUZZ
  hb_buffer_t *buf = NULL;
  hb_blob_t *blob = NULL;
  hb_face_t *face = NULL;
  hb_font_t *hb_font = NULL;
  unsigned int glyph_count = 0;
  hb_glyph_info_t *glyph_info = NULL;
  hb_glyph_position_t *glyph_pos = NULL;
  const unsigned char *font_data = NULL;
  size_t font_size_bytes = 0;
  float cur_x = 0.0f;
  float cur_y = 0.0f;
  float scale = 0.0f;
  float ascent = 0.0f, descent = 0.0f, line_gap = 0.0f;
  unsigned int i;
  ui_error_t rc;
#endif

  if (!layout || !font || !text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (direction != UI_TEXT_DIRECTION_LTR &&
      direction != UI_TEXT_DIRECTION_RTL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#ifdef UI_USE_HARFBUZZ
  rc = ui_font_get_vmetrics(font, font_size, &ascent, &descent, &line_gap);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_font_get_data(font, &font_data, &font_size_bytes);
  if (rc != UI_ERROR_NONE || !font_data || font_size_bytes == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  buf = hb_buffer_create();
  if (!buf) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  hb_buffer_set_direction(buf, direction == UI_TEXT_DIRECTION_RTL
                                   ? HB_DIRECTION_RTL
                                   : HB_DIRECTION_LTR);
  hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
  hb_buffer_set_language(buf, hb_language_from_string("en", -1));
  hb_buffer_add_utf8(buf, text, -1, 0, -1);

  blob = hb_blob_create((const char *)font_data, (unsigned int)font_size_bytes,
                        HB_MEMORY_MODE_READONLY, NULL, NULL);
  if (!blob) {
    hb_buffer_destroy(buf);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  face = hb_face_create(blob, 0);
  hb_blob_destroy(blob);
  if (!face) {
    hb_buffer_destroy(buf);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  hb_font = hb_font_create(face);
  hb_face_destroy(face);
  if (!hb_font) {
    hb_buffer_destroy(buf);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  /* 26.6 fractional fixed point coordinates */
  scale = font_size * 64.0f;
  hb_font_set_scale(hb_font, (int)scale, (int)scale);

  hb_shape(hb_font, buf, NULL, 0);

  glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
  glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);

  if (layout->capacity < (size_t)glyph_count) {
    struct ui_positioned_glyph *new_glyphs;
    new_glyphs = (struct ui_positioned_glyph *)C_MULTIPLATFORM_MALLOC(
        (size_t)glyph_count * sizeof(struct ui_positioned_glyph));
    if (!new_glyphs) {
      hb_font_destroy(hb_font);
      hb_buffer_destroy(buf);
      return UI_ERROR_OUT_OF_MEMORY;
    }
    if (layout->glyphs) {
      C_MULTIPLATFORM_FREE(layout->glyphs);
    }
    layout->glyphs = new_glyphs;
    layout->capacity = glyph_count;
  }

  layout->count = glyph_count;
  cur_y = ascent;

  for (i = 0; i < glyph_count; ++i) {
    float x_offset = (float)glyph_pos[i].x_offset / 64.0f;
    float y_offset = (float)glyph_pos[i].y_offset / 64.0f;
    float x_advance = (float)glyph_pos[i].x_advance / 64.0f;

    layout->glyphs[i].codepoint = (int)glyph_info[i].codepoint;
    layout->glyphs[i].x = cur_x + x_offset;
    layout->glyphs[i].y = cur_y - y_offset;
    layout->glyphs[i].advance = x_advance;

    cur_x += x_advance;
  }

  layout->bounds_width = cur_x;
  layout->bounds_height = ascent - descent;

  hb_font_destroy(hb_font);
  hb_buffer_destroy(buf);
  return UI_ERROR_NONE;
#else
  if (font_size > 0.0f || max_width > 0.0f) {
  }
  return UI_ERROR_UNSUPPORTED;
#endif
}
