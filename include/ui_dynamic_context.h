/**
 * @file ui_dynamic_context.h
 * @brief Scoped dynamic data context for runtime widgets, form controls, and
 * reactive bindings.
 */

#ifndef UI_DYNAMIC_CONTEXT_H
#define UI_DYNAMIC_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_signal.h"
#include "ui_computed.h"
#include "ui_form_group.h"
#include "ui_form_control.h"
#include "ui_app_state_registry.h"
#include "ui_arena.h"
#include <stddef.h>
/* clang-format on */

struct ui_dynamic_context;

/**
 * @brief Creates a new dynamic data context allocated from an arena.
 *
 * @param arena The memory arena to allocate from.
 * @param out_ctx Pointer to receive the allocated context.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_dynamic_context_create(
    struct ui_arena *arena, struct ui_dynamic_context **out_ctx);

/**
 * @brief Destroys a dynamic context.
 *
 * @param ctx The context to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_dynamic_context_destroy(struct ui_dynamic_context *ctx);

/**
 * @brief Registers a scoped signal into the dynamic context.
 *
 * @param ctx The dynamic context.
 * @param name Signal identifier name.
 * @param signal The signal instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_dynamic_context_register_signal(
    struct ui_dynamic_context *ctx, const char *name, struct ui_signal *signal);

/**
 * @brief Retrieves a scoped signal from the context by name.
 *
 * @param ctx The dynamic context.
 * @param name Signal identifier name.
 * @param out_signal Pointer to receive the found signal.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if not present.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_dynamic_context_get_signal(const struct ui_dynamic_context *ctx,
                              const char *name, struct ui_signal **out_signal);

/**
 * @brief Registers a form group into the dynamic context.
 *
 * @param ctx The dynamic context.
 * @param name Form group identifier name (e.g. "user").
 * @param group The form group instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_dynamic_context_register_form_group(
    struct ui_dynamic_context *ctx, const char *name, ui_form_group_t *group);

/**
 * @brief Retrieves a form group by name from the context.
 *
 * @param ctx The dynamic context.
 * @param name Form group identifier name.
 * @param out_group Pointer to receive the form group.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if not present.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_dynamic_context_get_form_group(
    const struct ui_dynamic_context *ctx, const char *name,
    ui_form_group_t **out_group);

/**
 * @brief Registers a computed signal value into the dynamic context.
 *
 * @param ctx The dynamic context.
 * @param name Identifier name.
 * @param comp The computed value instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_dynamic_context_register_computed(
    struct ui_dynamic_context *ctx, const char *name, struct ui_computed *comp);

/**
 * @brief Retrieves a computed value by name from the context.
 *
 * @param ctx The dynamic context.
 * @param name Identifier name.
 * @param out_comp Pointer to receive the computed value.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if not present.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_dynamic_context_get_computed(
    const struct ui_dynamic_context *ctx, const char *name,
    struct ui_computed **out_comp);

/**
 * @brief Resolves a form control by path (e.g. "user.email").
 * Looks up the "user" form group and extracts the "email" control.
 *
 * @param ctx The dynamic context.
 * @param path Dotted path to form control.
 * @param out_ctrl Pointer to receive the resolved form control.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND /
 * UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_dynamic_context_resolve_form_control(const struct ui_dynamic_context *ctx,
                                        const char *path,
                                        ui_form_control_t **out_ctrl);

/**
 * @brief Resolves a signal by path, supporting global "app.<name>" routing.
 * If path starts with "app.", queries the app state registry.
 * Otherwise, queries the local dynamic context.
 *
 * @param ctx The dynamic context.
 * @param app_state The optional app state registry (uses global default if
 * NULL).
 * @param path Signal identifier path (e.g. "app.cart_count" or "user_id").
 * @param out_signal Pointer to receive the resolved signal.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_dynamic_context_resolve_signal(
    const struct ui_dynamic_context *ctx,
    const struct ui_app_state_registry *app_state, const char *path,
    struct ui_signal **out_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_DYNAMIC_CONTEXT_H */
