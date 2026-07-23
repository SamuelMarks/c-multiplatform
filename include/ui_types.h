#ifndef UI_TYPES_H
#define UI_TYPES_H

/* clang-format off */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Standard boolean type.
 */
typedef int ui_bool_t;
#define UI_TRUE 1
#define UI_FALSE 0

/**
 * @brief Signed 8-bit integer.
 */
#if defined(_MSC_VER)
typedef signed __int8 ui_int8;
#else
typedef signed char ui_int8;
#endif

/**
 * @brief Unsigned 8-bit integer.
 */
#if defined(_MSC_VER)
typedef unsigned __int8 ui_uint8;
#else
typedef unsigned char ui_uint8;
#endif

/**
 * @brief Signed 16-bit integer.
 */
#if defined(_MSC_VER)
typedef signed __int16 ui_int16;
#else
typedef signed short ui_int16;
#endif

/**
 * @brief Unsigned 16-bit integer.
 */
#if defined(_MSC_VER)
typedef unsigned __int16 ui_uint16;
#else
typedef unsigned short ui_uint16;
#endif

/**
 * @brief Signed 32-bit integer.
 */
#if defined(_MSC_VER)
typedef signed __int32 ui_int32;
#else
typedef signed int ui_int32;
#endif

/**
 * @brief Unsigned 32-bit integer.
 */
#if defined(_MSC_VER)
typedef unsigned __int32 ui_uint32;
#else
typedef unsigned int ui_uint32;
#endif

/**
 * @brief Signed 64-bit integer.
 */
#if defined(_MSC_VER)
typedef signed __int64 ui_int64;
#else
#if defined(__GNUC__) || defined(__clang__)
__extension__ typedef signed long long ui_int64;
#else
typedef signed long long ui_int64;
#endif
#endif

/**
 * @brief Unsigned 64-bit integer.
 */
#if defined(_MSC_VER)
typedef unsigned __int64 ui_uint64;
#else
#if defined(__GNUC__) || defined(__clang__)
__extension__ typedef unsigned long long ui_uint64;
#else
typedef unsigned long long ui_uint64;
#endif
#endif

/**
 * @brief Signed pointer-sized integer.
 */
#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) ||            \
    defined(__aarch64__) || defined(_M_X64) || defined(_M_ARM64)
typedef ui_int64 ui_intptr;
#else
typedef ui_int32 ui_intptr;
#endif

/**
 * @brief Unsigned pointer-sized integer.
 */
#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) ||            \
    defined(__aarch64__) || defined(_M_X64) || defined(_M_ARM64)
typedef ui_uint64 ui_uintptr;
#else
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

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* clang-format on */

#endif /* UI_TYPES_H */
