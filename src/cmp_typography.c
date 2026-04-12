/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_typography {
  cmp_font_t *primary_font;
  cmp_font_t **fallback_fonts;
  size_t fallback_count;
  size_t fallback_capacity;
  int is_rtl;
};

int cmp_typography_create(cmp_typography_t **out_typo) {
  cmp_typography_t *typo;
  if (!out_typo) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* Note: Assumes cmp_typography_init() is called globally by application */

  typo = (cmp_typography_t *)malloc(sizeof(cmp_typography_t));
  if (!typo) {
    return CMP_ERROR_OOM;
  }

  typo->primary_font = NULL;
  typo->fallback_capacity = 4;
  typo->fallback_count = 0;
  typo->fallback_fonts =
      (cmp_font_t **)malloc(typo->fallback_capacity * sizeof(cmp_font_t *));
  if (!typo->fallback_fonts) {
    free(typo);
    return CMP_ERROR_OOM;
  }

  typo->is_rtl = 0;

  *out_typo = typo;
  return CMP_SUCCESS;
}

int cmp_typography_destroy(cmp_typography_t *typo) {
  size_t i;
  if (!typo) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (typo->primary_font) {
    cmp_font_destroy(typo->primary_font);
  }

  for (i = 0; i < typo->fallback_count; i++) {
    cmp_font_destroy(typo->fallback_fonts[i]);
  }
  free(typo->fallback_fonts);
  free(typo);

  /* Assumes cmp_typography_shutdown() is handled globally */
  return CMP_SUCCESS;
}

int cmp_typography_set_primary_font(cmp_typography_t *typo,
                                    const char *font_path,
                                    int enable_ligatures) {
  cmp_font_t *new_font = NULL;
  int result;

  if (!typo || !font_path) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* Use default size of 14.0f */
  result = cmp_font_load(font_path, 14.0f, &new_font);
  if (result != CMP_SUCCESS) {
    return result;
  }

  /* Set OpenType features for programming ligatures if requested */
  if (enable_ligatures) {
    cmp_font_set_opentype_features(new_font, 1, 1,
                                   0); /* kerning=1, ligatures=1, tabular=0 */
  }

  if (typo->primary_font) {
    cmp_font_destroy(typo->primary_font);
  }

  typo->primary_font = new_font;

  /* Re-bind any existing fallbacks to the new primary font */
  if (typo->fallback_count > 0) {
    size_t i;
    for (i = 0; i < typo->fallback_count; i++) {
      cmp_font_add_fallback(typo->primary_font, typo->fallback_fonts[i]);
    }
  }

  return CMP_SUCCESS;
}

int cmp_typography_add_fallback_font(cmp_typography_t *typo,
                                     const char *font_path) {
  cmp_font_t *fallback = NULL;
  cmp_font_t **new_array;
  int result;

  if (!typo || !font_path) {
    return CMP_ERROR_INVALID_ARG;
  }

  result = cmp_font_load(font_path, 14.0f, &fallback);
  if (result != CMP_SUCCESS) {
    return result;
  }

  if (typo->fallback_count == typo->fallback_capacity) {
    typo->fallback_capacity *= 2;
    new_array = (cmp_font_t **)realloc(
        typo->fallback_fonts, typo->fallback_capacity * sizeof(cmp_font_t *));
    if (!new_array) {
      cmp_font_destroy(fallback);
      return CMP_ERROR_OOM;
    }
    typo->fallback_fonts = new_array;
  }

  typo->fallback_fonts[typo->fallback_count++] = fallback;

  /* Bind immediately to primary if it exists */
  if (typo->primary_font) {
    cmp_font_add_fallback(typo->primary_font, fallback);
  }

  return CMP_SUCCESS;
}

int cmp_typography_set_bidi_direction(cmp_typography_t *typo, int is_rtl) {
  if (!typo) {
    return CMP_ERROR_INVALID_ARG;
  }

  typo->is_rtl = is_rtl ? 1 : 0;

  /* Forward to global layout engine */
  return cmp_i18n_set_bidi_direction(is_rtl ? CMP_TEXT_DIR_RTL
                                            : CMP_TEXT_DIR_LTR);
}
int cmp_freetype_glyph_rasterize(const char *font_file_path, int glyph_index,
                                 cmp_texture_t **out_glyph_texture,
                                 cmp_glyph_metrics_t *out_metrics) {
  (void)glyph_index;
  if (!font_file_path || !out_glyph_texture || !out_metrics)
    return CMP_ERROR_INVALID_ARG;
  *out_glyph_texture = NULL; /* STUB */
  out_metrics->bearing_x = 0;
  out_metrics->bearing_y = 0;
  out_metrics->advance_x = 0;
  out_metrics->advance_y = 0;
  return CMP_SUCCESS;
}

int cmp_harfbuzz_text_shape(cmp_font_t *font, const char *utf8_text, int is_rtl,
                            int *out_glyph_count) {
  (void)is_rtl;
  if (!font || !utf8_text || !out_glyph_count)
    return CMP_ERROR_INVALID_ARG;
  *out_glyph_count = 0; /* STUB */
  return CMP_SUCCESS;
}

int cmp_arabic_indic_shape(cmp_font_t *font, const char *utf8_text,
                           int *out_glyph_count) {
  if (!font || !utf8_text || !out_glyph_count)
    return CMP_ERROR_INVALID_ARG;
  *out_glyph_count = 0; /* STUB */
  return CMP_SUCCESS;
}

int cmp_bidi_run_split(const char *utf8_text, int *out_run_count,
                       int **out_run_is_rtl) {
  if (!utf8_text || !out_run_count || !out_run_is_rtl)
    return CMP_ERROR_INVALID_ARG;
  *out_run_count = 0; /* STUB */
  *out_run_is_rtl = NULL;
  return CMP_SUCCESS;
}

int cmp_font_render_sdf(cmp_font_t *font, int glyph_index,
                        cmp_texture_t **out_sdf_texture) {
  (void)glyph_index;
  if (!font || !out_sdf_texture)
    return CMP_ERROR_INVALID_ARG;
  *out_sdf_texture = NULL; /* STUB */
  return CMP_SUCCESS;
}

int cmp_font_render_msdf(cmp_font_t *font, int glyph_index,
                         cmp_texture_t **out_msdf_texture) {
  (void)glyph_index;
  if (!font || !out_msdf_texture)
    return CMP_ERROR_INVALID_ARG;
  *out_msdf_texture = NULL; /* STUB */
  return CMP_SUCCESS;
}

int cmp_font_render_subpixel_lcd(cmp_font_t *font, int glyph_index,
                                 cmp_texture_t **out_lcd_texture) {
  (void)glyph_index;
  if (!font || !out_lcd_texture)
    return CMP_ERROR_INVALID_ARG;
  *out_lcd_texture = NULL; /* STUB */
  return CMP_SUCCESS;
}

int cmp_font_render_color_emoji(cmp_font_t *font, int glyph_index,
                                cmp_texture_t **out_emoji_texture) {
  (void)glyph_index;
  if (!font || !out_emoji_texture)
    return CMP_ERROR_INVALID_ARG;
  *out_emoji_texture = NULL; /* STUB */
  return CMP_SUCCESS;
}

int cmp_variable_font_axis_interpolate(cmp_font_t *font, const char *axis_tag,
                                       float value,
                                       cmp_font_t **out_interpolated_font) {
  (void)value;
  if (!font || !axis_tag || !out_interpolated_font)
    return CMP_ERROR_INVALID_ARG;
  *out_interpolated_font = font; /* STUB */
  return CMP_SUCCESS;
}
