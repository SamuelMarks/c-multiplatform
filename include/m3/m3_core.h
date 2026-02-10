#ifndef M3_CORE_H
#define M3_CORE_H

/**
 * @file m3_core.h
 * @brief Core types, error codes, and ABI helpers for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h>
#include <stddef.h>

/** @brief Major version number. */
#define M3_VERSION_MAJOR 0
/** @brief Minor version number. */
#define M3_VERSION_MINOR 1
/** @brief Patch version number. */
#define M3_VERSION_PATCH 0

/**
 * @def M3_CALL
 * @brief Calling convention macro for public API functions.
 */
#ifndef M3_CALL
#if defined(_WIN32) || defined(_WIN64)
#define M3_CALL __cdecl
#else
#define M3_CALL
#endif
#endif

/**
 * @def M3_API
 * @brief Import/export annotation macro for public symbols.
 */
#ifndef M3_API
#if defined(_WIN32) || defined(_WIN64)
#if defined(M3_BUILD_DLL)
#define M3_API __declspec(dllexport)
#elif defined(M3_USE_DLL)
#define M3_API __declspec(dllimport)
#else
#define M3_API
#endif
#else
#define M3_API
#endif
#endif

/** @brief Boolean type used across the ABI. */
typedef int M3Bool;
/** @brief Boolean true value. */
#define M3_TRUE 1
/** @brief Boolean false value. */
#define M3_FALSE 0

/** @brief Signed 8-bit integer. */
typedef signed char m3_i8;
/** @brief Unsigned 8-bit integer. */
typedef unsigned char m3_u8;
/** @brief Signed 16-bit integer. */
typedef signed short m3_i16;
/** @brief Unsigned 16-bit integer. */
typedef unsigned short m3_u16;
/** @brief Signed 32-bit integer. */
typedef signed int m3_i32;
/** @brief Unsigned 32-bit integer. */
typedef unsigned int m3_u32;
/** @brief Signed size type. */
typedef ptrdiff_t m3_isize;
/** @brief Unsigned size type. */
typedef size_t m3_usize;

/**
 * @def M3_STATIC_ASSERT
 * @brief Compile-time assertion helper.
 */
#define M3_STATIC_ASSERT(name, expr)                                           \
  typedef char m3_static_assert_##name[(expr) ? 1 : -1]

#ifndef M3_DOXYGEN
M3_STATIC_ASSERT(char_is_8_bits, CHAR_BIT == 8);
M3_STATIC_ASSERT(short_is_16_bits, sizeof(short) == 2);
M3_STATIC_ASSERT(int_is_32_bits, sizeof(int) == 4);
M3_STATIC_ASSERT(size_t_at_least_32_bits, sizeof(size_t) >= 4);
#endif

/** @brief Mark an unused variable or parameter. */
#define M3_UNUSED(x) ((void)(x))
/** @brief Count elements in a fixed-size array. */
#define M3_COUNTOF(x) (sizeof(x) / sizeof((x)[0]))

/** @brief Success result code. */
#define M3_OK 0
/** @brief Unknown failure. */
#define M3_ERR_UNKNOWN -1
/** @brief Invalid argument or null pointer. */
#define M3_ERR_INVALID_ARGUMENT -2
/** @brief Allocation failed. */
#define M3_ERR_OUT_OF_MEMORY -3
/** @brief Resource not found. */
#define M3_ERR_NOT_FOUND -4
/** @brief Resource not ready yet. */
#define M3_ERR_NOT_READY -5
/** @brief Operation is unsupported. */
#define M3_ERR_UNSUPPORTED -6
/** @brief I/O failure. */
#define M3_ERR_IO -7
/** @brief Invalid state for operation. */
#define M3_ERR_STATE -8
/** @brief Value outside allowed range. */
#define M3_ERR_RANGE -9
/** @brief Operation timed out. */
#define M3_ERR_TIMEOUT -10
/** @brief Permission denied. */
#define M3_ERR_PERMISSION -11
/** @brief Resource is busy. */
#define M3_ERR_BUSY -12
/** @brief Data corruption detected. */
#define M3_ERR_CORRUPT -13
/** @brief Arithmetic overflow. */
#define M3_ERR_OVERFLOW -14
/** @brief Arithmetic underflow. */
#define M3_ERR_UNDERFLOW -15

/**
 * @brief Allocation function signature.
 * @param ctx User-provided allocator context.
 * @param size Number of bytes to allocate.
 * @param out_ptr Receives the allocated block on success.
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3AllocFn)(void *ctx, m3_usize size, void **out_ptr);

/**
 * @brief Reallocation function signature.
 * @param ctx User-provided allocator context.
 * @param ptr Existing allocation to resize.
 * @param size New size in bytes.
 * @param out_ptr Receives the reallocated block on success.
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3ReallocFn)(void *ctx, void *ptr, m3_usize size,
                                  void **out_ptr);

/**
 * @brief Free function signature.
 * @param ctx User-provided allocator context.
 * @param ptr Allocation to release.
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3FreeFn)(void *ctx, void *ptr);

/**
 * @brief Allocator interface used throughout the ABI.
 */
typedef struct M3Allocator {
  void *ctx;           /**< User-defined allocator context. */
  M3AllocFn alloc;     /**< Allocation callback. */
  M3ReallocFn realloc; /**< Reallocation callback. */
  M3FreeFn free;       /**< Free callback. */
} M3Allocator;

/**
 * @brief Obtain the default system allocator.
 * @param out_alloc Receives the allocator interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_get_default_allocator(M3Allocator *out_alloc);

#ifdef M3_TESTING
/**
 * @brief Force default allocator retrieval to fail when enabled (tests only).
 * @param fail M3_TRUE to force failure, M3_FALSE to restore normal behavior.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_core_test_set_default_allocator_fail(M3Bool fail);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_CORE_H */
