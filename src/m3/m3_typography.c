/* clang-format off */
#include "m3/m3_typography.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_log.h"
#include <string.h>
/* clang-format on */

/** @brief Default font family for M3. */
#define M3_TYPOGRAPHY_DEFAULT_FAMILY "Roboto"

struct M3TypographyProps {
  cmp_i32 size_px;
  cmp_i32 weight;
  CMPScalar line_height_px;
  CMPScalar letter_spacing;
};

static const struct M3TypographyProps
    g_m3_default_props[M3_TYPOGRAPHY_ROLE_COUNT] = {
        {57, 400, 64.0f, -0.25f}, /* DISPLAY_LARGE */
        {45, 400, 52.0f, 0.0f},   /* DISPLAY_MEDIUM */
        {36, 400, 44.0f, 0.0f},   /* DISPLAY_SMALL */
        {32, 400, 40.0f, 0.0f},   /* HEADLINE_LARGE */
        {28, 400, 36.0f, 0.0f},   /* HEADLINE_MEDIUM */
        {24, 400, 32.0f, 0.0f},   /* HEADLINE_SMALL */
        {22, 400, 28.0f, 0.0f},   /* TITLE_LARGE */
        {16, 500, 24.0f, 0.15f},  /* TITLE_MEDIUM */
        {14, 500, 20.0f, 0.1f},   /* TITLE_SMALL */
        {14, 500, 20.0f, 0.1f},   /* LABEL_LARGE */
        {12, 500, 16.0f, 0.5f},   /* LABEL_MEDIUM */
        {11, 500, 16.0f, 0.5f},   /* LABEL_SMALL */
        {16, 400, 24.0f, 0.5f},   /* BODY_LARGE */
        {14, 400, 20.0f, 0.25f},  /* BODY_MEDIUM */
        {12, 400, 16.0f, 0.4f}    /* BODY_SMALL */
};

static const struct M3TypographyProps
    g_m3_expressive_props[M3_TYPOGRAPHY_ROLE_COUNT] = {
        {64, 400, 76.0f, -0.5f},  /* DISPLAY_LARGE */
        {52, 400, 64.0f, -0.25f}, /* DISPLAY_MEDIUM */
        {44, 400, 52.0f, 0.0f},   /* DISPLAY_SMALL */
        {40, 400, 52.0f, 0.0f},   /* HEADLINE_LARGE */
        {36, 400, 48.0f, 0.0f},   /* HEADLINE_MEDIUM */
        {32, 400, 40.0f, 0.0f},   /* HEADLINE_SMALL */
        {28, 400, 36.0f, 0.0f},   /* TITLE_LARGE */
        {16, 500, 24.0f, 0.15f},  /* TITLE_MEDIUM */
        {14, 500, 20.0f, 0.1f},   /* TITLE_SMALL */
        {14, 500, 20.0f, 0.1f},   /* LABEL_LARGE */
        {12, 500, 16.0f, 0.5f},   /* LABEL_MEDIUM */
        {11, 500, 16.0f, 0.5f},   /* LABEL_SMALL */
        {16, 400, 24.0f, 0.5f},   /* BODY_LARGE */
        {14, 400, 20.0f, 0.25f},  /* BODY_MEDIUM */
        {12, 400, 16.0f, 0.4f}    /* BODY_SMALL */
};

CMP_API int CMP_CALL m3_typography_scale_init(M3TypographyScale *scale) {
  cmp_u32 i;
  if (scale == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < (cmp_u32)M3_TYPOGRAPHY_ROLE_COUNT; i++) {
    CMPTextStyle *style = &scale->roles[i];
    style->utf8_family = M3_TYPOGRAPHY_DEFAULT_FAMILY;
    style->italic = CMP_FALSE;
    style->width_axis = 100.0f;
    style->slant = 0.0f;
    style->grade = 0.0f;
    style->color.r = 0.0f;
    style->color.g = 0.0f;
    style->color.b = 0.0f;
    style->color.a = 1.0f; /* Default solid black, typically overwritten. */

    style->size_px = g_m3_default_props[i].size_px;
    style->weight = g_m3_default_props[i].weight;
    style->line_height_px = g_m3_default_props[i].line_height_px;
    style->letter_spacing = g_m3_default_props[i].letter_spacing;

    /* Optical size typically matches the physical size in standard M3 usage. */
    style->optical_size = (CMPScalar)style->size_px;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL
m3_typography_scale_init_expressive(M3TypographyScale *scale) {
  cmp_u32 i;
  if (scale == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < (cmp_u32)M3_TYPOGRAPHY_ROLE_COUNT; i++) {
    CMPTextStyle *style = &scale->roles[i];
    style->utf8_family = M3_TYPOGRAPHY_DEFAULT_FAMILY;
    style->italic = CMP_FALSE;
    style->width_axis = 100.0f;
    style->slant = 0.0f;
    style->grade = 0.0f;
    style->color.r = 0.0f;
    style->color.g = 0.0f;
    style->color.b = 0.0f;
    style->color.a = 1.0f; /* Default solid black, typically overwritten. */

    style->size_px = g_m3_expressive_props[i].size_px;
    style->weight = g_m3_expressive_props[i].weight;
    style->line_height_px = g_m3_expressive_props[i].line_height_px;
    style->letter_spacing = g_m3_expressive_props[i].letter_spacing;

    /* Optical size typically matches the physical size in standard M3 usage. */
    style->optical_size = (CMPScalar)style->size_px;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL m3_typography_get_style(const M3TypographyScale *scale,
                                             M3TypographyRole role,
                                             CMPTextStyle *out_style) {
  if (scale == NULL || out_style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if ((int)role < 0 || role >= M3_TYPOGRAPHY_ROLE_COUNT) {
    return CMP_ERR_RANGE;
  }

  *out_style = scale->roles[role];
  return CMP_OK;
}

CMP_API int CMP_CALL m3_typography_scale_set_color(M3TypographyScale *scale,
                                                   CMPColor color) {
  cmp_u32 i;
  if (scale == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < (cmp_u32)M3_TYPOGRAPHY_ROLE_COUNT; i++) {
    scale->roles[i].color = color;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL m3_typography_scale_set_family(M3TypographyScale *scale,
                                                    const char *utf8_family) {
  cmp_u32 i;
  if (scale == NULL || utf8_family == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < (cmp_u32)M3_TYPOGRAPHY_ROLE_COUNT; i++) {
    scale->roles[i].utf8_family = utf8_family;
  }

  return CMP_OK;
}
