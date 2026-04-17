/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_dynamic_type {
  cmp_a11y_content_size_category_t category;
};

int cmp_dynamic_type_create(cmp_dynamic_type_t **out_dynamic_type) {
  int rc = CMP_SUCCESS;
  struct cmp_dynamic_type *dt = NULL;

  if (!out_dynamic_type) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dynamic_type_create: Invalid argument "
              "(out_dynamic_type=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_dynamic_type), (void **)&dt);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_dynamic_type_create: Out of memory\n");
    return rc;
  }

  dt->category = CMP_A11Y_CONTENT_SIZE_LARGE; /* Default */
  *out_dynamic_type = (cmp_dynamic_type_t *)dt;
  return rc;
}

int cmp_dynamic_type_destroy(cmp_dynamic_type_t *dynamic_type) {
  int rc = CMP_SUCCESS;
  if (dynamic_type) {
    CMP_FREE(dynamic_type);
  }
  return rc;
}

int cmp_dynamic_type_set_category(cmp_dynamic_type_t *dynamic_type,
                                  cmp_a11y_content_size_category_t category) {
  int rc = CMP_SUCCESS;
  struct cmp_dynamic_type *dt = (struct cmp_dynamic_type *)dynamic_type;

  if (!dt) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dynamic_type_set_category: Invalid argument "
              "(dynamic_type=NULL)\n");
    return rc;
  }

  dt->category = category;
  return rc;
}

int cmp_dynamic_type_apply_scale(cmp_dynamic_type_t *dynamic_type,
                                 float base_size, float *out_scaled_size) {
  int rc = CMP_SUCCESS;
  struct cmp_dynamic_type *dt = (struct cmp_dynamic_type *)dynamic_type;
  float scale_factor = 1.0f;

  if (!dt || !out_scaled_size) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dynamic_type_apply_scale: Invalid argument\n");
    return rc;
  }

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
  return rc;
}

int cmp_dynamic_type_should_reflow(cmp_dynamic_type_t *dynamic_type,
                                   int *out_should_reflow) {
  int rc = CMP_SUCCESS;
  struct cmp_dynamic_type *dt = (struct cmp_dynamic_type *)dynamic_type;

  if (!dt || !out_should_reflow) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dynamic_type_should_reflow: Invalid argument\n");
    return rc;
  }

  *out_should_reflow =
      (dt->category >= CMP_A11Y_CONTENT_SIZE_ACCESSIBILITY_LARGE) ? 1 : 0;
  return rc;
}

struct cmp_a11y_bold_text {
  int enabled;
};

int cmp_a11y_bold_text_create(cmp_a11y_bold_text_t **out_bold_text) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_bold_text *bt = NULL;

  if (!out_bold_text) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_a11y_bold_text_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_a11y_bold_text), (void **)&bt);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_a11y_bold_text_create: Out of memory\n");
    return rc;
  }

  bt->enabled = 0;
  *out_bold_text = (cmp_a11y_bold_text_t *)bt;
  return rc;
}

int cmp_a11y_bold_text_destroy(cmp_a11y_bold_text_t *bold_text) {
  int rc = CMP_SUCCESS;
  if (bold_text) {
    CMP_FREE(bold_text);
  }
  return rc;
}

int cmp_a11y_bold_text_set(cmp_a11y_bold_text_t *bold_text, int enabled) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_bold_text *bt = (struct cmp_a11y_bold_text *)bold_text;

  if (!bt) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_a11y_bold_text_set: Invalid argument\n");
    return rc;
  }

  bt->enabled = enabled;
  return rc;
}

int cmp_a11y_bold_text_apply(cmp_a11y_bold_text_t *bold_text, int base_weight,
                             int *out_weight) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_bold_text *bt = (struct cmp_a11y_bold_text *)bold_text;

  if (!bt || !out_weight) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_a11y_bold_text_apply: Invalid argument\n");
    return rc;
  }

  if (bt->enabled) {
    *out_weight = base_weight + 300;
    if (*out_weight > 900)
      *out_weight = 900;
  } else {
    *out_weight = base_weight;
  }
  return rc;
}

struct cmp_a11y_button_shapes {
  int enabled;
};

int cmp_a11y_button_shapes_create(
    cmp_a11y_button_shapes_t **out_button_shapes) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_button_shapes *bs = NULL;

  if (!out_button_shapes) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_a11y_button_shapes_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_a11y_button_shapes), (void **)&bs);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_a11y_button_shapes_create: Out of memory\n");
    return rc;
  }

  bs->enabled = 0;
  *out_button_shapes = (cmp_a11y_button_shapes_t *)bs;
  return rc;
}

int cmp_a11y_button_shapes_destroy(cmp_a11y_button_shapes_t *button_shapes) {
  int rc = CMP_SUCCESS;
  if (button_shapes) {
    CMP_FREE(button_shapes);
  }
  return rc;
}

int cmp_a11y_button_shapes_set(cmp_a11y_button_shapes_t *button_shapes,
                               int enabled) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_button_shapes *bs =
      (struct cmp_a11y_button_shapes *)button_shapes;

  if (!bs) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_a11y_button_shapes_set: Invalid argument\n");
    return rc;
  }

  bs->enabled = enabled;
  return rc;
}

