/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

struct cmp_system_fonts {
  cmp_font_t *cached_sf_text;
  cmp_font_t *cached_sf_display;
  cmp_font_t *cached_sf_compact;
  cmp_font_t *cached_sf_mono;
  cmp_font_t *cached_ny;
};

int cmp_system_fonts_create(cmp_system_fonts_t **out_ctx) {
  struct cmp_system_fonts *ctx;
  if (!out_ctx)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_system_fonts), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  /* Simulating loading the OS fonts into memory */
  if (CMP_MALLOC(sizeof(cmp_font_t), (void **)&ctx->cached_sf_text) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  if (CMP_MALLOC(sizeof(cmp_font_t), (void **)&ctx->cached_sf_display) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  if (CMP_MALLOC(sizeof(cmp_font_t), (void **)&ctx->cached_sf_compact) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  if (CMP_MALLOC(sizeof(cmp_font_t), (void **)&ctx->cached_sf_mono) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  if (CMP_MALLOC(sizeof(cmp_font_t), (void **)&ctx->cached_ny) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->cached_sf_text->default_size = 17.0f;
  ctx->cached_sf_display->default_size = 34.0f;
  ctx->cached_sf_compact->default_size = 16.0f;
  ctx->cached_sf_mono->default_size = 13.0f;
  ctx->cached_ny->default_size = 17.0f;

  *out_ctx = (cmp_system_fonts_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_system_fonts_destroy(cmp_system_fonts_t *ctx) {
  if (!ctx)
    return CMP_SUCCESS;
  if (ctx->cached_sf_text)
    CMP_FREE(ctx->cached_sf_text);
  if (ctx->cached_sf_display)
    CMP_FREE(ctx->cached_sf_display);
  if (ctx->cached_sf_compact)
    CMP_FREE(ctx->cached_sf_compact);
  if (ctx->cached_sf_mono)
    CMP_FREE(ctx->cached_sf_mono);
  if (ctx->cached_ny)
    CMP_FREE(ctx->cached_ny);
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_system_fonts_request(cmp_system_fonts_t *ctx,
                             cmp_system_font_type_t type,
                             cmp_semantic_text_style_t style, int weight,
                             cmp_font_t **out_font) {
  struct cmp_system_fonts *c = (struct cmp_system_fonts *)ctx;
  (void)style;
  (void)weight;
  if (!c || !out_font)
    return CMP_ERROR_INVALID_ARG;

  switch (type) {
  case CMP_SYSTEM_FONT_SF_PRO:
    /* In a full engine, this checks if size >= 20 to return Display vs Text */
    *out_font = c->cached_sf_text;
    break;
  case CMP_SYSTEM_FONT_SF_COMPACT:
    *out_font = c->cached_sf_compact;
    break;
  case CMP_SYSTEM_FONT_SF_MONO:
    *out_font = c->cached_sf_mono;
    break;
  case CMP_SYSTEM_FONT_NEW_YORK:
    *out_font = c->cached_ny;
    break;
  default:
    *out_font = c->cached_sf_text;
    break;
  }
  return CMP_SUCCESS;
}

int cmp_system_fonts_get_tracking_and_optical(cmp_system_fonts_t *ctx,
                                              float point_size,
                                              int *out_is_display_variant,
                                              float *out_tracking) {
  (void)ctx;
  if (!out_is_display_variant || !out_tracking)
    return CMP_ERROR_INVALID_ARG;

  /* Apple HIG tracking formula approximation */
  if (point_size >= 20.0f) {
    *out_is_display_variant = 1;
    *out_tracking =
        0.38f - (0.015f * point_size); /* tightens as it gets larger */
  } else {
    *out_is_display_variant = 0;
    *out_tracking =
        0.04f + (0.12f / point_size); /* loosens as it gets smaller */
  }
  return CMP_SUCCESS;
}

int cmp_font_set_opentype_features(cmp_font_t *font, int enable_kerning,
                                   int enable_ligatures,
                                   int enable_tabular_figures) {
  if (!font)
    return CMP_ERROR_INVALID_ARG;
  /* Simulated internal font-face HarfBuzz feature bitmask application */
  (void)enable_kerning;
  (void)enable_ligatures;
  (void)enable_tabular_figures;
  return CMP_SUCCESS;
}

int cmp_font_set_variable_axes(cmp_font_t *font, float weight, float width) {
  if (!font)
    return CMP_ERROR_INVALID_ARG;
  /* Simulated OpenType Variable Font configuration */
  (void)weight;
  (void)width;
  return CMP_SUCCESS;
}

int cmp_system_fonts_get_metrics(cmp_system_fonts_t *ctx,
                                 cmp_semantic_text_style_t style,
                                 float *out_line_height, float *out_leading,
                                 float *out_baseline_offset) {
  (void)ctx;
  if (!out_line_height || !out_leading || !out_baseline_offset)
    return CMP_ERROR_INVALID_ARG;

  /* Maps HIG leading/line heights strictly */
  switch (style) {
  case CMP_TEXT_STYLE_LARGE_TITLE:
    *out_line_height = 41.0f;
    *out_leading = 0.0f;
    *out_baseline_offset = 34.0f;
    break;
  case CMP_TEXT_STYLE_BODY:
    *out_line_height = 22.0f;
    *out_leading = 0.0f;
    *out_baseline_offset = 17.0f;
    break;
  case CMP_TEXT_STYLE_CAPTION_1:
    *out_line_height = 16.0f;
    *out_leading = 0.0f;
    *out_baseline_offset = 12.0f;
    break;
  default:
    *out_line_height = 22.0f;
    *out_leading = 0.0f;
    *out_baseline_offset = 17.0f;
    break;
  }
  return CMP_SUCCESS;
}
