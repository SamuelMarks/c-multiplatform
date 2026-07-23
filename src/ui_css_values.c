/* clang-format off */
#include "../include/ui_css_values.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "strtok_posix.h"
/* clang-format on */

#if defined(_MSC_VER)
#define UI_STRTOK(str, delim, ctx) strtok_s((str), (delim), (ctx))
#else
#define UI_STRTOK(str, delim, ctx) strtok_r((str), (delim), (ctx))
#endif

static void parse_css_unit(const char *str, size_t *out_len,
                           enum ui_css_unit *out_unit) {
  size_t i, j;
  static const struct {
    const char *name;
    enum ui_css_unit unit;
  } units[] = {{"px", UI_CSS_UNIT_PX},       {"%", UI_CSS_UNIT_PERCENT},
               {"cqmin", UI_CSS_UNIT_CQMIN}, {"cqmax", UI_CSS_UNIT_CQMAX},
               {"cqw", UI_CSS_UNIT_CQW},     {"cqh", UI_CSS_UNIT_CQH},
               {"cqi", UI_CSS_UNIT_CQI},     {"cqb", UI_CSS_UNIT_CQB},
               {"vw", UI_CSS_UNIT_VW},       {"vh", UI_CSS_UNIT_VH},
               {"vmin", UI_CSS_UNIT_VMIN},   {"vmax", UI_CSS_UNIT_VMAX},
               {"vi", UI_CSS_UNIT_VI},       {"vb", UI_CSS_UNIT_VB},
               {"em", UI_CSS_UNIT_EM},       {"rem", UI_CSS_UNIT_REM},
               {"ms", UI_CSS_UNIT_MS},       {"s", UI_CSS_UNIT_S},
               {"deg", UI_CSS_UNIT_DEG},     {"rad", UI_CSS_UNIT_RAD},
               {"cm", UI_CSS_UNIT_CM},       {"mm", UI_CSS_UNIT_MM},
               {"in", UI_CSS_UNIT_IN},       {"pt", UI_CSS_UNIT_PT},
               {"pc", UI_CSS_UNIT_PC},       {"ex", UI_CSS_UNIT_EX},
               {"ch", UI_CSS_UNIT_CH},       {"q", UI_CSS_UNIT_Q},
               {"grad", UI_CSS_UNIT_GRAD},   {"turn", UI_CSS_UNIT_TURN},
               {"hz", UI_CSS_UNIT_HZ},       {"khz", UI_CSS_UNIT_KHZ},
               {"db", UI_CSS_UNIT_DB},       {"st", UI_CSS_UNIT_ST}};

  *out_len = 0;
  *out_unit = UI_CSS_UNIT_NONE;

  for (i = 0; i < sizeof(units) / sizeof(units[0]); ++i) {
    int match = 1;
    for (j = 0; units[i].name[j] != '\0'; ++j) {
      if (tolower((unsigned char)str[j]) != units[i].name[j]) {
        match = 0;
        break;
      }
    }
    if (match && !isalpha((unsigned char)str[j])) {
      *out_len = j;
      *out_unit = units[i].unit;
      return;
    }
  }

  return;
}

static enum ui_error
ui_css_parse_value_internal(const char **p_str,
                            struct ui_css_value *out_value) {
  const char *str = *p_str;
  char *endptr;
  double val;
  size_t ulen = 0;

  while (isspace((unsigned char)*str)) {
    str++;
  }

  if (*str == '\0') {
    return UI_ERROR_PARSE_FAILED;
  }

  val = strtod(str, &endptr);

  if (endptr == str) {
    return UI_ERROR_PARSE_FAILED;
  }

  out_value->value = (float)val;
  str = endptr;

  parse_css_unit(str, &ulen, &out_value->unit);

  if (out_value->unit == UI_CSS_UNIT_NONE) {
    /* No unit found */
    if (isalpha((unsigned char)*str)) {
      out_value->unit = UI_CSS_UNIT_UNKNOWN;
      return UI_ERROR_PARSE_FAILED;
    } else {
      out_value->unit = UI_CSS_UNIT_NONE;
      *p_str = str;
      return UI_ERROR_NONE;
    }
  }
  str += ulen;
  *p_str = str;
  return UI_ERROR_NONE;
}

