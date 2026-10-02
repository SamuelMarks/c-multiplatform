/**
 * @file cupertino_typography.c
 * @brief Apple SF Typography hierarchy and Dynamic Type engine implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_typography.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_typography_mock_get_tracking_fail = 0;
int g_cupertino_typography_mock_get_style_fail = 0;
int g_cupertino_typography_mock_token_fail = 0;
int g_cupertino_typography_mock_token_fail_step = -1;
int g_cupertino_typography_mock_weight_override = 0;
static int g_cupertino_typography_token_call_count = 0;

static ui_error_t
mock_typography_token_set_number(struct ui_design_token_dict *dict,
                                 const char *name, float number) {
  if (g_cupertino_typography_mock_token_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  if (g_cupertino_typography_mock_token_fail_step >= 0 &&
      g_cupertino_typography_token_call_count ==
          g_cupertino_typography_mock_token_fail_step) {
    g_cupertino_typography_token_call_count++;
    return UI_ERROR_OUT_OF_MEMORY;
  }
  g_cupertino_typography_token_call_count++;
  return ui_design_token_set_number(dict, name, number);
}
#undef ui_design_token_set_number
/** @cond */
#define ui_design_token_set_number mock_typography_token_set_number
/** @endcond */

void cupertino_typography_mock_reset_token_count(void) {
  g_cupertino_typography_token_call_count = 0;
}
#endif

static const struct cupertino_type_metrics
    g_base_metrics[CUPERTINO_TEXT_STYLE_COUNT] = {
        {34.0f, 41.0f, 0.0f, 400, 1}, /* LARGE_TITLE */
        {28.0f, 34.0f, 0.0f, 400, 1}, /* TITLE1 */
        {22.0f, 28.0f, 0.0f, 400, 1}, /* TITLE2 */
        {20.0f, 25.0f, 0.0f, 400, 1}, /* TITLE3 */
        {17.0f, 22.0f, 0.0f, 600, 0}, /* HEADLINE */
        {17.0f, 22.0f, 0.0f, 400, 0}, /* BODY */
        {16.0f, 21.0f, 0.0f, 400, 0}, /* CALLOUT */
        {15.0f, 20.0f, 0.0f, 400, 0}, /* SUBHEADLINE */
        {13.0f, 18.0f, 0.0f, 400, 0}, /* FOOTNOTE */
        {12.0f, 16.0f, 0.0f, 400, 0}, /* CAPTION1 */
        {11.0f, 13.0f, 0.0f, 400, 0}  /* CAPTION2 */
};

static const float g_dt_multipliers[CUPERTINO_DYNAMIC_TYPE_SIZE_COUNT] = {
    0.8235f, /* XSMALL */
    0.8824f, /* SMALL */
    0.9412f, /* MEDIUM */
    1.0000f, /* LARGE (Reference) */
    1.1176f, /* XLARGE */
    1.2353f, /* XXLARGE */
    1.3529f, /* XXXLARGE */
    1.6471f, /* AX1 */
    1.9412f, /* AX2 */
    2.3529f, /* AX3 */
    2.7647f, /* AX4 */
    3.1176f  /* AX5 */
};

static const char *const g_style_names[CUPERTINO_TEXT_STYLE_COUNT] = {
    "large-title", "title1",      "title2",   "title3",   "headline", "body",
    "callout",     "subheadline", "footnote", "caption1", "caption2"};

/**
 * @brief Calculates Apple SF dynamic tracking (kerning) for a given point size.
 */
