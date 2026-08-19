/**
 * @file ui_e2e_headful.h
 * @brief End-to-end headful testing context (injects hardware events into real
 * OS windows).
 */

#ifndef UI_E2E_HEADFUL_H
#define UI_E2E_HEADFUL_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_event.h"
#include "ui_window_backend.h"
/* clang-format on */

/**
 * @brief Opaque context for a Headful E2E testing session.
 */
struct ui_e2e_headful_ctx;

/**
 * @brief Creates a headful E2E context bound to an existing actual window.
 *
 * @param backend The active OS window backend.
 * @param window The actual OS window.
 * @param out_ctx Pointer to receive the headful context.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_e2e_headful_create(struct ui_window_backend *backend,
                                 struct ui_window *window,
                                 struct ui_e2e_headful_ctx **out_ctx);

/**
 * @brief Destroys the headful E2E context.
 *
 * @param ctx The headful context.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_e2e_headful_destroy(struct ui_e2e_headful_ctx *ctx);

/**
 * @brief Synthesizes a sequence of pointer (mouse) events for a full click at
 * client coordinates.
 *
 * It will translate client x/y into OS screen coordinates and use OS APIs (e.g.
 * SendInput) to inject the hardware event.
 *
 * @param ctx The headful context.
 * @param x The X client coordinate.
 * @param y The Y client coordinate.
 * @return UI_ERROR_NONE on success, UI_ERROR_UNSUPPORTED if OS injection is not
 * implemented on this platform.
 */
ui_error_t ui_e2e_headful_click(struct ui_e2e_headful_ctx *ctx, int x, int y);

/**
 * @brief Synthesizes a sequence of keyboard events for a single keystroke.
 *
 * @param ctx The headful context.
 * @param key_code The ui_key_code to inject.
 * @param modifiers The active modifier flags.
 * @return UI_ERROR_NONE on success, UI_ERROR_UNSUPPORTED if OS injection is not
 * implemented on this platform.
 */
ui_error_t ui_e2e_headful_type_key(struct ui_e2e_headful_ctx *ctx, int key_code,
                                   unsigned int modifiers);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_E2E_HEADFUL_H */
