/**
 * @file md3_typography.c
 * @brief Implementation of Material 3 baseline and Expressive typography
 * scales.
 */

/* clang-format off */
#include "material3/md3_typography.h"
#include "ui_font_manager.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

static const struct md3_type_style g_base_styles[MD3_TYPESCALE_ROLE_COUNT] = {
    {57.0f, 64.0f, -0.25f, 400}, /* DISPLAY_LARGE */
    {45.0f, 52.0f, 0.00f, 400},  /* DISPLAY_MEDIUM */
    {36.0f, 44.0f, 0.00f, 400},  /* DISPLAY_SMALL */
    {32.0f, 40.0f, 0.00f, 400},  /* HEADLINE_LARGE */
    {28.0f, 36.0f, 0.00f, 400},  /* HEADLINE_MEDIUM */
    {24.0f, 32.0f, 0.00f, 400},  /* HEADLINE_SMALL */
    {22.0f, 28.0f, 0.00f, 400},  /* TITLE_LARGE */
    {16.0f, 24.0f, 0.15f, 500},  /* TITLE_MEDIUM */
    {14.0f, 20.0f, 0.10f, 500},  /* TITLE_SMALL */
    {16.0f, 24.0f, 0.50f, 400},  /* BODY_LARGE */
    {14.0f, 20.0f, 0.25f, 400},  /* BODY_MEDIUM */
    {12.0f, 16.0f, 0.40f, 400},  /* BODY_SMALL */
    {14.0f, 20.0f, 0.10f, 500},  /* LABEL_LARGE */
    {12.0f, 16.0f, 0.50f, 500},  /* LABEL_MEDIUM */
    {11.0f, 16.0f, 0.50f, 500}   /* LABEL_SMALL */
};

static const int g_emphasized_weights[MD3_TYPESCALE_ROLE_COUNT] = {
    800, /* DISPLAY_LARGE */
    800, /* DISPLAY_MEDIUM */
    800, /* DISPLAY_SMALL */
    800, /* HEADLINE_LARGE */
    800, /* HEADLINE_MEDIUM */
    800, /* HEADLINE_SMALL */
    700, /* TITLE_LARGE */
    700, /* TITLE_MEDIUM */
    700, /* TITLE_SMALL */
    600, /* BODY_LARGE */
    600, /* BODY_MEDIUM */
    600, /* BODY_SMALL */
    700, /* LABEL_LARGE */
    700, /* LABEL_MEDIUM */
    700  /* LABEL_SMALL */
};

static const char *const g_role_names[MD3_TYPESCALE_ROLE_COUNT] = {
    "display-large",   "display-medium", "display-small", "headline-large",
    "headline-medium", "headline-small", "title-large",   "title-medium",
    "title-small",     "body-large",     "body-medium",   "body-small",
    "label-large",     "label-medium",   "label-small"};

/**
 * @brief Retrieves typography metrics for a role.
 *
 * @param role Typography scale role.
 * @param is_emphasized Non-zero for Material 3 Expressive emphasized weight.
 * @param out_style Pointer to receive typographic metrics.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT on bad
 * role/pointer.
 */
ui_error_t md3_typography_get_style(enum md3_typescale_role role,
                                    int is_emphasized,
                                    struct md3_type_style *out_style) {
  if ((unsigned)role >= MD3_TYPESCALE_ROLE_COUNT || !out_style) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_style = g_base_styles[role];
  if (is_emphasized) {
    out_style->weight = g_emphasized_weights[role];
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Represents a typographic property to inject into the design token
 * dictionary.
 */
struct md3_type_prop {
  const char *suffix; /**< Property name suffix */
  float value;        /**< Token numeric value */
};

/**
 * @brief Injects typography tokens into a design token dictionary.
 *
 * @param is_expressive Non-zero to inject both baseline and emphasized
 * expressive tokens.
 * @param dict Pointer to the design token dictionary.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t md3_typography_apply_tokens(int is_expressive,
                                       struct ui_design_token_dict *dict) {
  int i;
  int p;
  int prop_count;
  char buf[96];
  struct md3_type_style style;
  struct md3_type_prop props[5];
  ui_error_t rc;

  if (!dict) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < MD3_TYPESCALE_ROLE_COUNT; i++) {
    style = g_base_styles[i];

    props[0].suffix = "size";
    props[0].value = style.size_sp;
    props[1].suffix = "line-height";
    props[1].value = style.line_height_sp;
    props[2].suffix = "tracking";
    props[2].value = style.tracking_sp;
    props[3].suffix = "weight";
    props[3].value = (float)style.weight;

    prop_count = 4;
    if (is_expressive) {
      props[4].suffix = "emphasized-weight";
      props[4].value = (float)g_emphasized_weights[i];
      prop_count = 5;
    }

    for (p = 0; p < prop_count; p++) {
#if defined(_MSC_VER)
      sprintf_s(buf, sizeof(buf), "--md-sys-typescale-%s-%s", g_role_names[i],
                props[p].suffix);
#else
      snprintf(buf, sizeof(buf), "--md-sys-typescale-%s-%s", g_role_names[i],
               props[p].suffix);
#endif
      rc = ui_design_token_set_number(dict, buf, props[p].value);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Applies Material 3 typographic variable font axis modulation
 * (weight 'wght', width 'wdth', optical size 'opsz') to a font instance.
 *
 * @param font Pointer to the ui_font instance.
 * @param role Typography scale role.
 * @param is_emphasized Non-zero for expressive emphasized font styling.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t md3_typography_apply_font_variations(struct ui_font *font,
                                                enum md3_typescale_role role,
                                                int is_emphasized) {
  struct md3_type_style style;
  struct ui_font_axis axes[3];

  if (!font || (unsigned)role >= MD3_TYPESCALE_ROLE_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  style = g_base_styles[role];
  if (is_emphasized) {
    style.weight = g_emphasized_weights[role];
  }

  axes[0].tag = (((unsigned int)'w' << 24) | ((unsigned int)'g' << 16) |
                 ((unsigned int)'h' << 8) | (unsigned int)'t');
  axes[0].value = (float)style.weight;

  axes[1].tag = (((unsigned int)'w' << 24) | ((unsigned int)'d' << 16) |
                 ((unsigned int)'t' << 8) | (unsigned int)'h');
  axes[1].value = is_emphasized ? 110.0f : 100.0f;

  axes[2].tag = (((unsigned int)'o' << 24) | ((unsigned int)'p' << 16) |
                 ((unsigned int)'s' << 8) | (unsigned int)'z');
  axes[2].value = style.size_sp;

  return ui_font_set_variations(font, axes, 3);
}
