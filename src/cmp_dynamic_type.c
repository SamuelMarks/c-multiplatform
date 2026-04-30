/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_dynamic_type {
  cmp_a11y_content_size_category_t category;
};

/**
 * @brief cmp_dynamic_type_create
 *
 * @param out_dynamic_type Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dynamic_type_create(cmp_dynamic_type_t **out_dynamic_type) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_dynamic_type *dt = NULL;

  if (out_dynamic_type == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dynamic_type_create: Invalid argument: %s\n", err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_dynamic_type), (void **)&dt);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dynamic_type_create: Out of memory: %s\n", err_str);

    return rc;
  }

  dt->category = CMP_A11Y_CONTENT_SIZE_LARGE; /* Default */
  *out_dynamic_type = (cmp_dynamic_type_t *)dt;
  cmp_log_debug(
      "cmp_dynamic_type_create: Successfully created dynamic type context\n");

  return rc;
}

/**
 * @brief cmp_dynamic_type_destroy
 *
 * @param dynamic_type Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dynamic_type_destroy(cmp_dynamic_type_t *dynamic_type) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (dynamic_type == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dynamic_type_destroy: Invalid argument: %s\n", err_str);

    return rc;
  }

  rc = CMP_FREE(dynamic_type);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_dynamic_type_destroy: CMP_FREE failed\n");
  }

  cmp_log_debug("cmp_dynamic_type_destroy: Successfully destroyed dynamic type "
                "context\n");
  return rc;
}

/**
 * @brief cmp_dynamic_type_set_category
 *
 * @param dynamic_type Parameter description.
 * @param category Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dynamic_type_set_category(cmp_dynamic_type_t *dynamic_type,
                                  cmp_a11y_content_size_category_t category) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_dynamic_type *dt = (struct cmp_dynamic_type *)dynamic_type;

  if (dt == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dynamic_type_set_category: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  dt->category = category;
  cmp_log_debug("cmp_dynamic_type_set_category: Set category\n");

  return rc;
}

/**
 * @brief cmp_dynamic_type_apply_scale
 *
 * @param dynamic_type Parameter description.
 * @param base_size Parameter description.
 * @param out_scaled_size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dynamic_type_apply_scale(cmp_dynamic_type_t *dynamic_type,
                                 float base_size, float *out_scaled_size) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_dynamic_type *dt = (struct cmp_dynamic_type *)dynamic_type;
  float scale_factor = 1.0f;

  if (dt == NULL || out_scaled_size == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dynamic_type_apply_scale: Invalid argument: %s\n",
                  err_str);

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
  cmp_log_debug("cmp_dynamic_type_apply_scale: Derived scale curve output\n");

  return rc;
}

/**
 * @brief cmp_dynamic_type_should_reflow
 *
 * @param dynamic_type Parameter description.
 * @param out_should_reflow Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dynamic_type_should_reflow(cmp_dynamic_type_t *dynamic_type,
                                   int *out_should_reflow) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_dynamic_type *dt = (struct cmp_dynamic_type *)dynamic_type;

  if (dt == NULL || out_should_reflow == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dynamic_type_should_reflow: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  *out_should_reflow =
      (dt->category >= CMP_A11Y_CONTENT_SIZE_ACCESSIBILITY_LARGE) ? 1 : 0;
  cmp_log_debug("cmp_dynamic_type_should_reflow: Derived reflow status\n");

  return rc;
}

struct cmp_a11y_bold_text {
  int enabled;
};

/**
 * @brief cmp_a11y_bold_text_create
 *
 * @param out_bold_text Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_bold_text_create(cmp_a11y_bold_text_t **out_bold_text) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_a11y_bold_text *bt = NULL;

  if (out_bold_text == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_bold_text_create: Invalid argument: %s\n", err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_a11y_bold_text), (void **)&bt);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_bold_text_create: Out of memory: %s\n", err_str);

    return rc;
  }

  bt->enabled = 0;
  *out_bold_text = (cmp_a11y_bold_text_t *)bt;

  return rc;
}

/**
 * @brief cmp_a11y_bold_text_destroy
 *
 * @param bold_text Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_bold_text_destroy(cmp_a11y_bold_text_t *bold_text) {
  int rc = CMP_SUCCESS;
  if (bold_text != NULL) {
    rc = CMP_FREE(bold_text);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_a11y_bold_text_destroy: CMP_FREE failed\n");
    }
  }
  return rc;
}

/**
 * @brief cmp_a11y_bold_text_set
 *
 * @param bold_text Parameter description.
 * @param enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_bold_text_set(cmp_a11y_bold_text_t *bold_text, int enabled) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_a11y_bold_text *bt = (struct cmp_a11y_bold_text *)bold_text;

  if (bt == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_bold_text_set: Invalid argument: %s\n", err_str);

    return rc;
  }

  bt->enabled = enabled;

  return rc;
}

/**
 * @brief cmp_a11y_bold_text_apply
 *
 * @param bold_text Parameter description.
 * @param base_weight Parameter description.
 * @param out_weight Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_bold_text_apply(cmp_a11y_bold_text_t *bold_text, int base_weight,
                             int *out_weight) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_a11y_bold_text *bt = (struct cmp_a11y_bold_text *)bold_text;

  if (bt == NULL || out_weight == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_bold_text_apply: Invalid argument: %s\n", err_str);

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

/**
 * @brief cmp_a11y_button_shapes_create
 *
 * @param out_button_shapes Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_button_shapes_create(
    cmp_a11y_button_shapes_t **out_button_shapes) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_a11y_button_shapes *bs = NULL;

  if (out_button_shapes == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_button_shapes_create: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_a11y_button_shapes), (void **)&bs);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_button_shapes_create: Out of memory: %s\n",
                  err_str);

    return rc;
  }

  bs->enabled = 0;
  *out_button_shapes = (cmp_a11y_button_shapes_t *)bs;

  return rc;
}

/**
 * @brief cmp_a11y_button_shapes_destroy
 *
 * @param button_shapes Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_button_shapes_destroy(cmp_a11y_button_shapes_t *button_shapes) {
  int rc = CMP_SUCCESS;
  if (button_shapes != NULL) {
    rc = CMP_FREE(button_shapes);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_a11y_button_shapes_destroy: CMP_FREE failed\n");
    }
  }
  return rc;
}

/**
 * @brief cmp_a11y_button_shapes_set
 *
 * @param button_shapes Parameter description.
 * @param enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_button_shapes_set(cmp_a11y_button_shapes_t *button_shapes,
                               int enabled) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_a11y_button_shapes *bs =
      (struct cmp_a11y_button_shapes *)button_shapes;

  if (bs == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_button_shapes_set: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  bs->enabled = enabled;

  return rc;
}

/**
 * @brief cmp_a11y_button_shapes_should_draw
 *
 * @param button_shapes Parameter description.
 * @param out_should_draw Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_button_shapes_should_draw(cmp_a11y_button_shapes_t *button_shapes,
                                       int *out_should_draw) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_a11y_button_shapes *bs =
      (struct cmp_a11y_button_shapes *)button_shapes;

  if (bs == NULL || out_should_draw == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_button_shapes_should_draw: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  *out_should_draw = bs->enabled;

  return rc;
}

struct cmp_a11y_increase_contrast {
  int enabled;
};

/**
 * @brief cmp_a11y_increase_contrast_create
 *
 * @param out_increase_contrast Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_increase_contrast_create(
    cmp_a11y_increase_contrast_t **out_increase_contrast) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_a11y_increase_contrast *ic = NULL;

  if (out_increase_contrast == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_increase_contrast_create: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_a11y_increase_contrast), (void **)&ic);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_increase_contrast_create: Out of memory: %s\n",
                  err_str);

    return rc;
  }

  ic->enabled = 0;
  *out_increase_contrast = (cmp_a11y_increase_contrast_t *)ic;

  return rc;
}

/**
 * @brief cmp_a11y_increase_contrast_destroy
 *
 * @param increase_contrast Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_increase_contrast_destroy(
    cmp_a11y_increase_contrast_t *increase_contrast) {
  int rc = CMP_SUCCESS;
  if (increase_contrast != NULL) {
    rc = CMP_FREE(increase_contrast);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_a11y_increase_contrast_destroy: CMP_FREE failed\n");
    }
  }
  return rc;
}

/**
 * @brief cmp_a11y_increase_contrast_set
 *
 * @param increase_contrast Parameter description.
 * @param enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_increase_contrast_set(
    cmp_a11y_increase_contrast_t *increase_contrast, int enabled) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_a11y_increase_contrast *ic =
      (struct cmp_a11y_increase_contrast *)increase_contrast;

  if (ic == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_increase_contrast_set: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  ic->enabled = enabled;

  return rc;
}

/**
 * @brief cmp_a11y_increase_contrast_apply
 *
 * @param increase_contrast Parameter description.
 * @param out_opacity_factor Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_increase_contrast_apply(
    cmp_a11y_increase_contrast_t *increase_contrast,
    float *out_opacity_factor) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_a11y_increase_contrast *ic =
      (struct cmp_a11y_increase_contrast *)increase_contrast;

  if (ic == NULL || out_opacity_factor == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_increase_contrast_apply: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  *out_opacity_factor =
      ic->enabled ? 1.0f : 0.85f; /* 100% opacity vs slight transparency */

  return rc;
}

