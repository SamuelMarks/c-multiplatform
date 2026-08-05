#ifndef UI_COERCION_UTILS_H
#define UI_COERCION_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Abstract format specifier macro for signed 64-bit integer.
 */
#if defined(_MSC_VER)
#define UI_FMT_I64 "%I64d"
#else
#define UI_FMT_I64 "%lld"
#endif

/**
 * @brief Abstract format specifier macro for unsigned 64-bit integer.
 */
#if defined(_MSC_VER)
#define UI_FMT_U64 "%I64u"
#else
#define UI_FMT_U64 "%llu"
#endif

/**
 * @brief Safely parses a string into a boolean.
 *
 * @param str The string to parse ("true", "false", "1", "0").
 * @param out_val Pointer to receive the parsed boolean value (1 or 0).
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on parsing
 * failure.
 */
ui_error_t ui_coerce_string_to_bool(const char *str, int *out_val);

/**
 * @brief Safely parses a string into an integer.
 *
 * @param str The string to parse.
 * @param out_val Pointer to receive the parsed integer.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on parsing
 * failure.
 */
ui_error_t ui_coerce_string_to_int(const char *str, int *out_val);

/**
 * @brief Safely parses a string into a float.
 *
 * @param str The string to parse.
 * @param out_val Pointer to receive the parsed float.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on parsing
 * failure.
 */
ui_error_t ui_coerce_string_to_float(const char *str, float *out_val);

/**
 * @brief Safely copies a string, guaranteeing null termination.
 * Uses strcpy_s / strncpy_s on MSVC, and strncpy on standard environments.
 *
 * @param dest The destination buffer.
 * @param dest_size The size of the destination buffer in bytes.
 * @param src The source string to copy.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if truncated.
 */
ui_error_t ui_safe_string_copy(char *dest, size_t dest_size, const char *src);

/**
 * @brief Safely formats a string.
 * Uses vsprintf_s on MSVC, vsnprintf on standard environments.
 *
 * @param dest Destination buffer.
 * @param dest_size Size of destination buffer in bytes.
 * @param format Format string.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if truncated.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 3, 4)))
#endif
/** \brief ui_error */
ui_error_t
ui_safe_string_format(char *dest, size_t dest_size, const char *format, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_COERCION_UTILS_H */
