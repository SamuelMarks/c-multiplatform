/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_gradient_create(cmp_gradient_t **out_gradient,
                        cmp_gradient_type_t type) {
  cmp_gradient_t *grad;
  if (!out_gradient)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(cmp_gradient_t), (void **)&grad) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(grad, 0, sizeof(cmp_gradient_t));
  grad->type = type;
  *out_gradient = grad;
  return CMP_SUCCESS;
}

int cmp_gradient_destroy(cmp_gradient_t *gradient) {
  if (!gradient)
    return CMP_ERROR_INVALID_ARG;
  if (gradient->stops)
    CMP_FREE(gradient->stops);
  CMP_FREE(gradient);
  return CMP_SUCCESS;
}

int cmp_gradient_add_stop(cmp_gradient_t *gradient, cmp_color_t color,
                          float position) {
  cmp_gradient_stop_t *new_stops;
  if (!gradient)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC((gradient->stop_count + 1) * sizeof(cmp_gradient_stop_t),
                 (void **)&new_stops) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  if (gradient->stops) {
    memcpy(new_stops, gradient->stops,
           gradient->stop_count * sizeof(cmp_gradient_stop_t));
    CMP_FREE(gradient->stops);
  }

  new_stops[gradient->stop_count].color = color;
  new_stops[gradient->stop_count].position = position;

  gradient->stops = new_stops;
  gradient->stop_count++;
  return CMP_SUCCESS;
}

int cmp_color_parse_p3(const char *color_str, cmp_color_t *out_color) {
  if (!color_str || !out_color)
    return CMP_ERROR_INVALID_ARG;
  /* Simplified stub for parsing "color(display-p3 r g b)" */
  if (strncmp(color_str, "color(display-p3", 16) == 0) {
    out_color->space = CMP_COLOR_SPACE_DISPLAY_P3;
    out_color->a = 1.0f;
    return CMP_SUCCESS;
  }
  return CMP_ERROR_INVALID_ARG;
}

int cmp_color_oklch_to_srgb(const cmp_color_t *in_color,
                            cmp_color_t *out_color) {
  if (!in_color || !out_color)
    return CMP_ERROR_INVALID_ARG;
  /* Matrix transformation stub */
  if (in_color->space == CMP_COLOR_SPACE_OKLCH) {
    out_color->space = CMP_COLOR_SPACE_SRGB;
    out_color->r = in_color->r; /* stub mapped values */
    out_color->g = in_color->g;
    out_color->b = in_color->b;
    out_color->a = in_color->a;
  }
  return CMP_SUCCESS;
}

int cmp_color_mix(const cmp_color_t *c1, const cmp_color_t *c2, float weight,
                  cmp_color_space_t space, cmp_color_t *out_color) {
  if (!c1 || !c2 || !out_color)
    return CMP_ERROR_INVALID_ARG;

  /* Linear interpolation stub based on target space */
  out_color->space = space;
  out_color->r = c1->r * weight + c2->r * (1.0f - weight);
  out_color->g = c1->g * weight + c2->g * (1.0f - weight);
  out_color->b = c1->b * weight + c2->b * (1.0f - weight);
  out_color->a = c1->a * weight + c2->a * (1.0f - weight);
  return CMP_SUCCESS;
}

int cmp_color_luminance(const cmp_color_t *color, float *out_luminance) {
  if (!color || !out_luminance)
    return CMP_ERROR_INVALID_ARG;
  /* WCAG relative luminance formula stub */
  *out_luminance = 0.2126f * color->r + 0.7152f * color->g + 0.0722f * color->b;
  return CMP_SUCCESS;
}

int cmp_color_contrast_ratio(const cmp_color_t *c1, const cmp_color_t *c2,
                             float *out_ratio) {
  float l1, l2;
  if (!c1 || !c2 || !out_ratio)
    return CMP_ERROR_INVALID_ARG;

  cmp_color_luminance(c1, &l1);
  cmp_color_luminance(c2, &l2);

  if (l1 > l2) {
    *out_ratio = (l1 + 0.05f) / (l2 + 0.05f);
  } else {
    *out_ratio = (l2 + 0.05f) / (l1 + 0.05f);
  }
  return CMP_SUCCESS;
}

int cmp_icc_profile_parse(const void *image_buffer, size_t size,
                          void **out_profile_handle) {
  if (!image_buffer || size == 0 || !out_profile_handle)
    return CMP_ERROR_INVALID_ARG;
  /* Read ICC chunks logic placeholder */
  *out_profile_handle = NULL;
  return CMP_SUCCESS;
}
