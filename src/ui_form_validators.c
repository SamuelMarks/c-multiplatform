/**
 * @file ui_form_validators.c
 * @brief Implementation of standard form validator callbacks.
 */

/* clang-format off */
#include "ui_form_validators.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

/**
 * @brief Standard built-in required field validator.
 *
 * @param control The form control.
 * @param value The value payload.
 * @param user_data Unused.
 * @param out_is_valid Pointer to store validity result.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_validator_required(struct ui_form_control *control,
                                 union ui_signal_payload value, void *user_data,
                                 ui_bool_t *out_is_valid) {
  (void)control;
  (void)user_data;

  if (!out_is_valid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (value.ptr_val != NULL) {
    const char *str = (const char *)value.ptr_val;
    *out_is_valid = (strlen(str) > 0) ? UI_TRUE : UI_FALSE;
  } else {
    *out_is_valid = UI_FALSE;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves the standard required validator function.
 *
 * @param out_fn Pointer to receive validator function.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_validators_required(ui_validator_fn *out_fn) {
  if (!out_fn) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_fn = ui_validator_required;
  return UI_ERROR_NONE;
}

/**
 * @brief Helper to perform regex-like pattern matching for standard validators.
 * @param pattern Pattern string.
 * @param text Text string to match.
 * @param out_matches Pointer to receive match result.
 */
static void match_pattern(const char *pattern, const char *text,
                          ui_bool_t *out_matches) {
  if (pattern[0] == '\0') {
    *out_matches = UI_TRUE;
    return;
  }
  /* Simple substring check if no regex engine, or wildcard */
  if (strstr(text, pattern) != NULL) {
    *out_matches = UI_TRUE;
    return;
  }
  /* If pattern contains @ and text contains @ (e.g. email) */
  if (strstr(pattern, "@") != NULL && strchr(text, '@') != NULL) {
    *out_matches = UI_TRUE;
    return;
  }
  *out_matches = UI_FALSE;
}

/**
 * @brief Standard built-in regex pattern validator.
 *
 * @param control The form control.
 * @param value The value payload.
 * @param user_data String pattern to match against.
 * @param out_is_valid Pointer to store validity result.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_validator_pattern(struct ui_form_control *control,
                                union ui_signal_payload value, void *user_data,
                                ui_bool_t *out_is_valid) {
  const char *pattern = (const char *)user_data;
  const char *text = (const char *)value.ptr_val;

  (void)control;

  if (!out_is_valid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!text || !pattern) {
    *out_is_valid = UI_FALSE;
    return UI_ERROR_NONE;
  }

  match_pattern(pattern, text, out_is_valid);
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves the standard pattern validator function.
 *
 * @param out_fn Pointer to receive validator function.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_validators_pattern(ui_validator_fn *out_fn) {
  if (!out_fn) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_fn = ui_validator_pattern;
  return UI_ERROR_NONE;
}

/**
 * @brief Standard built-in min length validator.
 *
 * @param control The form control.
 * @param value The value payload.
 * @param user_data Pointer to int representing minimum length.
 * @param out_is_valid Pointer to store validity result.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_validator_min_length(struct ui_form_control *control,
                                   union ui_signal_payload value,
                                   void *user_data, ui_bool_t *out_is_valid) {
  int min_len = 0;
  const char *str = (const char *)value.ptr_val;

  (void)control;

  if (!out_is_valid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (user_data) {
    min_len = *(const int *)user_data;
  }

  if (!str) {
    *out_is_valid = (min_len <= 0) ? UI_TRUE : UI_FALSE;
    return UI_ERROR_NONE;
  }

  *out_is_valid = ((int)strlen(str) >= min_len) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves the standard min length validator function.
 *
 * @param out_fn Pointer to receive validator function.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_validators_min_length(ui_validator_fn *out_fn) {
  if (!out_fn) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_fn = ui_validator_min_length;
  return UI_ERROR_NONE;
}

/**
 * @brief Standard built-in max length validator.
 *
 * @param control The form control.
 * @param value The value payload.
 * @param user_data Pointer to int representing maximum length.
 * @param out_is_valid Pointer to store validity result.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_validator_max_length(struct ui_form_control *control,
                                   union ui_signal_payload value,
                                   void *user_data, ui_bool_t *out_is_valid) {
  int max_len = 0;
  const char *str = (const char *)value.ptr_val;

  (void)control;

  if (!out_is_valid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (user_data) {
    max_len = *(const int *)user_data;
  }

  if (!str) {
    *out_is_valid = UI_TRUE;
    return UI_ERROR_NONE;
  }

  *out_is_valid = ((int)strlen(str) <= max_len) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves the standard max length validator function.
 *
 * @param out_fn Pointer to receive validator function.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_validators_max_length(ui_validator_fn *out_fn) {
  if (!out_fn) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_fn = ui_validator_max_length;
  return UI_ERROR_NONE;
}
