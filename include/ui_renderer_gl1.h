#ifndef UI_RENDERER_GL1_H
#define UI_RENDERER_GL1_H

/**
 * \file ui_renderer_gl1.h
 * \brief UI Renderer GL1 component.
 *
 * This file contains definitions for creating and destroying
 * a legacy OpenGL 1.1 renderer backend.
 */

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
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_renderer_gl1_create(struct ui_renderer_backend **out_backend);

/**
 * @brief Destroys a legacy OpenGL 1.1 renderer backend.
 *
 * @param backend The backend to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_renderer_gl1_destroy(struct ui_renderer_backend *backend);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_RENDERER_GL1_H */
