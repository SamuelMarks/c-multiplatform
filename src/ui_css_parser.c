/**
 * @file ui_css_parser.c
 * @brief ui_css_parser.c implementation.
 */
/* clang-format off */
#include "../include/ui_css_parser.h"
#include "../include/ui_error.h"
#include "ui_internal_mem.h"
#include <string.h>
#include <ctype.h>
/* clang-format on */

/** @brief internal */
enum ui_css_token_type {
  TOKEN_EOF,             /**< End of file */
  TOKEN_IDENT,           /**< Identifier */
  TOKEN_HASH,            /**< Hash (#) */
  TOKEN_DOT,             /**< Dot (.) */
  TOKEN_COLON,           /**< Colon (:) */
  TOKEN_SEMICOLON,       /**< Semicolon (;) */
  TOKEN_COMMA,           /**< Comma (,) */
  TOKEN_LBRACE,          /**< Left brace ({) */
  TOKEN_RBRACE,          /**< Right brace (}) */
  TOKEN_LBRACKET,        /**< Left bracket ([) */
  TOKEN_RBRACKET,        /**< Right bracket (]) */
  TOKEN_LPAREN,          /**< Left parenthesis (() */
  TOKEN_RPAREN,          /**< Right parenthesis ()) */
  TOKEN_EQUALS,          /**< Equals (=) */
  TOKEN_PREFIX_MATCH,    /**< Prefix match (^=) */
  TOKEN_SUFFIX_MATCH,    /**< Suffix match ($=) */
  TOKEN_SUBSTRING_MATCH, /**< Substring match (*=) */
  TOKEN_DASH_MATCH,      /**< Dash match (|=) */
  TOKEN_INCLUDE_MATCH,   /**< Include match (~=) */
  TOKEN_ASTERISK,        /**< Asterisk (*) */
  TOKEN_STRING,          /**< String */
  TOKEN_AT_KEYWORD,      /**< At keyword (@) */
  TOKEN_ERROR            /**< Error */
};

/**
 * @struct ui_css_token
 * \brief ui_css_token
 */
struct ui_css_token {
  enum ui_css_token_type type; /**< type */
  const char *start;           /**< start */
  size_t length;               /**< length */
};

/**
 * @struct ui_css_tokenizer
 * \brief ui_css_tokenizer
 */
struct ui_css_tokenizer {
  const char *text; /**< text */
  size_t pos;       /**< pos */
  size_t len;       /**< len */
};

/**
 * @brief advance.
 * @param tz Parameter tz.
 * @return Return value.
 */
static void advance(struct ui_css_tokenizer *tz) { tz->pos++; }

/**
 * @brief peek.
 * @param tz Parameter tz.
 * @param out_char Parameter out_char.
 * @return Return value.
 */
static void peek(struct ui_css_tokenizer *tz, char *out_char) {
  *out_char = tz->text[tz->pos];
}

/**
 * @brief is_ident_start.
 * @param c Parameter c.
 * @param out_is_start Parameter out_is_start.
 * @return Return value.
 */
static void is_ident_start(char c, int *out_is_start) {
  *out_is_start = isalpha((unsigned char)c) || c == '_' || c == '-';
}

/**
 * @brief is_ident_char.
 * @param c Parameter c.
 * @param out_is_char Parameter out_is_char.
 * @return Return value.
 */
static void is_ident_char(char c, int *out_is_char) {
  *out_is_char = isalnum((unsigned char)c) || c == '_' || c == '-';
}

/**
 * @brief skip_whitespace_and_comments.
 * @param tz Parameter tz.
 * @return Return value.
 */
static void skip_whitespace_and_comments(struct ui_css_tokenizer *tz) {
  char c;
  while (tz->pos < tz->len) {
    peek(tz, &c);
    if (isspace((unsigned char)c)) {
      advance(tz);
    } else if (c == '/' && tz->pos + 1 < tz->len &&
               tz->text[tz->pos + 1] == '*') {
      advance(tz);
      advance(tz);
      while (tz->pos < tz->len) {
        peek(tz, &c);
        if (c == '*' && tz->pos + 1 < tz->len && tz->text[tz->pos + 1] == '/') {
          advance(tz);
          advance(tz);
          break;
        }
        advance(tz);
      }
    } else {
      break;
    }
  }
}

/**
 * @brief next_token.
 * @param tz Parameter tz.
 * @param out_token Parameter out_token.
 * @return Return value.
 */
