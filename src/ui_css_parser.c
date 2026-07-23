/* clang-format off */
#include "../include/ui_css_parser.h"
#include "../include/ui_error.h"
#include "ui_internal_mem.h"
#include <string.h>
#include <ctype.h>
/* clang-format on */

enum ui_css_token_type {
  TOKEN_EOF,
  TOKEN_IDENT,
  TOKEN_HASH,
  TOKEN_DOT,
  TOKEN_COLON,
  TOKEN_SEMICOLON,
  TOKEN_COMMA,
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_LBRACKET,
  TOKEN_RBRACKET,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_EQUALS,
  TOKEN_PREFIX_MATCH,
  TOKEN_SUFFIX_MATCH,
  TOKEN_SUBSTRING_MATCH,
  TOKEN_DASH_MATCH,
  TOKEN_INCLUDE_MATCH,
  TOKEN_ASTERISK,
  TOKEN_STRING,
  TOKEN_AT_KEYWORD,
  TOKEN_ERROR
};

/** \brief ui_css_token */
struct ui_css_token {
  enum ui_css_token_type type;
  const char *start;
  size_t length;
};

/** \brief ui_css_tokenizer */
struct ui_css_tokenizer {
  const char *text;
  size_t pos;
  size_t len;
};

static enum ui_error advance(struct ui_css_tokenizer *tz) {
  tz->pos++;
  return UI_ERROR_NONE;
}

static enum ui_error peek(struct ui_css_tokenizer *tz, char *out_char) {
  *out_char = tz->text[tz->pos];
  return UI_ERROR_NONE;
}

static enum ui_error is_ident_start(char c, int *out_is_start) {
  *out_is_start = isalpha((unsigned char)c) || c == '_' || c == '-';
  return UI_ERROR_NONE;
}

static enum ui_error is_ident_char(char c, int *out_is_char) {
  *out_is_char = isalnum((unsigned char)c) || c == '_' || c == '-';
  return UI_ERROR_NONE;
}

static enum ui_error skip_whitespace_and_comments(struct ui_css_tokenizer *tz) {
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
  return UI_ERROR_NONE;
}

