/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_form_controls {
  int is_focused;
  int is_hovered;
};

int cmp_form_controls_create(cmp_form_controls_t **out_controls) {
  int rc = CMP_SUCCESS;
  struct cmp_form_controls *controls = NULL;

  if (!out_controls) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_form_controls_create: Invalid argument "
              "(out_controls=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_form_controls), (void **)&controls);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_form_controls_create: Out of memory\n");
    return rc;
  }

  memset(controls, 0, sizeof(struct cmp_form_controls));

  *out_controls = (cmp_form_controls_t *)controls;
  return rc;
}

int cmp_form_controls_destroy(cmp_form_controls_t *controls) {
  int rc = CMP_SUCCESS;
  struct cmp_form_controls *internal_controls =
      (struct cmp_form_controls *)controls;

  if (!internal_controls) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_form_controls_destroy: Invalid argument "
              "(controls=NULL)\n");
    return rc;
  }

  CMP_FREE(internal_controls);
  return rc;
}
