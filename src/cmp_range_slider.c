/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_range_slider {
  float values[2];
};

int cmp_range_slider_create(cmp_range_slider_t **out_slider) {
  struct cmp_range_slider *slider;

  if (!out_slider)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_range_slider), (void **)&slider) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(slider, 0, sizeof(struct cmp_range_slider));

  *out_slider = (cmp_range_slider_t *)slider;
  return CMP_SUCCESS;
}

int cmp_range_slider_destroy(cmp_range_slider_t *slider) {
  struct cmp_range_slider *internal_slider = (struct cmp_range_slider *)slider;
  if (!internal_slider)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_slider);
  return CMP_SUCCESS;
}

int cmp_range_slider_set_value(cmp_range_slider_t *slider, int thumb_index,
                               float value) {
  struct cmp_range_slider *internal_slider = (struct cmp_range_slider *)slider;

  if (!internal_slider || thumb_index < 0 || thumb_index >= 2)
    return CMP_ERROR_INVALID_ARG;

  internal_slider->values[thumb_index] = value;

  return CMP_SUCCESS;
}
