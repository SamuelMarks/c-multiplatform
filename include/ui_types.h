/**
 * @file ui_types.h
 * @brief Standard boolean and integer types for the UI engine.
 */

#ifndef UI_TYPES_H
#define UI_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup ui_types Standard Types
 * @brief Cross-platform integer and boolean definitions.
 * @{
 */

/**
 * @brief Standard boolean type.
 */
typedef int ui_bool_t;
/**
 * @brief Boolean true value.
 */
#define UI_TRUE 1
/**
 * @brief Boolean false value.
 */
#define UI_FALSE 0

#if defined(_MSC_VER)
/**
 * @brief Signed 8-bit integer.
 */
typedef signed __int8 ui_int8;
#else
/**
 * @brief Signed 8-bit integer.
 */
typedef signed char ui_int8;
#endif

#if defined(_MSC_VER)
/**
 * @brief Unsigned 8-bit integer.
 */
typedef unsigned __int8 ui_uint8;
#else
/**
 * @brief Unsigned 8-bit integer.
 */
typedef unsigned char ui_uint8;
#endif

#if defined(_MSC_VER)
/**
 * @brief Signed 16-bit integer.
 */
typedef signed __int16 ui_int16;
#else
/**
 * @brief Signed 16-bit integer.
 */
typedef signed short ui_int16;
#endif

#if defined(_MSC_VER)
/**
 * @brief Unsigned 16-bit integer.
 */
typedef unsigned __int16 ui_uint16;
#else
/**
 * @brief Unsigned 16-bit integer.
 */
typedef unsigned short ui_uint16;
#endif

#if defined(_MSC_VER)
/**
 * @brief Signed 32-bit integer.
 */
typedef signed __int32 ui_int32;
#else
/**
 * @brief Signed 32-bit integer.
 */
typedef signed int ui_int32;
#endif

#if defined(_MSC_VER)
/**
 * @brief Unsigned 32-bit integer.
 */
typedef unsigned __int32 ui_uint32;
#else
/**
 * @brief Unsigned 32-bit integer.
 */
typedef unsigned int ui_uint32;
#endif

#if defined(_MSC_VER)
/**
 * @brief Signed 64-bit integer.
 */
typedef signed __int64 ui_int64;
#else
#if defined(__GNUC__) || defined(__clang__)
/**
 * @brief Signed 64-bit integer.
 */
__extension__ typedef signed long long ui_int64;
#else
/**
 * @brief Signed 64-bit integer.
 */
typedef signed long long ui_int64;
#endif
#endif

#if defined(_MSC_VER)
/**
 * @brief Unsigned 64-bit integer.
 */
typedef unsigned __int64 ui_uint64;
#else
#if defined(__GNUC__) || defined(__clang__)
/**
 * @brief Unsigned 64-bit integer.
 */
__extension__ typedef unsigned long long ui_uint64;
#else
/**
 * @brief Unsigned 64-bit integer.
 */
typedef unsigned long long ui_uint64;
#endif
#endif

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) ||            \
    defined(__aarch64__) || defined(_M_X64) || defined(_M_ARM64)
/**
 * @brief Signed pointer-sized integer.
 */
typedef ui_int64 ui_intptr;
#else
/**
 * @brief Signed pointer-sized integer.
 */
typedef ui_int32 ui_intptr;
#endif

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) ||            \
    defined(__aarch64__) || defined(_M_X64) || defined(_M_ARM64)
/**
 * @brief Unsigned pointer-sized integer.
 */
typedef ui_uint64 ui_uintptr;
#else
/**
 * @brief Unsigned pointer-sized integer.
 */
typedef ui_uint32 ui_uintptr;
#endif

/**
 * @brief Format specifier for signed 64-bit integer.
 */
#if defined(_MSC_VER) && !defined(__clang__)
#define UI_PRId64 "I64d"
#else
#define UI_PRId64 "lld"
#endif

/**
 * @brief Format specifier for unsigned 64-bit integer.
 */
#if defined(_MSC_VER) && !defined(__clang__)
#define UI_PRIu64 "I64u"
#else
#define UI_PRIu64 "llu"
#endif

/**
 * @brief Format specifier for hexadecimal unsigned 64-bit integer.
 */
#if defined(_MSC_VER) && !defined(__clang__)
#define UI_PRIx64 "I64x"
#else
#define UI_PRIx64 "llx"
#endif

/**
 * @brief Thread-local storage qualifier.
 */
#if defined(_MSC_VER)
#define UI_THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__) || defined(__clang__)
#define UI_THREAD_LOCAL __thread
#else
#error "Thread local storage not supported on this compiler."
#endif

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TYPES_H */
