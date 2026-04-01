/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_dynamic_type {
  cmp_a11y_content_size_category_t category;
};

int cmp_dynamic_type_create(cmp_dynamic_type_t **out_dynamic_type) {
  struct cmp_dynamic_type *dt;

  if (!out_dynamic_type)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_dynamic_type), (void **)&dt) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  dt->category = CMP_A11Y_CONTENT_SIZE_LARGE; /* Default */
  *out_dynamic_type = (cmp_dynamic_type_t *)dt;
  return CMP_SUCCESS;
}

int cmp_dynamic_type_destroy(cmp_dynamic_type_t *dynamic_type) {
  if (dynamic_type) {
    CMP_FREE(dynamic_type);
  }
  return CMP_SUCCESS;
}

int cmp_dynamic_type_set_category(cmp_dynamic_type_t *dynamic_type,
                                  cmp_a11y_content_size_category_t category) {
  struct cmp_dynamic_type *dt = (struct cmp_dynamic_type *)dynamic_type;
  if (!dt)
    return CMP_ERROR_INVALID_ARG;
  dt->category = category;
  return CMP_SUCCESS;
}

int cmp_dynamic_type_apply_scale(cmp_dynamic_type_t *dynamic_type,
                                 float base_size, float *out_scaled_size) {
  struct cmp_dynamic_type *dt = (struct cmp_dynamic_type *)dynamic_type;
  float scale_factor = 1.0f;

  if (!dt || !out_scaled_size)
    return CMP_ERROR_INVALID_ARG;

  switch (dt->category) {
  case CMP_A11Y_CONTENT_SIZE_EXTRA_SMALL:
    scale_factor = 0.8f;
    break;
  case CMP_A11Y_CONTENT_SIZE_SMALL:
    scale_factor = 0.85f;
    break;
  case CMP_A11Y_CONTENT_SIZE_MEDIUM:
    scale_factor = 0.9f;
    break;
  case CMP_A11Y_CONTENT_SIZE_LARGE:
    scale_factor = 1.0f;
    break;
  case CMP_A11Y_CONTENT_SIZE_EXTRA_LARGE:
    scale_factor = 1.15f;
    break;
  case CMP_A11Y_CONTENT_SIZE_EXTRA_EXTRA_LARGE:
    scale_factor = 1.3f;
    break;
  case CMP_A11Y_CONTENT_SIZE_EXTRA_EXTRA_EXTRA_LARGE:
    scale_factor = 1.5f;
    break;
  case CMP_A11Y_CONTENT_SIZE_ACCESSIBILITY_MEDIUM:
    scale_factor = 2.0f;
    break;
  case CMP_A11Y_CONTENT_SIZE_ACCESSIBILITY_LARGE:
    scale_factor = 2.5f;
    break;
  case CMP_A11Y_CONTENT_SIZE_ACCESSIBILITY_EXTRA_LARGE:
    scale_factor = 3.0f;
    break;
  case CMP_A11Y_CONTENT_SIZE_ACCESSIBILITY_EXTRA_EXTRA_LARGE:
    scale_factor = 3.5f;
    break;
  case CMP_A11Y_CONTENT_SIZE_ACCESSIBILITY_EXTRA_EXTRA_EXTRA_LARGE:
    scale_factor = 4.0f;
    break;
  default:
    scale_factor = 1.0f;
    break;
  }

  *out_scaled_size = base_size * scale_factor;
  return CMP_SUCCESS;
}

int cmp_dynamic_type_should_reflow(cmp_dynamic_type_t *dynamic_type,
                                   int *out_should_reflow) {
  struct cmp_dynamic_type *dt = (struct cmp_dynamic_type *)dynamic_type;

  if (!dt || !out_should_reflow)
    return CMP_ERROR_INVALID_ARG;

  *out_should_reflow =
      (dt->category >= CMP_A11Y_CONTENT_SIZE_ACCESSIBILITY_LARGE) ? 1 : 0;
  return CMP_SUCCESS;
}

struct cmp_a11y_bold_text {
  int enabled;
};

int cmp_a11y_bold_text_create(cmp_a11y_bold_text_t **out_bold_text) {
  struct cmp_a11y_bold_text *bt;
  if (!out_bold_text)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_a11y_bold_text), (void **)&bt) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  bt->enabled = 0;
  *out_bold_text = (cmp_a11y_bold_text_t *)bt;
  return CMP_SUCCESS;
}

