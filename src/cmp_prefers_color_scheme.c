/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_prefers_color_scheme {
  cmp_color_scheme_t current_scheme;
};

int cmp_prefers_color_scheme_create(cmp_prefers_color_scheme_t **out_scheme) {
  cmp_prefers_color_scheme_t *scheme;
  if (!out_scheme) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (CMP_MALLOC(sizeof(cmp_prefers_color_scheme_t), (void **)&scheme) != 0) {
    return CMP_ERROR_OOM;
  }
  memset(scheme, 0, sizeof(cmp_prefers_color_scheme_t));
  scheme->current_scheme = CMP_COLOR_SCHEME_LIGHT; /* Default to light */
  *out_scheme = scheme;
  return CMP_SUCCESS;
}

int cmp_prefers_color_scheme_destroy(cmp_prefers_color_scheme_t *scheme) {
  if (!scheme) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(scheme);
  return CMP_SUCCESS;
}

int cmp_prefers_color_scheme_set(cmp_prefers_color_scheme_t *scheme,
                                 cmp_color_scheme_t color_scheme) {
  if (!scheme) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (color_scheme != CMP_COLOR_SCHEME_LIGHT &&
      color_scheme != CMP_COLOR_SCHEME_DARK) {
    return CMP_ERROR_INVALID_ARG;
  }
  scheme->current_scheme = color_scheme;
  return CMP_SUCCESS;
}

int cmp_prefers_color_scheme_get(const cmp_prefers_color_scheme_t *scheme,
                                 cmp_color_scheme_t *out_color_scheme) {
  if (!scheme || !out_color_scheme) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_color_scheme = scheme->current_scheme;
  return CMP_SUCCESS;
}