int cmp_a11y_button_shapes_should_draw(cmp_a11y_button_shapes_t *button_shapes,
                                       int *out_should_draw) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_button_shapes *bs =
      (struct cmp_a11y_button_shapes *)button_shapes;

  if (!bs || !out_should_draw) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_a11y_button_shapes_should_draw: Invalid argument\n");
    return rc;
  }

  *out_should_draw = bs->enabled;
  return rc;
}

struct cmp_a11y_increase_contrast {
  int enabled;
};

int cmp_a11y_increase_contrast_create(
    cmp_a11y_increase_contrast_t **out_increase_contrast) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_increase_contrast *ic = NULL;

  if (!out_increase_contrast) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_a11y_increase_contrast_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_a11y_increase_contrast), (void **)&ic);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_a11y_increase_contrast_create: Out of memory\n");
    return rc;
  }

  ic->enabled = 0;
  *out_increase_contrast = (cmp_a11y_increase_contrast_t *)ic;
  return rc;
}

int cmp_a11y_increase_contrast_destroy(
    cmp_a11y_increase_contrast_t *increase_contrast) {
  int rc = CMP_SUCCESS;
  if (increase_contrast) {
    CMP_FREE(increase_contrast);
  }
  return rc;
}

int cmp_a11y_increase_contrast_set(
    cmp_a11y_increase_contrast_t *increase_contrast, int enabled) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_increase_contrast *ic =
      (struct cmp_a11y_increase_contrast *)increase_contrast;

  if (!ic) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_a11y_increase_contrast_set: Invalid argument\n");
    return rc;
  }

  ic->enabled = enabled;
  return rc;
}

int cmp_a11y_increase_contrast_apply(
    cmp_a11y_increase_contrast_t *increase_contrast,
    float *out_opacity_factor) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_increase_contrast *ic =
      (struct cmp_a11y_increase_contrast *)increase_contrast;

  if (!ic || !out_opacity_factor) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_a11y_increase_contrast_apply: Invalid argument\n");
    return rc;
  }

  *out_opacity_factor =
      ic->enabled ? 1.0f : 0.85f; /* 100% opacity vs slight transparency */
  return rc;
}

struct cmp_a11y_hover_text {
  int enabled;
};

int cmp_a11y_hover_text_create(cmp_a11y_hover_text_t **out_hover_text) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_hover_text *ht = NULL;

  if (!out_hover_text) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_a11y_hover_text_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_a11y_hover_text), (void **)&ht);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_a11y_hover_text_create: Out of memory\n");
    return rc;
  }

  ht->enabled = 0;
  *out_hover_text = (cmp_a11y_hover_text_t *)ht;
  return rc;
}

int cmp_a11y_hover_text_destroy(cmp_a11y_hover_text_t *hover_text) {
  int rc = CMP_SUCCESS;
  if (hover_text) {
    CMP_FREE(hover_text);
  }
  return rc;
}

int cmp_a11y_hover_text_get_bubble(cmp_a11y_hover_text_t *hover_text,
                                   int node_id, char *out_text,
                                   size_t capacity) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_hover_text *ht = (struct cmp_a11y_hover_text *)hover_text;

  if (!ht || !out_text) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_a11y_hover_text_get_bubble: Invalid argument\n");
    return rc;
  }

  (void)node_id;
  /* Simulating fetching text from a separate mapped registry or tree hook for
   * hover texts. */
#if defined(_MSC_VER)
  strcpy_s(out_text, capacity, "");
#else
  strcpy(out_text, "");
#endif
  return rc;
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
  int rc = CMP_SUCCESS;
  float lum1 = get_luminance(foreground_rgba);
  float lum2 = get_luminance(background_rgba);
  float ratio;

  if (!out_passes_wcag) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_color_verify_contrast_ratio: Invalid argument\n");
    return rc;
  }

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

  return rc;
}

struct cmp_a11y_autoplay_avoidance {
  int enabled;
};

int cmp_a11y_autoplay_avoidance_create(
    cmp_a11y_autoplay_avoidance_t **out_ctx) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_autoplay_avoidance *ctx = NULL;

  if (!out_ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_a11y_autoplay_avoidance_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_a11y_autoplay_avoidance), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_a11y_autoplay_avoidance_create: Out of memory\n");
    return rc;
  }

  ctx->enabled = 0;
  *out_ctx = (cmp_a11y_autoplay_avoidance_t *)ctx;
  return rc;
}

int cmp_a11y_autoplay_avoidance_destroy(cmp_a11y_autoplay_avoidance_t *ctx) {
  int rc = CMP_SUCCESS;
  if (ctx) {
    CMP_FREE(ctx);
  }
  return rc;
}

int cmp_a11y_autoplay_avoidance_set(cmp_a11y_autoplay_avoidance_t *ctx,
                                    int enabled) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_autoplay_avoidance *c =
      (struct cmp_a11y_autoplay_avoidance *)ctx;

  if (!c) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_a11y_autoplay_avoidance_set: Invalid argument\n");
    return rc;
  }

  c->enabled = enabled;
  return rc;
}

int cmp_a11y_autoplay_should_play(cmp_a11y_autoplay_avoidance_t *ctx,
                                  int *out_should_play) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_autoplay_avoidance *c =
      (struct cmp_a11y_autoplay_avoidance *)ctx;

  if (!c || !out_should_play) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_a11y_autoplay_should_play: Invalid argument\n");
    return rc;
  }

  *out_should_play = c->enabled ? 0 : 1;
  return rc;
}
