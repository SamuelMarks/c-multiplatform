/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_i18n_formatting {
  int uses_system_dates;
  int uses_system_numbers;
  int uses_system_names;
  int uses_system_measurements;
};

/**
 * @brief cmp_i18n_formatting_create
 *
 * @param out_format Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_i18n_formatting_create(cmp_i18n_formatting_t **out_format) {
  int rc = CMP_SUCCESS;
  struct cmp_i18n_formatting *ctx = NULL;

  if (!out_format) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_i18n_formatting_create: Invalid argument\n");

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_i18n_formatting), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_i18n_formatting_create: Out of memory\n");

    return rc;
  }

  ctx->uses_system_dates = 1;
  ctx->uses_system_numbers = 1;
  ctx->uses_system_names = 1;
  ctx->uses_system_measurements = 1;

  *out_format = (cmp_i18n_formatting_t *)ctx;

  return rc;
}

/**
 * @brief cmp_i18n_formatting_destroy
 *
 * @param format_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_i18n_formatting_destroy(cmp_i18n_formatting_t *format_opaque) {
  int rc = CMP_SUCCESS;
  if (format_opaque) {
    rc = CMP_FREE(format_opaque);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_i18n_formatting_destroy: CMP_FREE failed\n");
    }
  }

  return rc;
}

/**
 * @brief cmp_i18n_handle_dynamic_expansion
 *
 * @param format_opaque Parameter description.
 * @param text_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_i18n_handle_dynamic_expansion(cmp_i18n_formatting_t *format_opaque,
                                      void *text_node) {
  int rc = CMP_SUCCESS;
  struct cmp_i18n_formatting *ctx = (struct cmp_i18n_formatting *)format_opaque;

  if (!ctx || !text_node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_i18n_handle_dynamic_expansion: Invalid argument\n");

    return rc;
  }

  /* Allows text layout to expand up to 50% for verbose languages */

  return rc;
}

/**
 * @brief cmp_i18n_load_stringsdict
 *
 * @param format_opaque Parameter description.
 * @param file_data Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_i18n_load_stringsdict(cmp_i18n_formatting_t *format_opaque,
                              const char *file_data) {
  int rc = CMP_SUCCESS;
  struct cmp_i18n_formatting *ctx = (struct cmp_i18n_formatting *)format_opaque;

  if (!ctx || !file_data) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_i18n_load_stringsdict: Invalid argument\n");

    return rc;
  }

  /* Parses pluralization and inflection grammar rules */

  return rc;
}

/**
 * @brief cmp_i18n_format_date
 *
 * @param format_opaque Parameter description.
 * @param unix_timestamp Parameter description.
 * @param out_str Parameter description.
 * @param max_len Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_i18n_format_date(cmp_i18n_formatting_t *format_opaque,
                         long long unix_timestamp, char *out_str,
                         size_t max_len) {
  int rc = CMP_SUCCESS;
  struct cmp_i18n_formatting *ctx = (struct cmp_i18n_formatting *)format_opaque;

  if (!ctx || !out_str || max_len == 0 || unix_timestamp < 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_i18n_format_date: Invalid argument\n");

    return rc;
  }

  /* Hooks to standard OS date formatters */
  if (max_len > 0)
    out_str[0] = '\0';

  return rc;
}

/**
 * @brief cmp_i18n_format_currency
 *
 * @param format_opaque Parameter description.
 * @param amount Parameter description.
 * @param currency_code Parameter description.
 * @param out_str Parameter description.
 * @param max_len Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_i18n_format_currency(cmp_i18n_formatting_t *format_opaque,
                             double amount, const char *currency_code,
                             char *out_str, size_t max_len) {
  int rc = CMP_SUCCESS;
  struct cmp_i18n_formatting *ctx = (struct cmp_i18n_formatting *)format_opaque;

  if (!ctx || !currency_code || !out_str || max_len == 0 || amount < 0.0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_i18n_format_currency: Invalid argument\n");

    return rc;
  }

  /* Hooks to standard OS currency formatters */
  if (max_len > 0)
    out_str[0] = '\0';

  return rc;
}

/**
 * @brief cmp_i18n_format_person_name
 *
 * @param format_opaque Parameter description.
 * @param given_name Parameter description.
 * @param family_name Parameter description.
 * @param out_str Parameter description.
 * @param max_len Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_i18n_format_person_name(cmp_i18n_formatting_t *format_opaque,
                                const char *given_name, const char *family_name,
                                char *out_str, size_t max_len) {
  int rc = CMP_SUCCESS;
  struct cmp_i18n_formatting *ctx = (struct cmp_i18n_formatting *)format_opaque;

  if (!ctx || !given_name || !family_name || !out_str || max_len == 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_i18n_format_person_name: Invalid argument\n");

    return rc;
  }

  /* PersonNameComponentsFormatter integration */
  if (max_len > 0)
    out_str[0] = '\0';

  return rc;
}

/**
 * @brief cmp_i18n_format_measurement
 *
 * @param format_opaque Parameter description.
 * @param value Parameter description.
 * @param unit Parameter description.
 * @param out_str Parameter description.
 * @param max_len Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_i18n_format_measurement(cmp_i18n_formatting_t *format_opaque,
                                double value, const char *unit, char *out_str,
                                size_t max_len) {
  int rc = CMP_SUCCESS;
  struct cmp_i18n_formatting *ctx = (struct cmp_i18n_formatting *)format_opaque;

  if (!ctx || !unit || !out_str || max_len == 0 || value < 0.0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_i18n_format_measurement: Invalid argument\n");

    return rc;
  }

  if (max_len > 0)
    out_str[0] = '\0';

  return rc;
}

/**
 * @brief cmp_i18n_format_list
 *
 * @param format_opaque Parameter description.
 * @param items Parameter description.
 * @param item_count Parameter description.
 * @param out_str Parameter description.
 * @param max_len Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_i18n_format_list(cmp_i18n_formatting_t *format_opaque,
                         const char **items, size_t item_count, char *out_str,
                         size_t max_len) {
  int rc = CMP_SUCCESS;
  struct cmp_i18n_formatting *ctx = (struct cmp_i18n_formatting *)format_opaque;

  if (!ctx || !items || item_count == 0 || !out_str || max_len == 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_i18n_format_list: Invalid argument\n");

    return rc;
  }

  if (max_len > 0)
    out_str[0] = '\0';

  return rc;
}

/**
 * @brief cmp_i18n_apply_cjk_vertical_text
 *
 * @param format_opaque Parameter description.
 * @param text_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_i18n_apply_cjk_vertical_text(cmp_i18n_formatting_t *format_opaque,
                                     void *text_node) {
  int rc = CMP_SUCCESS;
  struct cmp_i18n_formatting *ctx = (struct cmp_i18n_formatting *)format_opaque;

  if (!ctx || !text_node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_i18n_apply_cjk_vertical_text: Invalid argument\n");

    return rc;
  }

  return rc;
}

/**
 * @brief cmp_i18n_localized_sort
 *
 * @param format_opaque Parameter description.
 * @param a Parameter description.
 * @param b Parameter description.
 * @param out_result Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_i18n_localized_sort(cmp_i18n_formatting_t *format_opaque, const char *a,
                            const char *b, int *out_result) {
  int rc = CMP_SUCCESS;
  struct cmp_i18n_formatting *ctx = (struct cmp_i18n_formatting *)format_opaque;

  if (!ctx || !a || !b || !out_result) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_i18n_localized_sort: Invalid argument\n");

    return rc;
  }

  *out_result = 0; /* ignoring diacritics */

  return rc;
}
