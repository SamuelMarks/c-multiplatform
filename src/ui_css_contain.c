/**
 * @file ui_css_contain.c
 * @brief ui_css_contain.c implementation.
 */
/* clang-format off */
#include "ui_css_contain.h"
#include <string.h>
#include <ctype.h>
#include "ui_internal_mem.h"
#include "strtok_posix.h"

#ifdef UI_TEST_MOCK_ALLOC
int g_contain_mock_fail = 0;
int g_contain_mock_fail_count = 0;
#endif
/* clang-format on */

#if defined(_MSC_VER)
/** @cond */
#define UI_STRTOK(str, delim, ctx) strtok_s((str), (delim), (ctx))
/** @endcond */
#else
/** @cond */
#define UI_STRTOK(str, delim, ctx) strtok_r((str), (delim), (ctx))
/** @endcond */
#endif

/**
 * @brief skip_whitespace.
 * @param p_str Parameter p_str.
 * @return Return value.
 */
static ui_error_t skip_whitespace(const char **p_str) {
#ifdef UI_TEST_MOCK_ALLOC
  if (g_contain_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_contain_mock_fail > 1) {
    g_contain_mock_fail_count++;
    if (g_contain_mock_fail_count == g_contain_mock_fail) {
      return UI_ERROR_UNKNOWN;
    }
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
 * @brief ui_css_parse_contain.
 * @param str Parameter str.
 * @param out_flags Parameter out_flags.
 * @return Return value.
 */
ui_error_t ui_css_parse_contain(const char *str, unsigned int *out_flags) {
  ui_error_t rc;
  char token_buf[256];
  char *token;
  char *next_token = NULL;
  unsigned int flags = UI_CSS_CONTAIN_NONE;
  int is_none = 0;
  int is_strict = 0;
  int is_content = 0;
  int other_flags = 0;

  if (!str || !out_flags)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = skip_whitespace(&str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  UI_STRNCPY(token_buf, sizeof(token_buf), str, sizeof(token_buf) - 1);
  token_buf[sizeof(token_buf) - 1] = '\0';

  token = UI_STRTOK(token_buf, " \t\r\n", &next_token);
  while (token) {
    if (strcmp(token, "none") == 0) {
      is_none = 1;
    } else if (strcmp(token, "strict") == 0) {
      is_strict = 1;
    } else if (strcmp(token, "content") == 0) {
      is_content = 1;
    } else if (strcmp(token, "size") == 0) {
      flags |= UI_CSS_CONTAIN_SIZE;
      other_flags = 1;
    } else if (strcmp(token, "layout") == 0) {
      flags |= UI_CSS_CONTAIN_LAYOUT;
      other_flags = 1;
    } else if (strcmp(token, "style") == 0) {
      flags |= UI_CSS_CONTAIN_STYLE;
      other_flags = 1;
    } else if (strcmp(token, "paint") == 0) {
      flags |= UI_CSS_CONTAIN_PAINT;
      other_flags = 1;
    } else if (strcmp(token, "inline-size") == 0) {
      flags |= UI_CSS_CONTAIN_INLINE_SIZE;
      other_flags = 1;
    } else {
      return UI_ERROR_PARSE_FAILED;
    }
    token = UI_STRTOK(NULL, " \t\r\n", &next_token);
  }

  if (is_none) {
    if (is_strict || is_content || other_flags) {
      return UI_ERROR_PARSE_FAILED; /* none must be alone */
    }
    *out_flags = UI_CSS_CONTAIN_NONE;
    return UI_ERROR_NONE;
  }

  if (is_strict) {
    if (is_content || other_flags) {
      return UI_ERROR_PARSE_FAILED; /* strict must be alone */
    }
    /* strict = size layout paint style */
    *out_flags = UI_CSS_CONTAIN_STRICT | UI_CSS_CONTAIN_SIZE |
                 UI_CSS_CONTAIN_LAYOUT | UI_CSS_CONTAIN_PAINT |
                 UI_CSS_CONTAIN_STYLE;
    return UI_ERROR_NONE;
  }

  if (is_content) {
    if (other_flags) {
      return UI_ERROR_PARSE_FAILED; /* content must be alone */
    }
    /* content = layout paint style */
    *out_flags = UI_CSS_CONTAIN_CONTENT | UI_CSS_CONTAIN_LAYOUT |
                 UI_CSS_CONTAIN_PAINT | UI_CSS_CONTAIN_STYLE;
    return UI_ERROR_NONE;
  }

  /* Note: size and inline-size are mutually exclusive, but CSS containment
     allows them to both be specified, where 'size' overrides 'inline-size' or
     makes 'inline-size' redundant. We just set both bits if parsed. */

  *out_flags = flags;
  return UI_ERROR_NONE;
}

/* \brief ui_css_parse_content_visibility
 */
ui_error_t ui_css_parse_content_visibility(
    const char *str, enum ui_css_content_visibility *out_visibility) {
  ui_error_t rc;

  if (!str || !out_visibility)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = skip_whitespace(&str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (strcmp(str, "visible") == 0) {
    *out_visibility = UI_CSS_CONTENT_VISIBILITY_VISIBLE;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "auto") == 0) {
    *out_visibility = UI_CSS_CONTENT_VISIBILITY_AUTO;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "hidden") == 0) {
    *out_visibility = UI_CSS_CONTENT_VISIBILITY_HIDDEN;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_PARSE_FAILED;
}

/**
 * @brief parse_intrinsic_dim.
 * @param p_str Parameter p_str.
 * @param dim Parameter dim.
 * @return Return value.
 */
static ui_error_t
parse_intrinsic_dim(const char **p_str,
                    struct ui_css_contain_intrinsic_dim *dim) {
  ui_error_t rc;
  char token_buf[64];
  const char *str = *p_str;
  size_t len;
  const char *space;

  dim->has_auto = 0;
  dim->is_none = 0;
  dim->length.unit = UI_CSS_UNIT_NONE;

  rc = skip_whitespace(&str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  if (!*str) {
    return UI_ERROR_PARSE_FAILED; /* Unexpected end of string */
  }

  /* Check for 'auto' */
  if (strncmp(str, "auto", 4) == 0 &&
      (isspace((unsigned char)str[4]) || str[4] == '\0')) {
    dim->has_auto = 1;
    str += 4;
    rc = skip_whitespace(&str);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    if (!*str) {
      /* 'auto' alone is valid, means auto none */
      dim->is_none = 1;
      *p_str = str;
      return UI_ERROR_NONE;
    }
  }

  space = str;
  while (*space && !isspace((unsigned char)*space)) {
    space++;
  }

  len = (size_t)(space - str);
  if (len >= sizeof(token_buf))
    len = sizeof(token_buf) - 1;
  memcpy(token_buf, str, len);
  token_buf[len] = '\0';

  if (strcmp(token_buf, "none") == 0) {
    dim->is_none = 1;
    *p_str = space;
    return UI_ERROR_NONE;
  }

  /* Try parsing as a length */
  {
    ui_error_t rc2 = ui_css_parse_value(token_buf, &dim->length);
    if (rc2 != UI_ERROR_NONE) {
      return rc2;
    }
    *p_str = space;
    return UI_ERROR_NONE;
  }
}

/* \brief ui_css_parse_contain_intrinsic_size
 */
ui_error_t ui_css_parse_contain_intrinsic_size(
    const char *str, struct ui_css_contain_intrinsic_size *out_size) {
  ui_error_t rc;

  if (!str || !out_size)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = skip_whitespace(&str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = parse_intrinsic_dim(&str, &out_size->width);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = skip_whitespace(&str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  if (*str == '\0') {
    /* If only one value is given, it applies to both */
    out_size->height = out_size->width;
    return UI_ERROR_NONE;
  }

  /* Parse second value */
  rc = parse_intrinsic_dim(&str, &out_size->height);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = skip_whitespace(&str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  if (*str != '\0') {
    /* Extra garbage at the end */
    return UI_ERROR_PARSE_FAILED;
  }

  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
/**
 * @brief run_contain_coverage.
 * @return Return value.
 */
ui_error_t run_contain_coverage(void);
ui_error_t run_contain_coverage(void) {
  const char *null_str = NULL;

  skip_whitespace(NULL);
  skip_whitespace(&null_str);

  return UI_ERROR_NONE;
}
#endif
