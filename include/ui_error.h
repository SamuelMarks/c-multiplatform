/**
 * @file ui_error.h
 * @brief Standard error return values for the UI engine.
 */

/* clang-format off */
#ifndef UI_ERROR_H
#define UI_ERROR_H

#include "no_discard.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents standard return discriminants for all engine APIs.
 */
enum ui_error {
  UI_ERROR_NONE = 0,             /**< Success */
  UI_ERROR_OUT_OF_MEMORY = 1,    /**< Allocation failed */
  UI_ERROR_INVALID_ARGUMENT = 2, /**< Null pointer or invalid parameter */
  UI_ERROR_QUEUE_FULL = 3,       /**< Ring buffer is full */
  UI_ERROR_QUEUE_EMPTY = 4,      /**< Ring buffer is empty */
  UI_ERROR_NOT_FOUND = 5,        /**< Item not found */
  UI_ERROR_PARSE_FAILED = 6,     /**< Parsing error */
  UI_ERROR_LAYOUT_VIOLATION = 7, /**< Layout constraints were violated */
  UI_ERROR_EOF = 8,              /**< End of file or stream */
  UI_ERROR_UNKNOWN = 9,          /**< Unspecified or platform error */
  UI_ERROR_IO_FAILED = 10,       /**< Input/Output error */
  UI_ERROR_UNSUPPORTED = 11,     /**< Operation is not supported */
  UI_ERROR_OUT_OF_BOUNDS = 12    /**< Index out of bounds */
};

/**
 * @brief Typedef for ui_error with NO_DISCARD automatically applied to
 * variables and returns.
 */
typedef enum ui_error ui_error_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_ERROR_H */
