/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_date_picker {
  int year;
  int month;
  int day;
};

int cmp_date_picker_create(cmp_date_picker_t **out_picker) {
  int rc = CMP_SUCCESS;
  struct cmp_date_picker *picker = NULL;

  if (!out_picker) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_date_picker_create: Invalid argument "
              "(out_picker=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_date_picker), (void **)&picker);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_date_picker_create: Out of memory\n");
    return rc;
  }

  memset(picker, 0, sizeof(struct cmp_date_picker));

  *out_picker = (cmp_date_picker_t *)picker;
  return rc;
}

int cmp_date_picker_destroy(cmp_date_picker_t *picker) {
  int rc = CMP_SUCCESS;
  struct cmp_date_picker *internal_picker = (struct cmp_date_picker *)picker;

  if (!internal_picker) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_date_picker_destroy: Invalid argument (picker=NULL)\n");
    return rc;
  }

  CMP_FREE(internal_picker);
  return rc;
}
