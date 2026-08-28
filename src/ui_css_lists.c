/**
 * @file ui_css_lists.c
 * @brief ui_css_lists.c implementation.
 */
/* clang-format off */
#include "ui_css_lists.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "ui_internal_mem.h"
#include "strtok_posix.h"
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
static void skip_whitespace(const char **p_str) {
  while (isspace((unsigned char)**p_str)) {
    (*p_str)++;
  }
}

/* \brief ui_error
 */
ui_error_t
ui_css_parse_list_style_type(const char *str,
                             struct ui_css_list_style_type_ext *out_type) {
  if (!str || !out_type)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);
  out_type->string_value[0] = '\0';

  if (strcmp(str, "disc") == 0)
    out_type->type = UI_CSS_LIST_STYLE_TYPE_DISC;
  else if (strcmp(str, "circle") == 0)
    out_type->type = UI_CSS_LIST_STYLE_TYPE_CIRCLE;
  else if (strcmp(str, "square") == 0)
    out_type->type = UI_CSS_LIST_STYLE_TYPE_SQUARE;
  else if (strcmp(str, "decimal") == 0)
    out_type->type = UI_CSS_LIST_STYLE_TYPE_DECIMAL;
  else if (strcmp(str, "decimal-leading-zero") == 0)
    out_type->type = UI_CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO;
  else if (strcmp(str, "lower-roman") == 0)
    out_type->type = UI_CSS_LIST_STYLE_TYPE_LOWER_ROMAN;
  else if (strcmp(str, "upper-roman") == 0)
    out_type->type = UI_CSS_LIST_STYLE_TYPE_UPPER_ROMAN;
  else if (strcmp(str, "lower-greek") == 0)
    out_type->type = UI_CSS_LIST_STYLE_TYPE_LOWER_GREEK;
  else if (strcmp(str, "lower-alpha") == 0)
    out_type->type = UI_CSS_LIST_STYLE_TYPE_LOWER_ALPHA;
  else if (strcmp(str, "lower-latin") == 0)
    out_type->type = UI_CSS_LIST_STYLE_TYPE_LOWER_LATIN;
  else if (strcmp(str, "upper-alpha") == 0)
    out_type->type = UI_CSS_LIST_STYLE_TYPE_UPPER_ALPHA;
  else if (strcmp(str, "upper-latin") == 0)
    out_type->type = UI_CSS_LIST_STYLE_TYPE_UPPER_LATIN;
  else if (strcmp(str, "armenian") == 0)
    out_type->type = UI_CSS_LIST_STYLE_TYPE_ARMENIAN;
  else if (strcmp(str, "georgian") == 0)
    out_type->type = UI_CSS_LIST_STYLE_TYPE_GEORGIAN;
  else if (strcmp(str, "hebrew") == 0)
    out_type->type = UI_CSS_LIST_STYLE_TYPE_HEBREW;
  else if (strcmp(str, "none") == 0)
    out_type->type = UI_CSS_LIST_STYLE_TYPE_NONE;
  else if (str[0] == '"' || str[0] == '\'') {
    char quote = str[0];
    const char *end = strchr(str + 1, quote);
    if (end) {
      size_t len = (size_t)(end - str) - 1;
      if (len >= sizeof(out_type->string_value)) {
        len = sizeof(out_type->string_value) - 1;
      }
      memcpy(out_type->string_value, str + 1, len);
      out_type->string_value[len] = '\0';
      out_type->type = UI_CSS_LIST_STYLE_TYPE_STRING;
      return UI_ERROR_NONE;
    }
    return UI_ERROR_PARSE_FAILED;
  } else {
    return UI_ERROR_PARSE_FAILED;
  }

  return UI_ERROR_NONE;
}

/* \brief ui_css_parse_list_style_position
 */
ui_error_t ui_css_parse_list_style_position(
    const char *str, enum ui_css_list_style_position *out_position) {
  if (!str || !out_position)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);

  if (strcmp(str, "outside") == 0) {
    *out_position = UI_CSS_LIST_STYLE_POSITION_OUTSIDE;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "inside") == 0) {
    *out_position = UI_CSS_LIST_STYLE_POSITION_INSIDE;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_PARSE_FAILED;
}

/**
 * @brief ui_css_parse_list_style_image.
 * @param str Parameter str.
 * @param out_image Parameter out_image.
 * @param out_is_none Parameter out_is_none.
 * @return Return value.
 */
ui_error_t ui_css_parse_list_style_image(const char *str,
                                         struct ui_css_image *out_image,
                                         int *out_is_none) {
  if (!str || !out_image || !out_is_none)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);

  if (strcmp(str, "none") == 0) {
    *out_is_none = 1;
    return UI_ERROR_NONE;
  }

  *out_is_none = 0;
  return ui_css_parse_image(str, out_image);
}

/**
 * @brief ui_css_parse_list_style.
 * @param str Parameter str.
 * @param out_style Parameter out_style.
 * @return Return value.
 */
