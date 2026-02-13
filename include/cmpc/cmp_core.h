#ifndef CMP_CORE_H
#define CMP_CORE_H

/**
 * @file cmp_core.h
 * @brief Core types, error codes, and ABI helpers for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h>
#include <stddef.h>

/** @brief Major version number. */
#define CMP_VERSION_MAJOR 0
/** @brief Minor version number. */
#define CMP_VERSION_MINOR 1
/** @brief Patch version number. */
#define CMP_VERSION_PATCH 0

/**
 * @def CMP_CALL
 * @brief Calling convention macro for public API functions.
 */
#ifndef CMP_CALL
#if defined(_WIN32) || defined(_WIN64)
#define CMP_CALL __cdecl
#else
#define CMP_CALL
#endif
#endif

/**
 * @def CMP_API
 * @brief Import/export annotation macro for public symbols.
 */
#ifndef CMP_API
#if defined(_WIN32) || defined(_WIN64)
#if defined(CMP_BUILD_DLL)
#define CMP_API __declspec(dllexport)
#elif defined(CMP_USE_DLL)
#define CMP_API __declspec(dllimport)
#else
#define CMP_API
#endif
#else
#define CMP_API
#endif
#endif

/** @brief Boolean type used across the ABI. */
typedef int CMPBool;
/** @brief Boolean true value. */
#define CMP_TRUE 1
/** @brief Boolean false value. */
#define CMP_FALSE 0

/** @brief Signed 8-bit integer. */
typedef signed char cmp_i8;
/** @brief Unsigned 8-bit integer. */
typedef unsigned char cmp_u8;
/** @brief Signed 16-bit integer. */
typedef signed short cmp_i16;
/** @brief Unsigned 16-bit integer. */
typedef unsigned short cmp_u16;
/** @brief Signed 32-bit integer. */
typedef signed int cmp_i32;
/** @brief Unsigned 32-bit integer. */
typedef unsigned int cmp_u32;
/** @brief Signed size type. */
typedef ptrdiff_t cmp_isize;
/** @brief Unsigned size type. */
typedef size_t cmp_usize;

/**
 * @def CMP_STATIC_ASSERT
 * @brief Compile-time assertion helper.
 */
#define CMP_STATIC_ASSERT(name, expr)                                          \
  typedef char cmp_static_assert_##name[(expr) ? 1 : -1]

#ifndef CMP_DOXYGEN
CMP_STATIC_ASSERT(char_is_8_bits, CHAR_BIT == 8);
CMP_STATIC_ASSERT(short_is_16_bits, sizeof(short) == 2);
CMP_STATIC_ASSERT(int_is_32_bits, sizeof(int) == 4);
CMP_STATIC_ASSERT(size_t_at_least_32_bits, sizeof(size_t) >= 4);
#endif

/** @brief Mark an unused variable or parameter. */
#define CMP_UNUSED(x) ((void)(x))
/** @brief Count elements in a fixed-size array. */
#define CMP_COUNTOF(x) (sizeof(x) / sizeof((x)[0]))

/** @brief Success result code. */
#define CMP_OK 0
/** @brief Unknown failure. */
#define CMP_ERR_UNKNOWN -1
/** @brief Invalid argument or null pointer. */
#define CMP_ERR_INVALID_ARGUMENT -2
/** @brief Allocation failed. */
#define CMP_ERR_OUT_OF_MEMORY -3
/** @brief Resource not found. */
#define CMP_ERR_NOT_FOUND -4
/** @brief Resource not ready yet. */
#define CMP_ERR_NOT_READY -5
/** @brief Operation is unsupported. */
#define CMP_ERR_UNSUPPORTED -6
/** @brief I/O failure. */
#define CMP_ERR_IO -7
/** @brief Invalid state for operation. */
#define CMP_ERR_STATE -8
/** @brief Value outside allowed range. */
#define CMP_ERR_RANGE -9
/** @brief Operation timed out. */
#define CMP_ERR_TIMEOUT -10
/** @brief Permission denied. */
#define CMP_ERR_PERMISSION -11
/** @brief Resource is busy. */
#define CMP_ERR_BUSY -12
/** @brief Data corruption detected. */
#define CMP_ERR_CORRUPT -13
/** @brief Arithmetic overflow. */
#define CMP_ERR_OVERFLOW -14
/** @brief Arithmetic underflow. */
#define CMP_ERR_UNDERFLOW -15

/**
 * @brief Allocation function signature.
 * @param ctx User-provided allocator context.
 * @param size Number of bytes to allocate.
 * @param out_ptr Receives the allocated block on success.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPAllocFn)(void *ctx, cmp_usize size, void **out_ptr);

/**
 * @brief Reallocation function signature.
 * @param ctx User-provided allocator context.
 * @param ptr Existing allocation to resize.
 * @param size New size in bytes.
 * @param out_ptr Receives the reallocated block on success.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPReallocFn)(void *ctx, void *ptr, cmp_usize size,
                                    void **out_ptr);

/**
 * @brief Free function signature.
 * @param ctx User-provided allocator context.
 * @param ptr Allocation to release.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPFreeFn)(void *ctx, void *ptr);

/**
 * @brief Allocator interface used throughout the ABI.
 */
typedef struct CMPAllocator {
  void *ctx;            /**< User-defined allocator context. */
  CMPAllocFn alloc;     /**< Allocation callback. */
  CMPReallocFn realloc; /**< Reallocation callback. */
  CMPFreeFn free;       /**< Free callback. */
} CMPAllocator;

/**
 * @brief Obtain the default system allocator.
 * @param out_alloc Receives the allocator interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_get_default_allocator(CMPAllocator *out_alloc);

#ifdef CMP_TESTING
/**
 * @brief Force default allocator retrieval to fail when enabled (tests only).
 * @param fail CMP_TRUE to force failure, CMP_FALSE to restore normal behavior.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_core_test_set_default_allocator_fail(CMPBool fail);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_CORE_H */
