#ifndef UI_RENDERER_GL1_H
#define UI_RENDERER_GL1_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_renderer.h"
/* clang-format on */

/**
 * @brief Creates a legacy OpenGL 1.1 renderer backend.
 *
 * @param out_backend Pointer to receive the allocated backend.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_renderer_gl1_create(struct ui_renderer_backend **out_backend);

/**
 * @brief Destroys a legacy OpenGL 1.1 renderer backend.
 *
 * @param backend The backend to destroy.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_renderer_gl1_destroy(struct ui_renderer_backend *backend);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_RENDERER_GL1_H */