int cmp_a11y_bold_text_destroy(cmp_a11y_bold_text_t *bold_text) {
  if (bold_text)
    CMP_FREE(bold_text);
  return CMP_SUCCESS;
}

int cmp_a11y_bold_text_set(cmp_a11y_bold_text_t *bold_text, int enabled) {
  struct cmp_a11y_bold_text *bt = (struct cmp_a11y_bold_text *)bold_text;
  if (!bt)
    return CMP_ERROR_INVALID_ARG;
  bt->enabled = enabled;
  return CMP_SUCCESS;
}

int cmp_a11y_bold_text_apply(cmp_a11y_bold_text_t *bold_text, int base_weight,
                             int *out_weight) {
  struct cmp_a11y_bold_text *bt = (struct cmp_a11y_bold_text *)bold_text;
  if (!bt || !out_weight)
    return CMP_ERROR_INVALID_ARG;

  if (bt->enabled) {
    *out_weight = base_weight + 300;
    if (*out_weight > 900)
      *out_weight = 900;
  } else {
    *out_weight = base_weight;
  }
  return CMP_SUCCESS;
}

struct cmp_a11y_button_shapes {
  int enabled;
};

int cmp_a11y_button_shapes_create(
    cmp_a11y_button_shapes_t **out_button_shapes) {
  struct cmp_a11y_button_shapes *bs;
  if (!out_button_shapes)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_a11y_button_shapes), (void **)&bs) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  bs->enabled = 0;
  *out_button_shapes = (cmp_a11y_button_shapes_t *)bs;
  return CMP_SUCCESS;
}

int cmp_a11y_button_shapes_destroy(cmp_a11y_button_shapes_t *button_shapes) {
  if (button_shapes)
    CMP_FREE(button_shapes);
  return CMP_SUCCESS;
}

int cmp_a11y_button_shapes_set(cmp_a11y_button_shapes_t *button_shapes,
                               int enabled) {
  struct cmp_a11y_button_shapes *bs =
      (struct cmp_a11y_button_shapes *)button_shapes;
  if (!bs)
    return CMP_ERROR_INVALID_ARG;
  bs->enabled = enabled;
  return CMP_SUCCESS;
}

int cmp_a11y_button_shapes_should_draw(cmp_a11y_button_shapes_t *button_shapes,
                                       int *out_should_draw) {
  struct cmp_a11y_button_shapes *bs =
      (struct cmp_a11y_button_shapes *)button_shapes;
  if (!bs || !out_should_draw)
    return CMP_ERROR_INVALID_ARG;
  *out_should_draw = bs->enabled;
  return CMP_SUCCESS;
}

struct cmp_a11y_increase_contrast {
  int enabled;
};

int cmp_a11y_increase_contrast_create(
    cmp_a11y_increase_contrast_t **out_increase_contrast) {
  struct cmp_a11y_increase_contrast *ic;
  if (!out_increase_contrast)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_a11y_increase_contrast), (void **)&ic) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  ic->enabled = 0;
  *out_increase_contrast = (cmp_a11y_increase_contrast_t *)ic;
  return CMP_SUCCESS;
}

int cmp_a11y_increase_contrast_destroy(
    cmp_a11y_increase_contrast_t *increase_contrast) {
  if (increase_contrast)
    CMP_FREE(increase_contrast);
  return CMP_SUCCESS;
}

int cmp_a11y_increase_contrast_set(
    cmp_a11y_increase_contrast_t *increase_contrast, int enabled) {
  struct cmp_a11y_increase_contrast *ic =
      (struct cmp_a11y_increase_contrast *)increase_contrast;
  if (!ic)
    return CMP_ERROR_INVALID_ARG;
  ic->enabled = enabled;
  return CMP_SUCCESS;
}

int cmp_a11y_increase_contrast_apply(
    cmp_a11y_increase_contrast_t *increase_contrast,
    float *out_opacity_factor) {
  struct cmp_a11y_increase_contrast *ic =
      (struct cmp_a11y_increase_contrast *)increase_contrast;
  if (!ic || !out_opacity_factor)
    return CMP_ERROR_INVALID_ARG;
  *out_opacity_factor =
      ic->enabled ? 1.0f : 0.85f; /* 100% opacity vs slight transparency */
  return CMP_SUCCESS;
}

