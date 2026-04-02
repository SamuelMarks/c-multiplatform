#ifndef CMP_FFI_H
#define CMP_FFI_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#if defined(_MSC_VER) && (_MSC_VER < 1600)
typedef signed __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
typedef unsigned char uint8_t;
#else
#include <stdint.h>
#endif

#if defined(_WIN32)
#if defined(CMP_BUILD_SHARED)
#define CMP_API __declspec(dllexport)
#elif defined(CMP_USE_SHARED)
#define CMP_API __declspec(dllimport)
#else
#define CMP_API
#endif
#else
#if defined(__GNUC__) && __GNUC__ >= 4
#define CMP_API __attribute__((visibility("default")))
#else
#define CMP_API
#endif
#endif
/* clang-format on */

typedef struct cmp_window cmp_window_t;
typedef struct cmp_ui_node cmp_ui_node_t;
typedef struct cmp_theme cmp_theme_t;
typedef struct cmp_theme_vtable_s cmp_theme_vtable_t;
typedef struct cmp_palette cmp_palette_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_FFI_H */