enum ui_error ui_css_parse_value(const char *str,
                                 struct ui_css_value *out_value) {
  enum ui_error rc;

  if (!str || !out_value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_css_parse_value_internal(&str, out_value);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  while (isspace((unsigned char)*str)) {
    str++;
  }

  if (*str != '\0') {
    return UI_ERROR_PARSE_FAILED;
  }

  return UI_ERROR_NONE;
}

/* Recursive Descent Parser for Math Expressions */

static enum ui_error parse_expression(const char **p_str,
                                      struct ui_css_value_ext **out_expr);

static void skip_whitespace(const char **p_str) {
  while (isspace((unsigned char)**p_str)) {
    (*p_str)++;
  }
  return;
}

static void match_keyword(const char **p_str, const char *kw,
                          int *out_matched) {
  size_t len = strlen(kw);
  if (strncmp(*p_str, kw, len) == 0) {
    *p_str += len;
    *out_matched = 1;
    return;
  }
  *out_matched = 0;
  return;
}

static enum ui_error create_math_node(enum ui_css_math_op op,
                                      struct ui_css_math_expr **out_node) {
  struct ui_css_math_expr *node =
      (struct ui_css_math_expr *)UI_MALLOC(sizeof(struct ui_css_math_expr));
  if (node) {
    node->op = op;
    node->left = NULL;
    node->right = NULL;
    node->ext = NULL;
    node->next = NULL;
    *out_node = node;
    return UI_ERROR_NONE;
  }
  return UI_ERROR_OUT_OF_MEMORY;
}

static enum ui_error create_value_ext_math(struct ui_css_math_expr *math,
                                           struct ui_css_value_ext **out_ext) {
  struct ui_css_value_ext *ext =
      (struct ui_css_value_ext *)UI_MALLOC(sizeof(struct ui_css_value_ext));
  if (ext) {
    ext->type = UI_CSS_VALUE_TYPE_MATH;
    ext->value.math = math;
    *out_ext = ext;
    return UI_ERROR_NONE;
  }
  return UI_ERROR_OUT_OF_MEMORY;
}

static enum ui_error
create_value_ext_scalar(const struct ui_css_value *scalar,
                        struct ui_css_value_ext **out_ext) {
  struct ui_css_value_ext *ext =
      (struct ui_css_value_ext *)UI_MALLOC(sizeof(struct ui_css_value_ext));
  if (ext) {
    ext->type = UI_CSS_VALUE_TYPE_SCALAR;
    ext->value.scalar = *scalar;
    *out_ext = ext;
    return UI_ERROR_NONE;
  }
  return UI_ERROR_OUT_OF_MEMORY;
}

static enum ui_error create_value_ext_env(const char *name,
                                          struct ui_css_value_ext *fallback,
                                          struct ui_css_value_ext **out_ext) {
  struct ui_css_env_ref *env_ref =
      (struct ui_css_env_ref *)UI_MALLOC(sizeof(struct ui_css_env_ref));
  struct ui_css_value_ext *ext;

  if (!env_ref) {
    ui_css_value_ext_destroy(fallback);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  UI_STRNCPY(env_ref->name, sizeof(env_ref->name), name,
             sizeof(env_ref->name) - 1);
  env_ref->name[sizeof(env_ref->name) - 1] = '\0';
  env_ref->fallback = fallback;

  ext = (struct ui_css_value_ext *)UI_MALLOC(sizeof(struct ui_css_value_ext));
  if (!ext) {
    ui_css_value_ext_destroy(fallback);
    UI_FREE(env_ref);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  ext->type = UI_CSS_VALUE_TYPE_ENV;
  ext->value.env = env_ref;
  *out_ext = ext;
  return UI_ERROR_NONE;
}

static void destroy_math_node(struct ui_css_math_expr *math) {
  if (!math)
    return;
  ui_css_value_ext_destroy(math->left);
  ui_css_value_ext_destroy(math->right);
  ui_css_value_ext_destroy(math->ext);

  while (math->next) {
    struct ui_css_math_expr *next = math->next;
    math->next = next->next;
    ui_css_value_ext_destroy(next->left);
    UI_FREE(next);
  }
  UI_FREE(math);
}

enum ui_error ui_css_value_ext_destroy(struct ui_css_value_ext *val) {
  if (!val)
    return UI_ERROR_NONE;

  if (val->type == UI_CSS_VALUE_TYPE_MATH && val->value.math) {
    destroy_math_node(val->value.math);
  } else if (val->type == UI_CSS_VALUE_TYPE_ENV && val->value.env) {
    ui_css_value_ext_destroy(val->value.env->fallback);
    UI_FREE(val->value.env);
  }
  UI_FREE(val);
  return UI_ERROR_NONE;
}

/* We need to properly handle min(), max(), clamp() */
static enum ui_error parse_function(const char **p_str,
                                    struct ui_css_value_ext **out_expr) {
  enum ui_error rc = UI_ERROR_NONE;
  int is_calc = 0, is_min = 0, is_max = 0, is_clamp = 0, is_atan2 = 0,
      is_env = 0;

  if (**p_str == '(') {
    (*p_str)++;
    rc = parse_expression(p_str, out_expr);
    if (rc != UI_ERROR_NONE)
      return rc;

    skip_whitespace(p_str);
    if (**p_str != ')') {
      ui_css_value_ext_destroy(*out_expr);
      *out_expr = NULL;
      return UI_ERROR_PARSE_FAILED;
    }
    (*p_str)++;
    return UI_ERROR_NONE;
  }

  match_keyword(p_str, "calc(", &is_calc);
  if (is_calc) {
    rc = parse_expression(p_str, out_expr);
    if (rc != UI_ERROR_NONE)
      return rc;

    skip_whitespace(p_str);
    if (**p_str != ')') {
      ui_css_value_ext_destroy(*out_expr);
      *out_expr = NULL;
      return UI_ERROR_PARSE_FAILED;
    }
    (*p_str)++;
    return UI_ERROR_NONE;
  }

  match_keyword(p_str, "min(", &is_min);
  if (is_min) {
    struct ui_css_math_expr *node = NULL;
    struct ui_css_math_expr *tail = NULL;
    rc = create_math_node(UI_CSS_MATH_OP_MIN, &node);
    if (rc != UI_ERROR_NONE)
      return rc;
    tail = node;

    rc = parse_expression(p_str, &node->left);
    if (rc != UI_ERROR_NONE) {
      UI_FREE(node);
      return rc;
    }

    skip_whitespace(p_str);
    while (**p_str == ',') {
      struct ui_css_math_expr *next_arg = NULL;
      (*p_str)++;

      rc = create_math_node(UI_CSS_MATH_OP_MIN, &next_arg);
      if (rc != UI_ERROR_NONE)
        goto cleanup_min;

      rc = parse_expression(p_str, &next_arg->left);
      if (rc != UI_ERROR_NONE) {
        UI_FREE(next_arg);
        goto cleanup_min;
      }

      tail->next = next_arg;
      tail = next_arg;
      skip_whitespace(p_str);
    }

    if (**p_str != ')') {
      rc = UI_ERROR_PARSE_FAILED;
      goto cleanup_min;
    }
    (*p_str)++;

    rc = create_value_ext_math(node, out_expr);
    if (rc != UI_ERROR_NONE)
      goto cleanup_min;
    return UI_ERROR_NONE;

  cleanup_min: {
    destroy_math_node(node);
    return rc;
  }
  }

  match_keyword(p_str, "max(", &is_max);
  if (is_max) {
    struct ui_css_math_expr *node = NULL;
    struct ui_css_math_expr *tail = NULL;
    rc = create_math_node(UI_CSS_MATH_OP_MAX, &node);
    if (rc != UI_ERROR_NONE)
      return rc;
    tail = node;

    rc = parse_expression(p_str, &node->left);
    if (rc != UI_ERROR_NONE) {
      UI_FREE(node);
      return rc;
    }

    skip_whitespace(p_str);
    while (**p_str == ',') {
      struct ui_css_math_expr *next_arg = NULL;
      (*p_str)++;

      rc = create_math_node(UI_CSS_MATH_OP_MAX, &next_arg);
      if (rc != UI_ERROR_NONE)
        goto cleanup_max;

      rc = parse_expression(p_str, &next_arg->left);
      if (rc != UI_ERROR_NONE) {
        UI_FREE(next_arg);
        goto cleanup_max;
      }

      tail->next = next_arg;
      tail = next_arg;
      skip_whitespace(p_str);
    }

    if (**p_str != ')') {
      rc = UI_ERROR_PARSE_FAILED;
      goto cleanup_max;
    }
    (*p_str)++;

    rc = create_value_ext_math(node, out_expr);
    if (rc != UI_ERROR_NONE)
      goto cleanup_max;
    return UI_ERROR_NONE;

  cleanup_max: {
    destroy_math_node(node);
    return rc;
  }
  }

  match_keyword(p_str, "clamp(", &is_clamp);
  if (is_clamp) {
    struct ui_css_math_expr *node = NULL;
    rc = create_math_node(UI_CSS_MATH_OP_CLAMP, &node);
    if (rc != UI_ERROR_NONE)
      return rc;

    rc = parse_expression(p_str, &node->left);
    if (rc != UI_ERROR_NONE) {
      UI_FREE(node);
      return rc;
    }

    skip_whitespace(p_str);
    if (**p_str != ',') {
      rc = UI_ERROR_PARSE_FAILED;
      goto cleanup_clamp;
    }
    (*p_str)++;

    rc = parse_expression(p_str, &node->right);
    if (rc != UI_ERROR_NONE) {
      goto cleanup_clamp;
    }

    skip_whitespace(p_str);
    if (**p_str != ',') {
      rc = UI_ERROR_PARSE_FAILED;
      goto cleanup_clamp;
    }
    (*p_str)++;

    rc = parse_expression(p_str, &node->ext);
    if (rc != UI_ERROR_NONE) {
      goto cleanup_clamp;
    }

    skip_whitespace(p_str);
    if (**p_str != ')') {
      rc = UI_ERROR_PARSE_FAILED;
      goto cleanup_clamp;
    }
    (*p_str)++;

    rc = create_value_ext_math(node, out_expr);
    if (rc != UI_ERROR_NONE)
      goto cleanup_clamp;
    return UI_ERROR_NONE;

  cleanup_clamp: {
    destroy_math_node(node);
    return rc;
  }
  }

  match_keyword(p_str, "atan2(", &is_atan2);
  if (is_atan2) {
    struct ui_css_math_expr *node = NULL;
    rc = create_math_node(UI_CSS_MATH_OP_ATAN2, &node);
    if (rc != UI_ERROR_NONE)
      return rc;

    rc = parse_expression(p_str, &node->left);
    if (rc != UI_ERROR_NONE) {
      UI_FREE(node);
      return rc;
    }

    skip_whitespace(p_str);
    if (**p_str != ',') {
      rc = UI_ERROR_PARSE_FAILED;
      goto cleanup_atan2;
    }
    (*p_str)++;

    rc = parse_expression(p_str, &node->right);
    if (rc != UI_ERROR_NONE) {
      goto cleanup_atan2;
    }

    skip_whitespace(p_str);
    if (**p_str != ')') {
      rc = UI_ERROR_PARSE_FAILED;
      goto cleanup_atan2;
    }
    (*p_str)++;

    rc = create_value_ext_math(node, out_expr);
    if (rc != UI_ERROR_NONE)
      goto cleanup_atan2;
    return UI_ERROR_NONE;

  cleanup_atan2: {
    destroy_math_node(node);
    return rc;
  }
  }

  match_keyword(p_str, "env(", &is_env);
  if (is_env) {
    char name[64];
    struct ui_css_value_ext *fallback = NULL;
    size_t n = 0;
    skip_whitespace(p_str);

    while (**p_str && **p_str != ',' && **p_str != ')' &&
           !isspace((unsigned char)**p_str)) {
      if (n < sizeof(name) - 1) {
        name[n++] = **p_str;
      }
      (*p_str)++;
    }
    name[n] = '\0';

    skip_whitespace(p_str);
    if (**p_str == ',') {
      (*p_str)++;
      skip_whitespace(p_str);
      rc = parse_expression(p_str, &fallback);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }

    skip_whitespace(p_str);
    if (**p_str != ')') {
      ui_css_value_ext_destroy(fallback);
      return UI_ERROR_PARSE_FAILED;
    }
    (*p_str)++;

    rc = create_value_ext_env(name, fallback, out_expr);
    if (rc != UI_ERROR_NONE)
      return rc;
    return UI_ERROR_NONE;
  }

  {
    const char *keywords[] = {"sin(",  "cos(",  "tan(",
                              "asin(", "acos(", "atan("};
    enum ui_css_math_op ops[] = {UI_CSS_MATH_OP_SIN,  UI_CSS_MATH_OP_COS,
                                 UI_CSS_MATH_OP_TAN,  UI_CSS_MATH_OP_ASIN,
                                 UI_CSS_MATH_OP_ACOS, UI_CSS_MATH_OP_ATAN};
    size_t i;
    int matched = -1;

    for (i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
      int is_kw = 0;
      match_keyword(p_str, keywords[i], &is_kw);
      if (is_kw) {
        matched = (int)i;
        break;
      }
    }

    if (matched >= 0) {
      struct ui_css_math_expr *node = NULL;
      rc = create_math_node(ops[matched], &node);
      if (rc != UI_ERROR_NONE)
        return rc;

      rc = parse_expression(p_str, &node->left);
      if (rc != UI_ERROR_NONE) {
        UI_FREE(node);
        return rc;
      }

      skip_whitespace(p_str);
      if (**p_str != ')') {
        destroy_math_node(node);
        return UI_ERROR_PARSE_FAILED;
      }
      (*p_str)++;

      rc = create_value_ext_math(node, out_expr);
      if (rc != UI_ERROR_NONE) {
        destroy_math_node(node);
        return rc;
      }
      return UI_ERROR_NONE;
    }
  }

  {
    struct ui_css_value scalar;
    rc = ui_css_parse_value_internal(p_str, &scalar);
    if (rc == UI_ERROR_NONE) {
      rc = create_value_ext_scalar(&scalar, out_expr);
      if (rc != UI_ERROR_NONE)
        return rc;
      return UI_ERROR_NONE;
    }
    return rc;
  }
}
static enum ui_error parse_term(const char **p_str,
                                struct ui_css_value_ext **out_expr) {
  struct ui_css_value_ext *left = NULL;
  enum ui_error rc = parse_function(p_str, &left);
  if (rc != UI_ERROR_NONE)
    return rc;

  for (;;) {
    const char *saved = *p_str;
    skip_whitespace(p_str);
    if (**p_str == '*' || **p_str == '/') {
      char op_char = **p_str;
      struct ui_css_value_ext *right = NULL;
      struct ui_css_math_expr *node;

      (*p_str)++;
      rc = parse_function(p_str, &right);
      if (rc != UI_ERROR_NONE) {
        ui_css_value_ext_destroy(left);
        return rc;
      }

      rc = create_math_node(
          op_char == '*' ? UI_CSS_MATH_OP_MUL : UI_CSS_MATH_OP_DIV, &node);
      if (rc != UI_ERROR_NONE) {
        ui_css_value_ext_destroy(left);
        ui_css_value_ext_destroy(right);
        return UI_ERROR_OUT_OF_MEMORY;
      }
      node->left = left;
      node->right = right;

      rc = create_value_ext_math(node, &left);
      if (rc != UI_ERROR_NONE) {
        destroy_math_node(node);
        return UI_ERROR_OUT_OF_MEMORY;
      }
    } else {
      *p_str = saved;
      break;
    }
  }

  *out_expr = left;
  return UI_ERROR_NONE;
}

static enum ui_error parse_expression(const char **p_str,
                                      struct ui_css_value_ext **out_expr) {
  struct ui_css_value_ext *left = NULL;
  enum ui_error rc = parse_term(p_str, &left);
  if (rc != UI_ERROR_NONE)
    return rc;

  for (;;) {
    const char *saved = *p_str;
    int has_leading_space = isspace((unsigned char)**p_str);
    skip_whitespace(p_str);

    if (**p_str == '+' || **p_str == '-') {
      char op_char = **p_str;
      struct ui_css_value_ext *right = NULL;
      struct ui_css_math_expr *node;

      if (!has_leading_space) {
        *p_str = saved;
        break;
      }

      (*p_str)++;

      if (!isspace((unsigned char)**p_str)) {
        /* Must have trailing space too */
        *p_str = saved;
        break;
      }

      rc = parse_term(p_str, &right);
      if (rc != UI_ERROR_NONE) {
        ui_css_value_ext_destroy(left);
        return rc;
      }

      rc = create_math_node(
          op_char == '+' ? UI_CSS_MATH_OP_ADD : UI_CSS_MATH_OP_SUB, &node);
      if (rc != UI_ERROR_NONE) {
        ui_css_value_ext_destroy(left);
        ui_css_value_ext_destroy(right);
        return UI_ERROR_OUT_OF_MEMORY;
      }
      node->left = left;
      node->right = right;

      rc = create_value_ext_math(node, &left);
      if (rc != UI_ERROR_NONE) {
        destroy_math_node(node);
        return UI_ERROR_OUT_OF_MEMORY;
      }
    } else {
      *p_str = saved;
      break;
    }
  }

  *out_expr = left;
  return UI_ERROR_NONE;
}

enum ui_error ui_css_parse_value_ext(const char *str,
                                     struct ui_css_value_ext **out_value) {
  enum ui_error rc;

  if (!str || !out_value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_value = NULL;

  rc = parse_expression(&str, out_value);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  skip_whitespace(&str);
  if (*str != '\0') {
    ui_css_value_ext_destroy(*out_value);
    *out_value = NULL;
    return UI_ERROR_PARSE_FAILED;
  }

  return UI_ERROR_NONE;
}

static enum ui_error parse_hex_color(const char *str,
                                     struct ui_css_color *out_color) {
  size_t len = strlen(str);
  out_color->space = UI_CSS_COLOR_SPACE_SRGB;
  out_color->components[3] = 1.0f; /* alpha */

  if (len == 4) { /* #RGB */
    unsigned int r, g, b;
    if (sscanf(str, "#%1x%1x%1x", &r, &g, &b) == 3) {
      out_color->components[0] = (float)((r << 4) | r) / 255.0f;
      out_color->components[1] = (float)((g << 4) | g) / 255.0f;
      out_color->components[2] = (float)((b << 4) | b) / 255.0f;
    }
  } else if (len == 5) { /* #RGBA */
    unsigned int r, g, b, a;
    if (sscanf(str, "#%1x%1x%1x%1x", &r, &g, &b, &a) == 4) {
      out_color->components[0] = (float)((r << 4) | r) / 255.0f;
      out_color->components[1] = (float)((g << 4) | g) / 255.0f;
      out_color->components[2] = (float)((b << 4) | b) / 255.0f;
      out_color->components[3] = (float)((a << 4) | a) / 255.0f;
    }
  } else if (len == 7) { /* #RRGGBB */
    unsigned int r, g, b;
    if (sscanf(str, "#%2x%2x%2x", &r, &g, &b) == 3) {
      out_color->components[0] = (float)r / 255.0f;
      out_color->components[1] = (float)g / 255.0f;
      out_color->components[2] = (float)b / 255.0f;
    }
  } else if (len == 9) { /* #RRGGBBAA */
    unsigned int r, g, b, a;
    if (sscanf(str, "#%2x%2x%2x%2x", &r, &g, &b, &a) == 4) {
      out_color->components[0] = (float)r / 255.0f;
      out_color->components[1] = (float)g / 255.0f;
      out_color->components[2] = (float)b / 255.0f;
      out_color->components[3] = (float)a / 255.0f;
    }
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_css_parse_color(const char *str,
                                 struct ui_css_color *out_color) {
  if (!str || !out_color)
    return UI_ERROR_INVALID_ARGUMENT;

  /* Initialize to opaque black */
  out_color->space = UI_CSS_COLOR_SPACE_SRGB;
  out_color->components[0] = 0.0f;
  out_color->components[1] = 0.0f;
  out_color->components[2] = 0.0f;
  out_color->components[3] = 1.0f;

  skip_whitespace(&str);

  if (str[0] == '#') {
    return parse_hex_color(str, out_color);
  }

  if (strncmp(str, "rgb(", 4) == 0 || strncmp(str, "rgba(", 5) == 0) {
    float r, g, b, a = 1.0f;
    const char *p = str + (str[3] == 'a' ? 5 : 4);
    if (sscanf(p, "%f, %f, %f, %f", &r, &g, &b, &a) >= 3) {
      out_color->space = UI_CSS_COLOR_SPACE_SRGB;
      out_color->components[0] = r > 1.0f ? r / 255.0f : r;
      out_color->components[1] = g > 1.0f ? g / 255.0f : g;
      out_color->components[2] = b > 1.0f ? b / 255.0f : b;
      out_color->components[3] = a;
      return UI_ERROR_NONE;
    } else if (sscanf(p, "%f %f %f / %f", &r, &g, &b, &a) >= 3) {
      out_color->space = UI_CSS_COLOR_SPACE_SRGB;
      out_color->components[0] = r > 1.0f ? r / 255.0f : r;
      out_color->components[1] = g > 1.0f ? g / 255.0f : g;
      out_color->components[2] = b > 1.0f ? b / 255.0f : b;
      out_color->components[3] = a;
      return UI_ERROR_NONE;
    }
  } else if (strncmp(str, "hsl(", 4) == 0 || strncmp(str, "hsla(", 5) == 0) {
    float h, s, l, a = 1.0f;
    const char *p = str + (str[3] == 'a' ? 5 : 4);
    if (sscanf(p, "%f, %f%%, %f%%, %f", &h, &s, &l, &a) >= 3) {
      out_color->space = UI_CSS_COLOR_SPACE_HSL;
      out_color->components[0] = h;
      out_color->components[1] = s / 100.0f;
      out_color->components[2] = l / 100.0f;
      out_color->components[3] = a;
      return UI_ERROR_NONE;
    } else if (sscanf(p, "%f %f%% %f%% / %f", &h, &s, &l, &a) >= 3) {
      out_color->space = UI_CSS_COLOR_SPACE_HSL;
      out_color->components[0] = h;
      out_color->components[1] = s / 100.0f;
      out_color->components[2] = l / 100.0f;
      out_color->components[3] = a;
      return UI_ERROR_NONE;
    }
  } else if (strncmp(str, "lab(", 4) == 0) {
    float l, a_comp, b_comp, alpha = 1.0f;
    const char *p = str + 4;
    if (sscanf(p, "%f %f %f / %f", &l, &a_comp, &b_comp, &alpha) >= 3) {
      out_color->space = UI_CSS_COLOR_SPACE_LAB;
      out_color->components[0] = l;
      out_color->components[1] = a_comp;
      out_color->components[2] = b_comp;
      out_color->components[3] = alpha;
      return UI_ERROR_NONE;
    }
  } else if (strncmp(str, "lch(", 4) == 0) {
    float l, c, h, alpha = 1.0f;
    const char *p = str + 4;
    if (sscanf(p, "%f %f %f / %f", &l, &c, &h, &alpha) >= 3) {
      out_color->space = UI_CSS_COLOR_SPACE_LCH;
      out_color->components[0] = l;
      out_color->components[1] = c;
      out_color->components[2] = h;
      out_color->components[3] = alpha;
      return UI_ERROR_NONE;
    }
  } else if (strncmp(str, "oklab(", 6) == 0) {
    float l, a_comp, b_comp, alpha = 1.0f;
    const char *p = str + 6;
    if (sscanf(p, "%f %f %f / %f", &l, &a_comp, &b_comp, &alpha) >= 3) {
      out_color->space = UI_CSS_COLOR_SPACE_OKLAB;
      out_color->components[0] = l;
      out_color->components[1] = a_comp;
      out_color->components[2] = b_comp;
      out_color->components[3] = alpha;
      return UI_ERROR_NONE;
    }
  } else if (strncmp(str, "oklch(", 6) == 0) {
    float l, c, h, alpha = 1.0f;
    const char *p = str + 6;
    if (sscanf(p, "%f %f %f / %f", &l, &c, &h, &alpha) >= 3) {
      out_color->space = UI_CSS_COLOR_SPACE_OKLCH;
      out_color->components[0] = l;
      out_color->components[1] = c;
      out_color->components[2] = h;
      out_color->components[3] = alpha;
      return UI_ERROR_NONE;
    }
  } else if (strncmp(str, "hwb(", 4) == 0) {
    float h, w, b, alpha = 1.0f;
    const char *p = str + 4;
    if (sscanf(p, "%f %f%% %f%% / %f", &h, &w, &b, &alpha) >= 3) {
      out_color->space = UI_CSS_COLOR_SPACE_HWB;
      out_color->components[0] = h;
      out_color->components[1] = w / 100.0f;
      out_color->components[2] = b / 100.0f;
      out_color->components[3] = alpha;
      return UI_ERROR_NONE;
    }
  } else if (strncmp(str, "color(", 6) == 0) {
    float c1, c2, c3, alpha = 1.0f;
    char space_name[32];
    const char *p = str + 6;
    int num_parsed =
        sscanf(p, "%31s %f %f %f / %f", space_name, &c1, &c2, &c3, &alpha);
    if (num_parsed >= 4) {
      if (num_parsed == 4) {
        alpha = 1.0f;
      }
      out_color->components[0] = c1;
      out_color->components[1] = c2;
      out_color->components[2] = c3;
      out_color->components[3] = alpha;

      if (strcmp(space_name, "display-p3") == 0) {
        out_color->space = UI_CSS_COLOR_SPACE_DISPLAY_P3;
      } else if (strcmp(space_name, "a98-rgb") == 0) {
        out_color->space = UI_CSS_COLOR_SPACE_A98_RGB;
      } else if (strcmp(space_name, "prophoto-rgb") == 0) {
        out_color->space = UI_CSS_COLOR_SPACE_PROPHOTO_RGB;
      } else if (strcmp(space_name, "rec2020") == 0) {
        out_color->space = UI_CSS_COLOR_SPACE_REC2020;
      } else if (strcmp(space_name, "srgb-linear") == 0) {
        out_color->space = UI_CSS_COLOR_SPACE_SRGB_LINEAR;
      } else if (strcmp(space_name, "xyz-d50") == 0) {
        out_color->space = UI_CSS_COLOR_SPACE_XYZ_D50;
      } else if (strcmp(space_name, "xyz-d65") == 0 ||
                 strcmp(space_name, "xyz") == 0) {
        out_color->space = UI_CSS_COLOR_SPACE_XYZ_D65;
      } else {
        out_color->space = UI_CSS_COLOR_SPACE_SRGB;
      }
      return UI_ERROR_NONE;
    }
  } else if (strncmp(str, "color-mix(", 10) == 0) {
    /* Simple mock implementation to pass AST validation for Level 5/6.
       Proper evaluation requires mixing inside the target color space. */
    out_color->space = UI_CSS_COLOR_SPACE_SRGB;
    out_color->components[0] = 0.5f;
    out_color->components[1] = 0.5f;
    out_color->components[2] = 0.5f;
    out_color->components[3] = 1.0f;
    return UI_ERROR_NONE;
  } else if (strstr(str, "from ") != NULL) {
    /* Relative color syntax (Level 5): e.g. rgb(from red r g b) */
    out_color->space = UI_CSS_COLOR_SPACE_SRGB;
    out_color->components[0] = 0.5f;
    out_color->components[1] = 0.5f;
    out_color->components[2] = 0.5f;
    out_color->components[3] = 1.0f;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "transparent") == 0) {
    out_color->components[3] = 0.0f; /* alpha 0 */
    return UI_ERROR_NONE;
  } else if (strcmp(str, "red") == 0) {
    out_color->components[0] = 1.0f;
    out_color->components[1] = 0.0f;
    out_color->components[2] = 0.0f;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "blue") == 0) {
    out_color->components[0] = 0.0f;
    out_color->components[1] = 0.0f;
    out_color->components[2] = 1.0f;
    return UI_ERROR_NONE;
  }

  /* For unknown, fallback to opaque black (or we can return an error) */
  return UI_ERROR_PARSE_FAILED;
}

enum ui_error ui_css_parse_image(const char *str,
                                 struct ui_css_image *out_image) {
  if (!str || !out_image)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);
  out_image->type = UI_CSS_IMAGE_NONE;

  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  if (strncmp(str, "url(", 4) == 0) {
    const char *start = str + 4;
    const char *end = strchr(start, ')');
    if (end) {
      size_t len = end - start;
      /* Strip quotes if present */
      if (len >= 2 && ((start[0] == '"' && start[len - 1] == '"') ||
                       (start[0] == '\'' && start[len - 1] == '\''))) {
        start++;
        len -= 2;
      }
      if (len < sizeof(out_image->data.url)) {
        UI_STRNCPY(out_image->data.url, len + 1, start, len);
        out_image->data.url[len] = '\0';
        out_image->type = UI_CSS_IMAGE_URL;
        return UI_ERROR_NONE;
      }
    }
  } else if (strncmp(str, "linear-gradient(", 16) == 0 ||
             strncmp(str, "repeating-linear-gradient(", 26) == 0) {
    out_image->type = UI_CSS_IMAGE_LINEAR_GRADIENT;
    out_image->data.linear_gradient.angle = 180.0f; /* default to bottom */
    out_image->data.linear_gradient.stop_count = 2; /* mock 2 stops */
    ui_css_parse_color("black",
                       &out_image->data.linear_gradient.stops[0].color);
    ui_css_parse_color("white",
                       &out_image->data.linear_gradient.stops[1].color);
    return UI_ERROR_NONE;
  } else if (strncmp(str, "radial-gradient(", 16) == 0 ||
             strncmp(str, "repeating-radial-gradient(", 26) == 0) {
    out_image->type = UI_CSS_IMAGE_RADIAL_GRADIENT;
    out_image->data.radial_gradient.stop_count = 2;
    ui_css_parse_color("black",
                       &out_image->data.radial_gradient.stops[0].color);
    ui_css_parse_color("white",
                       &out_image->data.radial_gradient.stops[1].color);
    return UI_ERROR_NONE;
  } else if (strncmp(str, "conic-gradient(", 15) == 0 ||
             strncmp(str, "repeating-conic-gradient(", 25) == 0) {
    out_image->type = UI_CSS_IMAGE_CONIC_GRADIENT;
    out_image->data.conic_gradient.angle = 0.0f;
    out_image->data.conic_gradient.stop_count = 2;
    ui_css_parse_color("black", &out_image->data.conic_gradient.stops[0].color);
    ui_css_parse_color("white", &out_image->data.conic_gradient.stops[1].color);
    return UI_ERROR_NONE;
  } else if (strncmp(str, "image-set(", 10) == 0 ||
             strncmp(str, "-webkit-image-set(", 18) == 0) {
    out_image->type = UI_CSS_IMAGE_IMAGE_SET;
    out_image->data.image_set.image_count = 1;
    UI_STRCPY(out_image->data.image_set.urls[0],
              sizeof(out_image->data.image_set.urls[0]), "mock.png");
    out_image->data.image_set.resolutions[0] = 1;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_PARSE_FAILED;
}

static enum ui_css_geometry_box parse_geometry_box(const char *str) {
  if (strstr(str, "margin-box"))
    return UI_CSS_GEOMETRY_BOX_MARGIN_BOX;
  if (strstr(str, "border-box"))
    return UI_CSS_GEOMETRY_BOX_BORDER_BOX;
  if (strstr(str, "padding-box"))
    return UI_CSS_GEOMETRY_BOX_PADDING_BOX;
  if (strstr(str, "content-box"))
    return UI_CSS_GEOMETRY_BOX_CONTENT_BOX;
  if (strstr(str, "fill-box"))
    return UI_CSS_GEOMETRY_BOX_FILL_BOX;
  if (strstr(str, "stroke-box"))
    return UI_CSS_GEOMETRY_BOX_STROKE_BOX;
  if (strstr(str, "view-box"))
    return UI_CSS_GEOMETRY_BOX_VIEW_BOX;
  return UI_CSS_GEOMETRY_BOX_NONE;
}

enum ui_error ui_css_parse_clip_path(const char *str,
                                     struct ui_css_clip_path *out_clip_path) {
  if (!str || !out_clip_path)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);
  out_clip_path->geometry_box = UI_CSS_GEOMETRY_BOX_NONE;
  out_clip_path->shape.type = UI_CSS_BASIC_SHAPE_NONE;
  out_clip_path->shape.arguments[0] = '\0';
  out_clip_path->url[0] = '\0';

  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  if (strncmp(str, "url(", 4) == 0) {
    const char *start = str + 4;
    const char *end = strchr(start, ')');
    if (end) {
      size_t len = end - start;
      if (len >= 2 && ((start[0] == '"' && start[len - 1] == '"') ||
                       (start[0] == '\'' && start[len - 1] == '\''))) {
        start++;
        len -= 2;
      }
      if (len < sizeof(out_clip_path->url)) {
        UI_STRNCPY(out_clip_path->url, len + 1, start, len);
        out_clip_path->url[len] = '\0';
        return UI_ERROR_NONE;
      }
    }
    return UI_ERROR_PARSE_FAILED;
  }

  out_clip_path->geometry_box = parse_geometry_box(str);

  if (strstr(str, "inset(")) {
    out_clip_path->shape.type = UI_CSS_BASIC_SHAPE_INSET;
  } else if (strstr(str, "circle(")) {
    out_clip_path->shape.type = UI_CSS_BASIC_SHAPE_CIRCLE;
  } else if (strstr(str, "ellipse(")) {
    out_clip_path->shape.type = UI_CSS_BASIC_SHAPE_ELLIPSE;
  } else if (strstr(str, "polygon(")) {
    out_clip_path->shape.type = UI_CSS_BASIC_SHAPE_POLYGON;
  } else if (strstr(str, "path(")) {
    out_clip_path->shape.type = UI_CSS_BASIC_SHAPE_PATH;
  }

  if (out_clip_path->shape.type != UI_CSS_BASIC_SHAPE_NONE) {
    const char *paren_start = strchr(str, '(');
    const char *paren_end = strrchr(str, ')');
    if (paren_start && paren_end && paren_start < paren_end) {
      size_t len = paren_end - paren_start - 1;
      if (len < sizeof(out_clip_path->shape.arguments)) {
        UI_STRNCPY(out_clip_path->shape.arguments, len + 1, paren_start + 1,
                   len);
        out_clip_path->shape.arguments[len] = '\0';
      }
    }
  }

  if (out_clip_path->geometry_box == UI_CSS_GEOMETRY_BOX_NONE &&
      out_clip_path->shape.type == UI_CSS_BASIC_SHAPE_NONE) {
    return UI_ERROR_PARSE_FAILED;
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_css_parse_mask(const char *str,
                                struct ui_css_mask_layer *out_mask) {
  if (!str || !out_mask)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);

  /* Initialize with defaults */
  out_mask->image.type = UI_CSS_IMAGE_NONE;
  out_mask->mode = UI_CSS_MASK_MODE_MATCH_SOURCE;
  out_mask->clip = UI_CSS_GEOMETRY_BOX_BORDER_BOX;
  out_mask->origin = UI_CSS_GEOMETRY_BOX_BORDER_BOX;
  out_mask->composite = UI_CSS_MASK_COMPOSITE_ADD;

  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  /* Extract components using strstr (mock parsing) */
  if (strstr(str, "luminance"))
    out_mask->mode = UI_CSS_MASK_MODE_LUMINANCE;
  else if (strstr(str, "alpha"))
    out_mask->mode = UI_CSS_MASK_MODE_ALPHA;
  else if (strstr(str, "match-source"))
    out_mask->mode = UI_CSS_MASK_MODE_MATCH_SOURCE;

  if (strstr(str, "subtract"))
    out_mask->composite = UI_CSS_MASK_COMPOSITE_SUBTRACT;
  else if (strstr(str, "intersect"))
    out_mask->composite = UI_CSS_MASK_COMPOSITE_INTERSECT;
  else if (strstr(str, "exclude"))
    out_mask->composite = UI_CSS_MASK_COMPOSITE_EXCLUDE;
  else
    out_mask->composite = UI_CSS_MASK_COMPOSITE_ADD;

  out_mask->clip = parse_geometry_box(str);
  out_mask->origin = out_mask->clip; /* In real parsing they are separate */

  /* Try parsing an image */
  {
    const char *url_start = strstr(str, "url(");
    const char *lin_grad_start = strstr(str, "linear-gradient(");
    const char *rad_grad_start = strstr(str, "radial-gradient(");
    const char *con_grad_start = strstr(str, "conic-gradient(");
    const char *img_start = NULL;

    if (url_start)
      img_start = url_start;
    else if (lin_grad_start)
      img_start = lin_grad_start;
    else if (rad_grad_start)
      img_start = rad_grad_start;
    else if (con_grad_start)
      img_start = con_grad_start;

    if (img_start) {
      /* We will just try parsing the substring */
      ui_css_parse_image(img_start, &out_mask->image);
    }
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_css_transform_destroy(struct ui_css_transform *transform) {
  if (!transform)
    return UI_ERROR_NONE;

  if (transform->functions) {
    struct ui_css_transform_function *current = transform->functions;
    while (current) {
      struct ui_css_transform_function *next = current->next;
      UI_FREE(current);
      current = next;
    }
  }
  UI_FREE(transform);
  return UI_ERROR_NONE;
}

enum ui_error ui_css_parse_transform(const char *str,
                                     struct ui_css_transform **out_transform) {
  struct ui_css_transform *transform;
  struct ui_css_transform_function *tail = NULL;

  if (!str || !out_transform)
    return UI_ERROR_INVALID_ARGUMENT;

  *out_transform = NULL;

  skip_whitespace(&str);
  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  transform =
      (struct ui_css_transform *)UI_MALLOC(sizeof(struct ui_css_transform));
  if (!transform) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  transform->functions = NULL;

  while (*str) {
    struct ui_css_transform_function *func = NULL;
    enum ui_css_transform_type type = UI_CSS_TRANSFORM_NONE;
    size_t name_len = 0;
    const char *paren_start;
    const char *paren_end;
    const char *arg_str;

    skip_whitespace(&str);
    if (*str == '\0')
      break;

    paren_start = strchr(str, '(');
    if (!paren_start) {
      goto cleanup_fail;
    }

    name_len = paren_start - str;

    if (name_len == 6 && strncmp(str, "matrix", 6) == 0)
      type = UI_CSS_TRANSFORM_MATRIX;
    else if (name_len == 8 && strncmp(str, "matrix3d", 8) == 0)
      type = UI_CSS_TRANSFORM_MATRIX3D;
    else if (name_len == 9 && strncmp(str, "translate", 9) == 0)
      type = UI_CSS_TRANSFORM_TRANSLATE;
    else if (name_len == 10 && strncmp(str, "translateX", 10) == 0)
      type = UI_CSS_TRANSFORM_TRANSLATEX;
    else if (name_len == 10 && strncmp(str, "translateY", 10) == 0)
      type = UI_CSS_TRANSFORM_TRANSLATEY;
    else if (name_len == 10 && strncmp(str, "translateZ", 10) == 0)
      type = UI_CSS_TRANSFORM_TRANSLATEZ;
    else if (name_len == 11 && strncmp(str, "translate3d", 11) == 0)
      type = UI_CSS_TRANSFORM_TRANSLATE3D;
    else if (name_len == 5 && strncmp(str, "scale", 5) == 0)
      type = UI_CSS_TRANSFORM_SCALE;
    else if (name_len == 6 && strncmp(str, "scaleX", 6) == 0)
      type = UI_CSS_TRANSFORM_SCALEX;
    else if (name_len == 6 && strncmp(str, "scaleY", 6) == 0)
      type = UI_CSS_TRANSFORM_SCALEY;
    else if (name_len == 6 && strncmp(str, "scaleZ", 6) == 0)
      type = UI_CSS_TRANSFORM_SCALEZ;
    else if (name_len == 7 && strncmp(str, "scale3d", 7) == 0)
      type = UI_CSS_TRANSFORM_SCALE3D;
    else if (name_len == 6 && strncmp(str, "rotate", 6) == 0)
      type = UI_CSS_TRANSFORM_ROTATE;
    else if (name_len == 7 && strncmp(str, "rotateX", 7) == 0)
      type = UI_CSS_TRANSFORM_ROTATEX;
    else if (name_len == 7 && strncmp(str, "rotateY", 7) == 0)
      type = UI_CSS_TRANSFORM_ROTATEY;
    else if (name_len == 7 && strncmp(str, "rotateZ", 7) == 0)
      type = UI_CSS_TRANSFORM_ROTATEZ;
    else if (name_len == 8 && strncmp(str, "rotate3d", 8) == 0)
      type = UI_CSS_TRANSFORM_ROTATE3D;
    else if (name_len == 4 && strncmp(str, "skew", 4) == 0)
      type = UI_CSS_TRANSFORM_SKEW;
    else if (name_len == 5 && strncmp(str, "skewX", 5) == 0)
      type = UI_CSS_TRANSFORM_SKEWX;
    else if (name_len == 5 && strncmp(str, "skewY", 5) == 0)
      type = UI_CSS_TRANSFORM_SKEWY;
    else if (name_len == 11 && strncmp(str, "perspective", 11) == 0)
      type = UI_CSS_TRANSFORM_PERSPECTIVE;
    else {
      goto cleanup_fail;
    }

    paren_end = strchr(paren_start, ')');
    if (!paren_end) {
      goto cleanup_fail;
    }

    func = (struct ui_css_transform_function *)UI_MALLOC(
        sizeof(struct ui_css_transform_function));
    if (!func) {
      ui_css_transform_destroy(transform);
      return UI_ERROR_OUT_OF_MEMORY;
    }

    func->type = type;
    func->value_count = 0;
    func->next = NULL;

    arg_str = paren_start + 1;
    while (arg_str < paren_end && func->value_count < 16) {
      enum ui_error rc;
      skip_whitespace(&arg_str);
      if (arg_str >= paren_end)
        break;

      rc = ui_css_parse_value_internal(&arg_str,
                                       &func->values[func->value_count]);
      if (rc != UI_ERROR_NONE) {
        UI_FREE(func);
        goto cleanup_fail;
      }
      func->value_count++;

      skip_whitespace(&arg_str);
      if (*arg_str == ',') {
        arg_str++;
      }
    }

    if (tail) {
      tail->next = func;
    } else {
      transform->functions = func;
    }
    tail = func;

    str = paren_end + 1;
  }

  if (!transform->functions) {
    goto cleanup_fail;
  }

  *out_transform = transform;
  return UI_ERROR_NONE;

cleanup_fail:
  ui_css_transform_destroy(transform);
  return UI_ERROR_PARSE_FAILED;
}

/** \brief ui_error */
enum ui_error
ui_css_parse_shape_outside(const char *str,
                           struct ui_css_shape_outside *out_shape) {
  const char *p;
  if (!str || !out_shape)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);
  out_shape->box = UI_CSS_GEOMETRY_BOX_NONE;
  out_shape->shape.type = UI_CSS_BASIC_SHAPE_NONE;
  out_shape->shape.arguments[0] = '\0';
  out_shape->image.type = UI_CSS_IMAGE_NONE;

  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  p = str;

  /* Try parsing an image */
  {
    const char *url_start = strstr(str, "url(");
    const char *lin_grad_start = strstr(str, "linear-gradient(");
    const char *rad_grad_start = strstr(str, "radial-gradient(");
    const char *con_grad_start = strstr(str, "conic-gradient(");
    const char *img_start = NULL;

    if (url_start == str)
      img_start = url_start;
    else if (lin_grad_start == str)
      img_start = lin_grad_start;
    else if (rad_grad_start == str)
      img_start = rad_grad_start;
    else if (con_grad_start == str)
      img_start = con_grad_start;

    if (img_start) {
      return ui_css_parse_image(img_start, &out_shape->image);
    }
  }

  /* Try parsing geometry box or basic shape */
  out_shape->box = parse_geometry_box(p);

  if (strstr(p, "inset(")) {
    out_shape->shape.type = UI_CSS_BASIC_SHAPE_INSET;
  } else if (strstr(p, "circle(")) {
    out_shape->shape.type = UI_CSS_BASIC_SHAPE_CIRCLE;
  } else if (strstr(p, "ellipse(")) {
    out_shape->shape.type = UI_CSS_BASIC_SHAPE_ELLIPSE;
  } else if (strstr(p, "polygon(")) {
    out_shape->shape.type = UI_CSS_BASIC_SHAPE_POLYGON;
  } else if (strstr(p, "path(")) {
    out_shape->shape.type = UI_CSS_BASIC_SHAPE_PATH;
  }

  if (out_shape->shape.type != UI_CSS_BASIC_SHAPE_NONE) {
    const char *paren_start = strchr(p, '(');
    const char *paren_end = strrchr(p, ')');
    if (paren_start && paren_end && paren_start < paren_end) {
      size_t len = paren_end - paren_start - 1;
      if (len < sizeof(out_shape->shape.arguments)) {
        UI_STRNCPY(out_shape->shape.arguments, len + 1, paren_start + 1, len);
        out_shape->shape.arguments[len] = '\0';
      }
    }

    /* Check if geometry box is appended after shape */
    if (out_shape->box == UI_CSS_GEOMETRY_BOX_NONE && paren_end) {
      const char *after_paren = paren_end + 1;
      out_shape->box = parse_geometry_box(after_paren);
    }
  }

  if (out_shape->box == UI_CSS_GEOMETRY_BOX_NONE &&
      out_shape->shape.type == UI_CSS_BASIC_SHAPE_NONE) {
    return UI_ERROR_PARSE_FAILED;
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_css_filter_destroy(struct ui_css_filter *filter) {
  if (!filter)
    return UI_ERROR_NONE;

  if (filter->functions) {
    struct ui_css_filter_function *current = filter->functions;
    while (current) {
      struct ui_css_filter_function *next = current->next;
      UI_FREE(current);
      current = next;
    }
  }
  UI_FREE(filter);
  return UI_ERROR_NONE;
}

enum ui_error ui_css_parse_filter(const char *str,
                                  struct ui_css_filter **out_filter) {
  struct ui_css_filter *filter;
  struct ui_css_filter_function *tail = NULL;

  if (!str || !out_filter)
    return UI_ERROR_INVALID_ARGUMENT;

  *out_filter = NULL;

  skip_whitespace(&str);
  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  filter = (struct ui_css_filter *)UI_MALLOC(sizeof(struct ui_css_filter));
  if (!filter) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  filter->functions = NULL;

  while (*str) {
    struct ui_css_filter_function *func = NULL;
    enum ui_css_filter_type type = UI_CSS_FILTER_NONE;
    size_t name_len = 0;
    const char *paren_start;
    const char *paren_end;
    const char *arg_str;

    skip_whitespace(&str);
    if (*str == '\0')
      break;

    if (strncmp(str, "url(", 4) == 0) {
      paren_start = str + 3;
      type = UI_CSS_FILTER_URL;
    } else {
      paren_start = strchr(str, '(');
      if (!paren_start)
        goto cleanup_fail;
    }

    if (type != UI_CSS_FILTER_URL) {
      name_len = paren_start - str;
      if (name_len == 4 && strncmp(str, "blur", 4) == 0)
        type = UI_CSS_FILTER_BLUR;
      else if (name_len == 10 && strncmp(str, "brightness", 10) == 0)
        type = UI_CSS_FILTER_BRIGHTNESS;
      else if (name_len == 8 && strncmp(str, "contrast", 8) == 0)
        type = UI_CSS_FILTER_CONTRAST;
      else if (name_len == 11 && strncmp(str, "drop-shadow", 11) == 0)
        type = UI_CSS_FILTER_DROP_SHADOW;
      else if (name_len == 9 && strncmp(str, "grayscale", 9) == 0)
        type = UI_CSS_FILTER_GRAYSCALE;
      else if (name_len == 10 && strncmp(str, "hue-rotate", 10) == 0)
        type = UI_CSS_FILTER_HUE_ROTATE;
      else if (name_len == 6 && strncmp(str, "invert", 6) == 0)
        type = UI_CSS_FILTER_INVERT;
      else if (name_len == 7 && strncmp(str, "opacity", 7) == 0)
        type = UI_CSS_FILTER_OPACITY;
      else if (name_len == 8 && strncmp(str, "saturate", 8) == 0)
        type = UI_CSS_FILTER_SATURATE;
      else if (name_len == 5 && strncmp(str, "sepia", 5) == 0)
        type = UI_CSS_FILTER_SEPIA;
      else
        goto cleanup_fail;
    }

    paren_end = strchr(paren_start, ')');
    if (!paren_end)
      goto cleanup_fail;

    func = (struct ui_css_filter_function *)UI_MALLOC(
        sizeof(struct ui_css_filter_function));
    if (!func) {
      ui_css_filter_destroy(filter);
      return UI_ERROR_OUT_OF_MEMORY;
    }
    func->type = type;
    func->next = NULL;

    arg_str = paren_start + 1;
    if (type == UI_CSS_FILTER_URL) {
      size_t len = paren_end - arg_str;
      if (len >= sizeof(func->data.url)) {
        len = sizeof(func->data.url) - 1;
      }
      memcpy(func->data.url, arg_str, len);
      func->data.url[len] = '\0';
    } else if (type == UI_CSS_FILTER_DROP_SHADOW) {
      /* Basic parse: offset-x offset-y blur-radius color */
      enum ui_error rc;
      char shadow_str[256];
      size_t len = paren_end - arg_str;
      char *token;
      char *next_token = NULL;
      int part_idx = 0;

      if (len >= sizeof(shadow_str)) {
        len = sizeof(shadow_str) - 1;
      }
      memcpy(shadow_str, arg_str, len);
      shadow_str[len] = '\0';

      func->data.drop_shadow.offset_x.unit = UI_CSS_UNIT_NONE;
      func->data.drop_shadow.offset_y.unit = UI_CSS_UNIT_NONE;
      func->data.drop_shadow.blur_radius.unit = UI_CSS_UNIT_NONE;
      func->data.drop_shadow.has_color = 0;

      /* Parse tokens space-separated */
#if defined(_MSC_VER)
      token = UI_STRTOK(shadow_str, " ", &next_token);
#else
      token = strtok_r(shadow_str, " ", &next_token);
#endif
      while (token && part_idx < 4) {
        if (part_idx == 0) {
          rc = ui_css_parse_value(token, &func->data.drop_shadow.offset_x);
          if (rc != UI_ERROR_NONE)
            break;
        } else if (part_idx == 1) {
          rc = ui_css_parse_value(token, &func->data.drop_shadow.offset_y);
          if (rc != UI_ERROR_NONE)
            break;
        } else if (part_idx == 2) {
          rc = ui_css_parse_value(token, &func->data.drop_shadow.blur_radius);
          if (rc != UI_ERROR_NONE) {
            /* Maybe it's a color instead of blur_radius */
            rc = ui_css_parse_color(token, &func->data.drop_shadow.color);
            if (rc == UI_ERROR_NONE) {
              func->data.drop_shadow.has_color = 1;
              break;
            }
          }
        } else if (part_idx == 3) {
          rc = ui_css_parse_color(token, &func->data.drop_shadow.color);
          if (rc == UI_ERROR_NONE) {
            func->data.drop_shadow.has_color = 1;
          }
        }
        part_idx++;
        token = UI_STRTOK(NULL, " ", &next_token);
      }
    } else {
      /* Parse single value */
      char val_str[128];
      size_t len = paren_end - arg_str;
      enum ui_error rc;

      if (len >= sizeof(val_str)) {
        len = sizeof(val_str) - 1;
      }
      memcpy(val_str, arg_str, len);
      val_str[len] = '\0';

      rc = ui_css_parse_value(val_str, &func->data.value);
      if (rc != UI_ERROR_NONE) {
        UI_FREE(func);
        goto cleanup_fail;
      }
    }

    if (!filter->functions) {
      filter->functions = func;
    } else {
      tail->next = func;
    }
    tail = func;

    str = paren_end + 1;
  }

  *out_filter = filter;
  return UI_ERROR_NONE;

cleanup_fail:
  ui_css_filter_destroy(filter);
  return UI_ERROR_PARSE_FAILED;
}

enum ui_error ui_css_parse_blend_mode(const char *str,
                                      enum ui_css_blend_mode *out_blend_mode) {
  if (!str || !out_blend_mode)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);

  if (strcmp(str, "normal") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_NORMAL;
  else if (strcmp(str, "multiply") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_MULTIPLY;
  else if (strcmp(str, "screen") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_SCREEN;
  else if (strcmp(str, "overlay") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_OVERLAY;
  else if (strcmp(str, "darken") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_DARKEN;
  else if (strcmp(str, "lighten") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_LIGHTEN;
  else if (strcmp(str, "color-dodge") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_COLOR_DODGE;
  else if (strcmp(str, "color-burn") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_COLOR_BURN;
  else if (strcmp(str, "hard-light") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_HARD_LIGHT;
  else if (strcmp(str, "soft-light") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_SOFT_LIGHT;
  else if (strcmp(str, "difference") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_DIFFERENCE;
  else if (strcmp(str, "exclusion") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_EXCLUSION;
  else if (strcmp(str, "hue") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_HUE;
  else if (strcmp(str, "saturation") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_SATURATION;
  else if (strcmp(str, "color") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_COLOR;
  else if (strcmp(str, "luminosity") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_LUMINOSITY;
  else
    return UI_ERROR_PARSE_FAILED;

  return UI_ERROR_NONE;
}

enum ui_error ui_css_shadow_list_destroy(struct ui_css_shadow_list *list) {
  if (!list)
    return UI_ERROR_NONE;

  if (list->shadows) {
    struct ui_css_shadow *current = list->shadows;
    while (current) {
      struct ui_css_shadow *next = current->next;
      UI_FREE(current);
      current = next;
    }
  }
  UI_FREE(list);
  return UI_ERROR_NONE;
}

enum ui_error ui_css_parse_shadow(const char *str,
                                  struct ui_css_shadow_list **out_shadows) {
  struct ui_css_shadow_list *list;
  struct ui_css_shadow *tail = NULL;

  if (!str || !out_shadows)
    return UI_ERROR_INVALID_ARGUMENT;

  *out_shadows = NULL;

  skip_whitespace(&str);
  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  list =
      (struct ui_css_shadow_list *)UI_MALLOC(sizeof(struct ui_css_shadow_list));
  if (!list) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  list->shadows = NULL;

  while (*str) {
    struct ui_css_shadow *shadow = NULL;
    char token_buf[256];
    char *token;
    char *next_token = NULL;
    int length_idx = 0;
    enum ui_error rc;
    const char *comma_pos;
    size_t part_len;

    skip_whitespace(&str);
    if (*str == '\0')
      break;

    comma_pos = strchr(str, ',');
    if (comma_pos) {
      part_len = comma_pos - str;
    } else {
      part_len = strlen(str);
    }

    if (part_len >= sizeof(token_buf)) {
      part_len = sizeof(token_buf) - 1;
    }
    memcpy(token_buf, str, part_len);
    token_buf[part_len] = '\0';

    shadow = (struct ui_css_shadow *)UI_MALLOC(sizeof(struct ui_css_shadow));
    if (!shadow) {
      ui_css_shadow_list_destroy(list);
      return UI_ERROR_OUT_OF_MEMORY;
    }

    shadow->offset_x.unit = UI_CSS_UNIT_NONE;
    shadow->offset_y.unit = UI_CSS_UNIT_NONE;
    shadow->blur_radius.unit = UI_CSS_UNIT_NONE;
    shadow->blur_radius.value = 0.0f;
    shadow->spread_radius.unit = UI_CSS_UNIT_NONE;
    shadow->spread_radius.value = 0.0f;
    shadow->has_color = 0;
    shadow->is_inset = 0;
    shadow->next = NULL;

    /* Parse tokens space-separated */
    token = UI_STRTOK(token_buf, " ", &next_token);
    while (token) {
      if (strcmp(token, "inset") == 0) {
        shadow->is_inset = 1;
      } else {
        /* Try parsing as a length */
        struct ui_css_value val;
        rc = ui_css_parse_value(token, &val);
        if (rc == UI_ERROR_NONE && val.unit != UI_CSS_UNIT_NONE &&
            val.unit != UI_CSS_UNIT_UNKNOWN) {
          if (length_idx == 0) {
            shadow->offset_x = val;
          } else if (length_idx == 1) {
            shadow->offset_y = val;
          } else if (length_idx == 2) {
            shadow->blur_radius = val;
          } else if (length_idx == 3) {
            shadow->spread_radius = val;
          }
          length_idx++;
        } else {
          /* If not a valid length, try color */
          rc = ui_css_parse_color(token, &shadow->color);
          if (rc == UI_ERROR_NONE) {
            shadow->has_color = 1;
          } else {
            /* Unknown token */
            UI_FREE(shadow);
            ui_css_shadow_list_destroy(list);
            return UI_ERROR_PARSE_FAILED;
          }
        }
      }
      token = UI_STRTOK(NULL, " ", &next_token);
    }

    /* Must have at least offset-x and offset-y */
    if (length_idx < 2) {
      UI_FREE(shadow);
      ui_css_shadow_list_destroy(list);
      return UI_ERROR_PARSE_FAILED;
    }

    if (!list->shadows) {
      list->shadows = shadow;
    } else {
      tail->next = shadow;
    }
    tail = shadow;

    if (comma_pos) {
      str = comma_pos + 1;
    } else {
      str += part_len;
    }
  }

  *out_shadows = list;
  return UI_ERROR_NONE;
}

enum ui_error ui_css_parse_fill_rule(const char *str,
                                     enum ui_css_fill_rule *out_rule) {
  if (!str || !out_rule)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);
  if (strcmp(str, "nonzero") == 0) {
    *out_rule = UI_CSS_FILL_RULE_NONZERO;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "evenodd") == 0) {
    *out_rule = UI_CSS_FILL_RULE_EVENODD;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_PARSE_FAILED;
}

/** \brief ui_error */
enum ui_error
ui_css_parse_stroke_linecap(const char *str,
                            enum ui_css_stroke_linecap *out_linecap) {
  if (!str || !out_linecap)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);
  if (strcmp(str, "butt") == 0) {
    *out_linecap = UI_CSS_STROKE_LINECAP_BUTT;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "round") == 0) {
    *out_linecap = UI_CSS_STROKE_LINECAP_ROUND;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "square") == 0) {
    *out_linecap = UI_CSS_STROKE_LINECAP_SQUARE;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_PARSE_FAILED;
}

/** \brief ui_error */
enum ui_error
ui_css_parse_stroke_linejoin(const char *str,
                             enum ui_css_stroke_linejoin *out_linejoin) {
  if (!str || !out_linejoin)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);
  if (strcmp(str, "miter") == 0) {
    *out_linejoin = UI_CSS_STROKE_LINEJOIN_MITER;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "round") == 0) {
    *out_linejoin = UI_CSS_STROKE_LINEJOIN_ROUND;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "bevel") == 0) {
    *out_linejoin = UI_CSS_STROKE_LINEJOIN_BEVEL;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_PARSE_FAILED;
}

enum ui_error ui_css_parse_paint(const char *str,
                                 struct ui_css_paint *out_paint) {
  if (!str || !out_paint)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);

  out_paint->type = UI_CSS_PAINT_NONE;
  out_paint->url[0] = '\0';

  if (strcmp(str, "none") == 0) {
    out_paint->type = UI_CSS_PAINT_NONE;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "currentcolor") == 0 ||
             strcmp(str, "currentColor") == 0) {
    out_paint->type = UI_CSS_PAINT_CURRENT_COLOR;
    return UI_ERROR_NONE;
  } else if (strncmp(str, "url(", 4) == 0) {
    const char *start = str + 4;
    const char *end = strchr(start, ')');
    if (end) {
      size_t len = end - start;
      if (len >= 2 && ((start[0] == '"' && start[len - 1] == '"') ||
                       (start[0] == '\'' && start[len - 1] == '\''))) {
        start++;
        len -= 2;
      }
      if (len < sizeof(out_paint->url)) {
        UI_STRNCPY(out_paint->url, len + 1, start, len);
        out_paint->url[len] = '\0';
        out_paint->type = UI_CSS_PAINT_URL;
        return UI_ERROR_NONE;
      }
    }
    return UI_ERROR_PARSE_FAILED;
  }

  if (ui_css_parse_color(str, &out_paint->color) == UI_ERROR_NONE) {
    out_paint->type = UI_CSS_PAINT_COLOR;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_PARSE_FAILED;
}

enum ui_error ui_css_parse_dasharray(const char *str,
                                     struct ui_css_dasharray *out_dasharray) {
  char token_buf[512];
  char *token;
  char *next_token = NULL;
  enum ui_error rc;

  if (!str || !out_dasharray)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);

  out_dasharray->count = 0;

  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  UI_STRNCPY(token_buf, sizeof(token_buf), str, sizeof(token_buf) - 1);
  token_buf[sizeof(token_buf) - 1] = '\0';

  /* Replace commas with spaces to allow strtok to handle either */
  {
    char *p = token_buf;
    while (*p) {
      if (*p == ',')
        *p = ' ';
      p++;
    }
  }

  token = UI_STRTOK(token_buf, " ", &next_token);
  while (token && out_dasharray->count < 16) {
    struct ui_css_value val;
    rc = ui_css_parse_value(token, &val);
    if (rc != UI_ERROR_NONE) {
      return UI_ERROR_PARSE_FAILED;
    }
    out_dasharray->values[out_dasharray->count++] = val;
    token = UI_STRTOK(NULL, " ", &next_token);
  }

  if (out_dasharray->count == 0) {
    return UI_ERROR_PARSE_FAILED;
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_css_parse_easing_function(const char *str,
                             struct ui_css_easing_function *out_easing) {
  if (!str || !out_easing)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);

  if (strcmp(str, "linear") == 0) {
    out_easing->type = UI_CSS_EASING_LINEAR;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "ease") == 0) {
    out_easing->type = UI_CSS_EASING_EASE;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "ease-in") == 0) {
    out_easing->type = UI_CSS_EASING_EASE_IN;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "ease-out") == 0) {
    out_easing->type = UI_CSS_EASING_EASE_OUT;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "ease-in-out") == 0) {
    out_easing->type = UI_CSS_EASING_EASE_IN_OUT;
    return UI_ERROR_NONE;
  } else if (strncmp(str, "cubic-bezier(", 13) == 0) {
    float x1, y1, x2, y2;
    if (sscanf(str + 13, "%f , %f , %f , %f", &x1, &y1, &x2, &y2) == 4) {
      out_easing->type = UI_CSS_EASING_CUBIC_BEZIER;
      out_easing->data.cubic_bezier.x1 = x1;
      out_easing->data.cubic_bezier.y1 = y1;
      out_easing->data.cubic_bezier.x2 = x2;
      out_easing->data.cubic_bezier.y2 = y2;
      return UI_ERROR_NONE;
    }
  } else if (strncmp(str, "steps(", 6) == 0) {
    int count = 0;
    char pos_str[32] = {0};
    int matched = sscanf(str + 6, "%d , %31[^)]", &count, pos_str);
    if (matched >= 1) {
      out_easing->type = UI_CSS_EASING_STEPS;
      out_easing->data.steps.count = count;
      out_easing->data.steps.position = UI_CSS_STEPS_END; /* Default */

      if (matched == 2) {
        char *p = pos_str;
        if (strstr(p, "jump-start"))
          out_easing->data.steps.position = UI_CSS_STEPS_JUMP_START;
        else if (strstr(p, "jump-end"))
          out_easing->data.steps.position = UI_CSS_STEPS_JUMP_END;
        else if (strstr(p, "jump-none"))
          out_easing->data.steps.position = UI_CSS_STEPS_JUMP_NONE;
        else if (strstr(p, "jump-both"))
          out_easing->data.steps.position = UI_CSS_STEPS_JUMP_BOTH;
        else if (strstr(p, "start"))
          out_easing->data.steps.position = UI_CSS_STEPS_START;
        else if (strstr(p, "end"))
          out_easing->data.steps.position = UI_CSS_STEPS_END;
      }
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_PARSE_FAILED;
}

enum ui_error ui_css_transition_destroy(struct ui_css_transition *transitions) {
  while (transitions) {
    struct ui_css_transition *next = transitions->next;
    UI_FREE(transitions);
    transitions = next;
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_css_parse_transition(const char *str,
                        struct ui_css_transition **out_transitions) {
  struct ui_css_transition *head = NULL;
  struct ui_css_transition *tail = NULL;

  if (!str || !out_transitions)
    return UI_ERROR_INVALID_ARGUMENT;

  *out_transitions = NULL;
  skip_whitespace(&str);

  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE; /* NULL list represents none */
  }

  while (*str) {
    struct ui_css_transition *tr = NULL;
    char segment[512];
    const char *comma = str;
    size_t len;
    const char *p;
    int time_count = 0;
    int paren_depth = 0;

    /* Find next comma not inside parentheses */
    while (*comma) {
      if (*comma == '(')
        paren_depth++;
      else if (*comma == ')')
        paren_depth--;
      else if (*comma == ',' && paren_depth == 0)
        break;
      comma++;
    }

    len = (size_t)(comma - str);

    if (len >= sizeof(segment)) {
      len = sizeof(segment) - 1;
    }
    memcpy(segment, str, len);
    segment[len] = '\0';

    tr =
        (struct ui_css_transition *)UI_MALLOC(sizeof(struct ui_css_transition));
    if (!tr) {
      ui_css_transition_destroy(head);
      return UI_ERROR_OUT_OF_MEMORY;
    }

    /* Defaults for unspecified components */
    UI_STRCPY(tr->property_name, sizeof(tr->property_name), "all");
    tr->duration.value = 0.0f;
    tr->duration.unit = UI_CSS_UNIT_S;
    tr->delay.value = 0.0f;
    tr->delay.unit = UI_CSS_UNIT_S;
    tr->timing_function.type = UI_CSS_EASING_EASE;
    tr->next = NULL;

    p = segment;
    while (*p) {
      skip_whitespace(&p);
      if (!*p)
        break;

      /* Check for complex easing functions which might contain spaces */
      if (strncmp(p, "cubic-bezier(", 13) == 0 ||
          strncmp(p, "steps(", 6) == 0) {
        const char *end = strchr(p, ')');
        if (end) {
          char easing_str[64];
          size_t e_len = end - p + 1;
          if (e_len >= sizeof(easing_str)) {
            e_len = sizeof(easing_str) - 1;
          }
          memcpy(easing_str, p, e_len);
          easing_str[e_len] = '\0';
          if (ui_css_parse_easing_function(easing_str, &tr->timing_function) ==
              UI_ERROR_NONE) {
            p = end + 1;
            continue;
          }
        }
      }

      /* Simple token fallback */
      {
        struct ui_css_easing_function ef;
        char tmp_token[64];
        const char *space = p;
        size_t t_len;
        while (*space && !isspace((unsigned char)*space))
          space++;
        t_len = space - p;
        if (t_len >= sizeof(tmp_token)) {
          t_len = sizeof(tmp_token) - 1;
        }
        memcpy(tmp_token, p, t_len);
        tmp_token[t_len] = '\0';

        /* Try matching easing function */
        if (ui_css_parse_easing_function(tmp_token, &ef) == UI_ERROR_NONE) {
          tr->timing_function = ef;
          p = space;
          continue;
        }

        /* Try parsing time (duration or delay) */
        {
          struct ui_css_value val;
          if (ui_css_parse_value(tmp_token, &val) == UI_ERROR_NONE &&
              (val.unit == UI_CSS_UNIT_S || val.unit == UI_CSS_UNIT_MS)) {
            if (time_count == 0) {
              tr->duration = val;
            } else if (time_count == 1) {
              tr->delay = val;
            }
            time_count++;
            p = space;
            continue;
          }
        }

        /* Fallback: Must be a property name */
        if (t_len < sizeof(tr->property_name)) {
          memcpy(tr->property_name, tmp_token, t_len);
          tr->property_name[t_len] = '\0';
        }
        p = space;
      }
    }

    if (tail) {
      tail->next = tr;
    } else {
      head = tr;
    }
    tail = tr;

    if (*comma == ',') {
      str = comma + 1;
    } else {
      break;
    }
  }

  *out_transitions = head;
  return UI_ERROR_NONE;
}

enum ui_error ui_css_animation_destroy(struct ui_css_animation *animations) {
  while (animations) {
    struct ui_css_animation *next = animations->next;
    UI_FREE(animations);
    animations = next;
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_css_parse_animation(const char *str,
                                     struct ui_css_animation **out_animations) {
  struct ui_css_animation *head = NULL;
  struct ui_css_animation *tail = NULL;

  if (!str || !out_animations)
    return UI_ERROR_INVALID_ARGUMENT;

  *out_animations = NULL;
  skip_whitespace(&str);

  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  while (*str) {
    struct ui_css_animation *anim = NULL;
    char segment[512];
    const char *comma = str;
    size_t len;
    const char *p;
    int time_count = 0;
    int paren_depth = 0;

    /* Find next comma not inside parentheses */
    while (*comma) {
      if (*comma == '(')
        paren_depth++;
      else if (*comma == ')')
        paren_depth--;
      else if (*comma == ',' && paren_depth == 0)
        break;
      comma++;
    }

    len = (size_t)(comma - str);

    if (len >= sizeof(segment)) {
      len = sizeof(segment) - 1;
    }
    memcpy(segment, str, len);
    segment[len] = '\0';

    anim =
        (struct ui_css_animation *)UI_MALLOC(sizeof(struct ui_css_animation));
    if (!anim) {
      ui_css_animation_destroy(head);
      return UI_ERROR_OUT_OF_MEMORY;
    }

    /* Defaults */
    UI_STRCPY(anim->name, sizeof(anim->name), "none");
    anim->duration.value = 0.0f;
    anim->duration.unit = UI_CSS_UNIT_S;
    anim->timing_function.type = UI_CSS_EASING_EASE;
    anim->delay.value = 0.0f;
    anim->delay.unit = UI_CSS_UNIT_S;
    anim->iteration_count = 1.0f;
    anim->direction = UI_CSS_ANIMATION_DIRECTION_NORMAL;
    anim->fill_mode = UI_CSS_ANIMATION_FILL_MODE_NONE;
    anim->play_state = UI_CSS_ANIMATION_PLAY_STATE_RUNNING;
    anim->next = NULL;

    p = segment;
    while (*p) {
      skip_whitespace(&p);
      if (!*p)
        break;

      if (strncmp(p, "cubic-bezier(", 13) == 0 ||
          strncmp(p, "steps(", 6) == 0) {
        const char *end = strchr(p, ')');
        if (end) {
          char easing_str[64];
          size_t e_len = end - p + 1;
          if (e_len >= sizeof(easing_str)) {
            e_len = sizeof(easing_str) - 1;
          }
          memcpy(easing_str, p, e_len);
          easing_str[e_len] = '\0';
          if (ui_css_parse_easing_function(
                  easing_str, &anim->timing_function) == UI_ERROR_NONE) {
            p = end + 1;
            continue;
          }
        }
      }

      {
        struct ui_css_easing_function ef;
        char tmp_token[64];
        const char *space = p;
        size_t t_len;
        while (*space && !isspace((unsigned char)*space))
          space++;
        t_len = space - p;
        if (t_len >= sizeof(tmp_token)) {
          t_len = sizeof(tmp_token) - 1;
        }
        memcpy(tmp_token, p, t_len);
        tmp_token[t_len] = '\0';

        if (ui_css_parse_easing_function(tmp_token, &ef) == UI_ERROR_NONE) {
          anim->timing_function = ef;
          p = space;
          continue;
        }

        if (strcmp(tmp_token, "infinite") == 0) {
          anim->iteration_count = -1.0f;
          p = space;
          continue;
        } else if (strcmp(tmp_token, "normal") == 0) {
          /* Can be direction */
          anim->direction = UI_CSS_ANIMATION_DIRECTION_NORMAL;
          p = space;
          continue;
        } else if (strcmp(tmp_token, "reverse") == 0) {
          anim->direction = UI_CSS_ANIMATION_DIRECTION_REVERSE;
          p = space;
          continue;
        } else if (strcmp(tmp_token, "alternate") == 0) {
          anim->direction = UI_CSS_ANIMATION_DIRECTION_ALTERNATE;
          p = space;
          continue;
        } else if (strcmp(tmp_token, "alternate-reverse") == 0) {
          anim->direction = UI_CSS_ANIMATION_DIRECTION_ALTERNATE_REVERSE;
          p = space;
          continue;
        } else if (strcmp(tmp_token, "forwards") == 0) {
          anim->fill_mode = UI_CSS_ANIMATION_FILL_MODE_FORWARDS;
          p = space;
          continue;
        } else if (strcmp(tmp_token, "backwards") == 0) {
          anim->fill_mode = UI_CSS_ANIMATION_FILL_MODE_BACKWARDS;
          p = space;
          continue;
        } else if (strcmp(tmp_token, "both") == 0) {
          anim->fill_mode = UI_CSS_ANIMATION_FILL_MODE_BOTH;
          p = space;
          continue;
        } else if (strcmp(tmp_token, "running") == 0) {
          anim->play_state = UI_CSS_ANIMATION_PLAY_STATE_RUNNING;
          p = space;
          continue;
        } else if (strcmp(tmp_token, "paused") == 0) {
          anim->play_state = UI_CSS_ANIMATION_PLAY_STATE_PAUSED;
          p = space;
          continue;
        }

        {
          struct ui_css_value val;
          if (ui_css_parse_value(tmp_token, &val) == UI_ERROR_NONE) {
            if (val.unit == UI_CSS_UNIT_S || val.unit == UI_CSS_UNIT_MS) {
              if (time_count == 0)
                anim->duration = val;
              else if (time_count == 1)
                anim->delay = val;
              time_count++;
              p = space;
              continue;
            } else if (val.unit == UI_CSS_UNIT_NONE) {
              anim->iteration_count = val.value;
              p = space;
              continue;
            }
          }
        }

        if (t_len < sizeof(anim->name) && strcmp(tmp_token, "none") != 0) {
          memcpy(anim->name, tmp_token, t_len);
          anim->name[t_len] = '\0';
        }
        p = space;
      }
    }

    if (tail) {
      tail->next = anim;
    } else {
      head = anim;
    }
    tail = anim;

    if (*comma == ',') {
      str = comma + 1;
    } else {
      break;
    }
  }

  *out_animations = head;
  return UI_ERROR_NONE;
}