static enum ui_error next_token(struct ui_css_tokenizer *tz,
                                struct ui_css_token *out_token) {
  struct ui_css_token t;
  char c;

  skip_whitespace_and_comments(tz);

  t.start = tz->text + tz->pos;
  t.length = 0;

  if (tz->pos >= tz->len) {
    t.type = TOKEN_EOF;
    *out_token = t;
    return UI_ERROR_NONE;
  }

  peek(tz, &c);

  switch (c) {
  case '{':
    advance(tz);
    t.type = TOKEN_LBRACE;
    t.length = 1;
    *out_token = t;
    return UI_ERROR_NONE;
  case '}':
    advance(tz);
    t.type = TOKEN_RBRACE;
    t.length = 1;
    *out_token = t;
    return UI_ERROR_NONE;
  case '[':
    advance(tz);
    t.type = TOKEN_LBRACKET;
    t.length = 1;
    *out_token = t;
    return UI_ERROR_NONE;
  case ']':
    advance(tz);
    t.type = TOKEN_RBRACKET;
    t.length = 1;
    *out_token = t;
    return UI_ERROR_NONE;
  case '(':
    advance(tz);
    t.type = TOKEN_LPAREN;
    t.length = 1;
    *out_token = t;
    return UI_ERROR_NONE;
  case ')':
    advance(tz);
    t.type = TOKEN_RPAREN;
    t.length = 1;
    *out_token = t;
    return UI_ERROR_NONE;
  case ':':
    advance(tz);
    t.type = TOKEN_COLON;
    t.length = 1;
    *out_token = t;
    return UI_ERROR_NONE;
  case ';':
    advance(tz);
    t.type = TOKEN_SEMICOLON;
    t.length = 1;
    *out_token = t;
    return UI_ERROR_NONE;
  case ',':
    advance(tz);
    t.type = TOKEN_COMMA;
    t.length = 1;
    *out_token = t;
    return UI_ERROR_NONE;
  case '.':
    advance(tz);
    t.type = TOKEN_DOT;
    t.length = 1;
    *out_token = t;
    return UI_ERROR_NONE;
  case '#':
    advance(tz);
    t.type = TOKEN_HASH;
    t.length = 1;
    *out_token = t;
    return UI_ERROR_NONE;
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
    return UI_ERROR_NONE;
  case '=':
    advance(tz);
    t.type = TOKEN_EQUALS;
    t.length = 1;
    *out_token = t;
    return UI_ERROR_NONE;
  case '^':
    advance(tz);
    peek(tz, &c);
    if (c == '=') {
      advance(tz);
      t.type = TOKEN_PREFIX_MATCH;
      t.length = 2;
      *out_token = t;
      return UI_ERROR_NONE;
    }
    t.type = TOKEN_ERROR;
    t.length = 1;
    *out_token = t;
    return UI_ERROR_NONE;
  case '$':
    advance(tz);
    peek(tz, &c);
    if (c == '=') {
      advance(tz);
      t.type = TOKEN_SUFFIX_MATCH;
      t.length = 2;
      *out_token = t;
      return UI_ERROR_NONE;
    }
    t.type = TOKEN_ERROR;
    t.length = 1;
    *out_token = t;
    return UI_ERROR_NONE;
  case '|':
    advance(tz);
    peek(tz, &c);
    if (c == '=') {
      advance(tz);
      t.type = TOKEN_DASH_MATCH;
      t.length = 2;
      *out_token = t;
      return UI_ERROR_NONE;
    }
    t.type = TOKEN_ERROR;
    t.length = 1;
    *out_token = t;
    return UI_ERROR_NONE;
  case '~':
    advance(tz);
    peek(tz, &c);
    if (c == '=') {
      advance(tz);
      t.type = TOKEN_INCLUDE_MATCH;
      t.length = 2;
      *out_token = t;
      return UI_ERROR_NONE;
    }
    t.type = TOKEN_ERROR;
    t.length = 1;
    *out_token = t;
    return UI_ERROR_NONE;
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
      return UI_ERROR_NONE;
    }
    t.type = TOKEN_ERROR;
    t.length = 1;
    *out_token = t;
    return UI_ERROR_NONE;
  }
  case '"':
  case '\'': {
    char quote = c;
    advance(tz);
    while (tz->pos < tz->len) {
      peek(tz, &c);
      if (c == quote)
        break;
      if (c == '\\')
        advance(tz);
      advance(tz);
    }
    if (tz->pos < tz->len)
      advance(tz);
    t.type = TOKEN_STRING;
    t.length = (size_t)((tz->text + tz->pos) - t.start);
    *out_token = t;
    return UI_ERROR_NONE;
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
      return UI_ERROR_NONE;
    } else if (isdigit((unsigned char)c) || c == '+' || c == '-') {
      while (tz->pos < tz->len) {
        peek(tz, &c);
        if (isspace((unsigned char)c) || c == ';' || c == '}' || c == '{')
          break;
        advance(tz);
      }
      t.type = TOKEN_IDENT;
      t.length = (size_t)((tz->text + tz->pos) - t.start);
      *out_token = t;
      return UI_ERROR_NONE;
    } else {
      advance(tz);
      t.type = TOKEN_ERROR;
      t.length = 1;
      *out_token = t;
      return UI_ERROR_NONE;
    }
  }
  }
}

