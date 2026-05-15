/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include "cmp_log.h"
/* clang-format on */

struct cmp_typography {
  cmp_font_t *primary_font;
  cmp_font_t **fallback_fonts;
  size_t fallback_count;
  size_t fallback_capacity;
  int is_rtl;
};

/**
 * @brief cmp_typography_create
 *
 * @param out_typo Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_typography_create(cmp_typography_t **out_typo) {
  int rc = CMP_SUCCESS;
  cmp_typography_t *typo;
  if (!out_typo) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* Note: Assumes cmp_typography_init() is called globally by application */

  rc = CMP_MALLOC(sizeof(cmp_typography_t), (void **)&(typo));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  typo->primary_font = NULL;
  typo->fallback_capacity = 4;
  typo->fallback_count = 0;
  rc = CMP_MALLOC(typo->fallback_capacity * sizeof(cmp_font_t *),
                  (void **)&typo->fallback_fonts);
  if (rc != CMP_SUCCESS) {
    CMP_FREE(typo);
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  typo->is_rtl = 0;

  *out_typo = typo;
  return rc;
}

/**
 * @brief cmp_typography_destroy
 *
 * @param typo Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_typography_destroy(cmp_typography_t *typo) {
  int rc = CMP_SUCCESS;
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
  rc = CMP_FREE(typo->fallback_fonts);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  rc = CMP_FREE(typo);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }

  /* Assumes cmp_typography_shutdown() is handled globally */
  return rc;
}

/**
 * @brief cmp_typography_set_primary_font
 *
 * @param typo Parameter description.
 * @param font_path Parameter description.
 * @param enable_ligatures Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_typography_set_primary_font(cmp_typography_t *typo,
                                    const char *font_path,
                                    int enable_ligatures) {
  int rc = CMP_SUCCESS;
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

  return rc;
}

/**
 * @brief cmp_typography_add_fallback_font
 *
 * @param typo Parameter description.
 * @param font_path Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_typography_add_fallback_font(cmp_typography_t *typo,
                                     const char *font_path) {
  cmp_font_t *fallback = NULL;
  cmp_font_t **new_array;
  int result;
  int rc = CMP_SUCCESS;

  if (!typo || !font_path) {
    return CMP_ERROR_INVALID_ARG;
  }

  result = cmp_font_load(font_path, 14.0f, &fallback);
  if (result != CMP_SUCCESS) {
    return result;
  }

  if (typo->fallback_count == typo->fallback_capacity) {
    size_t new_cap = typo->fallback_capacity * 2;
    rc = CMP_MALLOC(new_cap * sizeof(cmp_font_t *), (void **)&new_array);
    if (rc != CMP_SUCCESS) {
      cmp_font_destroy(fallback);
      return CMP_ERROR_OOM;
    }
    memcpy(new_array, typo->fallback_fonts,
           typo->fallback_count * sizeof(cmp_font_t *));
    CMP_FREE(typo->fallback_fonts);
    typo->fallback_fonts = new_array;
    typo->fallback_capacity = new_cap;
  }

  typo->fallback_fonts[typo->fallback_count++] = fallback;

  /* Bind immediately to primary if it exists */
  if (typo->primary_font) {
    cmp_font_add_fallback(typo->primary_font, fallback);
  }

  return rc;
}

/**
 * @brief cmp_typography_set_bidi_direction
 *
 * @param typo Parameter description.
 * @param is_rtl Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_typography_set_bidi_direction(cmp_typography_t *typo, int is_rtl) {
  int rc = CMP_SUCCESS;
  if (!typo) {
    return CMP_ERROR_INVALID_ARG;
  }

  typo->is_rtl = is_rtl ? 1 : 0;

  /* Forward to global layout engine */

  rc =
      cmp_i18n_set_bidi_direction(is_rtl ? CMP_TEXT_DIR_RTL : CMP_TEXT_DIR_LTR);
  return rc;
}
/**
 * @brief cmp_freetype_glyph_rasterize
 *
 * @param font_file_path Parameter description.
 * @param glyph_index Parameter description.
 * @param out_glyph_texture Parameter description.
 * @param out_metrics Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_freetype_glyph_rasterize(const char *font_file_path, int glyph_index,
                                 cmp_texture_t **out_glyph_texture,
                                 cmp_glyph_metrics_t *out_metrics) {
  int rc = CMP_SUCCESS;
  (void)glyph_index;
  if (!font_file_path || !out_glyph_texture || !out_metrics)
    return CMP_ERROR_INVALID_ARG;

  *out_glyph_texture = NULL;
  out_metrics->bearing_x = 0;
  out_metrics->bearing_y = 0;
  out_metrics->advance_x = 0;
  out_metrics->advance_y = 0;

  cmp_log_debug(
      "cmp_freetype_glyph_rasterize: Mocked Freetype glyph rasterization\n");

  return rc;
}

/**
 * @brief cmp_harfbuzz_text_shape
 *
 * @param font Parameter description.
 * @param utf8_text Parameter description.
 * @param is_rtl Parameter description.
 * @param out_glyph_count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_harfbuzz_text_shape(cmp_font_t *font, const char *utf8_text, int is_rtl,
                            int *out_glyph_count) {
  int rc = CMP_SUCCESS;
  (void)is_rtl;
  if (!font || !utf8_text || !out_glyph_count)
    return CMP_ERROR_INVALID_ARG;

  /* Mocked text shaping length logic */
  *out_glyph_count = (int)strlen(utf8_text);

  cmp_log_debug("cmp_harfbuzz_text_shape: Mocked HarfBuzz text shaping\n");

  return rc;
}

