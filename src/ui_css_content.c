/* clang-format off */
#include "ui_css_content.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "ui_internal_mem.h"
/* clang-format on */

/*
 * @brief skip_whitespace.
 * @param p_str Parameter p_str.
 * @return Return value.
 */
static void skip_whitespace(const char **p_str) {
  while (isspace((unsigned char)**p_str)) {
    (*p_str)++;
  }
}

/*
 * @brief match_keyword.
 * @param p_str Parameter p_str.
 * @param keyword Parameter keyword.
 * @param out_match Parameter out_match.
 * @return Return value.
 */
static void match_keyword(const char **p_str, const char *keyword,
                          int *out_match) {
  size_t len = strlen(keyword);
  if (strncmp(*p_str, keyword, len) == 0) {
    if (isalpha((unsigned char)keyword[len - 1])) {
      if (isalnum((unsigned char)(*p_str)[len]) || (*p_str)[len] == '-' ||
          (*p_str)[len] == '_') {
        *out_match = 0;
        return;
      }
    }
    *p_str += len;
    *out_match = 1;
    return;
  }
  *out_match = 0;
}

/*
 * @brief parse_string.
 * @param p_str Parameter p_str.
 * @param out_buf Parameter out_buf.
 * @param max_len Parameter max_len.
 * @return Return value.
 */
static ui_error_t parse_string(const char **p_str, char *out_buf,
                               size_t max_len) {
  char quote;
  const char *str = *p_str;
  size_t i = 0;

  if (*str != '"' && *str != '\'')
    return UI_ERROR_PARSE_FAILED;
  quote = *str++;

  while (*str && *str != quote) {
    if (*str == '\\' && *(str + 1)) {
      str++; /* skip backslash */
      if (i < max_len - 1) {
        out_buf[i++] = *str;
      }
    } else {
      if (i < max_len - 1) {
        out_buf[i++] = *str;
      }
    }
    str++;
  }

  if (*str != quote)
    return UI_ERROR_PARSE_FAILED; /* Unclosed quote */
  str++;                          /* skip closing quote */

  out_buf[i] = '\0';
  *p_str = str;
  return UI_ERROR_NONE;
}

/*
 * @brief parse_ident.
 * @param p_str Parameter p_str.
 * @param out_buf Parameter out_buf.
 * @param max_len Parameter max_len.
 * @return Return value.
 */
