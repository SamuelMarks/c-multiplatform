/**
 * @file ui_window_backend_android.h
 * @brief Android NativeActivity + EGL window backend.
 */

#ifndef UI_WINDOW_BACKEND_ANDROID_H
#define UI_WINDOW_BACKEND_ANDROID_H

/* clang-format off */
#include "ui_error.h"
#include "ui_window_backend.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup ui_window_backend Window Backend
 * @{
 */

/**
 * @brief Creates an Android NativeActivity + EGL backend instance.
 *
 * @param out_backend Pointer to receive the allocated backend instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_window_backend_android_create(struct ui_window_backend **out_backend);

/**
 * @brief Destroys an Android NativeActivity + EGL backend instance.
 *
 * @param backend The backend instance to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_window_backend_android_destroy(struct ui_window_backend *backend);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_WINDOW_BACKEND_ANDROID_H */
