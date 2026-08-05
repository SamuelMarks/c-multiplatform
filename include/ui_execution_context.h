#ifndef UI_EXECUTION_CONTEXT_H
#define UI_EXECUTION_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Opaque handle representing an isolated execution context.
 */
struct ui_execution_context;

/**
 * @brief Creates a new execution context.
 *
 * @param out_ctx Pointer to receive the new context handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_execution_context_create(struct ui_execution_context **out_ctx);

/**
 * @brief Destroys an execution context and frees its resources.
 *
 * @param ctx The context to destroy.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT if ctx is NULL.
 */
ui_error_t ui_execution_context_destroy(struct ui_execution_context *ctx);

/**
 * @brief Schedules a task for execution in the context.
 *
 * @param ctx The context to schedule the task on.
 * @param callback The function to execute.
 * @param user_data Opaque pointer passed to the callback.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_execution_context_schedule(struct ui_execution_context *ctx,
                                         ui_error_t (*callback)(void *),
                                         void *user_data);

/**
 * @brief Processes scheduled tasks.
 *
 * @param ctx The context to tick.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_execution_context_tick(struct ui_execution_context *ctx);

/**
 * @brief Sets the current execution context for the calling thread.
 *
 * @param ctx The execution context to set.
 * @return ui_error_t UI_ERROR_NONE on success.
 */
ui_error_t ui_execution_context_set_current(struct ui_execution_context *ctx);

/**
 * @brief Retrieves the current execution context for the calling thread.
 *
 * @param out_ctx Pointer to receive the current execution context.
 * @return ui_error_t UI_ERROR_NONE on success.
 */
ui_error_t
ui_execution_context_get_current(struct ui_execution_context **out_ctx);

ui_error_t ui_execution_context_cancel(struct ui_execution_context *ctx);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_EXECUTION_CONTEXT_H */
