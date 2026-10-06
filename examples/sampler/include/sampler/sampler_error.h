/**
 * @file sampler_error.h
 * @brief Error codes and status reporting for Compose Material Catalog.
 */

#ifndef SAMPLER_ERROR_H
#define SAMPLER_ERROR_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @enum sampler_error_t
 * @brief Status and error return codes for sampler operations.
 */
typedef enum sampler_error {
  /** @brief Operation completed successfully without error. */
  SAMPLER_SUCCESS = 0,
  /** @brief A null pointer was passed for a mandatory argument. */
  SAMPLER_ERROR_NULL_POINTER = 1,
  /** @brief Memory allocation failed due to insufficient memory. */
  SAMPLER_ERROR_OUT_OF_MEMORY = 2,
  /** @brief An invalid or out-of-range argument was supplied. */
  SAMPLER_ERROR_INVALID_ARGUMENT = 3,
  /** @brief An index or offset was out of allowable bounds. */
  SAMPLER_ERROR_OUT_OF_BOUNDS = 4,
  /** @brief The requested route was not recognized or found in nav graph. */
  SAMPLER_ERROR_ROUTE_NOT_FOUND = 5,
  /** @brief Storage read, write, or persistence operation failed. */
  SAMPLER_ERROR_STORAGE_FAILURE = 6,
  /** @brief Parsing or deserialization of structured data failed. */
  SAMPLER_ERROR_PARSE_FAILURE = 7,
  /** @brief Failed to attach or mount component to DOM tree. */
  SAMPLER_ERROR_DOM_ATTACH_FAILED = 8,
  /** @brief Application of CSS style property failed. */
  SAMPLER_ERROR_STYLE_PROP_FAILED = 9,
  /** @brief Component or screen layout calculation failed. */
  SAMPLER_ERROR_LAYOUT_FAILED = 10,
  /** @brief Failed to dispatch or propagate UI event. */
  SAMPLER_ERROR_EVENT_DISPATCH_FAILED = 11,
  /** @brief Animation or physics step execution failed. */
  SAMPLER_ERROR_ANIMATION_FAILED = 12
} sampler_error_t;

/**
 * @brief Convert a sampler error code into a human-readable description string.
 * @param error Error code to translate.
 * @param out_str Pointer to const char* receiving the static string
 * description.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_NULL_POINTER if out_str
 * is NULL.
 */
sampler_error_t sampler_error_to_string(sampler_error_t error,
                                        const char **out_str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SAMPLER_ERROR_H */
