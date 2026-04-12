#ifndef CMP_SSE_PARSER_H
#define CMP_SSE_PARSER_H

/* clang-format off */
#include "cmp_ffi.h"
/* clang-format on */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_sse_parser.h
 * @brief Server-Sent Events (SSE) base parser for LLM streams.
 */

typedef struct cmp_sse_parser cmp_sse_parser_t;

/**
 * @brief Event callback function signature.
 * @param event_name The event name (e.g. "message"). Can be NULL.
 * @param data The event data. Can be NULL.
 * @param user_data User provided pointer.
 */
typedef void (*cmp_sse_event_cb)(const char *event_name, const char *data,
                                 void *user_data);

/**
 * @brief Creates an SSE parser instance.
 * @param out_parser Pointer to receive the created instance.
 * @param callback The callback to invoke on each event.
 * @param user_data User pointer to pass to the callback.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_sse_parser_create(cmp_sse_parser_t **out_parser,
                                  cmp_sse_event_cb callback, void *user_data);

/**
 * @brief Destroys an SSE parser instance.
 * @param parser The instance to destroy.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_sse_parser_destroy(cmp_sse_parser_t *parser);

/**
 * @brief Feeds a chunk of data into the parser.
 * @param parser The parser instance.
 * @param chunk The data chunk.
 * @param len The length of the chunk.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_sse_parser_feed(cmp_sse_parser_t *parser, const char *chunk,
                                unsigned int len);

#ifdef __cplusplus
}
#endif

#endif /* CMP_SSE_PARSER_H */