struct cmp_a11y_hover_text {
  int enabled;
};

/**
 * @brief cmp_a11y_hover_text_create
 *
 * @param out_hover_text Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_hover_text_create(cmp_a11y_hover_text_t **out_hover_text) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_a11y_hover_text *ht = NULL;

  if (out_hover_text == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_hover_text_create: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_a11y_hover_text), (void **)&ht);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_hover_text_create: Out of memory: %s\n", err_str);

    return rc;
  }

  ht->enabled = 0;
  *out_hover_text = (cmp_a11y_hover_text_t *)ht;

  return rc;
}

/**
 * @brief cmp_a11y_hover_text_destroy
 *
 * @param hover_text Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_hover_text_destroy(cmp_a11y_hover_text_t *hover_text) {
  int rc = CMP_SUCCESS;
  if (hover_text != NULL) {
    rc = CMP_FREE(hover_text);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_a11y_hover_text_destroy: CMP_FREE failed\n");
    }
  }
  return rc;
}

/**
 * @brief cmp_a11y_hover_text_get_bubble
 *
 * @param hover_text Parameter description.
 * @param node_id Parameter description.
 * @param out_text Parameter description.
 * @param capacity Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_hover_text_get_bubble(cmp_a11y_hover_text_t *hover_text,
                                   int node_id, char *out_text,
                                   size_t capacity) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_a11y_hover_text *ht = (struct cmp_a11y_hover_text *)hover_text;

  if (ht == NULL || out_text == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_hover_text_get_bubble: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  (void)node_id;
  /* Simulating fetching text from a separate mapped registry or tree hook for
   * hover texts. */
