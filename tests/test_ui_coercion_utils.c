/* clang-format off */
#include "ui_coercion_utils.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int run_normal_tests(void) {
  int bval = -1;
  int ival = -1;
  float fval = -1.0f;
  char buffer[32];
  enum ui_error rc;

  printf("Testing invalid arguments...\n");
  if (ui_coerce_string_to_bool(NULL, &bval) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_coerce_string_to_bool("1", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_coerce_string_to_int(NULL, &ival) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_coerce_string_to_int("42", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_coerce_string_to_float(NULL, &fval) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_coerce_string_to_float("42.5", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_safe_string_copy(NULL, sizeof(buffer), "test") !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_safe_string_copy(buffer, 0, "test") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_safe_string_copy(buffer, sizeof(buffer), NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_safe_string_format(NULL, sizeof(buffer), "%d", 42) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_safe_string_format(buffer, 0, "%d", 42) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_safe_string_format(buffer, sizeof(buffer), NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  printf("Testing string_to_bool...\n");
  if (ui_coerce_string_to_bool("1", &bval) != UI_ERROR_NONE || bval != 1)
    return 1;
  if (ui_coerce_string_to_bool("true", &bval) != UI_ERROR_NONE || bval != 1)
    return 1;
  if (ui_coerce_string_to_bool("TRUE", &bval) != UI_ERROR_NONE || bval != 1)
    return 1;
  if (ui_coerce_string_to_bool("True", &bval) != UI_ERROR_NONE || bval != 1)
    return 1;
  if (ui_coerce_string_to_bool("0", &bval) != UI_ERROR_NONE || bval != 0)
    return 1;
  if (ui_coerce_string_to_bool("false", &bval) != UI_ERROR_NONE || bval != 0)
    return 1;
  if (ui_coerce_string_to_bool("FALSE", &bval) != UI_ERROR_NONE || bval != 0)
    return 1;
  if (ui_coerce_string_to_bool("False", &bval) != UI_ERROR_NONE || bval != 0)
    return 1;
  if (ui_coerce_string_to_bool("invalid", &bval) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  printf("Testing string_to_int...\n");
  if (ui_coerce_string_to_int("42", &ival) != UI_ERROR_NONE || ival != 42)
    return 1;
  if (ui_coerce_string_to_int("-42", &ival) != UI_ERROR_NONE || ival != -42)
    return 1;
  if (ui_coerce_string_to_int("42a", &ival) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_coerce_string_to_int("abc", &ival) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  printf("Testing string_to_float...\n");
  if (ui_coerce_string_to_float("42.5", &fval) != UI_ERROR_NONE ||
      fval != 42.5f)
    return 1;
  if (ui_coerce_string_to_float("-42.5", &fval) != UI_ERROR_NONE ||
      fval != -42.5f)
    return 1;
  if (ui_coerce_string_to_float("42.5a", &fval) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_coerce_string_to_float("abc", &fval) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  printf("Testing safe_string_copy...\n");
  rc = ui_safe_string_copy(buffer, sizeof(buffer), "hello");
  if (rc != UI_ERROR_NONE || strcmp(buffer, "hello") != 0) {
    printf("fail 1: rc=%d, buffer=%s\n", rc, buffer);
    return 1;
  }

  rc = ui_safe_string_copy(buffer, 6, "1234567");
  if (rc != UI_ERROR_OUT_OF_BOUNDS || strcmp(buffer, "12345") != 0) {
    printf("fail 2: rc=%d, buffer=%s\n", rc, buffer);
    return 1;
  }

  printf("Testing safe_string_format...\n");
  rc =
      ui_safe_string_format(buffer, sizeof(buffer), "Hello %s %d", "World", 42);
  if (rc != UI_ERROR_NONE || strcmp(buffer, "Hello World 42") != 0) {
    printf("fail 3: rc=%d, buffer=%s\n", rc, buffer);
    return 1;
  }

  rc = ui_safe_string_format(buffer, 6, "12345678");
  if (rc != UI_ERROR_OUT_OF_BOUNDS) {
    printf("fail 4: rc=%d\n", rc);
    return 1;
  }

  return 0;
}

static int run_oom_tests(void) {
  /* No heap allocations in coercion utils currently */
  return 0;
}

int main(void) {
  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }

  if (run_oom_tests() != 0) {
    printf("OOM tests failed.\n");
    return 1;
  }

  printf("All ui_coercion_utils tests passed.\n");
  return 0;
}
