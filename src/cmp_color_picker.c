/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_color_picker {
  float r, g, b, a;
};

int cmp_color_picker_create(cmp_color_picker_t **out_picker) {
  struct cmp_color_picker *picker;

  if (!out_picker)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_color_picker), (void **)&picker) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(picker, 0, sizeof(struct cmp_color_picker));
  picker->a = 1.0f;

  *out_picker = (cmp_color_picker_t *)picker;
  return CMP_SUCCESS;
}

int cmp_color_picker_destroy(cmp_color_picker_t *picker) {
  struct cmp_color_picker *internal_picker = (struct cmp_color_picker *)picker;
  if (!internal_picker)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_picker);
  return CMP_SUCCESS;
}