static void next_token(struct ui_css_tokenizer *tz,
                       struct ui_css_token *out_token) {
  struct ui_css_token t;
  char c;

  skip_whitespace_and_comments(tz);

  t.start = tz->text + tz->pos;
  t.length = 0;

  if (tz->pos >= tz->len) {
    t.type = TOKEN_EOF;
    *out_token = t;
    return;
  }

  peek(tz, &c);

  switch (c) {
  case '{':
    advance(tz);
    t.type = TOKEN_LBRACE;
    t.length = 1;
    *out_token = t;
    return;
  case '}':
    advance(tz);
    t.type = TOKEN_RBRACE;
    t.length = 1;
    *out_token = t;
    return;
  case '[':
    advance(tz);
    t.type = TOKEN_LBRACKET;
    t.length = 1;
    *out_token = t;
    return;
  case ']':
    advance(tz);
    t.type = TOKEN_RBRACKET;
    t.length = 1;
    *out_token = t;
    return;
  case '(':
    advance(tz);
    t.type = TOKEN_LPAREN;
    t.length = 1;
    *out_token = t;
    return;
  case ')':
    advance(tz);
    t.type = TOKEN_RPAREN;
    t.length = 1;
    *out_token = t;
    return;
  case ':':
    advance(tz);
    t.type = TOKEN_COLON;
    t.length = 1;
    *out_token = t;
    return;
  case ';':
    advance(tz);
    t.type = TOKEN_SEMICOLON;
    t.length = 1;
    *out_token = t;
    return;
  case ',':
    advance(tz);
    t.type = TOKEN_COMMA;
    t.length = 1;
    *out_token = t;
    return;
  case '.':
    advance(tz);
    t.type = TOKEN_DOT;
    t.length = 1;
    *out_token = t;
    return;
  case '#':
    advance(tz);
    t.type = TOKEN_HASH;
    t.length = 1;
    *out_token = t;
    return;
  case '*':
    advance(tz);
    peek(tz, &c);
    if (c == '=') {
      advance(tz);
      t.type = TOKEN_SUBSTRING_MATCH;
      t.length = 2;
    } else {
      t.type = TOKEN_ASTERISK;
      t.length = 1;
    }
    *out_token = t;
    return;
  case '=':
    advance(tz);
    t.type = TOKEN_EQUALS;
    t.length = 1;
    *out_token = t;
    return;
  case '^':
    advance(tz);
    peek(tz, &c);
    if (c == '=') {
      advance(tz);
      t.type = TOKEN_PREFIX_MATCH;
      t.length = 2;
      *out_token = t;
      return;
    }
    t.type = TOKEN_ERROR;
    t.length = 1;
    *out_token = t;
    return;
  case '$':
    advance(tz);
    peek(tz, &c);
    if (c == '=') {
      advance(tz);
      t.type = TOKEN_SUFFIX_MATCH;
      t.length = 2;
      *out_token = t;
      return;
    }
    t.type = TOKEN_ERROR;
    t.length = 1;
    *out_token = t;
    return;
  case '|':
    advance(tz);
    peek(tz, &c);
    if (c == '=') {
      advance(tz);
      t.type = TOKEN_DASH_MATCH;
      t.length = 2;
      *out_token = t;
      return;
    }
    t.type = TOKEN_ERROR;
    t.length = 1;
    *out_token = t;
    return;
  case '~':
    advance(tz);
    peek(tz, &c);
    if (c == '=') {
      advance(tz);
      t.type = TOKEN_INCLUDE_MATCH;
      t.length = 2;
      *out_token = t;
      return;
    }
    t.type = TOKEN_ERROR;
    t.length = 1;
    *out_token = t;
    return;
  case '@': {
    int is_start = 0;
    advance(tz);
    peek(tz, &c);
    is_ident_start(c, &is_start);
    if (is_start) {
      while (tz->pos < tz->len) {
        int is_char = 0;
        peek(tz, &c);
        is_ident_char(c, &is_char);
        if (!is_char)
          break;
        advance(tz);
      }
      t.type = TOKEN_AT_KEYWORD;
      t.length = (size_t)((tz->text + tz->pos) - t.start);
      *out_token = t;
      return;
    }
    t.type = TOKEN_ERROR;
    t.length = 1;
    *out_token = t;
    return;
  }
  case '"':
  case '\'': {
    char quote = c;
    advance(tz);
    while (tz->pos < tz->len) {
      peek(tz, &c);
      if (c == quote)
        break;
      if (c == '\\') {
        advance(tz);
      }
      advance(tz);
    }
    if (tz->pos < tz->len)
      advance(tz);
    t.type = TOKEN_STRING;
    t.length = (size_t)((tz->text + tz->pos) - t.start);
    *out_token = t;
    return;
  }
  default: {
    int is_start = 0;
    is_ident_start(c, &is_start);
    if (is_start) {
      while (tz->pos < tz->len) {
        int is_char = 0;
        peek(tz, &c);
        is_ident_char(c, &is_char);
        if (!is_char)
          break;
        advance(tz);
      }
      t.type = TOKEN_IDENT;
      t.length = (size_t)((tz->text + tz->pos) - t.start);
      *out_token = t;
      return;
    } else if (isdigit((unsigned char)c) || c == '+') {
      while (tz->pos < tz->len) {
        peek(tz, &c);
        if (isspace((unsigned char)c) || c == ';' || c == '}' || c == '{')
          break;
        advance(tz);
      }
      t.type = TOKEN_IDENT;
      t.length = (size_t)((tz->text + tz->pos) - t.start);
      *out_token = t;
      return;
    } else {
      advance(tz);
      t.type = TOKEN_ERROR;
      t.length = 1;
      *out_token = t;
      return;
    }
  }
  }
}

/**
 * @brief dup_token_str.
 * @param t Parameter t.
 * @param out_str Parameter out_str.
 * @return Return value.
 */
