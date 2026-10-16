/**
 * @file ui_export.h
 * @brief Dynamic link library import/export macros.
 */

#ifndef UI_EXPORT_H
#define UI_EXPORT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef C_MULTIPLATFORM_EXPORT
#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(C_MULTIPLATFORM_BUILD_SHARED)
#if defined(ui_engine_EXPORTS) || defined(c_multiplatform_cdk_EXPORTS) ||      \
    defined(c_multiplatform_material2_EXPORTS) ||                              \
    defined(c_multiplatform_material3_EXPORTS) ||                              \
    defined(c_multiplatform_cupertino_EXPORTS)
/** @brief Dynamic library export attribute. */
#define C_MULTIPLATFORM_EXPORT __declspec(dllexport)
#else
/** @brief Dynamic library import attribute. */
#define C_MULTIPLATFORM_EXPORT __declspec(dllimport)
#endif
#else
/** @brief Static library export attribute. */
#define C_MULTIPLATFORM_EXPORT
#endif
#elif defined(__GNUC__) && __GNUC__ >= 4
/** @brief GCC/Clang default visibility attribute. */
#define C_MULTIPLATFORM_EXPORT __attribute__((visibility("default")))
#else
/** @brief Default empty export attribute. */
#define C_MULTIPLATFORM_EXPORT
#endif
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_EXPORT_H */
