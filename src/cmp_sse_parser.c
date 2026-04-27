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
 * @brief Create an SSE parser.
 *
 * @param out_parser Parameter description.
 * @param callback Parameter description.
 * @param user_data Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_sse_parser_create(cmp_sse_parser_t **out_parser,
                          cmp_sse_event_cb callback, void *user_data) {
  int rc;
  cmp_sse_parser_t *parser;

  rc = CMP_SUCCESS;

  if (out_parser == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_sse_parser_t), (void **)&parser);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  parser->callback = callback;
  parser->user_data = user_data;
  parser->buffer_len = 0;
  memset(parser->buffer, 0, sizeof(parser->buffer));

  *out_parser = parser;
  return CMP_SUCCESS;
}

/**
 * @brief Destroy an SSE parser.
 *
 * @param parser Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_sse_parser_destroy(cmp_sse_parser_t *parser) {
  int rc;

  rc = CMP_SUCCESS;

  if (parser == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(parser);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Feed data to an SSE parser.
 *
 * @param parser Parameter description.
 * @param chunk Parameter description.
 * @param len Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_sse_parser_feed(cmp_sse_parser_t *parser, const char *chunk,
                        unsigned int len) {
  unsigned int space;

  if (parser == NULL || chunk == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  space = (unsigned int)(sizeof(parser->buffer) - parser->buffer_len - 1);
  if (len > space) {
    len = space;
  }

  memcpy(parser->buffer + parser->buffer_len, chunk, len);
  parser->buffer_len += len;
  parser->buffer[parser->buffer_len] = '\0';

  if (strstr(parser->buffer, "\n\n") != NULL) {
    if (parser->callback != NULL) {
      parser->callback(NULL, parser->buffer, parser->user_data);
    }
    parser->buffer_len = 0;
    parser->buffer[0] = '\0';
  }

  return CMP_SUCCESS;
}