struct cmp_a11y_hover_text {
  int enabled;
};

int cmp_a11y_hover_text_create(cmp_a11y_hover_text_t **out_hover_text) {
  struct cmp_a11y_hover_text *ht;
  if (!out_hover_text)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_a11y_hover_text), (void **)&ht) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  ht->enabled = 0;
  *out_hover_text = (cmp_a11y_hover_text_t *)ht;
  return CMP_SUCCESS;
}

int cmp_a11y_hover_text_destroy(cmp_a11y_hover_text_t *hover_text) {
  if (hover_text)
    CMP_FREE(hover_text);
  return CMP_SUCCESS;
}

int cmp_a11y_hover_text_get_bubble(cmp_a11y_hover_text_t *hover_text,
                                   int node_id, char *out_text,
                                   size_t capacity) {
  struct cmp_a11y_hover_text *ht = (struct cmp_a11y_hover_text *)hover_text;
  if (!ht || !out_text)
    return CMP_ERROR_INVALID_ARG;
  (void)node_id;
  /* Simulating fetching text from a separate mapped registry or tree hook for
   * hover texts. */
#if defined(_MSC_VER)
  strcpy_s(out_text, capacity, "");
#else
  strcpy(out_text, "");
#endif
  return CMP_SUCCESS;
}

#include <math.h>

float get_luminance(uint32_t rgba) {
  float r = (float)((rgba >> 24) & 0xFF) / 255.0f;
  float g = (float)((rgba >> 16) & 0xFF) / 255.0f;
  float b = (float)((rgba >> 8) & 0xFF) / 255.0f;
  r = r <= 0.03928f ? r / 12.92f : (float)pow((r + 0.055f) / 1.055f, 2.4f);
  g = g <= 0.03928f ? g / 12.92f : (float)pow((g + 0.055f) / 1.055f, 2.4f);
  b = b <= 0.03928f ? b / 12.92f : (float)pow((b + 0.055f) / 1.055f, 2.4f);
  return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

int cmp_color_verify_contrast_ratio(uint32_t foreground_rgba,
                                    uint32_t background_rgba, int is_large_text,
                                    int *out_passes_wcag) {
  float lum1 = get_luminance(foreground_rgba);
  float lum2 = get_luminance(background_rgba);
  float ratio;

  if (!out_passes_wcag)
    return CMP_ERROR_INVALID_ARG;

  if (lum1 > lum2) {
    ratio = (lum1 + 0.05f) / (lum2 + 0.05f);
  } else {
    ratio = (lum2 + 0.05f) / (lum1 + 0.05f);
  }

  if (is_large_text) {
    *out_passes_wcag = ratio >= 3.0f ? 1 : 0;
  } else {
    *out_passes_wcag = ratio >= 4.5f ? 1 : 0;
  }

  return CMP_SUCCESS;
}

struct cmp_a11y_autoplay_avoidance {
  int enabled;
};

int cmp_a11y_autoplay_avoidance_create(
    cmp_a11y_autoplay_avoidance_t **out_ctx) {
  struct cmp_a11y_autoplay_avoidance *ctx;
  if (!out_ctx)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_a11y_autoplay_avoidance), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  ctx->enabled = 0;
  *out_ctx = (cmp_a11y_autoplay_avoidance_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_a11y_autoplay_avoidance_destroy(cmp_a11y_autoplay_avoidance_t *ctx) {
  if (ctx)
    CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_a11y_autoplay_avoidance_set(cmp_a11y_autoplay_avoidance_t *ctx,
                                    int enabled) {
  struct cmp_a11y_autoplay_avoidance *c =
      (struct cmp_a11y_autoplay_avoidance *)ctx;
  if (!c)
    return CMP_ERROR_INVALID_ARG;
  c->enabled = enabled;
  return CMP_SUCCESS;
}

int cmp_a11y_autoplay_should_play(cmp_a11y_autoplay_avoidance_t *ctx,
                                  int *out_should_play) {
  struct cmp_a11y_autoplay_avoidance *c =
      (struct cmp_a11y_autoplay_avoidance *)ctx;
  if (!c || !out_should_play)
    return CMP_ERROR_INVALID_ARG;
  *out_should_play = c->enabled ? 0 : 1;
  return CMP_SUCCESS;
}