static enum ui_error dup_token_str(const struct ui_css_token *t,
                                   char **out_str) {
  char *s = (char *)UI_MALLOC(t->length + 1);
  if (!s) {
    *out_str = NULL;
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memcpy(s, t->start, t->length);
  s[t->length] = '\0';
  *out_str = s;
  return UI_ERROR_NONE;
}

static enum ui_error dup_range_trim(const char *start, const char *end,
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
  res = (char *)UI_MALLOC(len + 1);
  if (!res) {
    *out_str = NULL;
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memcpy(res, s, len);
  res[len] = '\0';
  *out_str = res;
  return UI_ERROR_NONE;
}

static enum ui_error parse_selectors(struct ui_css_tokenizer *tz,
                                     struct ui_css_token *inout_token,
                                     struct ui_css_selector **out_selectors) {
  struct ui_css_rule *dummy_rule = NULL;
  struct ui_css_token token = *inout_token;
  enum ui_error rc;

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
        if (!val) {
          rc = UI_ERROR_OUT_OF_MEMORY;
          goto cleanup;
        }
        rc = ui_css_rule_append_selector(dummy_rule, UI_CSS_SELECTOR_TYPE_CLASS,
                                         val);
        UI_FREE(val);
        if (rc != UI_ERROR_NONE) {
          goto cleanup;
        }
        next_token(tz, &token);
      }
    } else if (token.type == TOKEN_HASH) {
      next_token(tz, &token);
      if (token.type == TOKEN_IDENT) {
        char *val = NULL;
        rc = dup_token_str(&token, &val);
        if (!val) {
          rc = UI_ERROR_OUT_OF_MEMORY;
          goto cleanup;
        }
        rc = ui_css_rule_append_selector(dummy_rule, UI_CSS_SELECTOR_TYPE_ID,
                                         val);
        UI_FREE(val);
        if (rc != UI_ERROR_NONE) {
          goto cleanup;
        }
        next_token(tz, &token);
      }
    } else if (token.type == TOKEN_IDENT) {
      char *val = NULL;
      rc = dup_token_str(&token, &val);
      if (!val) {
        rc = UI_ERROR_OUT_OF_MEMORY;
        goto cleanup;
      }
      rc = ui_css_rule_append_selector(dummy_rule, UI_CSS_SELECTOR_TYPE_TAG,
                                       val);
      UI_FREE(val);
      if (rc != UI_ERROR_NONE) {
        goto cleanup;
      }
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

        if (!attr_name) {
          rc = UI_ERROR_OUT_OF_MEMORY;
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
              attr_val = (char *)UI_MALLOC(token.length - 1);
              if (attr_val) {
                memcpy(attr_val, token.start + 1, token.length - 2);
                attr_val[token.length - 2] = '\0';
              }
            } else {
              attr_val = NULL;
              rc = dup_token_str(&token, &attr_val);
            }
            if (!attr_val) {
              UI_FREE(attr_name);
              rc = UI_ERROR_OUT_OF_MEMORY;
              goto cleanup;
            }
            next_token(tz, &token);
          }
        }

        if (token.type == TOKEN_RBRACKET) {
          rc = ui_css_rule_append_selector_attr(dummy_rule, attr_name, op,
                                                attr_val);
          next_token(tz, &token);
        }

        UI_FREE(attr_name);
        if (attr_val)
          UI_FREE(attr_val);
        if (rc != UI_ERROR_NONE) {
          goto cleanup;
        }
      } else {
        while (token.type != TOKEN_RBRACKET && token.type != TOKEN_EOF &&
               token.type != TOKEN_LBRACE) {
          next_token(tz, &token);
        }
        if (token.type == TOKEN_RBRACKET)
          next_token(tz, &token);
      }
    } else if (token.type == TOKEN_COLON) {
      next_token(tz, &token);
      if (token.type == TOKEN_COLON) {
        /* Pseudo-element :: */
        next_token(tz, &token);
        if (token.type == TOKEN_IDENT) {
          char *pseudo_val = NULL;
          rc = dup_token_str(&token, &pseudo_val);
          if (!pseudo_val) {
            rc = UI_ERROR_OUT_OF_MEMORY;
            goto cleanup;
          }

          rc = ui_css_rule_append_selector(
              dummy_rule, UI_CSS_SELECTOR_TYPE_PSEUDO_ELEMENT, pseudo_val);
          UI_FREE(pseudo_val);
          if (rc != UI_ERROR_NONE) {
            goto cleanup;
          }

          next_token(tz, &token);
        }
      } else if (token.type == TOKEN_IDENT) {
        char *pseudo_val = NULL;
        rc = dup_token_str(&token, &pseudo_val);
        if (!pseudo_val) {
          rc = UI_ERROR_OUT_OF_MEMORY;
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
        } else {
          rc = ui_css_rule_append_selector(
              dummy_rule, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS, pseudo_val);
        }

        if (rc != UI_ERROR_NONE) {
          UI_FREE(pseudo_val);
          goto cleanup;
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
            next_token(tz, &token);
            rc = parse_selectors(tz, &token, &nested);
            if (rc == UI_ERROR_NONE) {
              struct ui_css_selector *last = dummy_rule->selectors;
              while (last && last->next)
                last = last->next;
              if (last) {
                last->nested_selector = nested;
              } else {
                /* fallback should not happen */
              }
            } else {
              UI_FREE(pseudo_val);
              goto cleanup;
            }
            if (token.type == TOKEN_RPAREN) {
              next_token(tz, &token);
            }
          } else {
            int paren_depth = 1;
            next_token(tz, &token);
            while (paren_depth > 0 && token.type != TOKEN_EOF) {
              if (token.type == TOKEN_LPAREN)
                paren_depth++;
              if (token.type == TOKEN_RPAREN)
                paren_depth--;
              if (paren_depth > 0)
                next_token(tz, &token);
            }
            if (token.type == TOKEN_RPAREN) {
              next_token(tz, &token);
            }
          }
        }

        UI_FREE(pseudo_val);
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

cleanup:
  ui_css_rule_destroy(dummy_rule);
  *inout_token = token;
  return rc;
}