/**
 * @brief cmp_arabic_indic_shape
 *
 * @param font Parameter description.
 * @param utf8_text Parameter description.
 * @param out_glyph_count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_arabic_indic_shape(cmp_font_t *font, const char *utf8_text,
                           int *out_glyph_count) {
  int rc = CMP_SUCCESS;
  if (!font || !utf8_text || !out_glyph_count)
    return CMP_ERROR_INVALID_ARG;

  /* Mocked complex shape counting logic */
  *out_glyph_count = (int)strlen(utf8_text);

  cmp_log_debug(
      "cmp_arabic_indic_shape: Mocked Arabic/Indic complex shaping\n");

  return rc;
}

/**
 * @brief cmp_bidi_run_split
 *
 * @param utf8_text Parameter description.
 * @param out_run_count Parameter description.
 * @param out_run_is_rtl Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_bidi_run_split(const char *utf8_text, int *out_run_count,
                       int **out_run_is_rtl) {
  int rc = CMP_SUCCESS;
  if (!utf8_text || !out_run_count || !out_run_is_rtl)
    return CMP_ERROR_INVALID_ARG;

  /* Mocked single LTR run fallback */
  *out_run_count = 1;
  *out_run_is_rtl = NULL;

  cmp_log_debug("cmp_bidi_run_split: Mocked BiDi run splitting fallback\n");

  return rc;
}

/**
 * @brief cmp_font_render_sdf
 *
 * @param font Parameter description.
 * @param glyph_index Parameter description.
 * @param out_sdf_texture Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_font_render_sdf(cmp_font_t *font, int glyph_index,
                        cmp_texture_t **out_sdf_texture) {
  int rc = CMP_SUCCESS;
  (void)glyph_index;
  if (!font || !out_sdf_texture)
    return CMP_ERROR_INVALID_ARG;

  *out_sdf_texture = NULL;
  cmp_log_debug("cmp_font_render_sdf: Mocked SDF font rendering\n");

  return rc;
}

/**
 * @brief cmp_font_render_msdf
 *
 * @param font Parameter description.
 * @param glyph_index Parameter description.
 * @param out_msdf_texture Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_font_render_msdf(cmp_font_t *font, int glyph_index,
                         cmp_texture_t **out_msdf_texture) {
  int rc = CMP_SUCCESS;
  (void)glyph_index;
  if (!font || !out_msdf_texture)
    return CMP_ERROR_INVALID_ARG;

  *out_msdf_texture = NULL;
  cmp_log_debug("cmp_font_render_msdf: Mocked MSDF font rendering\n");

  return rc;
}

/**
 * @brief cmp_font_render_subpixel_lcd
 *
 * @param font Parameter description.
 * @param glyph_index Parameter description.
 * @param out_lcd_texture Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_font_render_subpixel_lcd(cmp_font_t *font, int glyph_index,
                                 cmp_texture_t **out_lcd_texture) {
  int rc = CMP_SUCCESS;
  (void)glyph_index;
  if (!font || !out_lcd_texture)
    return CMP_ERROR_INVALID_ARG;

  *out_lcd_texture = NULL;
  cmp_log_debug(
      "cmp_font_render_subpixel_lcd: Mocked subpixel LCD font rendering\n");

  return rc;
}

/**
 * @brief cmp_font_render_color_emoji
 *
 * @param font Parameter description.
 * @param glyph_index Parameter description.
 * @param out_emoji_texture Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_font_render_color_emoji(cmp_font_t *font, int glyph_index,
                                cmp_texture_t **out_emoji_texture) {
  int rc = CMP_SUCCESS;
  (void)glyph_index;
  if (!font || !out_emoji_texture)
    return CMP_ERROR_INVALID_ARG;

  *out_emoji_texture = NULL;
  cmp_log_debug("cmp_font_render_color_emoji: Mocked Color Emoji rendering\n");

  return rc;
}

/**
 * @brief cmp_variable_font_axis_interpolate
 *
 * @param font Parameter description.
 * @param axis_tag Parameter description.
 * @param value Parameter description.
 * @param out_interpolated_font Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_variable_font_axis_interpolate(cmp_font_t *font, const char *axis_tag,
                                       float value,
                                       cmp_font_t **out_interpolated_font) {
  int rc = CMP_SUCCESS;
  (void)value;
  if (!font || !axis_tag || !out_interpolated_font)
    return CMP_ERROR_INVALID_ARG;

  /* Fallback interpolates back to base font */
  *out_interpolated_font = font;
  cmp_log_debug("cmp_variable_font_axis_interpolate: Mocked variable font axis "
                "interpolation fallback\n");

  return rc;
}
