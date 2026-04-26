/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cmp_log.h"
/* clang-format on */

struct cmp_syntax_highlighter {
  int is_initialized;
};

/**
 * @brief cmp_syntax_highlighter_create
 *
 * @param out_hl Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_syntax_highlighter_create(cmp_syntax_highlighter_t **out_hl) {
  int rc = CMP_SUCCESS;
  cmp_syntax_highlighter_t *hl;

  if (!out_hl) {
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_syntax_highlighter_t), (void **)&(hl));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  hl->is_initialized = 1;
  *out_hl = hl;

  return CMP_SUCCESS;
}

/**
 * @brief cmp_syntax_highlighter_destroy
 *
 * @param hl Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_syntax_highlighter_destroy(cmp_syntax_highlighter_t *hl) {
  int rc = CMP_SUCCESS;
  if (!hl) {
    return CMP_ERROR_INVALID_ARG;
  }
  rc = CMP_FREE(hl);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_syntax_highlighter_parse
 *
 * @param hl Parameter description.
 * @param source_code Parameter description.
 * @param language Parameter description.
 * @param out_spans Parameter description.
 * @param out_count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_syntax_highlighter_parse(cmp_syntax_highlighter_t *hl,
                                 const char *source_code, const char *language,
                                 cmp_highlight_span_t **out_spans,
                                 size_t *out_count) {
  int rc = CMP_SUCCESS;
  cmp_highlight_span_t *spans;
  size_t capacity;
  size_t count;
  size_t i;
  size_t len;

  if (!hl || !source_code || !out_spans || !out_count) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* Dummy language ignore */
  (void)language;

  len = strlen(source_code);
  capacity = 16;
  count = 0;
  rc = CMP_MALLOC(capacity * sizeof(cmp_highlight_span_t), (void **)&spans);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  /* Basic fallback lexer. A real integration bridges to Tree-sitter here. */
  for (i = 0; i < len; i++) {
    if (capacity - count < 4) {
      cmp_highlight_span_t *new_spans;
      size_t new_capacity = capacity * 2;
      rc = CMP_MALLOC(new_capacity * sizeof(cmp_highlight_span_t),
                      (void **)&new_spans);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("OOM\n");
        CMP_FREE(spans);
        return CMP_ERROR_OOM;
      }
      memcpy(new_spans, spans, count * sizeof(cmp_highlight_span_t));
      CMP_FREE(spans);
      spans = new_spans;
      capacity = new_capacity;
    }

    if (source_code[i] == '/' && i + 1 < len && source_code[i + 1] == '*') {
      /* Block comment */
      size_t start = i;
      i += 2;
      while (i + 1 < len &&
             !(source_code[i] == '*' && source_code[i + 1] == '/')) {
        i++;
      }
      if (i + 1 < len) {
        i++;
      }
      spans[count].start_offset = start;
      spans[count].length = (i - start) + 1;
      spans[count].type = CMP_TOKEN_COMMENT;
      count++;
    } else if (source_code[i] == '/' && i + 1 < len &&
               source_code[i + 1] == '/') {
      /* Line comment */
      size_t start = i;
      while (i < len && source_code[i] != '\n') {
        i++;
      }
      spans[count].start_offset = start;
      spans[count].length = (i - start);
      spans[count].type = CMP_TOKEN_COMMENT;
      count++;
      i--;
    } else if (source_code[i] == '"') {
      /* String */
      size_t start = i;
      i++;
      while (i < len && source_code[i] != '"') {
        if (source_code[i] == '\\' && i + 1 < len) {
          i++; /* escape */
        }
        i++;
      }
      spans[count].start_offset = start;
      spans[count].length = (i - start) + 1;
      spans[count].type = CMP_TOKEN_STRING;
      count++;
    } else if (isdigit((unsigned char)source_code[i])) {
      /* Number */
      size_t start = i;
      while (i < len && isdigit((unsigned char)source_code[i])) {
        i++;
      }
      spans[count].start_offset = start;
      spans[count].length = (i - start);
      spans[count].type = CMP_TOKEN_NUMBER;
      count++;
      i--; /* Backtrack one */
    } else if (isalpha((unsigned char)source_code[i]) ||
               source_code[i] == '_') {
      /* Identifier/Keyword */
      size_t start = i;
      int is_keyword = 0;
      int is_function = 0;
      size_t ident_len;
      while (i < len && (isalnum((unsigned char)source_code[i]) ||
                         source_code[i] == '_')) {
        i++;
      }
      ident_len = i - start;

      if (ident_len == 2 && strncmp(source_code + start, "if", 2) == 0)
        is_keyword = 1;
      else if (ident_len == 4 && strncmp(source_code + start, "else", 4) == 0)
        is_keyword = 1;
      else if (ident_len == 6 && strncmp(source_code + start, "return", 6) == 0)
        is_keyword = 1;
      else if (ident_len == 5 && strncmp(source_code + start, "while", 5) == 0)
        is_keyword = 1;
      else if (ident_len == 3 && strncmp(source_code + start, "for", 3) == 0)
        is_keyword = 1;
      else if (ident_len == 3 && strncmp(source_code + start, "int", 3) == 0)
        is_keyword = 1;
      else if (ident_len == 4 && strncmp(source_code + start, "void", 4) == 0)
        is_keyword = 1;

      /* Lookahead for function */
      if (!is_keyword) {
        size_t j = i;
        while (j < len && isspace((unsigned char)source_code[j]))
          j++;
        if (j < len && source_code[j] == '(')
          is_function = 1;
      }

      if (is_keyword || is_function) {
        spans[count].start_offset = start;
        spans[count].length = ident_len;
        spans[count].type = is_keyword ? CMP_TOKEN_KEYWORD : CMP_TOKEN_FUNCTION;
        count++;
      }
      i--; /* Backtrack one */
    } else if (strchr("+-*/=<>!&|", source_code[i])) {
      spans[count].start_offset = i;
      spans[count].length = 1;
      spans[count].type = CMP_TOKEN_OPERATOR;
      count++;
    }
  }

  *out_spans = spans;
  *out_count = count;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_syntax_highlighter_free_spans
 *
 * @param spans Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_syntax_highlighter_free_spans(cmp_highlight_span_t *spans) {
  int rc = CMP_SUCCESS;
  if (spans) {
    rc = CMP_FREE(spans);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
  }
  return CMP_SUCCESS;
}