static enum ui_error is_nested_rule(struct ui_css_tokenizer *tz,
                                    struct ui_css_token first_token,
                                    int *out_is_nested) {
  struct ui_css_tokenizer temp_tz = *tz;
  struct ui_css_token t = first_token;
  *out_is_nested = 0;
  if (t.type == TOKEN_AT_KEYWORD) {
    *out_is_nested = 1;
    return UI_ERROR_NONE;
  }
  if (t.type == TOKEN_DOT || t.type == TOKEN_HASH || t.type == TOKEN_COLON ||
      t.type == TOKEN_LBRACKET || t.type == TOKEN_ASTERISK ||
      t.type == TOKEN_COMMA) {
    *out_is_nested = 1;
    return UI_ERROR_NONE;
  }

  while (t.type != TOKEN_EOF && t.type != TOKEN_LBRACE &&
         t.type != TOKEN_SEMICOLON && t.type != TOKEN_RBRACE) {
    next_token(&temp_tz, &t);
  }

  if (t.type == TOKEN_LBRACE) {
    *out_is_nested = 1;
  }
  return UI_ERROR_NONE;
}

static enum ui_error parse_rule_list(struct ui_css_tokenizer *tz,
                                     struct ui_css_token *inout_token,
                                     struct ui_css_stylesheet *sheet,
                                     struct ui_css_rule **out_rules);

