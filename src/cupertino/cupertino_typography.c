#include "cupertino/cupertino_typography.h"

static void cupertino_typography_init_style(CMPTextStyle *style,
                                            cmp_i32 size_px, cmp_i32 weight,
                                            CMPScalar letter_spacing) {
  if (style) {
    cmp_text_style_init(style);
    style->size_px = size_px;
    style->weight = weight;
    style->letter_spacing = letter_spacing;
    style->italic = CMP_FALSE;
    style->width_axis = 100.0f;
    style->optical_size = (CMPScalar)size_px;
    style->slant = 0.0f;
    style->grade = 0.0f;
    style->line_height_px =
        (CMPScalar)size_px * 1.2f; /* Approximate San Francisco line height */
    style->color.r = 0.0f;
    style->color.g = 0.0f;
    style->color.b = 0.0f;
    style->color.a = 1.0f;
    /* Apple systems fall back to San Francisco (-apple-system) automatically on
     * their platforms */
    style->utf8_family = "-apple-system, BlinkMacSystemFont, 'Segoe UI', "
                         "Roboto, Helvetica, Arial, sans-serif";
  }
}

CMP_API int CMP_CALL
cupertino_typography_scale_init(CupertinoTypographyScale *scale) {
  if (!scale) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  /* iOS 15+ Dynamic Type sizes for "Large (Default)" content size category */
  cupertino_typography_init_style(
      &scale->styles[CUPERTINO_TYPOGRAPHY_LARGE_TITLE], 34, 400, 0.374f);
  cupertino_typography_init_style(&scale->styles[CUPERTINO_TYPOGRAPHY_TITLE_1],
                                  28, 400, 0.364f);
  cupertino_typography_init_style(&scale->styles[CUPERTINO_TYPOGRAPHY_TITLE_2],
                                  22, 400, 0.352f);
  cupertino_typography_init_style(&scale->styles[CUPERTINO_TYPOGRAPHY_TITLE_3],
                                  20, 400, 0.38f);
  cupertino_typography_init_style(&scale->styles[CUPERTINO_TYPOGRAPHY_HEADLINE],
                                  17, 600, -0.408f);
  cupertino_typography_init_style(&scale->styles[CUPERTINO_TYPOGRAPHY_BODY], 17,
                                  400, -0.408f);
  cupertino_typography_init_style(&scale->styles[CUPERTINO_TYPOGRAPHY_CALLOUT],
                                  16, 400, -0.32f);
  cupertino_typography_init_style(
      &scale->styles[CUPERTINO_TYPOGRAPHY_SUBHEADLINE], 15, 400, -0.24f);
  cupertino_typography_init_style(&scale->styles[CUPERTINO_TYPOGRAPHY_FOOTNOTE],
                                  13, 400, -0.078f);
  cupertino_typography_init_style(
      &scale->styles[CUPERTINO_TYPOGRAPHY_CAPTION_1], 12, 400, 0.0f);
  cupertino_typography_init_style(
      &scale->styles[CUPERTINO_TYPOGRAPHY_CAPTION_2], 11, 400, 0.066f);

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_typography_get_style(
    const CupertinoTypographyScale *scale, CupertinoTypographyStyle style_id,
    CMPTextStyle *out_style) {
  if (!scale || !out_style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (style_id < 0 || style_id >= CUPERTINO_TYPOGRAPHY_STYLE_COUNT) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_style = scale->styles[style_id];
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_typography_scale_set_color(
    CupertinoTypographyScale *scale, CMPColor color) {
  int i;
  if (!scale) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < CUPERTINO_TYPOGRAPHY_STYLE_COUNT; i++) {
    scale->styles[i].color = color;
  }

  return CMP_OK;
}
