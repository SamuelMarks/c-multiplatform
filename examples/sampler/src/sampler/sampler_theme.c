/**
 * @file sampler_theme.c
 * @brief Implementation of runtime theme state management and serialization.
 */

/* clang-format off */
#include "sampler/sampler_theme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

sampler_error_t sampler_theme_init_default(struct sampler_theme *out_theme) {
  if (out_theme == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  out_theme->theme_color_mode = SAMPLER_THEME_COLOR_SYSTEM;
  out_theme->color_mode = SAMPLER_COLOR_BASELINE;
  out_theme->expressive_theme_mode = SAMPLER_EXPRESSIVE_NON_EXPRESSIVE;
  out_theme->focus_indication_style = SAMPLER_FOCUS_INDICATION_OPACITY;
  out_theme->font_scale_mode = SAMPLER_FONT_SCALE_SYSTEM;
  out_theme->text_direction = SAMPLER_TEXT_DIRECTION_SYSTEM;
  out_theme->font_scale = 1.0f;
  out_theme->show_only_expressive_components = 0;
  out_theme->mark_expressive_components = 1;

  return SAMPLER_SUCCESS;
}

sampler_error_t sampler_theme_copy(const struct sampler_theme *src,
                                   struct sampler_theme *dst) {
  if (src == NULL || dst == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  dst->theme_color_mode = src->theme_color_mode;
  dst->color_mode = src->color_mode;
  dst->expressive_theme_mode = src->expressive_theme_mode;
  dst->focus_indication_style = src->focus_indication_style;
  dst->font_scale_mode = src->font_scale_mode;
  dst->text_direction = src->text_direction;
  dst->font_scale = src->font_scale;
  dst->show_only_expressive_components = src->show_only_expressive_components;
  dst->mark_expressive_components = src->mark_expressive_components;

  return SAMPLER_SUCCESS;
}

sampler_error_t sampler_theme_equals(const struct sampler_theme *a,
                                     const struct sampler_theme *b,
                                     int *out_equals) {
  float diff;

  if (a == NULL || b == NULL || out_equals == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  if (a->theme_color_mode != b->theme_color_mode ||
      a->color_mode != b->color_mode ||
      a->expressive_theme_mode != b->expressive_theme_mode ||
      a->focus_indication_style != b->focus_indication_style ||
      a->font_scale_mode != b->font_scale_mode ||
      a->text_direction != b->text_direction ||
      a->show_only_expressive_components !=
          b->show_only_expressive_components ||
      a->mark_expressive_components != b->mark_expressive_components) {
    *out_equals = 0;
    return SAMPLER_SUCCESS;
  }

  diff = a->font_scale - b->font_scale;
  if (diff < -0.001f || diff > 0.001f) {
    *out_equals = 0;
    return SAMPLER_SUCCESS;
  }

  *out_equals = 1;
  return SAMPLER_SUCCESS;
}

sampler_error_t sampler_theme_serialize(const struct sampler_theme *theme,
                                        char *out_buf, size_t buf_size,
                                        size_t *out_written) {
  int written;

  if (theme == NULL || out_buf == NULL || out_written == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }
  if (buf_size < 128) {
    return SAMPLER_ERROR_OUT_OF_BOUNDS;
  }

#if defined(_MSC_VER)
  written = sprintf_s(
      out_buf, buf_size,
      "{themeMode=%.1f, colorMode=%.1f, expressiveThemeMode=%.1f, "
      "focusIndicationStyle=%.1f, fontScale=%.2f, fontScaleMode=%.1f, "
      "textDirection=%.1f, showOnlyExpressiveComponents=%.1f, "
      "markExpressiveComponents=%.1f}",
      (double)theme->theme_color_mode, (double)theme->color_mode,
      (double)theme->expressive_theme_mode,
      (double)theme->focus_indication_style, (double)theme->font_scale,
      (double)theme->font_scale_mode, (double)theme->text_direction,
      (double)theme->show_only_expressive_components,
      (double)theme->mark_expressive_components);
#else
  written =
      sprintf(out_buf,
              "{themeMode=%.1f, colorMode=%.1f, expressiveThemeMode=%.1f, "
              "focusIndicationStyle=%.1f, fontScale=%.2f, fontScaleMode=%.1f, "
              "textDirection=%.1f, showOnlyExpressiveComponents=%.1f, "
              "markExpressiveComponents=%.1f}",
              (double)theme->theme_color_mode, (double)theme->color_mode,
              (double)theme->expressive_theme_mode,
              (double)theme->focus_indication_style, (double)theme->font_scale,
              (double)theme->font_scale_mode, (double)theme->text_direction,
              (double)theme->show_only_expressive_components,
              (double)theme->mark_expressive_components);
#endif

  if (written < 0 || (size_t)written >= buf_size) {
    return SAMPLER_ERROR_STORAGE_FAILURE;
  }

  *out_written = (size_t)written;
  return SAMPLER_SUCCESS;
}

sampler_error_t sampler_theme_deserialize(const char *buf,
                                          struct sampler_theme *out_theme) {
  double tm = 0.0, cm = 0.0, em = 0.0, fs = 0.0, f_val = 1.0, fm = 0.0,
         td = 0.0;
  double soe = 0.0, mec = 1.0;
  const char *p;

  if (buf == NULL || out_theme == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  p = strstr(buf, "themeMode=");
  if (p) {
    tm = atof(p + 10);
  }
  p = strstr(buf, "colorMode=");
  if (p) {
    cm = atof(p + 10);
  }
  p = strstr(buf, "expressiveThemeMode=");
  if (p) {
    em = atof(p + 20);
  }
  p = strstr(buf, "focusIndicationStyle=");
  if (p) {
    fs = atof(p + 21);
  }
  p = strstr(buf, "fontScale=");
  if (p) {
    f_val = atof(p + 10);
  }
  p = strstr(buf, "fontScaleMode=");
  if (p) {
    fm = atof(p + 14);
  }
  p = strstr(buf, "textDirection=");
  if (p) {
    td = atof(p + 14);
  }
  p = strstr(buf, "showOnlyExpressiveComponents=");
  if (p) {
    soe = atof(p + 29);
  }
  p = strstr(buf, "markExpressiveComponents=");
  if (p) {
    mec = atof(p + 25);
  }

  if (f_val < (double)SAMPLER_MIN_FONT_SCALE) {
    f_val = (double)SAMPLER_MIN_FONT_SCALE;
  }
  if (f_val > (double)SAMPLER_MAX_FONT_SCALE) {
    f_val = (double)SAMPLER_MAX_FONT_SCALE;
  }

  out_theme->theme_color_mode = (sampler_theme_color_mode_t)(int)tm;
  out_theme->color_mode = (sampler_color_mode_t)(int)cm;
  out_theme->expressive_theme_mode = (sampler_expressive_theme_mode_t)(int)em;
  out_theme->focus_indication_style = (sampler_focus_indication_style_t)(int)fs;
  out_theme->font_scale = (float)f_val;
  out_theme->font_scale_mode = (sampler_font_scale_mode_t)(int)fm;
  out_theme->text_direction = (sampler_text_direction_t)(int)td;
  out_theme->show_only_expressive_components = (soe > 0.5) ? 1 : 0;
  out_theme->mark_expressive_components = (mec > 0.5) ? 1 : 0;

  return SAMPLER_SUCCESS;
}
