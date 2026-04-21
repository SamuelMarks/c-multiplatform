/* clang-format off */
#include "cmp_sse_parser.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_sse_parser {
  cmp_sse_event_cb callback;
  void *user_data;
  char buffer[4096];
  unsigned int buffer_len;
};

int cmp_sse_parser_create(cmp_sse_parser_t **out_parser,
                          cmp_sse_event_cb callback, void *user_data) {
  cmp_sse_parser_t *parser;

  if (!out_parser) {
    return -1;
  }

  parser = (cmp_sse_parser_t *)malloc(sizeof(cmp_sse_parser_t));
  if (!parser) {
    return -2;
  }

  parser->callback = callback;
  parser->user_data = user_data;
  parser->buffer_len = 0;
  memset(parser->buffer, 0, sizeof(parser->buffer));

  *out_parser = parser;
  return 0;
}

int cmp_sse_parser_destroy(cmp_sse_parser_t *parser) {
  if (!parser) {
    return -1;
  }
  free(parser);
  return 0;
}

int cmp_sse_parser_feed(cmp_sse_parser_t *parser, const char *chunk,
                        unsigned int len) {
  unsigned int space;

  if (!parser || !chunk) {
    return -1;
  }

  space = sizeof(parser->buffer) - parser->buffer_len - 1;
  if (len > space) {
    len = space; /* Truncate if overflowing buffer for this mock */
  }

  memcpy(parser->buffer + parser->buffer_len, chunk, len);
  parser->buffer_len += len;
  parser->buffer[parser->buffer_len] = '\0';

  /* Basic check for double newline (end of event) */
  if (strstr(parser->buffer, "\n\n")) {
    if (parser->callback) {
      parser->callback(NULL, parser->buffer, parser->user_data);
    }
    parser->buffer_len = 0;
    parser->buffer[0] = '\0';
  }

  return 0;
}
