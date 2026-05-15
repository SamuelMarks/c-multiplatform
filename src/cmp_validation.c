/* clang-format off */
#include "cmp.h"
#include "cmp_regex.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_validation {
  int is_dirty;
  int is_required;
  size_t min_length;
  size_t max_length;
  cmp_regex_t *regex;
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

  if (internal_validation->regex) {
    cmp_regex_free(internal_validation->regex);
  }

  CMP_FREE(internal_validation);

  return rc;
}

int cmp_validation_set_regex(cmp_validation_t *validation,
                             const char *pattern) {
  struct cmp_validation *internal_validation =
      (struct cmp_validation *)validation;
  if (!internal_validation)
    return CMP_ERROR_INVALID_ARG;

  if (internal_validation->regex) {
    cmp_regex_free(internal_validation->regex);
    internal_validation->regex = NULL;
  }

  if (pattern) {
    return cmp_regex_compile(&internal_validation->regex, pattern);
  }
  return CMP_SUCCESS;
}

int cmp_validation_set_min_length(cmp_validation_t *validation,
                                  size_t min_len) {
  struct cmp_validation *internal_validation =
      (struct cmp_validation *)validation;
  if (!internal_validation)
    return CMP_ERROR_INVALID_ARG;

  internal_validation->min_length = min_len;
  return CMP_SUCCESS;
}

int cmp_validation_set_max_length(cmp_validation_t *validation,
                                  size_t max_len) {
  struct cmp_validation *internal_validation =
      (struct cmp_validation *)validation;
  if (!internal_validation)
    return CMP_ERROR_INVALID_ARG;

  internal_validation->max_length = max_len;
  return CMP_SUCCESS;
}

int cmp_validation_set_required(cmp_validation_t *validation, int is_required) {
  struct cmp_validation *internal_validation =
      (struct cmp_validation *)validation;
  if (!internal_validation)
    return CMP_ERROR_INVALID_ARG;

  internal_validation->is_required = is_required;
  return CMP_SUCCESS;
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
  size_t len;
  int matched = 0;

  if (!internal_validation || !input_value || !out_is_valid)
    return CMP_ERROR_INVALID_ARG;

  len = strlen(input_value);

  if (internal_validation->is_required && len == 0) {
    *out_is_valid = 0;
    return rc;
  }

  if (len > 0) {
    if (internal_validation->min_length > 0 &&
        len < internal_validation->min_length) {
      *out_is_valid = 0;
      return rc;
    }

    if (internal_validation->max_length > 0 &&
        len > internal_validation->max_length) {
      *out_is_valid = 0;
      return rc;
    }

    if (internal_validation->regex) {
      rc = cmp_regex_match(internal_validation->regex, input_value, &matched);
      if (rc != CMP_SUCCESS || !matched) {
        *out_is_valid = 0;
        return rc;
      }
    }
  }

  *out_is_valid = 1;

  return rc;
}
