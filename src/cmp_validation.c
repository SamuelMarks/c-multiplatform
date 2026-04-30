/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_validation {
  int is_dirty;
};

/**
 * @brief cmp_validation_create
 *
 * @param out_validation Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_validation_create(cmp_validation_t **out_validation) {
  int rc = CMP_SUCCESS;
  struct cmp_validation *validation;

  if (!out_validation)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_validation), (void **)&validation) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(validation, 0, sizeof(struct cmp_validation));

  *out_validation = (cmp_validation_t *)validation;

  return rc;
}

/**
 * @brief cmp_validation_destroy
 *
 * @param validation Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_validation_destroy(cmp_validation_t *validation) {
  int rc = CMP_SUCCESS;
  struct cmp_validation *internal_validation =
      (struct cmp_validation *)validation;
  if (!internal_validation)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_validation);

  return rc;
}

/**
 * @brief cmp_validation_check
 *
 * @param validation Parameter description.
 * @param input_value Parameter description.
 * @param out_is_valid Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_validation_check(cmp_validation_t *validation, const char *input_value,
                         int *out_is_valid) {
  int rc = CMP_SUCCESS;
  struct cmp_validation *internal_validation =
      (struct cmp_validation *)validation;

  if (!internal_validation || !input_value || !out_is_valid)
    return CMP_ERROR_INVALID_ARG;

  /* Simple dummy logic for testing: empty strings are invalid */
  if (strlen(input_value) == 0) {
    *out_is_valid = 0;
  } else {
    *out_is_valid = 1;
  }

  return rc;
}
