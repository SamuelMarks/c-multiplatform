/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/* clang-format on */

struct cmp_syntax_highlighter {
  int is_initialized;
};

int cmp_syntax_highlighter_create(cmp_syntax_highlighter_t **out_hl) {
  cmp_syntax_highlighter_t *hl;

  if (!out_hl) {
    return CMP_ERROR_INVALID_ARG;
  }

  hl = (cmp_syntax_highlighter_t *)malloc(sizeof(cmp_syntax_highlighter_t));
  if (!hl) {
    return CMP_ERROR_OOM;
  }

  hl->is_initialized = 1;
  *out_hl = hl;

  return CMP_SUCCESS;
}

int cmp_syntax_highlighter_destroy(cmp_syntax_highlighter_t *hl) {
  if (!hl) {
    return CMP_ERROR_INVALID_ARG;
  }
  free(hl);
  return CMP_SUCCESS;
}

int cmp_syntax_highlighter_parse(cmp_syntax_highlighter_t *hl,
                                 const char *source_code, const char *language,
                                 cmp_highlight_span_t **out_spans,
                                 size_t *out_count) {
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
  spans =
      (cmp_highlight_span_t *)malloc(capacity * sizeof(cmp_highlight_span_t));

  if (!spans) {
    return CMP_ERROR_OOM;
  }

  /* Naive lexer stub for basic string testing.
     A real integration bridges to Tree-sitter here. */
  for (i = 0; i < len; i++) {
    if (source_code[i] == '"') {
      size_t start = i;
      i++;
      while (i < len && source_code[i] != '"') {
        i++;
      }
      if (count >= capacity) {
        capacity *= 2;
        spans = (cmp_highlight_span_t *)realloc(
            spans, capacity * sizeof(cmp_highlight_span_t));
      }
      spans[count].start_offset = start;
      spans[count].length = (i - start) + 1;
      spans[count].type = CMP_TOKEN_STRING;
      count++;
    } else if (isdigit((unsigned char)source_code[i])) {
      size_t start = i;
      while (i < len && isdigit((unsigned char)source_code[i])) {
        i++;
      }
      if (count >= capacity) {
        capacity *= 2;
        spans = (cmp_highlight_span_t *)realloc(
            spans, capacity * sizeof(cmp_highlight_span_t));
      }
      spans[count].start_offset = start;
      spans[count].length = (i - start);
      spans[count].type = CMP_TOKEN_NUMBER;
      count++;
      i--; /* Backtrack one */
    }
  }

  *out_spans = spans;
  *out_count = count;
  return CMP_SUCCESS;
}

int cmp_syntax_highlighter_free_spans(cmp_highlight_span_t *spans) {
  if (spans) {
    free(spans);
  }
  return CMP_SUCCESS;
}