ui_error_t ui_css_parse_list_style(const char *str,
                                   struct ui_css_list_style *out_style) {
  char token_buf[512];
  char *token;
  char *next_token = NULL;
  int parsed_type = 0;
  int parsed_position = 0;
  int parsed_image = 0;

  if (!str || !out_style)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);

  if (strcmp(str, "none") == 0) {
    out_style->type.type = UI_CSS_LIST_STYLE_TYPE_NONE;
    out_style->position = UI_CSS_LIST_STYLE_POSITION_OUTSIDE;
    out_style->has_image = 0;
    return UI_ERROR_NONE;
  }

  UI_STRNCPY(token_buf, sizeof(token_buf), str, sizeof(token_buf) - 1);
  token_buf[sizeof(token_buf) - 1] = '\0';

  out_style->type.type = UI_CSS_LIST_STYLE_TYPE_DISC;       /* initial */
  out_style->position = UI_CSS_LIST_STYLE_POSITION_OUTSIDE; /* initial */
  out_style->has_image = 0;

  token = UI_STRTOK(token_buf, " \t\r\n", &next_token);
  while (token) {
    /* try parsing as position first since it has fewer keywords */
    if (!parsed_position) {
      ui_error_t err =
          ui_css_parse_list_style_position(token, &out_style->position);
      if (err != UI_ERROR_NONE) {
        if (0)
          return err;
      } else {
        parsed_position = 1;
        token = UI_STRTOK(NULL, " \t\r\n", &next_token);
        continue;
      }
    }
    /* try parsing as type */
    if (!parsed_type) {
      ui_error_t err = ui_css_parse_list_style_type(token, &out_style->type);
      if (err != UI_ERROR_NONE) {
        if (0)
          return err;
      } else {
        parsed_type = 1;
        token = UI_STRTOK(NULL, " \t\r\n", &next_token);
        continue;
      }
    }
    /* try parsing as image */
    if (!parsed_image) {
      int is_none = 0;
      ui_error_t err =
          ui_css_parse_list_style_image(token, &out_style->image, &is_none);
      if (err != UI_ERROR_NONE) {
        return err;
      } else {
        parsed_image = 1;
        out_style->has_image = !is_none;
        token = UI_STRTOK(NULL, " \t\r\n", &next_token);
        continue;
      }
    } else {
      return UI_ERROR_PARSE_FAILED; /* Unrecognized token or duplicate */
    }
  }

  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
ui_error_t
ui_css_parse_counter_action(const char *str,
                            struct ui_css_counter_action **out_actions) {
  char token_buf[512];
  char *token;
  char *next_token = NULL;
  struct ui_css_counter_action *head = NULL;
  struct ui_css_counter_action *tail = NULL;
  size_t len;

  if (!str || !out_actions)
    return UI_ERROR_INVALID_ARGUMENT;

  *out_actions = NULL;
  skip_whitespace(&str);

  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  len = strlen(str);
  if (len >= sizeof(token_buf))
    len = sizeof(token_buf) - 1;
  memcpy(token_buf, str, len);
  token_buf[len] = '\0';

  token = UI_STRTOK(token_buf, " \t\r\n", &next_token);
  while (token) {
    char *ident = token;
    int value = 1; /* Default increment/reset */
    struct ui_css_counter_action *node;

    /* Look ahead for an optional integer */
    char *next = UI_STRTOK(NULL, " \t\r\n", &next_token);
    if (next) {
      char *endptr;
      long val = strtol(next, &endptr, 10);
      if (endptr != next && *endptr == '\0') {
        value = (int)val;
      } else {
        /* Not an integer, push it back for the next iteration (we can't easily
         * push back in strtok, so we simulate) */
        /* Actually, to handle pushing back, we just adjust the next_token
           pointer roughly. Better: if it's not an integer, it's the next
           identifier. */
        /* Let's rewind next_token by copying the logic. */
        /* For C89 simplicity, we will just manually parse the string without
           strtok if this fails, or just use a custom simple scanner. */
      }
    }

    node = (struct ui_css_counter_action *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_css_counter_action));
    if (!node) {
      ui_css_counter_action_destroy(head);
      return UI_ERROR_OUT_OF_MEMORY;
    }

    UI_STRNCPY(node->name, sizeof(node->name), ident, sizeof(node->name) - 1);
    node->name[sizeof(node->name) - 1] = '\0';
    node->value = value;
    node->next = NULL;

    if (!head)
      head = node;
    else
      tail->next = node;
    tail = node;

    if (next) {
      char *endptr;
      strtol(next, &endptr, 10);
      if (endptr != next && *endptr == '\0') {
        /* it was a number, already consumed */
        token = UI_STRTOK(NULL, " \t\r\n", &next_token);
      } else {
        /* it was another identifier */
        token = next;
      }
    } else {
      token = NULL;
    }
  }

  if (!head)
    return UI_ERROR_PARSE_FAILED; /* Empty string but not 'none' */

  *out_actions = head;
  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
ui_error_t
ui_css_counter_action_destroy(struct ui_css_counter_action *actions) {
  while (actions) {
    struct ui_css_counter_action *next = actions->next;
    C_MULTIPLATFORM_FREE(actions);
    actions = next;
  }
  return UI_ERROR_NONE;
}
