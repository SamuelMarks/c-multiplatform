/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_ipados_multitasking {
  cmp_ipados_features_t *features;
};

int cmp_ipados_multitasking_create(cmp_ipados_multitasking_t **out_mt) {
  cmp_ipados_multitasking_t *mt;
  int err;

  if (!out_mt)
    return CMP_ERROR_INVALID_ARG;

  mt = (cmp_ipados_multitasking_t *)malloc(sizeof(cmp_ipados_multitasking_t));
  if (!mt)
    return CMP_ERROR_OOM;

  err = cmp_ipados_features_create(&mt->features);
  if (err != CMP_SUCCESS) {
    free(mt);
    return err;
  }

  *out_mt = mt;
  return CMP_SUCCESS;
}

int cmp_ipados_multitasking_destroy(cmp_ipados_multitasking_t *mt) {
  int err = CMP_SUCCESS;
  if (!mt)
    return CMP_ERROR_INVALID_ARG;

  if (mt->features) {
    err = cmp_ipados_features_destroy(mt->features);
  }
  free(mt);
  return err;
}

int cmp_ipados_multitasking_request_scene(cmp_ipados_multitasking_t *mt,
                                          const char *activity_identifier) {
  if (!mt || !activity_identifier)
    return CMP_ERROR_INVALID_ARG;

  return cmp_ipados_request_scene_activation(mt->features, activity_identifier);
}

int cmp_ipados_multitasking_resolve_layout(cmp_ipados_multitasking_t *mt,
                                           float width, float height,
                                           cmp_size_class_t *out_horizontal,
                                           cmp_size_class_t *out_vertical) {
  if (!mt || !out_horizontal || !out_vertical)
    return CMP_ERROR_INVALID_ARG;

  return cmp_ipados_resolve_size_classes(width, height, out_horizontal,
                                         out_vertical);
}
