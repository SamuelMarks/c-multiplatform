/* clang-format off */
#include "ui_coercion_utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "c89stringutils_string_extras.h"
/* clang-format on */

enum ui_error ui_coerce_string_to_bool(const char *str, int *out_val) {
  if (str == NULL || out_val == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (strcmp(str, "1") == 0 || strcmp(str, "true") == 0 ||
      strcmp(str, "TRUE") == 0 || strcmp(str, "True") == 0) {
    *out_val = 1;
    return UI_ERROR_NONE;
  }

  if (strcmp(str, "0") == 0 || strcmp(str, "false") == 0 ||
      strcmp(str, "FALSE") == 0 || strcmp(str, "False") == 0) {
    *out_val = 0;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_INVALID_ARGUMENT;
}

enum ui_error ui_coerce_string_to_int(const char *str, int *out_val) {
  char *endptr;
  long val;

  if (str == NULL || out_val == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  val = strtol(str, &endptr, 10);
  if (endptr == str || *endptr != '\0') {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_val = (int)val;
  return UI_ERROR_NONE;
}

enum ui_error ui_coerce_string_to_float(const char *str, float *out_val) {
  char *endptr;
  double val;

  if (str == NULL || out_val == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  val = strtod(str, &endptr);
  if (endptr == str || *endptr != '\0') {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_val = (float)val;
  return UI_ERROR_NONE;
}

enum ui_error ui_safe_string_copy(char *dest, size_t dest_size,
                                  const char *src) {
  if (dest == NULL || src == NULL || dest_size == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  if (strncpy_s(dest, dest_size, src, _TRUNCATE) == STRUNCATE) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }
#else
  if (strlen(src) >= dest_size) {
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
    return UI_ERROR_OUT_OF_BOUNDS;
  }
  strcpy(dest, src);
#endif

  return UI_ERROR_NONE;
}

enum ui_error ui_safe_string_format(char *dest, size_t dest_size,
                                    const char *format, ...) {
  va_list args;
  int rc;

  if (dest == NULL || format == NULL || dest_size == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  va_start(args, format);

#if defined(_MSC_VER)
  rc = _vsnprintf_s(dest, dest_size, _TRUNCATE, format, args);
#else
  rc = c89stringutils_vsnprintf(dest, dest_size, format, args);
#endif

  va_end(args);

  if ((size_t)rc >= dest_size) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  return UI_ERROR_NONE;
}