#if defined(_MSC_VER)
  if (strcpy_s(out_text, capacity, "") != 0) {
    cmp_log_debug("cmp_a11y_hover_text_get_bubble: strcpy_s failed\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strcpy(out_text, "");
#endif

  return rc;
}
#include <math.h>

/**
 * @brief cmp_math_get_luminance
 *
 * @param rgba Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_EXEMPT(static float cmp_math_get_luminance(uint32_t rgba)) {
  float r = (float)((rgba >> 24) & 0xFF) / 255.0f;
  float g = (float)((rgba >> 16) & 0xFF) / 255.0f;
  float b = (float)((rgba >> 8) & 0xFF) / 255.0f;
  r = r <= 0.03928f ? r / 12.92f : (float)pow((r + 0.055f) / 1.055f, 2.4f);
  g = g <= 0.03928f ? g / 12.92f : (float)pow((g + 0.055f) / 1.055f, 2.4f);
  b = b <= 0.03928f ? b / 12.92f : (float)pow((b + 0.055f) / 1.055f, 2.4f);
  return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

/**
 * @brief cmp_color_verify_contrast_ratio
 *
 * @param foreground_rgba Parameter description.
 * @param background_rgba Parameter description.
 * @param is_large_text Parameter description.
 * @param out_passes_wcag Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_color_verify_contrast_ratio(uint32_t foreground_rgba,
                                    uint32_t background_rgba, int is_large_text,
                                    int *out_passes_wcag) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  float lum1 = cmp_math_get_luminance(foreground_rgba);
  float lum2 = cmp_math_get_luminance(background_rgba);
  float ratio;

  if (out_passes_wcag == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_color_verify_contrast_ratio: Invalid argument: %s\n",
                  err_str);

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

/**
 * @brief cmp_a11y_autoplay_avoidance_create
 *
 * @param out_ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_autoplay_avoidance_create(
    cmp_a11y_autoplay_avoidance_t **out_ctx) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_a11y_autoplay_avoidance *ctx = NULL;

  if (out_ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_autoplay_avoidance_create: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_a11y_autoplay_avoidance), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_autoplay_avoidance_create: Out of memory: %s\n",
                  err_str);

    return rc;
  }

  ctx->enabled = 0;
  *out_ctx = (cmp_a11y_autoplay_avoidance_t *)ctx;

  return rc;
}

/**
 * @brief cmp_a11y_autoplay_avoidance_destroy
 *
 * @param ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_autoplay_avoidance_destroy(cmp_a11y_autoplay_avoidance_t *ctx) {
  int rc = CMP_SUCCESS;
  if (ctx != NULL) {
    rc = CMP_FREE(ctx);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_a11y_autoplay_avoidance_destroy: CMP_FREE failed\n");
    }
  }
  return rc;
}

/**
 * @brief cmp_a11y_autoplay_avoidance_set
 *
 * @param ctx Parameter description.
 * @param enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_autoplay_avoidance_set(cmp_a11y_autoplay_avoidance_t *ctx,
                                    int enabled) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_a11y_autoplay_avoidance *c =
      (struct cmp_a11y_autoplay_avoidance *)ctx;

  if (c == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_autoplay_avoidance_set: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  c->enabled = enabled;

  return rc;
}

/**
 * @brief cmp_a11y_autoplay_should_play
 *
 * @param ctx Parameter description.
 * @param out_should_play Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_autoplay_should_play(cmp_a11y_autoplay_avoidance_t *ctx,
                                  int *out_should_play) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_a11y_autoplay_avoidance *c =
      (struct cmp_a11y_autoplay_avoidance *)ctx;

  if (c == NULL || out_should_play == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_autoplay_should_play: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  *out_should_play = c->enabled ? 0 : 1;

  return rc;
}
