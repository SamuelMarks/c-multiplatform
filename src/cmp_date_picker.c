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

/**
 * @brief cmp_date_picker_create
 *
 * @param out_picker Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_date_picker_create(cmp_date_picker_t **out_picker) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_date_picker *picker = NULL;

  if (out_picker == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_date_picker_create: Invalid argument (out_picker=NULL): %s\n",
        err_str);
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_date_picker), (void **)&picker);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_date_picker_create: Out of memory: %s\n", err_str);
    return rc;
  }

  memset(picker, 0, sizeof(struct cmp_date_picker));

  *out_picker = (cmp_date_picker_t *)picker;
  cmp_log_debug(
      "cmp_date_picker_create: Successfully created date picker context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_date_picker_destroy
 *
 * @param picker Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_date_picker_destroy(cmp_date_picker_t *picker) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_date_picker *internal_picker = (struct cmp_date_picker *)picker;

  if (internal_picker == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_date_picker_destroy: Invalid argument: %s\n", err_str);
    return rc;
  }

  rc = CMP_FREE(internal_picker);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_date_picker_destroy: CMP_FREE failed\n");
  }

  cmp_log_debug(
      "cmp_date_picker_destroy: Successfully destroyed date picker context\n");
  return CMP_SUCCESS;
}