static enum ui_error parse_rule_list(struct ui_css_tokenizer *tz,
                                     struct ui_css_token *inout_token,
                                     struct ui_css_stylesheet *sheet,
                                     struct ui_css_rule **out_rules) {
  struct ui_css_token token = *inout_token;
  struct ui_css_rule *rule_head = NULL;
  struct ui_css_rule *rule_tail = NULL;
  enum ui_error rc = UI_ERROR_NONE;

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
      if (!at_name) {
        rc = UI_ERROR_OUT_OF_MEMORY;
        goto cleanup;
      }

      if (strcmp(at_name, "@layer") == 0) {
        UI_FREE(at_name);
        next_token(tz, &token);

        if (token.type == TOKEN_IDENT) {
          layer_name = NULL;
          rc = dup_token_str(&token, &layer_name);
          next_token(tz, &token);

          if (token.type == TOKEN_COMMA || token.type == TOKEN_SEMICOLON) {
            ui_css_stylesheet_register_layer(sheet, layer_name, &order);
            UI_FREE(layer_name);
            while (token.type == TOKEN_COMMA) {
              next_token(tz, &token);
              if (token.type == TOKEN_IDENT) {
                layer_name = NULL;
                rc = dup_token_str(&token, &layer_name);
                ui_css_stylesheet_register_layer(sheet, layer_name, &order);
                UI_FREE(layer_name);
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
          if (layer_name)
            UI_FREE(layer_name);
          goto cleanup;
        }

        layer_rule->layer_name = layer_name; /* Ownership transfer */
        if (layer_name) {
          ui_css_stylesheet_register_layer(sheet, layer_name, &order);
        }

        if (token.type == TOKEN_LBRACE) {
          next_token(tz, &token);
          rc = parse_rule_list(tz, &token, sheet, &layer_rule->nested_rules);
          if (rc != UI_ERROR_NONE) {
            ui_css_rule_destroy(layer_rule);
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
        char *scope_start = NULL;
        char *scope_end = NULL;

        UI_FREE(at_name);
        next_token(tz, &token);

        /* Very naive @scope parser: capture (start) to (end) as string blocks
         * for later matching */
        if (token.type != TOKEN_LBRACE && token.type != TOKEN_EOF) {
          const char *s_start = token.start;
          while (token.type != TOKEN_LBRACE && token.type != TOKEN_EOF) {
            if (token.type == TOKEN_IDENT &&
                strncmp(token.start, "to", 2) == 0 && token.length == 2) {
              break;
            }
            next_token(tz, &token);
          }
          if (token.start > s_start) {
            scope_start = NULL;
            rc = dup_range_trim(s_start, token.start, &scope_start);
          }

          if (token.type == TOKEN_IDENT && strncmp(token.start, "to", 2) == 0 &&
              token.length == 2) {
            next_token(tz, &token);
            s_start = token.start;
            while (token.type != TOKEN_LBRACE && token.type != TOKEN_EOF) {
              next_token(tz, &token);
            }
            if (token.start > s_start) {
              scope_end = NULL;
              rc = dup_range_trim(s_start, token.start, &scope_end);
            }
          }
        }

        rc = ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &scope_rule);
        if (rc != UI_ERROR_NONE) {
          if (scope_start)
            UI_FREE(scope_start);
          if (scope_end)
            UI_FREE(scope_end);
          goto cleanup;
        }
        scope_rule->scope_start = scope_start;
        scope_rule->scope_end = scope_end;

        if (token.type == TOKEN_LBRACE) {
          next_token(tz, &token);
          rc = parse_rule_list(tz, &token, sheet, &scope_rule->nested_rules);
          if (rc != UI_ERROR_NONE) {
            ui_css_rule_destroy(scope_rule);
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

        UI_FREE(at_name);
        next_token(tz, &token);

        /* Read custom property name */
        if (token.type != TOKEN_LBRACE && token.type != TOKEN_EOF) {
          const char *s_start = token.start;
          while (token.type != TOKEN_LBRACE && token.type != TOKEN_EOF) {
            next_token(tz, &token);
          }
          if (token.start > s_start) {
            prop_name = NULL;
            rc = dup_range_trim(s_start, token.start, &prop_name);
          }
        }

        rc = ui_css_rule_create(UI_CSS_RULE_TYPE_PROPERTY, &prop_rule);
        if (rc != UI_ERROR_NONE) {
          if (prop_name)
            UI_FREE(prop_name);
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
              if (!desc_name) {
                rc = UI_ERROR_OUT_OF_MEMORY;
                ui_css_rule_destroy(prop_rule);
                goto cleanup;
              }

              next_token(tz, &token);
              if (token.type == TOKEN_COLON) {
                const char *v_start;
                char *desc_val;

                next_token(tz, &token);
                v_start = token.start;
                while (token.type != TOKEN_SEMICOLON &&
                       token.type != TOKEN_RBRACE && token.type != TOKEN_EOF) {
                  next_token(tz, &token);
                }
                desc_val = NULL;
                rc = dup_range_trim(v_start, token.start, &desc_val);

                if (strcmp(desc_name, "syntax") == 0) {
                  prop_rule->property_syntax = desc_val;
                } else if (strcmp(desc_name, "inherits") == 0) {
                  if (desc_val && strcmp(desc_val, "true") == 0) {
                    prop_rule->property_inherits = 1;
                  } else {
                    prop_rule->property_inherits = 0;
                  }
                  if (desc_val)
                    UI_FREE(desc_val);
                } else if (strcmp(desc_name, "initial-value") == 0) {
                  prop_rule->property_initial_value = desc_val;
                } else {
                  if (desc_val)
                    UI_FREE(desc_val);
                }

                if (token.type == TOKEN_SEMICOLON) {
                  next_token(tz, &token);
                }
              } else {
                while (token.type != TOKEN_SEMICOLON &&
                       token.type != TOKEN_RBRACE && token.type != TOKEN_EOF) {
                  next_token(tz, &token);
                }
                if (token.type == TOKEN_SEMICOLON) {
                  next_token(tz, &token);
                }
              }
              UI_FREE(desc_name);
            } else {
              while (token.type != TOKEN_SEMICOLON &&
                     token.type != TOKEN_RBRACE && token.type != TOKEN_EOF) {
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
        UI_FREE(at_name);
        next_token(tz, &token);

        if (token.type == TOKEN_IDENT && strncmp(token.start, "url", 3) != 0) {
          prefix = NULL;
          rc = dup_token_str(&token, &prefix);
          next_token(tz, &token);
        }

        if (token.type == TOKEN_STRING || token.type == TOKEN_IDENT) {
          if (token.type == TOKEN_STRING && token.length >= 2) {
            uri = (char *)UI_MALLOC(token.length - 1);
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
            if (token.start > s_start) {
              /* Simplistic capture for url(...) */
              const char *p1 = NULL;
              const char *p2 = NULL;
              const char *p;
              for (p = s_start; p < token.start; p++) {
                if (*p == '(' && !p1)
                  p1 = p;
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
                }
              }
            }
          } else {
            uri = NULL;
            rc = dup_token_str(&token, &uri);
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
          ui_css_stylesheet_register_namespace(sheet, prefix, uri);
          UI_FREE(uri);
        }
        if (prefix) {
          UI_FREE(prefix);
        }
        continue;
      } else if (strcmp(at_name, "@media") == 0 ||
                 strcmp(at_name, "@supports") == 0 ||
                 strcmp(at_name, "@container") == 0) {
        struct ui_css_rule *cond_rule = NULL;
        char *condition_text = NULL;
        int is_media = (strcmp(at_name, "@media") == 0);
        int is_container = (strcmp(at_name, "@container") == 0);

        UI_FREE(at_name);
        next_token(tz, &token);

        if (token.type != TOKEN_LBRACE && token.type != TOKEN_EOF) {
          const char *c_start = token.start;
          while (token.type != TOKEN_LBRACE && token.type != TOKEN_EOF) {
            next_token(tz, &token);
          }
          if (token.start > c_start) {
            condition_text = NULL;
            rc = dup_range_trim(c_start, token.start, &condition_text);
          }
        }

        rc = ui_css_rule_create(is_media ? UI_CSS_RULE_TYPE_MEDIA
                                         : (is_container
                                                ? UI_CSS_RULE_TYPE_CONTAINER
                                                : UI_CSS_RULE_TYPE_SUPPORTS),
                                &cond_rule);
        if (rc != UI_ERROR_NONE) {
          if (condition_text)
            UI_FREE(condition_text);
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
            ui_css_rule_destroy(cond_rule);
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
        UI_FREE(at_name);
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
      ui_css_rule_destroy(current_rule);
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
            ui_css_rule_destroy(current_rule);
            goto cleanup;
          }
          if (nested_rules) {
            current_rule->nested_rules = nested_rules;
          }
        } else if (token.type == TOKEN_IDENT) {
          char *prop = NULL;
          rc = dup_token_str(&token, &prop);
          if (!prop) {
            rc = UI_ERROR_OUT_OF_MEMORY;
            ui_css_rule_destroy(current_rule);
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
                   token.type != TOKEN_RBRACE && token.type != TOKEN_EOF) {
              next_token(tz, &token);
            }

            val_end = token.start;
            val = NULL;
            rc = dup_range_trim(val_start, val_end, &val);
            if (!val) {
              UI_FREE(prop);
              rc = UI_ERROR_OUT_OF_MEMORY;
              ui_css_rule_destroy(current_rule);
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
                while (vlen > 0 && isspace((unsigned char)val[vlen - 1])) {
                  vlen--;
                }
                val[vlen] = '\0';
              }
            }

            rc = ui_css_rule_append_declaration(current_rule, prop, val,
                                                is_important);
            UI_FREE(prop);
            UI_FREE(val);

            if (rc != UI_ERROR_NONE) {
              ui_css_rule_destroy(current_rule);
              goto cleanup;
            }

            if (token.type == TOKEN_SEMICOLON) {
              next_token(tz, &token);
            }
          } else {
            UI_FREE(prop);
            while (token.type != TOKEN_SEMICOLON &&
                   token.type != TOKEN_RBRACE && token.type != TOKEN_EOF) {
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
    ui_css_rule_destroy(curr);
    curr = next;
  }
}
  *inout_token = token;
  return rc;
}

/** \brief ui_error */
enum ui_error
ui_css_parse_stylesheet(const char *css_text,
                        struct ui_css_stylesheet **out_stylesheet) {
  struct ui_css_tokenizer tz;
  struct ui_css_token token;
  struct ui_css_stylesheet *sheet = NULL;
  enum ui_error rc = UI_ERROR_NONE;

  if (!css_text || !out_stylesheet) {
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
