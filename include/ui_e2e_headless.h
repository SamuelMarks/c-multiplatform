#ifndef UI_E2E_HEADLESS_H
#define UI_E2E_HEADLESS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_event.h"
#include "ui_window_backend.h"
#include "ui_engine.h"
/* clang-format on */

/**
 * @brief Opaque context for a Headless E2E testing session.
 */
struct ui_e2e_headless_ctx;

/**
 * @brief Creates a headless E2E context and an associated dummy window backend.
 *
 * @param width The initial dummy window width.
 * @param height The initial dummy window height.
 * @param out_ctx Pointer to receive the headless context.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_e2e_headless_create(int width, int height,
                                     struct ui_e2e_headless_ctx **out_ctx);

/**
 * @brief Destroys the headless E2E context.
 *
 * @param ctx The headless context.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_e2e_headless_destroy(struct ui_e2e_headless_ctx *ctx);

/**
 * @brief Gets the underlying window backend which can be injected into the
 * engine.
 *
 * @param ctx The headless context.
 * @param out_backend Output pointer for the backend.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_e2e_headless_get_backend(struct ui_e2e_headless_ctx *ctx,
                            struct ui_window_backend **out_backend);

/**
 * @brief Directly pushes a synthetic input event into the headless event queue.
 *
 * @param ctx The headless context.
 * @param event The event to push.
 * @return UI_ERROR_NONE on success, or UI_ERROR_OUT_OF_MEMORY.
 */
enum ui_error ui_e2e_headless_push_event(struct ui_e2e_headless_ctx *ctx,
                                         const struct ui_event *event);

/**
 * @brief Convenience function to synthesize a sequence of pointer (mouse)
 * events for a full click.
 *
 * It will enqueue MouseDown followed by MouseUp at the specified coordinates.
 *
 * @param ctx The headless context.
 * @param x The X coordinate.
 * @param y The Y coordinate.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_e2e_headless_click(struct ui_e2e_headless_ctx *ctx, int x,
                                    int y);

/**
 * @brief Convenience function to synthesize a sequence of keyboard events for a
 * single keystroke.
 *
 * It will enqueue KeyDown followed by KeyUp.
 *
 * @param ctx The headless context.
 * @param key_code The ui_key_code to inject.
 * @param modifiers The active modifier flags.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_e2e_headless_type_key(struct ui_e2e_headless_ctx *ctx,
                                       int key_code, unsigned int modifiers);

/**
 * \brief Advances the virtual time in the headless context to trigger
 * animations and timers.
 *
 * @param ctx The headless context.
 * @param delta_ms The time in milliseconds to advance.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_e2e_advance_time(struct ui_e2e_headless_ctx *ctx,
                                  double delta_ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_E2E_HEADLESS_H */

/**
 * @brief Advances the virtual time in the headless context to trigger
 * animations and timers.
 *
 * @param ctx The headless context.
 * @param delta_ms The time in milliseconds to advance.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_e2e_advance_time(struct ui_e2e_headless_ctx *ctx,
                                  double delta_ms);
