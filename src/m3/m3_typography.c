#include "m3/m3_typography.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_log.h"
#include <string.h>

/** @brief Default font family for M3. */
#define M3_TYPOGRAPHY_DEFAULT_FAMILY "Roboto"

CMP_API int CMP_CALL m3_typography_scale_init(M3TypographyScale *scale) {
  cmp_u32 i;
  if (scale == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < (cmp_u32)M3_TYPOGRAPHY_ROLE_COUNT; i++) {
    CMPTextStyle *style = &scale->roles[i];
    style->utf8_family = M3_TYPOGRAPHY_DEFAULT_FAMILY;
    style->italic = CMP_FALSE;
    style->color.r = 0.0f;
    style->color.g = 0.0f;
    style->color.b = 0.0f;
    style->color.a = 1.0f; /* Default solid black, typically overwritten. */

    switch ((M3TypographyRole)i) {
    case M3_TYPOGRAPHY_DISPLAY_LARGE:
      style->size_px = 57;
      style->weight = 400;
      break;
    case M3_TYPOGRAPHY_DISPLAY_MEDIUM:
      style->size_px = 45;
      style->weight = 400;
      break;
    case M3_TYPOGRAPHY_DISPLAY_SMALL:
      style->size_px = 36;
      style->weight = 400;
      break;
    case M3_TYPOGRAPHY_HEADLINE_LARGE:
      style->size_px = 32;
      style->weight = 400;
      break;
    case M3_TYPOGRAPHY_HEADLINE_MEDIUM:
      style->size_px = 28;
      style->weight = 400;
      break;
    case M3_TYPOGRAPHY_HEADLINE_SMALL:
      style->size_px = 24;
      style->weight = 400;
      break;
    case M3_TYPOGRAPHY_TITLE_LARGE:
      style->size_px = 22;
      style->weight = 400;
      break;
    case M3_TYPOGRAPHY_TITLE_MEDIUM:
      style->size_px = 16;
      style->weight = 500;
      break;
    case M3_TYPOGRAPHY_TITLE_SMALL:
      style->size_px = 14;
      style->weight = 500;
      break;
    case M3_TYPOGRAPHY_LABEL_LARGE:
      style->size_px = 14;
      style->weight = 500;
      break;
    case M3_TYPOGRAPHY_LABEL_MEDIUM:
      style->size_px = 12;
      style->weight = 500;
      break;
    case M3_TYPOGRAPHY_LABEL_SMALL:
      style->size_px = 11;
      style->weight = 500;
      break;
    case M3_TYPOGRAPHY_BODY_LARGE:
      style->size_px = 16;
      style->weight = 400;
      break;
    case M3_TYPOGRAPHY_BODY_MEDIUM:
      style->size_px = 14;
      style->weight = 400;
      break;
    case M3_TYPOGRAPHY_BODY_SMALL:
      style->size_px = 12;
      style->weight = 400;
      break;
    case M3_TYPOGRAPHY_ROLE_COUNT:
      break;
    }
  }

  return CMP_OK;
}

CMP_API int CMP_CALL m3_typography_get_style(const M3TypographyScale *scale,
                                             M3TypographyRole role,
                                             CMPTextStyle *out_style) {
  if (scale == NULL || out_style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (role < 0 || role >= M3_TYPOGRAPHY_ROLE_COUNT) {
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
