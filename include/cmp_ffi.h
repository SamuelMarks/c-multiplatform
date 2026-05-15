#ifndef CMP_FFI_H
#define CMP_FFI_H

/**
 * @file cmp_ffi.h
 * @brief Foreign Function Interface (FFI) and visibility macros.
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(_MSC_VER) && (_MSC_VER < 1600)
/** @brief 32-bit signed integer. */
typedef signed __int32 int32_t;
/** @brief 32-bit unsigned integer. */
typedef unsigned __int32 uint32_t;
/** @brief 64-bit signed integer. */
typedef signed __int64 int64_t;
/** @brief 64-bit unsigned integer. */
typedef unsigned __int64 uint64_t;
/** @brief 8-bit unsigned integer. */
typedef unsigned char uint8_t;
#else
/* clang-format off */
#if !defined(_MSC_VER) || _MSC_VER >= 1600
#include <stdint.h>
#endif
/* clang-format on */
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(CMP_BUILD_SHARED)
/** @brief Export macro for shared libraries. */
#define CMP_API __declspec(dllexport)
#elif defined(CMP_USE_SHARED)
/** @brief Import macro for shared libraries. */
#define CMP_API __declspec(dllimport)
#else
/** @brief Empty macro for static linkage. */
#define CMP_API
#endif
#else
#if defined(__GNUC__) && __GNUC__ >= 4
/** @brief Default visibility macro. */
#define CMP_API __attribute__((visibility("default")))
#else
/** @brief Empty macro for unhandled platforms. */
#define CMP_API
#endif
#endif

/** @brief Opaque handle for a window. */
typedef struct cmp_window cmp_window_t;
/** @brief Opaque handle for a UI node. */
typedef struct cmp_ui_node cmp_ui_node_t;
/** @brief Opaque handle for a theme. */
typedef struct cmp_theme cmp_theme_t;
/** @brief Opaque handle for a theme vtable. */
typedef struct cmp_theme_vtable_s cmp_theme_vtable_t;
/** @brief Opaque handle for a color palette. */
typedef struct cmp_palette cmp_palette_t;
/** @brief Opaque handle for dynamic type scaling. */
typedef struct cmp_dynamic_type cmp_dynamic_type_t;
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_FFI_H */
