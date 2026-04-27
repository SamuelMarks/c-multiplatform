/* clang-format off */
#include "cmp_sse_parser.h"
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include "cmp_log.h"
/* clang-format on */

struct cmp_sse_parser {
  cmp_sse_event_cb callback;
  void *user_data;
  char buffer[4096];
  unsigned int buffer_len;
};

/**
 * @brief cmp_sse_parser_create
 *
 * @param out_parser Parameter description.
 * @param callback Parameter description.
 * @param user_data Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_sse_parser_create(cmp_sse_parser_t **out_parser,
                          cmp_sse_event_cb callback, void *user_data) {
  int rc = CMP_SUCCESS;
  cmp_sse_parser_t *parser = NULL;

  if (!out_parser) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_sse_parser_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_sse_parser_t), (void **)&parser);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_sse_parser_create: Out of memory\n");
    return CMP_ERROR_OOM;
  }

  parser->callback = callback;
  parser->user_data = user_data;
  parser->buffer_len = 0;
  memset(parser->buffer, 0, sizeof(parser->buffer));

  *out_parser = parser;
  return rc;
}

/**
 * @brief cmp_sse_parser_destroy
 *
 * @param parser Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_sse_parser_destroy(cmp_sse_parser_t *parser) {
  int rc = CMP_SUCCESS;

  if (!parser) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_sse_parser_destroy: Invalid argument\n");
    return rc;
  }

  rc = CMP_FREE(parser);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_sse_parser_destroy: CMP_FREE failed\n");
    return rc;
  }

  return rc;
}

/**
 * @brief cmp_sse_parser_feed
 *
 * @param parser Parameter description.
 * @param chunk Parameter description.
 * @param len Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_sse_parser_feed(cmp_sse_parser_t *parser, const char *chunk,
                        unsigned int len) {
  int rc = CMP_SUCCESS;
  unsigned int space;

  if (!parser || !chunk) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_sse_parser_feed: Invalid argument\n");
    return rc;
  }

  space = (unsigned int)(sizeof(parser->buffer) - parser->buffer_len - 1);
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

  return rc;
}