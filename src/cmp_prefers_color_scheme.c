/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_prefers_color_scheme {
  cmp_color_scheme_t current_scheme;
};

int cmp_prefers_color_scheme_create(cmp_prefers_color_scheme_t **out_scheme) {
  int rc = CMP_SUCCESS;
  cmp_prefers_color_scheme_t *scheme = NULL;

  if (!out_scheme) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_prefers_color_scheme_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_prefers_color_scheme_t), (void **)&scheme);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_prefers_color_scheme_create: Out of memory\n");
    return rc;
  }

  memset(scheme, 0, sizeof(cmp_prefers_color_scheme_t));
  scheme->current_scheme = CMP_COLOR_SCHEME_LIGHT; /* Default to light */
  *out_scheme = scheme;
  return rc;
}

int cmp_prefers_color_scheme_destroy(cmp_prefers_color_scheme_t *scheme) {
  int rc = CMP_SUCCESS;

  if (!scheme) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_prefers_color_scheme_destroy: Invalid argument\n");
    return rc;
  }
  CMP_FREE(scheme);
  return rc;
}

int cmp_prefers_color_scheme_set(cmp_prefers_color_scheme_t *scheme,
                                 cmp_color_scheme_t color_scheme) {
  int rc = CMP_SUCCESS;

  if (!scheme) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_prefers_color_scheme_set: Invalid argument\n");
    return rc;
  }

  if (color_scheme != CMP_COLOR_SCHEME_LIGHT &&
      color_scheme != CMP_COLOR_SCHEME_DARK) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_prefers_color_scheme_set: Invalid scheme\n");
    return rc;
  }

  scheme->current_scheme = color_scheme;
  return rc;
}

int cmp_prefers_color_scheme_get(const cmp_prefers_color_scheme_t *scheme,
                                 cmp_color_scheme_t *out_color_scheme) {
  int rc = CMP_SUCCESS;

  if (!scheme || !out_color_scheme) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_prefers_color_scheme_get: Invalid argument\n");
    return rc;
  }

  *out_color_scheme = scheme->current_scheme;
  return rc;
}
