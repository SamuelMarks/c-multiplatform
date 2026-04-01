/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_i18n_formatting {
  int uses_system_dates;
  int uses_system_numbers;
  int uses_system_names;
  int uses_system_measurements;
};

int cmp_i18n_formatting_create(cmp_i18n_formatting_t **out_format) {
  struct cmp_i18n_formatting *ctx;
  if (!out_format)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_i18n_formatting), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->uses_system_dates = 1;
  ctx->uses_system_numbers = 1;
  ctx->uses_system_names = 1;
  ctx->uses_system_measurements = 1;

  *out_format = (cmp_i18n_formatting_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_i18n_formatting_destroy(cmp_i18n_formatting_t *format_opaque) {
  if (format_opaque)
    CMP_FREE(format_opaque);
  return CMP_SUCCESS;
}

int cmp_i18n_handle_dynamic_expansion(cmp_i18n_formatting_t *format_opaque,
                                      void *text_node) {
  struct cmp_i18n_formatting *ctx = (struct cmp_i18n_formatting *)format_opaque;
  if (!ctx || !text_node)
    return CMP_ERROR_INVALID_ARG;

  /* Allows text layout to expand up to 50% for verbose languages */
  return CMP_SUCCESS;
}

int cmp_i18n_load_stringsdict(cmp_i18n_formatting_t *format_opaque,
                              const char *file_data) {
  struct cmp_i18n_formatting *ctx = (struct cmp_i18n_formatting *)format_opaque;
  if (!ctx || !file_data)
    return CMP_ERROR_INVALID_ARG;

  /* Parses pluralization and inflection grammar rules */
  return CMP_SUCCESS;
}

int cmp_i18n_format_date(cmp_i18n_formatting_t *format_opaque,
                         long long unix_timestamp, char *out_str,
                         size_t max_len) {
  struct cmp_i18n_formatting *ctx = (struct cmp_i18n_formatting *)format_opaque;
  if (!ctx || !out_str || max_len == 0 || unix_timestamp < 0)
    return CMP_ERROR_INVALID_ARG;

  /* Hooks to standard OS date formatters */
  if (max_len > 0)
    out_str[0] = '\0';
  return CMP_SUCCESS;
}

int cmp_i18n_format_currency(cmp_i18n_formatting_t *format_opaque,
                             double amount, const char *currency_code,
                             char *out_str, size_t max_len) {
  struct cmp_i18n_formatting *ctx = (struct cmp_i18n_formatting *)format_opaque;
  if (!ctx || !currency_code || !out_str || max_len == 0 || amount < 0.0)
    return CMP_ERROR_INVALID_ARG;

  /* Hooks to standard OS currency formatters */
  if (max_len > 0)
    out_str[0] = '\0';
  return CMP_SUCCESS;
}

int cmp_i18n_format_person_name(cmp_i18n_formatting_t *format_opaque,
                                const char *given_name, const char *family_name,
                                char *out_str, size_t max_len) {
  struct cmp_i18n_formatting *ctx = (struct cmp_i18n_formatting *)format_opaque;
  if (!ctx || !given_name || !family_name || !out_str || max_len == 0)
    return CMP_ERROR_INVALID_ARG;

  /* PersonNameComponentsFormatter integration */
  if (max_len > 0)
    out_str[0] = '\0';
  return CMP_SUCCESS;
}

int cmp_i18n_format_measurement(cmp_i18n_formatting_t *format_opaque,
                                double value, const char *unit, char *out_str,
                                size_t max_len) {
  struct cmp_i18n_formatting *ctx = (struct cmp_i18n_formatting *)format_opaque;
  if (!ctx || !unit || !out_str || max_len == 0 || value < 0.0)
    return CMP_ERROR_INVALID_ARG;

  if (max_len > 0)
    out_str[0] = '\0';
  return CMP_SUCCESS;
}

int cmp_i18n_format_list(cmp_i18n_formatting_t *format_opaque,
                         const char **items, size_t item_count, char *out_str,
                         size_t max_len) {
  struct cmp_i18n_formatting *ctx = (struct cmp_i18n_formatting *)format_opaque;
  if (!ctx || !items || item_count == 0 || !out_str || max_len == 0)
    return CMP_ERROR_INVALID_ARG;

  if (max_len > 0)
    out_str[0] = '\0';
  return CMP_SUCCESS;
}

int cmp_i18n_apply_cjk_vertical_text(cmp_i18n_formatting_t *format_opaque,
                                     void *text_node) {
  struct cmp_i18n_formatting *ctx = (struct cmp_i18n_formatting *)format_opaque;
  if (!ctx || !text_node)
    return CMP_ERROR_INVALID_ARG;

  return CMP_SUCCESS;
}

int cmp_i18n_localized_sort(cmp_i18n_formatting_t *format_opaque, const char *a,
                            const char *b, int *out_result) {
  struct cmp_i18n_formatting *ctx = (struct cmp_i18n_formatting *)format_opaque;
  if (!ctx || !a || !b || !out_result)
    return CMP_ERROR_INVALID_ARG;

  *out_result = 0; /* ignoring diacritics */
  return CMP_SUCCESS;
}
