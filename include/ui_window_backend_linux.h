#ifndef UI_WINDOW_BACKEND_LINUX_H
#define UI_WINDOW_BACKEND_LINUX_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_window_backend.h"
/* clang-format on */

/**
 * @brief Creates a Linux X11 (GLX) or Wayland (EGL) backend instance.
 *
 * @param out_backend Pointer to receive the allocated backend instance.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_window_backend_linux_create(struct ui_window_backend **out_backend);

/**
 * @brief Destroys a Linux backend instance.
 *
 * @param backend The backend instance to destroy.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_window_backend_linux_destroy(struct ui_window_backend *backend);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_WINDOW_BACKEND_LINUX_H */
