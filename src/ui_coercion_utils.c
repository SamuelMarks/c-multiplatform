/**
 * @file ui_coercion_utils.c
 * @brief Implementation of ui_coercion_utils.c.
 */
/* clang-format off */
#include "ui_coercion_utils.h"
#include "ui_internal_mem.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "c89stringutils_string_extras.h"
/* clang-format on */

/**
 * @brief ui_coerce_string_to_bool.
 * @param str Parameter str.
 * @param out_val Parameter out_val.
 * @return Return value.
 */
ui_error_t ui_coerce_string_to_bool(const char *str, int *out_val) {
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

/**
 * @brief ui_coerce_string_to_int.
 * @param str Parameter str.
 * @param out_val Parameter out_val.
 * @return Return value.
 */
ui_error_t ui_coerce_string_to_int(const char *str, int *out_val) {
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

/**
 * @brief ui_coerce_string_to_float.
 * @param str Parameter str.
 * @param out_val Parameter out_val.
 * @return Return value.
 */
ui_error_t ui_coerce_string_to_float(const char *str, float *out_val) {
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

/**
 * @brief Converts a hexadecimal ASCII character to its integer value.
 * @param c The hexadecimal character.
 * @param out_val Pointer to receive integer value between 0 and 15.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT if not hex.
 */
static ui_error_t hex_char_to_int(char c, int *out_val) {
  if (c >= '0' && c <= '9') {
    *out_val = c - '0';
    return UI_ERROR_NONE;
  }
  if (c >= 'a' && c <= 'f') {
    *out_val = c - 'a' + 10;
    return UI_ERROR_NONE;
  }
  if (c >= 'A' && c <= 'F') {
    *out_val = c - 'A' + 10;
    return UI_ERROR_NONE;
  }
  return UI_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief Safely parses a string into an ARGB color value.
 * @param str The string to parse.
 * @param out_color Pointer to receive the parsed ARGB color.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on failure.
 */
ui_error_t ui_coerce_string_to_color(const char *str, ui_color_t *out_color) {
  size_t len;
  int r, g, b, a;
  int h0, h1, h2, h3;
  int r0, r1, g0, g1, b0, b1, a0, a1;
  ui_error_t rc;

  if (str == NULL || out_color == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Named colors */
  if (strcmp(str, "transparent") == 0) {
    *out_color = 0x00000000;
    return UI_ERROR_NONE;
  }
  if (strcmp(str, "black") == 0) {
    *out_color = 0xFF000000;
    return UI_ERROR_NONE;
  }
  if (strcmp(str, "white") == 0) {
    *out_color = 0xFFFFFFFF;
    return UI_ERROR_NONE;
  }
  if (strcmp(str, "red") == 0) {
    *out_color = 0xFFFF0000;
    return UI_ERROR_NONE;
  }
  if (strcmp(str, "green") == 0) {
    *out_color = 0xFF008000;
    return UI_ERROR_NONE;
  }
  if (strcmp(str, "blue") == 0) {
    *out_color = 0xFF0000FF;
    return UI_ERROR_NONE;
  }
  if (strcmp(str, "yellow") == 0) {
    *out_color = 0xFFFFFF00;
    return UI_ERROR_NONE;
  }
  if (strcmp(str, "cyan") == 0) {
    *out_color = 0xFF00FFFF;
    return UI_ERROR_NONE;
  }
  if (strcmp(str, "magenta") == 0) {
    *out_color = 0xFFFF00FF;
    return UI_ERROR_NONE;
  }
  if (strcmp(str, "gray") == 0 || strcmp(str, "grey") == 0) {
    *out_color = 0xFF808080;
    return UI_ERROR_NONE;
  }

  /* Hex formats */
  if (str[0] == '#') {
    len = strlen(str + 1);
    if (len == 3) {
      rc = hex_char_to_int(str[1], &h0);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = hex_char_to_int(str[2], &h1);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = hex_char_to_int(str[3], &h2);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      r = h0 * 17;
      g = h1 * 17;
      b = h2 * 17;
      a = 255;
      *out_color = UI_COLOR_ARGB(a, r, g, b);
      return UI_ERROR_NONE;
    }
    if (len == 4) {
      rc = hex_char_to_int(str[1], &h0);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = hex_char_to_int(str[2], &h1);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = hex_char_to_int(str[3], &h2);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = hex_char_to_int(str[4], &h3);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      r = h0 * 17;
      g = h1 * 17;
      b = h2 * 17;
      a = h3 * 17;
      *out_color = UI_COLOR_ARGB(a, r, g, b);
      return UI_ERROR_NONE;
    }
    if (len == 6) {
      rc = hex_char_to_int(str[1], &r0);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = hex_char_to_int(str[2], &r1);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = hex_char_to_int(str[3], &g0);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = hex_char_to_int(str[4], &g1);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = hex_char_to_int(str[5], &b0);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = hex_char_to_int(str[6], &b1);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      r = (r0 << 4) | r1;
      g = (g0 << 4) | g1;
      b = (b0 << 4) | b1;
      a = 255;
      *out_color = UI_COLOR_ARGB(a, r, g, b);
      return UI_ERROR_NONE;
    }
    if (len == 8) {
      rc = hex_char_to_int(str[1], &r0);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = hex_char_to_int(str[2], &r1);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = hex_char_to_int(str[3], &g0);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = hex_char_to_int(str[4], &g1);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = hex_char_to_int(str[5], &b0);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = hex_char_to_int(str[6], &b1);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = hex_char_to_int(str[7], &a0);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = hex_char_to_int(str[8], &a1);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      r = (r0 << 4) | r1;
      g = (g0 << 4) | g1;
      b = (b0 << 4) | b1;
      a = (a0 << 4) | a1;
      *out_color = UI_COLOR_ARGB(a, r, g, b);
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief ui_safe_string_copy.
 * @param dest Parameter dest.
 * @param dest_size Parameter dest_size.
 * @param src Parameter src.
 * @return Return value.
 */
ui_error_t ui_safe_string_copy(char *dest, size_t dest_size, const char *src) {
  if (dest == NULL || src == NULL || dest_size == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (strlen(src) >= dest_size) {
#if defined(_MSC_VER)
    strncpy_s(dest, dest_size, src, _TRUNCATE);
#else
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
#endif
    return UI_ERROR_OUT_OF_BOUNDS;
  }
  if (UI_STRCPY(dest, dest_size, src) != 0) {
    return UI_ERROR_UNKNOWN;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_safe_string_format.
 * @param dest Parameter dest.
 * @param dest_size Parameter dest_size.
 * @param format Parameter format.
 * @param ... Parameter ....
 * @return Return value.
 */
ui_error_t ui_safe_string_format(char *dest, size_t dest_size,
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