static ui_error_t dup_token_str(const struct ui_css_token *t, char **out_str) {
  char *s = (char *)C_MULTIPLATFORM_MALLOC(t->length + 1);
  if (!s) {
    *out_str = NULL;
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memcpy(s, t->start, t->length);
  s[t->length] = '\0';
  *out_str = s;
  return UI_ERROR_NONE;
}

/**
 * @brief dup_range_trim.
 * @param start Parameter start.
 * @param end Parameter end.
 * @param out_str Parameter out_str.
 * @return Return value.
 */
static ui_error_t dup_range_trim(const char *start, const char *end,
                                 char **out_str) {
  const char *s = start;
  const char *e = end;
  size_t len;
  char *res;

  /* Leading spaces are usually skipped by the tokenizer,
   * but we trim trailing spaces safely. */
  while (e > s && isspace((unsigned char)*(e - 1)))
    e--;

  len = (size_t)(e - s);
  res = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
  if (!res) {
    *out_str = NULL;
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memcpy(res, s, len);
  res[len] = '\0';
  *out_str = res;
  return UI_ERROR_NONE;
}

/**
 * @brief parse_selectors.
 * @param tz Parameter tz.
 * @param inout_token Parameter inout_token.
 * @param out_selectors Parameter out_selectors.
 * @return Return value.
 */
static ui_error_t parse_selectors(struct ui_css_tokenizer *tz,
                                  struct ui_css_token *inout_token,
                                  struct ui_css_selector **out_selectors) {
  struct ui_css_rule *dummy_rule = NULL;
  struct ui_css_token token = *inout_token;
  ui_error_t rc;

  rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &dummy_rule);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  while (token.type != TOKEN_LBRACE && token.type != TOKEN_RPAREN &&
         token.type != TOKEN_EOF) {
    if (token.type == TOKEN_DOT) {
      next_token(tz, &token);
      if (token.type == TOKEN_IDENT) {
        char *val = NULL;
        rc = dup_token_str(&token, &val);
        if (rc != UI_ERROR_NONE) {
          goto cleanup;
        }
        rc = ui_css_rule_append_selector(dummy_rule, UI_CSS_SELECTOR_TYPE_CLASS,
                                         val);
        if (rc != UI_ERROR_NONE) {
          C_MULTIPLATFORM_FREE(val);
          goto cleanup;
        }
        C_MULTIPLATFORM_FREE(val);
        next_token(tz, &token);
      }
    } else if (token.type == TOKEN_HASH) {
      next_token(tz, &token);
      if (token.type == TOKEN_IDENT) {
        char *val = NULL;
        rc = dup_token_str(&token, &val);
        if (rc != UI_ERROR_NONE) {
          goto cleanup;
        }
        rc = ui_css_rule_append_selector(dummy_rule, UI_CSS_SELECTOR_TYPE_ID,
                                         val);
        if (rc != UI_ERROR_NONE) {
          C_MULTIPLATFORM_FREE(val);
          goto cleanup;
        }
        C_MULTIPLATFORM_FREE(val);
        next_token(tz, &token);
      }
    } else if (token.type == TOKEN_IDENT) {
      char *val = NULL;
      rc = dup_token_str(&token, &val);
      if (rc != UI_ERROR_NONE) {
        goto cleanup;
      }
      rc = ui_css_rule_append_selector(dummy_rule, UI_CSS_SELECTOR_TYPE_TAG,
                                       val);
      if (rc != UI_ERROR_NONE) {
        C_MULTIPLATFORM_FREE(val);
        goto cleanup;
      }
      C_MULTIPLATFORM_FREE(val);
      next_token(tz, &token);
    } else if (token.type == TOKEN_ASTERISK) {
      rc = ui_css_rule_append_selector(dummy_rule,
                                       UI_CSS_SELECTOR_TYPE_UNIVERSAL, "*");
      if (rc != UI_ERROR_NONE) {
        goto cleanup;
      }
      next_token(tz, &token);
    } else if (token.type == TOKEN_LBRACKET) {
      next_token(tz, &token);
      if (token.type == TOKEN_IDENT) {
        char *attr_name = NULL;
        enum ui_css_attr_operator op = UI_CSS_ATTR_OP_NONE;
        char *attr_val = NULL;
        rc = dup_token_str(&token, &attr_name);

        if (rc != UI_ERROR_NONE) {
          goto cleanup;
        }

        next_token(tz, &token);
        if (token.type == TOKEN_EQUALS)
          op = UI_CSS_ATTR_OP_EQUALS;
        else if (token.type == TOKEN_INCLUDE_MATCH)
          op = UI_CSS_ATTR_OP_INCLUDES;
        else if (token.type == TOKEN_DASH_MATCH)
          op = UI_CSS_ATTR_OP_DASH;
        else if (token.type == TOKEN_PREFIX_MATCH)
          op = UI_CSS_ATTR_OP_PREFIX;
        else if (token.type == TOKEN_SUFFIX_MATCH)
          op = UI_CSS_ATTR_OP_SUFFIX;
        else if (token.type == TOKEN_SUBSTRING_MATCH)
          op = UI_CSS_ATTR_OP_SUBSTRING;

        if (op != UI_CSS_ATTR_OP_NONE) {
          next_token(tz, &token);
          if (token.type == TOKEN_IDENT || token.type == TOKEN_STRING) {
            if (token.type == TOKEN_STRING && token.length >= 2) {
              attr_val = (char *)C_MULTIPLATFORM_MALLOC(token.length - 1);
              if (attr_val) {
                memcpy(attr_val, token.start + 1, token.length - 2);
                attr_val[token.length - 2] = '\0';
              }
            } else {
              attr_val = NULL;
              rc = dup_token_str(&token, &attr_val);
              if (rc != UI_ERROR_NONE) {
                C_MULTIPLATFORM_FREE(attr_name);
                goto cleanup;
              }
            }
            next_token(tz, &token);
          }
        }

        if (token.type == TOKEN_RBRACKET) {
          rc = ui_css_rule_append_selector_attr(dummy_rule, attr_name, op,
                                                attr_val);
          if (rc != UI_ERROR_NONE) {
            C_MULTIPLATFORM_FREE(attr_name);
            if (attr_val)
              C_MULTIPLATFORM_FREE(attr_val);
            goto cleanup;
          }
          next_token(tz, &token);
        }

        C_MULTIPLATFORM_FREE(attr_name);
        if (attr_val)
          C_MULTIPLATFORM_FREE(attr_val);
      } else {
        while (token.type != TOKEN_RBRACKET && token.type != TOKEN_EOF &&
               token.type != TOKEN_LBRACE) {
          next_token(tz, &token);
        }
        if (token.type == TOKEN_RBRACKET) {
          next_token(tz, &token);
        }
      }
    } else if (token.type == TOKEN_COLON) {
      next_token(tz, &token);
      if (token.type == TOKEN_COLON) {
        /* Pseudo-element :: */
        next_token(tz, &token);
        if (token.type == TOKEN_IDENT) {
          char *pseudo_val = NULL;
          rc = dup_token_str(&token, &pseudo_val);
          if (rc != UI_ERROR_NONE) {
            goto cleanup;
          }

          rc = ui_css_rule_append_selector(
              dummy_rule, UI_CSS_SELECTOR_TYPE_PSEUDO_ELEMENT, pseudo_val);
          if (rc != UI_ERROR_NONE) {
            C_MULTIPLATFORM_FREE(pseudo_val);
            goto cleanup;
          }
          C_MULTIPLATFORM_FREE(pseudo_val);

          next_token(tz, &token);
        }
      } else if (token.type == TOKEN_IDENT) {
        char *pseudo_val = NULL;
        rc = dup_token_str(&token, &pseudo_val);
        if (rc != UI_ERROR_NONE) {
          goto cleanup;
        }

        /* Before/after were historically single-colon, map them appropriately
           or treat all single-colon as classes. Strictly speaking, CSS2 used
           :before/:after. We'll map them explicitly to PSEUDO_ELEMENT if they
           match. */
        if (strcmp(pseudo_val, "before") == 0 ||
            strcmp(pseudo_val, "after") == 0 ||
            strcmp(pseudo_val, "first-line") == 0 ||
            strcmp(pseudo_val, "first-letter") == 0) {
          rc = ui_css_rule_append_selector(
              dummy_rule, UI_CSS_SELECTOR_TYPE_PSEUDO_ELEMENT, pseudo_val);
          if (rc != UI_ERROR_NONE) {
            C_MULTIPLATFORM_FREE(pseudo_val);
            goto cleanup;
          }
        } else {
          rc = ui_css_rule_append_selector(
              dummy_rule, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS, pseudo_val);
          if (rc != UI_ERROR_NONE) {
            C_MULTIPLATFORM_FREE(pseudo_val);
            goto cleanup;
          }
        }

        next_token(tz, &token);
        if (token.type == TOKEN_LPAREN) {
          if (strcmp(pseudo_val, "is") == 0 ||
              strcmp(pseudo_val, "where") == 0 ||
              strcmp(pseudo_val, "has") == 0 ||
              strcmp(pseudo_val, "not") == 0 ||
              strcmp(pseudo_val, "host") == 0 ||
              strcmp(pseudo_val, "host-context") == 0) {
            struct ui_css_selector *nested = NULL;
            struct ui_css_selector *last;
            next_token(tz, &token);
            rc = parse_selectors(tz, &token, &nested);
            if (rc != UI_ERROR_NONE) {
              C_MULTIPLATFORM_FREE(pseudo_val);
              goto cleanup;
            }
            last = dummy_rule->selectors;
            while (last->next)
              last = last->next;
            last->nested_selector = nested;
            { next_token(tz, &token); }
          } else {
            int paren_depth = 1;
            next_token(tz, &token);
            while (paren_depth > 0 && token.type != TOKEN_EOF) {
              if (token.type == TOKEN_LPAREN)
                paren_depth++;
              if (token.type == TOKEN_RPAREN)
                paren_depth--;
              if (paren_depth > 0) {
                next_token(tz, &token);
              }
            }
            { next_token(tz, &token); }
          }
        }

        C_MULTIPLATFORM_FREE(pseudo_val);
      }
    } else if (token.type == TOKEN_COMMA) {
      next_token(tz, &token);
    } else {
      /* Unknown selector token, advance to recover */
      next_token(tz, &token);
    }
  }

  *out_selectors = dummy_rule->selectors;
  dummy_rule->selectors = NULL; /* Transfer ownership */
  rc = UI_ERROR_NONE;

cleanup: {
  ui_error_t destroy_rc = ui_css_rule_destroy(dummy_rule);
  (void)destroy_rc;
}
  *inout_token = token;
  return rc;
}

/**
 * @brief is_nested_rule.
 * @param tz Parameter tz.
 * @param first_token Parameter first_token.
 * @param out_is_nested Parameter out_is_nested.
 * @return Return value.
 */
static void is_nested_rule(struct ui_css_tokenizer *tz,
                           struct ui_css_token first_token,
                           int *out_is_nested) {
  struct ui_css_tokenizer temp_tz = *tz;
  struct ui_css_token t = first_token;

  *out_is_nested = 0;
  if (t.type == TOKEN_AT_KEYWORD) {
    *out_is_nested = 1;
    return;
  }
  if (t.type == TOKEN_DOT || t.type == TOKEN_HASH || t.type == TOKEN_COLON ||
      t.type == TOKEN_LBRACKET || t.type == TOKEN_ASTERISK ||
      t.type == TOKEN_COMMA) {
    *out_is_nested = 1;
    return;
  }

  while (t.type != TOKEN_EOF && t.type != TOKEN_LBRACE &&
         t.type != TOKEN_SEMICOLON && t.type != TOKEN_RBRACE) {
    next_token(&temp_tz, &t);
  }

  if (t.type == TOKEN_LBRACE) {
    *out_is_nested = 1;
  }
  return;
}

static ui_error_t parse_rule_list(struct ui_css_tokenizer *tz,
                                  struct ui_css_token *inout_token,
                                  struct ui_css_stylesheet *sheet,
                                  struct ui_css_rule **out_rules);

/**
 * @brief parse_rule_list.
 * @param tz Parameter tz.
 * @param inout_token Parameter inout_token.
 * @param sheet Parameter sheet.
 * @param out_rules Parameter out_rules.
 * @return Return value.
 */
static ui_error_t parse_rule_list(struct ui_css_tokenizer *tz,
                                  struct ui_css_token *inout_token,
                                  struct ui_css_stylesheet *sheet,
                                  struct ui_css_rule **out_rules) {
  struct ui_css_token token = *inout_token;
  struct ui_css_rule *rule_head = NULL;
  struct ui_css_rule *rule_tail = NULL;
  ui_error_t rc = UI_ERROR_NONE;

  while (token.type != TOKEN_EOF && token.type != TOKEN_RBRACE) {
    struct ui_css_rule *current_rule = NULL;
    char *layer_name = NULL;
    struct ui_css_rule *layer_rule = NULL;
    int order = 0;
    char *at_name = NULL;

    if (token.type == TOKEN_ERROR) {
      next_token(tz, &token);
      continue;
    }

    if (token.type == TOKEN_AT_KEYWORD) {
      at_name = NULL;
      rc = dup_token_str(&token, &at_name);
      if (rc != UI_ERROR_NONE) {
        goto cleanup;
      }

      if (strcmp(at_name, "@layer") == 0) {
        C_MULTIPLATFORM_FREE(at_name);
        next_token(tz, &token);

        if (token.type == TOKEN_IDENT) {
          layer_name = NULL;
          rc = dup_token_str(&token, &layer_name);
          if (rc != UI_ERROR_NONE)
            return rc;
          next_token(tz, &token);

          if (token.type == TOKEN_COMMA || token.type == TOKEN_SEMICOLON) {
            rc = ui_css_stylesheet_register_layer(sheet, layer_name, &order);
            if (rc != UI_ERROR_NONE)
              return rc;
            C_MULTIPLATFORM_FREE(layer_name);
            while (token.type == TOKEN_COMMA) {
              next_token(tz, &token);
              if (token.type == TOKEN_IDENT) {
                layer_name = NULL;
                rc = dup_token_str(&token, &layer_name);
                if (rc != UI_ERROR_NONE)
                  return rc;
                rc =
                    ui_css_stylesheet_register_layer(sheet, layer_name, &order);
                if (rc != UI_ERROR_NONE) {
                  C_MULTIPLATFORM_FREE(layer_name);
                  goto cleanup;
                }
                C_MULTIPLATFORM_FREE(layer_name);
                next_token(tz, &token);
              }
            }
            if (token.type == TOKEN_SEMICOLON) {
              next_token(tz, &token);
            }
            continue; /* It was a statement, not a block */
          }
        }

        rc = ui_css_rule_create(UI_CSS_RULE_TYPE_LAYER, &layer_rule);
        if (rc != UI_ERROR_NONE) {
          C_MULTIPLATFORM_FREE(layer_name);
          goto cleanup;
        }

        layer_rule->layer_name = layer_name; /* Ownership transfer */
        if (layer_name) {
          rc = ui_css_stylesheet_register_layer(sheet, layer_name, &order);
          if (rc != UI_ERROR_NONE) {
            C_MULTIPLATFORM_FREE(layer_name);
            goto cleanup;
          }
        }

        if (token.type == TOKEN_LBRACE) {
          next_token(tz, &token);
          rc = parse_rule_list(tz, &token, sheet, &layer_rule->nested_rules);
          if (rc != UI_ERROR_NONE) {
            {
              ui_error_t destroy_rc = ui_css_rule_destroy(layer_rule);
              (void)destroy_rc;
            }
            goto cleanup;
          }
          if (token.type == TOKEN_RBRACE) {
            next_token(tz, &token);
          }
        }

        if (!rule_head)
          rule_head = layer_rule;
        else
          rule_tail->next = layer_rule;
        rule_tail = layer_rule;
        continue;
      } else if (strcmp(at_name, "@scope") == 0) {
        struct ui_css_rule *scope_rule = NULL;
        struct ui_css_selector *scope_start = NULL;
        struct ui_css_selector *scope_end = NULL;

        C_MULTIPLATFORM_FREE(at_name);
        next_token(tz, &token);

        /* Parse scope start if present. Wait, it could be `@scope {` */
        if (token.type == TOKEN_LPAREN) {
          rc = parse_selectors(tz, &token, &scope_start);
          if (rc != UI_ERROR_NONE)
            goto cleanup;
          { next_token(tz, &token); }
        }

        /* Parse optional 'to' followed by scope end */
        if (token.type == TOKEN_IDENT && strncmp(token.start, "to", 2) == 0 &&
            token.length == 2) {
          next_token(tz, &token);
          if (token.type == TOKEN_LPAREN) {
            rc = parse_selectors(tz, &token, &scope_end);
            if (rc != UI_ERROR_NONE) {
              {
                ui_error_t _ign_rc = ui_css_selector_destroy(scope_start);
                (void)_ign_rc;
              }
              goto cleanup;
            }
            { next_token(tz, &token); }
          }
        }

        /* Recover until we find '{' or EOF if something was unparsed */
        while (token.type != TOKEN_LBRACE && token.type != TOKEN_EOF) {
          next_token(tz, &token);
        }

        rc = ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &scope_rule);
        if (rc != UI_ERROR_NONE) {
          {
            ui_error_t _ign_rc = ui_css_selector_destroy(scope_start);
            (void)_ign_rc;
          }
          {
            ui_error_t _ign_rc = ui_css_selector_destroy(scope_end);
            (void)_ign_rc;
          }
          goto cleanup;
        }
        scope_rule->scope_start = scope_start;
        scope_rule->scope_end = scope_end;

        if (token.type == TOKEN_LBRACE) {
          next_token(tz, &token);
          rc = parse_rule_list(tz, &token, sheet, &scope_rule->nested_rules);
          if (rc != UI_ERROR_NONE) {
            {
              ui_error_t destroy_rc = ui_css_rule_destroy(scope_rule);
              (void)destroy_rc;
            }
            goto cleanup;
          }
          if (token.type == TOKEN_RBRACE) {
            next_token(tz, &token);
          }
        }

        if (!rule_head)
          rule_head = scope_rule;
        else
          rule_tail->next = scope_rule;
        rule_tail = scope_rule;
        continue;
      } else if (strcmp(at_name, "@property") == 0) {
        struct ui_css_rule *prop_rule = NULL;
        char *prop_name = NULL;

        C_MULTIPLATFORM_FREE(at_name);
        next_token(tz, &token);

        /* Read custom property name */
        if (token.type != TOKEN_LBRACE) {
          const char *s_start = token.start;
          while (token.type != TOKEN_LBRACE && token.type != TOKEN_EOF) {
            next_token(tz, &token);
          }
          {
            prop_name = NULL;
            rc = dup_range_trim(s_start, token.start, &prop_name);
            if (rc != UI_ERROR_NONE)
              goto cleanup;
          }
        }

        rc = ui_css_rule_create(UI_CSS_RULE_TYPE_PROPERTY, &prop_rule);
        if (rc != UI_ERROR_NONE) {
          C_MULTIPLATFORM_FREE(prop_name);
          goto cleanup;
        }
        prop_rule->property_name = prop_name;

        if (token.type == TOKEN_LBRACE) {
          next_token(tz, &token);
          /* Parse descriptors (syntax, inherits, initial-value) */
          while (token.type != TOKEN_RBRACE && token.type != TOKEN_EOF) {
            if (token.type == TOKEN_IDENT) {
              char *desc_name = NULL;
              rc = dup_token_str(&token, &desc_name);
              if (rc != UI_ERROR_NONE) {
                {
                  ui_error_t destroy_rc = ui_css_rule_destroy(prop_rule);
                  (void)destroy_rc;
                }
                goto cleanup;
              }

              next_token(tz, &token);
              if (token.type == TOKEN_COLON) {
                const char *v_start;
                char *desc_val;

                next_token(tz, &token);
                v_start = token.start;
                while (token.type != TOKEN_SEMICOLON &&
                       token.type != TOKEN_RBRACE) {
                  if (token.type == TOKEN_EOF)
                    break;
                  next_token(tz, &token);
                }
                desc_val = NULL;
                rc = dup_range_trim(v_start, token.start, &desc_val);
                if (rc != UI_ERROR_NONE) {
                  C_MULTIPLATFORM_FREE(desc_name);
                  {
                    ui_error_t destroy_rc = ui_css_rule_destroy(prop_rule);
                    (void)destroy_rc;
                  }
                  goto cleanup;
                }

                if (strcmp(desc_name, "syntax") == 0) {
                  prop_rule->property_syntax = desc_val;
                } else if (strcmp(desc_name, "inherits") == 0) {
                  if (strcmp(desc_val, "true") == 0) {
                    prop_rule->property_inherits = 1;
                  } else {
                    prop_rule->property_inherits = 0;
                  }
                  C_MULTIPLATFORM_FREE(desc_val);
                } else if (strcmp(desc_name, "initial-value") == 0) {
                  prop_rule->property_initial_value = desc_val;
                } else {
                  C_MULTIPLATFORM_FREE(desc_val);
                }

                if (token.type == TOKEN_SEMICOLON) {
                  next_token(tz, &token);
                }
              } else {
                while (token.type != TOKEN_SEMICOLON &&
                       token.type != TOKEN_RBRACE) {
                  if (token.type == TOKEN_EOF)
                    break;
                  next_token(tz, &token);
                }
                if (token.type == TOKEN_SEMICOLON) {
                  next_token(tz, &token);
                }
              }
              C_MULTIPLATFORM_FREE(desc_name);
            } else {
              while (token.type != TOKEN_SEMICOLON &&
                     token.type != TOKEN_RBRACE) {
                if (token.type == TOKEN_EOF)
                  break;
                next_token(tz, &token);
              }
              if (1) {
                next_token(tz, &token);
              }
            }
          }
          if (token.type == TOKEN_RBRACE) {
            next_token(tz, &token);
          }
        }

        if (!rule_head)
          rule_head = prop_rule;
        else
          rule_tail->next = prop_rule;
        rule_tail = prop_rule;
        continue;
      } else if (strcmp(at_name, "@namespace") == 0) {
        char *prefix = NULL;
        char *uri = NULL;
        C_MULTIPLATFORM_FREE(at_name);
        next_token(tz, &token);

        if (token.type == TOKEN_IDENT && strncmp(token.start, "url", 3) != 0) {
          prefix = NULL;
          rc = dup_token_str(&token, &prefix);
          if (rc != UI_ERROR_NONE)
            return rc;
          next_token(tz, &token);
        }

        if (token.type == TOKEN_STRING || token.type == TOKEN_IDENT) {
          if (token.type == TOKEN_STRING && token.length >= 2) {
            uri = (char *)C_MULTIPLATFORM_MALLOC(token.length - 1);
            if (uri) {
              memcpy(uri, token.start + 1, token.length - 2);
              uri[token.length - 2] = '\0';
            }
          } else if (token.type == TOKEN_IDENT &&
                     strncmp(token.start, "url", 3) == 0) {
            const char *s_start = token.start;
            while (token.type != TOKEN_SEMICOLON && token.type != TOKEN_EOF) {
              next_token(tz, &token);
            }
            { /* Simplistic capture for url(...) */
              const char *p1 = NULL;
              const char *p2 = NULL;
              const char *p;
              for (p = s_start; p < token.start; p++) {
                if (*p == '(') {
                  if (!p1)
                    p1 = p;
                }
                if (*p == ')')
                  p2 = p;
              }
              if (p1 && p2 && p2 > p1) {
                p1++;
                while (isspace((unsigned char)*p1) || *p1 == '"' || *p1 == '\'')
                  p1++;
                while (isspace((unsigned char)*(p2 - 1)) || *(p2 - 1) == '"' ||
                       *(p2 - 1) == '\'')
                  p2--;
                if (p2 > p1) {
                  uri = NULL;
                  rc = dup_range_trim(p1, p2, &uri);
                  if (rc != UI_ERROR_NONE) {
                    C_MULTIPLATFORM_FREE(prefix);
                    goto cleanup;
                  }
                }
              }
            }
          } else {
            uri = NULL;
            rc = dup_token_str(&token, &uri);
            if (rc != UI_ERROR_NONE) {
              C_MULTIPLATFORM_FREE(prefix);
              goto cleanup;
            }
          }
          if (token.type != TOKEN_SEMICOLON && token.type != TOKEN_EOF) {
            while (token.type != TOKEN_SEMICOLON && token.type != TOKEN_EOF) {
              next_token(tz, &token);
            }
          }
        }

        if (token.type == TOKEN_SEMICOLON) {
          next_token(tz, &token);
        }

        if (uri) {
          /* we only support one default namespace for now per namespace rules,
           * or ignore prefixes */
          if (!prefix) {
            rc = ui_css_stylesheet_register_namespace(sheet, NULL, uri);
            if (rc != UI_ERROR_NONE) {
              C_MULTIPLATFORM_FREE(uri);
              goto cleanup;
            }
          } else {
            rc = ui_css_stylesheet_register_namespace(sheet, prefix, uri);
            if (rc != UI_ERROR_NONE) {
              C_MULTIPLATFORM_FREE(uri);
              C_MULTIPLATFORM_FREE(prefix);
              goto cleanup;
            }
          }
          C_MULTIPLATFORM_FREE(uri);
        }
        C_MULTIPLATFORM_FREE(prefix);
        continue;
      } else if (strcmp(at_name, "@media") == 0 ||
                 strcmp(at_name, "@supports") == 0 ||
                 strcmp(at_name, "@container") == 0) {
        struct ui_css_rule *cond_rule = NULL;
        char *condition_text = NULL;
        int is_media = (strcmp(at_name, "@media") == 0);
        int is_container = (strcmp(at_name, "@container") == 0);

        C_MULTIPLATFORM_FREE(at_name);
        next_token(tz, &token);

        if (token.type != TOKEN_LBRACE) {
          const char *c_start = token.start;
          while (token.type != TOKEN_LBRACE && token.type != TOKEN_EOF) {
            next_token(tz, &token);
          }
          {
            condition_text = NULL;
            rc = dup_range_trim(c_start, token.start, &condition_text);
            if (rc != UI_ERROR_NONE)
              goto cleanup;
          }
        }

        rc = ui_css_rule_create(is_media ? UI_CSS_RULE_TYPE_MEDIA
                                         : (is_container
                                                ? UI_CSS_RULE_TYPE_CONTAINER
                                                : UI_CSS_RULE_TYPE_SUPPORTS),
                                &cond_rule);
        if (rc != UI_ERROR_NONE) {
          C_MULTIPLATFORM_FREE(condition_text);
          goto cleanup;
        }

        if (is_media) {
          cond_rule->media_condition = condition_text;
        } else if (is_container) {
          cond_rule->container_condition = condition_text;
        } else {
          cond_rule->supports_condition = condition_text;
        }

        if (token.type == TOKEN_LBRACE) {
          next_token(tz, &token);
          rc = parse_rule_list(tz, &token, sheet, &cond_rule->nested_rules);
          if (rc != UI_ERROR_NONE) {
            {
              ui_error_t destroy_rc = ui_css_rule_destroy(cond_rule);
              (void)destroy_rc;
            }
            goto cleanup;
          }
          if (token.type == TOKEN_RBRACE) {
            next_token(tz, &token);
          }
        }

        if (!rule_head)
          rule_head = cond_rule;
        else
          rule_tail->next = cond_rule;
        rule_tail = cond_rule;
        continue;
      } else {
        /* Unknown @ rule, skip to { ... } or ; */
        C_MULTIPLATFORM_FREE(at_name);
        while (token.type != TOKEN_SEMICOLON && token.type != TOKEN_LBRACE &&
               token.type != TOKEN_EOF) {
          next_token(tz, &token);
        }
        if (token.type == TOKEN_LBRACE) {
          int brace_depth = 1;
          next_token(tz, &token);
          while (brace_depth > 0 && token.type != TOKEN_EOF) {
            if (token.type == TOKEN_LBRACE)
              brace_depth++;
            if (token.type == TOKEN_RBRACE)
              brace_depth--;
            next_token(tz, &token);
          }
        } else if (token.type == TOKEN_SEMICOLON) {
          next_token(tz, &token);
        }
        continue;
      }
    }

    /* STYLE RULE */
    rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &current_rule);
    if (rc != UI_ERROR_NONE) {
      goto cleanup;
    }

    /* Parse selectors */
    rc = parse_selectors(tz, &token, &current_rule->selectors);
    if (rc != UI_ERROR_NONE) {
      {
        ui_error_t destroy_rc = ui_css_rule_destroy(current_rule);
        (void)destroy_rc;
      }
      goto cleanup;
    }

    if (token.type == TOKEN_LBRACE) {
      next_token(tz, &token);

      /* Parse declarations and nested rules */
      while (token.type != TOKEN_RBRACE && token.type != TOKEN_EOF) {
        int is_nested = 0;
        is_nested_rule(tz, token, &is_nested);
        if (is_nested) {
          struct ui_css_rule *nested_rules = NULL;
          rc = parse_rule_list(tz, &token, sheet, &nested_rules);
          if (rc != UI_ERROR_NONE) {
            {
              ui_error_t destroy_rc = ui_css_rule_destroy(current_rule);
              (void)destroy_rc;
            }
            goto cleanup;
          }
          current_rule->nested_rules = nested_rules;
        } else if (token.type == TOKEN_IDENT) {
          char *prop = NULL;
          rc = dup_token_str(&token, &prop);
          if (rc != UI_ERROR_NONE) {
            {
              ui_error_t destroy_rc = ui_css_rule_destroy(current_rule);
              (void)destroy_rc;
            }
            goto cleanup;
          }

          next_token(tz, &token);
          if (token.type == TOKEN_COLON) {
            const char *val_start;
            const char *val_end;
            char *val;
            int is_important = 0;
            size_t vlen;

            next_token(tz, &token);
            val_start = token.start;

            while (token.type != TOKEN_SEMICOLON &&
                   token.type != TOKEN_RBRACE) {
              if (token.type == TOKEN_EOF)
                break;
              next_token(tz, &token);
            }

            val_end = token.start;
            val = NULL;
            rc = dup_range_trim(val_start, val_end, &val);
            if (rc != UI_ERROR_NONE) {
              C_MULTIPLATFORM_FREE(prop);
              {
                ui_error_t destroy_rc = ui_css_rule_destroy(current_rule);
                (void)destroy_rc;
              }
              goto cleanup;
            }

            vlen = strlen(val);
            if (vlen > 10) {
              const char *suffix = val + vlen - 10;
              int match = 1;
              const char *imp = "!important";
              int i;
              for (i = 0; i < 10; i++) {
                if (tolower((unsigned char)suffix[i]) != imp[i]) {
                  match = 0;
                  break;
                }
              }
              if (match) {
                is_important = 1;
                vlen -= 10;
                while (isspace((unsigned char)val[vlen - 1])) {
                  vlen--;
                }
                val[vlen] = '\0';
              }
            }

            rc = ui_css_rule_append_declaration(current_rule, prop, val,
                                                is_important);
            if (rc != UI_ERROR_NONE) {
              C_MULTIPLATFORM_FREE(prop);
              C_MULTIPLATFORM_FREE(val);
              {
                ui_error_t destroy_rc = ui_css_rule_destroy(current_rule);
                (void)destroy_rc;
              }
              goto cleanup;
            }
            C_MULTIPLATFORM_FREE(prop);
            C_MULTIPLATFORM_FREE(val);

            if (token.type == TOKEN_SEMICOLON) {
              next_token(tz, &token);
            }
          } else {
            C_MULTIPLATFORM_FREE(prop);
            while (token.type != TOKEN_SEMICOLON &&
                   token.type != TOKEN_RBRACE) {
              if (token.type == TOKEN_EOF)
                break;
              next_token(tz, &token);
            }
            if (token.type == TOKEN_SEMICOLON) {
              next_token(tz, &token);
            }
          }
        } else {
          while (token.type != TOKEN_SEMICOLON && token.type != TOKEN_RBRACE &&
                 token.type != TOKEN_EOF) {
            next_token(tz, &token);
          }
          if (token.type == TOKEN_SEMICOLON) {
            next_token(tz, &token);
          }
        }
      }
      if (token.type == TOKEN_RBRACE) {
        next_token(tz, &token);
      }
    } else {
      /* Missing block, consume up to semicolon or EOF */
      while (token.type != TOKEN_SEMICOLON && token.type != TOKEN_RBRACE &&
             token.type != TOKEN_EOF && token.type != TOKEN_LBRACE) {
        next_token(tz, &token);
      }
      if (token.type == TOKEN_SEMICOLON) {
        next_token(tz, &token);
      } else if (token.type == TOKEN_LBRACE) {
        /* consume block to recover */
        int brace_depth = 1;
        next_token(tz, &token);
        while (brace_depth > 0 && token.type != TOKEN_EOF) {
          if (token.type == TOKEN_LBRACE)
            brace_depth++;
          if (token.type == TOKEN_RBRACE)
            brace_depth--;
          next_token(tz, &token);
        }
      }
    }

    if (!rule_head)
      rule_head = current_rule;
    else
      rule_tail->next = current_rule;
    rule_tail = current_rule;
  }

  *out_rules = rule_head;
  *inout_token = token;
  return UI_ERROR_NONE;

cleanup: {
  struct ui_css_rule *curr = rule_head;
  while (curr) {
    struct ui_css_rule *next = curr->next;
    {
      ui_error_t destroy_rc = ui_css_rule_destroy(curr);
      (void)destroy_rc;
    }
    curr = next;
  }
}
  *inout_token = token;
  return rc;
}

/* \brief ui_error
 */
ui_error_t ui_css_parse_stylesheet(const char *css_text,
                                   struct ui_css_stylesheet **out_stylesheet) {
  struct ui_css_tokenizer tz;
  struct ui_css_token token;
  struct ui_css_stylesheet *sheet = NULL;
  ui_error_t rc = UI_ERROR_NONE;

  if (!css_text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!out_stylesheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tz.text = css_text;
  tz.pos = 0;
  tz.len = strlen(css_text);

  rc = ui_css_stylesheet_create(&sheet);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  next_token(&tz, &token);

  rc = parse_rule_list(&tz, &token, sheet, &sheet->rules);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  *out_stylesheet = sheet;
  return UI_ERROR_NONE;

cleanup:
  if (sheet) {
    ui_css_stylesheet_destroy(sheet);
  }
  return rc;
}
