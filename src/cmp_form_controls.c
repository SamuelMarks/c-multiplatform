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

/**
 * @brief cmp_form_controls_create
 *
 * @param out_controls Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_form_controls_create(cmp_form_controls_t **out_controls) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_form_controls *controls = NULL;

  if (out_controls == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_form_controls_create: Invalid argument (out_controls=NULL): %s\n",
        err_str);
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_form_controls), (void **)&controls);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_form_controls_create: Out of memory: %s\n", err_str);
    return rc;
  }

  memset(controls, 0, sizeof(struct cmp_form_controls));

  *out_controls = (cmp_form_controls_t *)controls;
  cmp_log_debug(
      "cmp_form_controls_create: Successfully created form controls context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_form_controls_destroy
 *
 * @param controls Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_form_controls_destroy(cmp_form_controls_t *controls) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_form_controls *internal_controls =
      (struct cmp_form_controls *)controls;

  if (internal_controls == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_form_controls_destroy: Invalid argument: %s\n", err_str);
    return rc;
  }

  rc = CMP_FREE(internal_controls);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_form_controls_destroy: CMP_FREE failed\n");
  }

  cmp_log_debug("cmp_form_controls_destroy: Successfully destroyed form "
                "controls context\n");
  return CMP_SUCCESS;
}
