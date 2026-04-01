/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_form_controls {
  int is_focused;
  int is_hovered;
};

int cmp_form_controls_create(cmp_form_controls_t **out_controls) {
  struct cmp_form_controls *controls;

  if (!out_controls)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_form_controls), (void **)&controls) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(controls, 0, sizeof(struct cmp_form_controls));

  *out_controls = (cmp_form_controls_t *)controls;
  return CMP_SUCCESS;
}

int cmp_form_controls_destroy(cmp_form_controls_t *controls) {
  struct cmp_form_controls *internal_controls =
      (struct cmp_form_controls *)controls;
  if (!internal_controls)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_controls);
  return CMP_SUCCESS;
}