static ui_error_t parse_ident(const char **p_str, char *out_buf,
                              size_t max_len) {
  const char *str = *p_str;
  size_t i = 0;

  if (!isalpha((unsigned char)*str) && *str != '_' && *str != '-')
    return UI_ERROR_PARSE_FAILED;

  while (isalnum((unsigned char)*str) || *str == '-' || *str == '_') {
    if (i < max_len - 1) {
      out_buf[i++] = *str;
    }
    str++;
  }

  out_buf[i] = '\0';
  *p_str = str;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_css_parse_content.
 * @param str Parameter str.
 * @param out_content Parameter out_content.
 * @return Return value.
 */
ui_error_t ui_css_parse_content(const char *str,
                                struct ui_css_content *out_content) {
  struct ui_css_content_item *head = NULL;
  struct ui_css_content_item *tail = NULL;
  struct ui_css_content_item *item = NULL;
  ui_error_t rc = UI_ERROR_NONE;

  if (!str || !out_content)
    return UI_ERROR_INVALID_ARGUMENT;

  out_content->is_none = 0;
  out_content->is_normal = 0;
  out_content->items = NULL;

  skip_whitespace(&str);

  if (strcmp(str, "none") == 0) {
    out_content->is_none = 1;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "normal") == 0) {
    out_content->is_normal = 1;
    return UI_ERROR_NONE;
  }

  while (*str) {
    int matched = 0;
    item = (struct ui_css_content_item *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_css_content_item));
    if (!item) {
      out_content->items = head;
      (void)ui_css_content_destroy(out_content);
      return UI_ERROR_OUT_OF_MEMORY;
    }
    memset(item, 0, sizeof(struct ui_css_content_item));

    if (*str == '"' || *str == '\'') {
      item->type = UI_CSS_CONTENT_ITEM_STRING;
      rc = parse_string(&str, item->value.string_val,
                        sizeof(item->value.string_val));
      if (rc != UI_ERROR_NONE) {
        goto parse_failed;
      }
    } else {
      match_keyword(&str, "url(", &matched);
      if (matched) {
        /* Hand over to image parser */
        const char *start = str - 4;
        const char *end = strchr(start, ')');
        if (end) {
          char url_buf[512];
          size_t len = (size_t)(end - start) + 1;
          if (len >= sizeof(url_buf))
            len = sizeof(url_buf) - 1;
          memcpy(url_buf, start, len);
          url_buf[len] = '\0';

          item->type = UI_CSS_CONTENT_ITEM_IMAGE;
          rc = ui_css_parse_image(url_buf, &item->value.image);
          if (rc != UI_ERROR_NONE) {
            goto parse_failed;
          }
          str = end + 1;
        } else {
          goto parse_failed;
        }
      } else {
        match_keyword(&str, "attr(", &matched);
        if (matched) {
          item->type = UI_CSS_CONTENT_ITEM_ATTR;
          skip_whitespace(&str);
          rc = parse_ident(&str, item->value.attr_name,
                           sizeof(item->value.attr_name));
          if (rc != UI_ERROR_NONE) {
            goto parse_failed;
          }
          skip_whitespace(&str);
          if (*str != ')') {
            goto parse_failed;
          }
          str++;
        } else {
          match_keyword(&str, "counters(", &matched);
          if (matched) {
            item->type = UI_CSS_CONTENT_ITEM_COUNTERS;
            skip_whitespace(&str);
            rc = parse_ident(&str, item->value.counters.name,
                             sizeof(item->value.counters.name));
            if (rc != UI_ERROR_NONE) {
              goto parse_failed;
            }
            skip_whitespace(&str);
            if (*str != ',') {
              goto parse_failed;
            }
            str++;
            skip_whitespace(&str);
            rc = parse_string(&str, item->value.counters.separator,
                              sizeof(item->value.counters.separator));
            if (rc != UI_ERROR_NONE) {
              goto parse_failed;
            }
            skip_whitespace(&str);
            if (*str == ',') {
              str++;
              skip_whitespace(&str);
              rc = parse_ident(&str, item->value.counters.style,
                               sizeof(item->value.counters.style));
              if (rc != UI_ERROR_NONE) {
                goto parse_failed;
              }
            } else {
              item->value.counters.style[0] = '\0'; /* default */
            }
            skip_whitespace(&str);
            if (*str != ')') {
              goto parse_failed;
            }
            str++;
          } else {
            match_keyword(&str, "counter(", &matched);
            if (matched) {
              item->type = UI_CSS_CONTENT_ITEM_COUNTER;
              skip_whitespace(&str);
              rc = parse_ident(&str, item->value.counter.name,
                               sizeof(item->value.counter.name));
              if (rc != UI_ERROR_NONE) {
                goto parse_failed;
              }
              skip_whitespace(&str);
              if (*str == ',') {
                str++;
                skip_whitespace(&str);
                rc = parse_ident(&str, item->value.counter.style,
                                 sizeof(item->value.counter.style));
                if (rc != UI_ERROR_NONE) {
                  goto parse_failed;
                }
              } else {
                item->value.counter.style[0] = '\0'; /* default */
              }
              skip_whitespace(&str);
              if (*str != ')') {
                goto parse_failed;
              }
              str++;
            } else {
              match_keyword(&str, "open-quote", &matched);
              if (matched) {
                item->type = UI_CSS_CONTENT_ITEM_OPEN_QUOTE;
              } else {
                match_keyword(&str, "close-quote", &matched);
                if (matched) {
                  item->type = UI_CSS_CONTENT_ITEM_CLOSE_QUOTE;
                } else {
                  match_keyword(&str, "no-open-quote", &matched);
                  if (matched) {
                    item->type = UI_CSS_CONTENT_ITEM_NO_OPEN_QUOTE;
                  } else {
                    match_keyword(&str, "no-close-quote", &matched);
                    if (matched) {
                      item->type = UI_CSS_CONTENT_ITEM_NO_CLOSE_QUOTE;
                    } else {
                      goto parse_failed;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    if (!head)
      head = item;
    else
      tail->next = item;
    tail = item;

    skip_whitespace(&str);
  }

  out_content->items = head;
  return UI_ERROR_NONE;

parse_failed:
  C_MULTIPLATFORM_FREE(item);
  out_content->items = head;
  (void)ui_css_content_destroy(out_content);
  return rc != UI_ERROR_NONE ? rc : UI_ERROR_PARSE_FAILED;
}

/*
 * @brief ui_css_content_destroy.
 * @param content Parameter content.
 * @return Return value.
 */
void ui_css_content_destroy(struct ui_css_content *content) {
  if (!content)
    return;

  while (content->items) {
    struct ui_css_content_item *next = content->items->next;
    C_MULTIPLATFORM_FREE(content->items);
    content->items = next;
  }
}
