/**
 * @file ui_export.h
 * @brief Dynamic link library import/export macros.
 */

#ifndef UI_EXPORT_H
#define UI_EXPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef C_MULTIPLATFORM_EXPORT
#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(C_MULTIPLATFORM_BUILD_SHARED)
#if defined(ui_engine_EXPORTS)
#define C_MULTIPLATFORM_EXPORT __declspec(dllexport)
#else
#define C_MULTIPLATFORM_EXPORT __declspec(dllimport)
#endif
#else
#define C_MULTIPLATFORM_EXPORT
#endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#define C_MULTIPLATFORM_EXPORT __attribute__((visibility("default")))
#else
#define C_MULTIPLATFORM_EXPORT
#endif
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_EXPORT_H */