ui_error_t cupertino_typography_get_tracking(float point_size,
                                             float *out_tracking) {
#ifdef UI_TEST_MOCK_ALLOC
  if (g_cupertino_typography_mock_get_tracking_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (point_size <= 0.0f || !out_tracking) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (point_size < 20.0f) {
    *out_tracking = (20.0f - point_size) * 0.02f;
  } else {
    *out_tracking = -((point_size - 20.0f) * 0.025f);
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Computes typographic metrics for a style under a given Dynamic Type
 * scale.
 */
ui_error_t cupertino_typography_get_style(
    enum cupertino_text_style style, enum cupertino_dynamic_type_size dt_size,
    int is_bold_text_enabled, struct cupertino_type_metrics *out_metrics) {
  float mult;
  float scaled_size;
  float scaled_leading;
  float tracking;
  int weight;
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  if (g_cupertino_typography_mock_get_style_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#endif

  if ((int)style < 0 || style >= CUPERTINO_TEXT_STYLE_COUNT ||
      (int)dt_size < 0 || dt_size >= CUPERTINO_DYNAMIC_TYPE_SIZE_COUNT ||
      !out_metrics) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  mult = g_dt_multipliers[dt_size];
  scaled_size = g_base_metrics[style].point_size * mult;
  scaled_leading = g_base_metrics[style].leading * mult;

  rc = cupertino_typography_get_tracking(scaled_size, &tracking);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  weight = g_base_metrics[style].weight;
#ifdef UI_TEST_MOCK_ALLOC
  if (g_cupertino_typography_mock_weight_override > 0) {
    weight = g_cupertino_typography_mock_weight_override;
  }
#endif
  if (is_bold_text_enabled) {
    if (weight <= 400) {
      weight = 600;
    } else if (weight <= 600) {
      weight = 700;
    } else if (weight <= 700) {
      weight = 800;
    } else {
      weight = 900;
    }
  }

  out_metrics->point_size = scaled_size;
  out_metrics->leading = scaled_leading;
  out_metrics->tracking = tracking;
  out_metrics->weight = weight;
  out_metrics->use_display_face = (scaled_size >= 20.0f) ? 1 : 0;

  return UI_ERROR_NONE;
}

/**
 * @brief Clamps dimensions to Apple's minimum 44x44pt tap target boundary.
 */
ui_error_t cupertino_typography_ensure_min_tap_target(float width, float height,
                                                      float *out_w,
                                                      float *out_h) {
  if (!out_w || !out_h) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_w = (width > CUPERTINO_MINIMUM_TAP_TARGET_PT)
               ? width
               : CUPERTINO_MINIMUM_TAP_TARGET_PT;
  *out_h = (height > CUPERTINO_MINIMUM_TAP_TARGET_PT)
               ? height
               : CUPERTINO_MINIMUM_TAP_TARGET_PT;

  return UI_ERROR_NONE;
}

/**
 * @brief Injects Apple typographic tokens into a design token dictionary.
 */
ui_error_t
cupertino_typography_apply_tokens(struct ui_design_token_dict *dict) {
  int i;
  char buf[96];
  struct cupertino_type_metrics metrics;
  ui_error_t rc;

  if (!dict) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < CUPERTINO_TEXT_STYLE_COUNT; i++) {
    rc = cupertino_typography_get_style((enum cupertino_text_style)i,
                                        CUPERTINO_DYNAMIC_TYPE_LARGE, 0,
                                        &metrics);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "--apple-typescale-%s-size", g_style_names[i]);
#else
    snprintf(buf, sizeof(buf), "--apple-typescale-%s-size", g_style_names[i]);
#endif
    rc = ui_design_token_set_number(dict, buf, metrics.point_size);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "--apple-typescale-%s-leading",
              g_style_names[i]);
#else
    snprintf(buf, sizeof(buf), "--apple-typescale-%s-leading",
             g_style_names[i]);
#endif
    rc = ui_design_token_set_number(dict, buf, metrics.leading);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "--apple-typescale-%s-tracking",
              g_style_names[i]);
#else
    snprintf(buf, sizeof(buf), "--apple-typescale-%s-tracking",
             g_style_names[i]);
#endif
    rc = ui_design_token_set_number(dict, buf, metrics.tracking);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "--apple-typescale-%s-weight",
              g_style_names[i]);
#else
    snprintf(buf, sizeof(buf), "--apple-typescale-%s-weight", g_style_names[i]);
#endif
    rc = ui_design_token_set_number(dict, buf, (float)metrics.weight);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}
