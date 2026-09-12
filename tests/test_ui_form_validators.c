/**
 * @file test_ui_form_validators.c
 * @brief Unit tests for ui_form_validators.
 */

/* clang-format off */
#include "ui_form_validators.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

int main(void) {
  ui_validator_fn fn = NULL;
  ui_bool_t is_valid = UI_FALSE;
  union ui_signal_payload val;
  int min_len = 5;
  int max_len = 10;
  int zero_len = 0;
  ui_error_t rc;

  /* 1. ui_validators_required and ui_validator_required */
  rc = ui_validators_required(NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_validators_required(&fn);
  assert(rc == UI_ERROR_NONE);
  assert(fn == ui_validator_required);

  /* NULL out_is_valid */
  val.ptr_val = "test";
  rc = ui_validator_required(NULL, val, NULL, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* String non-empty */
  val.ptr_val = "hello";
  rc = ui_validator_required(NULL, val, NULL, &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_TRUE);

  /* String empty */
  val.ptr_val = "";
  rc = ui_validator_required(NULL, val, NULL, &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_FALSE);

  /* NULL ptr_val */
  val.ptr_val = NULL;
  rc = ui_validator_required(NULL, val, NULL, &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_FALSE);

  /* 2. ui_validators_pattern and ui_validator_pattern */
  rc = ui_validators_pattern(NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_validators_pattern(&fn);
  assert(rc == UI_ERROR_NONE);
  assert(fn == ui_validator_pattern);

  /* NULL out_is_valid */
  val.ptr_val = "abc";
  rc = ui_validator_pattern(NULL, val, (void *)"a", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* NULL text */
  val.ptr_val = NULL;
  rc = ui_validator_pattern(NULL, val, (void *)"a", &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_FALSE);

  /* NULL pattern */
  val.ptr_val = "abc";
  rc = ui_validator_pattern(NULL, val, NULL, &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_FALSE);

  /* Empty pattern */
  val.ptr_val = "abc";
  rc = ui_validator_pattern(NULL, val, (void *)"", &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_TRUE);

  /* Substring match */
  val.ptr_val = "abcdef";
  rc = ui_validator_pattern(NULL, val, (void *)"cde", &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_TRUE);

  /* Email pattern with @ match */
  val.ptr_val = "user@example.com";
  rc = ui_validator_pattern(NULL, val, (void *)".+@.+", &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_TRUE);

  /* Email pattern with @ on text without @ */
  val.ptr_val = "user_example.com";
  rc = ui_validator_pattern(NULL, val, (void *)".+@.+", &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_FALSE);

  /* Pattern without @ on text with @ */
  val.ptr_val = "user@example.com";
  rc = ui_validator_pattern(NULL, val, (void *)"nomatch", &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_FALSE);

  /* Pattern mismatch */
  val.ptr_val = "hello";
  rc = ui_validator_pattern(NULL, val, (void *)"world", &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_FALSE);

  /* 3. ui_validators_min_length and ui_validator_min_length */
  rc = ui_validators_min_length(NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_validators_min_length(&fn);
  assert(rc == UI_ERROR_NONE);
  assert(fn == ui_validator_min_length);

  /* NULL out_is_valid */
  val.ptr_val = "abc";
  rc = ui_validator_min_length(NULL, val, &min_len, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* NULL user_data */
  val.ptr_val = "abc";
  rc = ui_validator_min_length(NULL, val, NULL, &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_TRUE);

  /* NULL str with min_len > 0 */
  val.ptr_val = NULL;
  rc = ui_validator_min_length(NULL, val, &min_len, &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_FALSE);

  /* NULL str with min_len <= 0 */
  val.ptr_val = NULL;
  rc = ui_validator_min_length(NULL, val, &zero_len, &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_TRUE);

  /* Valid min length */
  val.ptr_val = "12345";
  rc = ui_validator_min_length(NULL, val, &min_len, &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_TRUE);

  /* Invalid min length */
  val.ptr_val = "1234";
  rc = ui_validator_min_length(NULL, val, &min_len, &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_FALSE);

  /* 4. ui_validators_max_length and ui_validator_max_length */
  rc = ui_validators_max_length(NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_validators_max_length(&fn);
  assert(rc == UI_ERROR_NONE);
  assert(fn == ui_validator_max_length);

  /* NULL out_is_valid */
  val.ptr_val = "abc";
  rc = ui_validator_max_length(NULL, val, &max_len, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* NULL user_data */
  val.ptr_val = "";
  rc = ui_validator_max_length(NULL, val, NULL, &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_TRUE);

  /* NULL str */
  val.ptr_val = NULL;
  rc = ui_validator_max_length(NULL, val, &max_len, &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_TRUE);

  /* Valid max length */
  val.ptr_val = "1234567890";
  rc = ui_validator_max_length(NULL, val, &max_len, &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_TRUE);

  /* Invalid max length */
  val.ptr_val = "12345678901";
  rc = ui_validator_max_length(NULL, val, &max_len, &is_valid);
  assert(rc == UI_ERROR_NONE);
  assert(is_valid == UI_FALSE);

  printf("test_ui_form_validators passed\n");
  return 0;
}
