/* clang-format off */
#include "ui_coercion_utils.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

extern int g_mock_strcpy_fail;

static int run_normal_tests(void) {
  int bval = -1;
  int ival = -1;
  float fval = -1.0f;
  ui_color_t color = 0;
  char buffer[32];
  ui_error_t rc;

  /* Invalid arguments */
  rc = ui_coerce_string_to_bool(NULL, &bval);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_bool("1", NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  rc = ui_coerce_string_to_int(NULL, &ival);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_int("42", NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  rc = ui_coerce_string_to_float(NULL, &fval);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_float("42.5", NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  rc = ui_coerce_string_to_color(NULL, &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#fff", NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  rc = ui_safe_string_copy(NULL, sizeof(buffer), "test");
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_safe_string_copy(buffer, 0, "test");
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_safe_string_copy(buffer, sizeof(buffer), NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  rc = ui_safe_string_format(NULL, sizeof(buffer), "%d", 42);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_safe_string_format(buffer, 0, "%d", 42);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_safe_string_format(buffer, sizeof(buffer), NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  /* string_to_bool */
  rc = ui_coerce_string_to_bool("1", &bval);
  if (rc != UI_ERROR_NONE || bval != 1) {
    return 1;
  }
  rc = ui_coerce_string_to_bool("true", &bval);
  if (rc != UI_ERROR_NONE || bval != 1) {
    return 1;
  }
  rc = ui_coerce_string_to_bool("TRUE", &bval);
  if (rc != UI_ERROR_NONE || bval != 1) {
    return 1;
  }
  rc = ui_coerce_string_to_bool("True", &bval);
  if (rc != UI_ERROR_NONE || bval != 1) {
    return 1;
  }
  rc = ui_coerce_string_to_bool("0", &bval);
  if (rc != UI_ERROR_NONE || bval != 0) {
    return 1;
  }
  rc = ui_coerce_string_to_bool("false", &bval);
  if (rc != UI_ERROR_NONE || bval != 0) {
    return 1;
  }
  rc = ui_coerce_string_to_bool("FALSE", &bval);
  if (rc != UI_ERROR_NONE || bval != 0) {
    return 1;
  }
  rc = ui_coerce_string_to_bool("False", &bval);
  if (rc != UI_ERROR_NONE || bval != 0) {
    return 1;
  }
  rc = ui_coerce_string_to_bool("invalid", &bval);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  /* string_to_int */
  rc = ui_coerce_string_to_int("42", &ival);
  if (rc != UI_ERROR_NONE || ival != 42) {
    return 1;
  }
  rc = ui_coerce_string_to_int("-42", &ival);
  if (rc != UI_ERROR_NONE || ival != -42) {
    return 1;
  }
  rc = ui_coerce_string_to_int("", &ival);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_int("42a", &ival);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_int("abc", &ival);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  /* string_to_float */
  rc = ui_coerce_string_to_float("42.5", &fval);
  if (rc != UI_ERROR_NONE || fval != 42.5f) {
    return 1;
  }
  rc = ui_coerce_string_to_float("-42.5", &fval);
  if (rc != UI_ERROR_NONE || fval != -42.5f) {
    return 1;
  }
  rc = ui_coerce_string_to_float("", &fval);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_float("42.5a", &fval);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_float("abc", &fval);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  /* Named colors */
  rc = ui_coerce_string_to_color("transparent", &color);
  if (rc != UI_ERROR_NONE || color != 0x00000000) {
    return 1;
  }
  rc = ui_coerce_string_to_color("black", &color);
  if (rc != UI_ERROR_NONE || color != 0xFF000000) {
    return 1;
  }
  rc = ui_coerce_string_to_color("white", &color);
  if (rc != UI_ERROR_NONE || color != 0xFFFFFFFF) {
    return 1;
  }
  rc = ui_coerce_string_to_color("red", &color);
  if (rc != UI_ERROR_NONE || color != 0xFFFF0000) {
    return 1;
  }
  rc = ui_coerce_string_to_color("green", &color);
  if (rc != UI_ERROR_NONE || color != 0xFF008000) {
    return 1;
  }
  rc = ui_coerce_string_to_color("blue", &color);
  if (rc != UI_ERROR_NONE || color != 0xFF0000FF) {
    return 1;
  }
  rc = ui_coerce_string_to_color("yellow", &color);
  if (rc != UI_ERROR_NONE || color != 0xFFFFFF00) {
    return 1;
  }
  rc = ui_coerce_string_to_color("cyan", &color);
  if (rc != UI_ERROR_NONE || color != 0xFF00FFFF) {
    return 1;
  }
  rc = ui_coerce_string_to_color("magenta", &color);
  if (rc != UI_ERROR_NONE || color != 0xFFFF00FF) {
    return 1;
  }
  rc = ui_coerce_string_to_color("gray", &color);
  if (rc != UI_ERROR_NONE || color != 0xFF808080) {
    return 1;
  }
  rc = ui_coerce_string_to_color("grey", &color);
  if (rc != UI_ERROR_NONE || color != 0xFF808080) {
    return 1;
  }

  /* Hex formats: len 3 */
  rc = ui_coerce_string_to_color("#123", &color);
  if (rc != UI_ERROR_NONE || color != UI_COLOR_ARGB(255, 17, 34, 51)) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#aBc", &color);
  if (rc != UI_ERROR_NONE || color != UI_COLOR_ARGB(255, 170, 187, 204)) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#/23", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#@23", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#z23", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#1z3", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#12z", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  /* Hex formats: len 4 */
  rc = ui_coerce_string_to_color("#1234", &color);
  if (rc != UI_ERROR_NONE || color != UI_COLOR_ARGB(68, 17, 34, 51)) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#z234", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#1z34", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#12z4", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#123z", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  /* Hex formats: len 6 */
  rc = ui_coerce_string_to_color("#123456", &color);
  if (rc != UI_ERROR_NONE || color != UI_COLOR_ARGB(255, 0x12, 0x34, 0x56)) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#z23456", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#1z3456", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#12z456", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#123z56", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#1234z6", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#12345z", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  /* Hex formats: len 8 */
  rc = ui_coerce_string_to_color("#12345678", &color);
  if (rc != UI_ERROR_NONE || color != UI_COLOR_ARGB(0x78, 0x12, 0x34, 0x56)) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#z2345678", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#1z345678", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#12z45678", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#123z5678", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#1234z678", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#12345z78", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#123456z8", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#1234567z", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  /* Hex invalid lengths and non-hex prefix */
  rc = ui_coerce_string_to_color("#", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#1", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#12", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#12345", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#1234567", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("#123456789", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = ui_coerce_string_to_color("not_a_color", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  /* safe_string_copy */
  rc = ui_safe_string_copy(buffer, sizeof(buffer), "hello");
  if (rc != UI_ERROR_NONE || strcmp(buffer, "hello") != 0) {
    return 1;
  }

  rc = ui_safe_string_copy(buffer, 6, "1234567");
  if (rc != UI_ERROR_OUT_OF_BOUNDS || strcmp(buffer, "12345") != 0) {
    return 1;
  }

  g_mock_strcpy_fail = 1;
  rc = ui_safe_string_copy(buffer, sizeof(buffer), "hello");
  g_mock_strcpy_fail = 0;
  if (rc != UI_ERROR_UNKNOWN) {
    return 1;
  }

  /* safe_string_format */
  rc =
      ui_safe_string_format(buffer, sizeof(buffer), "Hello %s %d", "World", 42);
  if (rc != UI_ERROR_NONE || strcmp(buffer, "Hello World 42") != 0) {
    return 1;
  }

  rc = ui_safe_string_format(buffer, 6, "12345678");
  if (rc != UI_ERROR_OUT_OF_BOUNDS) {
    return 1;
  }

  return 0;
}

int main(void) {
  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }

  printf("All ui_coercion_utils tests passed.\n");
  return 0;
}
