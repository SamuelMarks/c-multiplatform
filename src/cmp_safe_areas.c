/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_safe_areas {
  float insets[4]; /* 0: top, 1: right, 2: bottom, 3: left */
};

int cmp_safe_areas_create(cmp_safe_areas_t **out_safe_areas) {
  cmp_safe_areas_t *safe_areas;

  if (!out_safe_areas) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_safe_areas_t), (void **)&safe_areas) != 0) {
    return CMP_ERROR_OOM;
  }

  memset(safe_areas, 0, sizeof(cmp_safe_areas_t));

  *out_safe_areas = safe_areas;
  return CMP_SUCCESS;
}

int cmp_safe_areas_destroy(cmp_safe_areas_t *safe_areas) {
  if (!safe_areas) {
    return CMP_ERROR_INVALID_ARG;
  }

  CMP_FREE(safe_areas);
  return CMP_SUCCESS;
}

int cmp_safe_areas_set_inset(cmp_safe_areas_t *safe_areas, int edge,
                             float inset) {
  if (!safe_areas || edge < 0 || edge > 3) {
    return CMP_ERROR_INVALID_ARG;
  }

  safe_areas->insets[edge] = inset;
  return CMP_SUCCESS;
}

int cmp_safe_areas_get_inset(const cmp_safe_areas_t *safe_areas, int edge,
                             float *out_inset) {
  if (!safe_areas || !out_inset || edge < 0 || edge > 3) {
    return CMP_ERROR_INVALID_ARG;
  }

  *out_inset = safe_areas->insets[edge];
  return CMP_SUCCESS;
}
