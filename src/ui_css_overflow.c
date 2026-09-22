/**
 * @file ui_css_overflow.c
 * @brief ui_css_overflow.c implementation.
 */
/* clang-format off */
#include "ui_css_overflow.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "ui_internal_mem.h"

#ifdef UI_TEST_MOCK_ALLOC
int g_overflow_mock_fail = 0;
#endif
/* clang-format on */

/**
 * @brief skip_whitespace.
 * @param p_str Parameter p_str.
 * @return Return value.
 */
static ui_error_t skip_whitespace(const char **p_str) {
#ifdef UI_TEST_MOCK_ALLOC
  if (g_overflow_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
#endif
  if (!p_str || !*p_str) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  while (isspace((unsigned char)**p_str)) {
    (*p_str)++;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_css_parse_overflow.
 * @param str Parameter str.
 * @param out_overflow Parameter out_overflow.
 * @return Return value.
 */
ui_error_t ui_css_parse_overflow(const char *str,
                                 enum ui_css_overflow *out_overflow) {
  ui_error_t rc;
  if (!str || !out_overflow)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = skip_whitespace(&str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (strcmp(str, "visible") == 0) {
    *out_overflow = UI_CSS_OVERFLOW_VISIBLE;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "hidden") == 0) {
    *out_overflow = UI_CSS_OVERFLOW_HIDDEN;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "clip") == 0) {
    *out_overflow = UI_CSS_OVERFLOW_CLIP;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "scroll") == 0) {
    *out_overflow = UI_CSS_OVERFLOW_SCROLL;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "auto") == 0) {
    *out_overflow = UI_CSS_OVERFLOW_AUTO;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_PARSE_FAILED;
}

/* \brief ui_error
 */
ui_error_t
ui_css_parse_text_overflow(const char *str,
                           struct ui_css_text_overflow *out_overflow) {
  ui_error_t rc;
  if (!str || !out_overflow)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = skip_whitespace(&str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  out_overflow->string[0] = '\0';

  if (strcmp(str, "clip") == 0) {
    out_overflow->type = UI_CSS_TEXT_OVERFLOW_CLIP;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "ellipsis") == 0) {
    out_overflow->type = UI_CSS_TEXT_OVERFLOW_ELLIPSIS;
    return UI_ERROR_NONE;
  } else if (str[0] == '"' || str[0] == '\'') {
    char quote = str[0];
    const char *end = strchr(str + 1, quote);
    if (end) {
      size_t len = (size_t)(end - str) - 1;
      if (len >= sizeof(out_overflow->string)) {
        len = sizeof(out_overflow->string) - 1;
      }
      memcpy(out_overflow->string, str + 1, len);
      out_overflow->string[len] = '\0';
      out_overflow->type = UI_CSS_TEXT_OVERFLOW_STRING;
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_PARSE_FAILED;
}

/* \brief ui_error
 */
ui_error_t
ui_css_parse_block_ellipsis(const char *str,
                            struct ui_css_block_ellipsis *out_ellipsis) {
  ui_error_t rc;
  if (!str || !out_ellipsis)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = skip_whitespace(&str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  out_ellipsis->string[0] = '\0';

  if (strcmp(str, "none") == 0) {
    out_ellipsis->type = UI_CSS_TEXT_OVERFLOW_CLIP;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "auto") == 0) {
    out_ellipsis->type = UI_CSS_TEXT_OVERFLOW_ELLIPSIS;
    return UI_ERROR_NONE;
  } else if (str[0] == '"' || str[0] == '\'') {
    char quote = str[0];
    const char *end = strchr(str + 1, quote);
    if (end) {
      size_t len = (size_t)(end - str) - 1;
      if (len >= sizeof(out_ellipsis->string)) {
        len = sizeof(out_ellipsis->string) - 1;
      }
      memcpy(out_ellipsis->string, str + 1, len);
      out_ellipsis->string[len] = '\0';
      out_ellipsis->type = UI_CSS_TEXT_OVERFLOW_STRING;
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_PARSE_FAILED;
}

/**
 * @brief ui_css_parse_line_clamp.
 * @param str Parameter str.
 * @param out_clamp Parameter out_clamp.
 * @return Return value.
 */
ui_error_t ui_css_parse_line_clamp(const char *str,
                                   struct ui_css_line_clamp *out_clamp) {
  ui_error_t rc;
  if (!str || !out_clamp)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = skip_whitespace(&str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (strcmp(str, "none") == 0) {
    out_clamp->is_none = 1;
    out_clamp->lines = 0;
    return UI_ERROR_NONE;
  }

  /* Try parsing an integer */
  {
    char *endptr;
    long val = strtol(str, &endptr, 10);
    if (endptr != str && val > 0) {
      out_clamp->is_none = 0;
      out_clamp->lines = (int)val;
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_PARSE_FAILED;
}

/**
 * @brief ui_css_parse_max_lines.
 * @param str Parameter str.
 * @param out_max_lines Parameter out_max_lines.
 * @return Return value.
 */
ui_error_t ui_css_parse_max_lines(const char *str,
                                  struct ui_css_max_lines *out_max_lines) {
  ui_error_t rc;
  if (!str || !out_max_lines)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = skip_whitespace(&str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (strcmp(str, "none") == 0) {
    out_max_lines->is_none = 1;
    out_max_lines->lines = 0;
    return UI_ERROR_NONE;
  }

  /* Try parsing an integer */
  {
    char *endptr;
    long val = strtol(str, &endptr, 10);
    if (endptr != str && val >= 0) {
      out_max_lines->is_none = 0;
      out_max_lines->lines = (int)val;
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_PARSE_FAILED;
}

/* \brief ui_error
 */
ui_error_t ui_css_parse_overflow_clip_margin(const char *str,
                                             struct ui_css_value *out_margin) {
  ui_error_t rc;
  if (!str || !out_margin)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = skip_whitespace(&str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  return ui_css_parse_value(str, out_margin);
}

#ifdef UI_TEST_MOCK_ALLOC
/**
 * @brief run_overflow_coverage.
 * @return Return value.
 */
ui_error_t run_overflow_coverage(void);
ui_error_t run_overflow_coverage(void) {
  const char *null_str = NULL;

  skip_whitespace(NULL);
  skip_whitespace(&null_str);

  return UI_ERROR_NONE;
}
#endif
