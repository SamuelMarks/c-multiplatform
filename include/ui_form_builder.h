#ifndef UI_FORM_BUILDER_H
#define UI_FORM_BUILDER_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
#include "ui_arena.h"
#include "ui_form_control.h"
#include "ui_form_group.h"
#include "ui_form_array.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle to a form builder.
 */
typedef struct ui_form_builder ui_form_builder_t;

/**
 * @brief Creates a new form builder instance.
 *
 * @param arena The arena for allocating form structures.
 * @param out_builder Pointer to store the created form builder.
 * @return ui_error_t
 */
ui_error_t ui_form_builder_create(struct ui_arena *arena,
                                  ui_form_builder_t **out_builder);

/**
 * @brief Starts a new form group in the current hierarchy.
 *
 * @param builder The form builder.
 * @param name The name of the group (if within another group) or NULL for root.
 * @return ui_error_t
 */
ui_error_t ui_form_builder_group_start(ui_form_builder_t *builder,
                                       const char *name);

/**
 * @brief Ends the current form group.
 *
 * @param builder The form builder.
 * @return ui_error_t
 */
ui_error_t ui_form_builder_group_end(ui_form_builder_t *builder);

/**
 * @brief Starts a new form array in the current hierarchy.
 *
 * @param builder The form builder.
 * @param name The name of the array (if within a group).
 * @return ui_error_t
 */
ui_error_t ui_form_builder_array_start(ui_form_builder_t *builder,
                                       const char *name);

/**
 * @brief Ends the current form array.
 *
 * @param builder The form builder.
 * @return ui_error_t
 */
ui_error_t ui_form_builder_array_end(ui_form_builder_t *builder);

/**
 * @brief Adds a control to the current group or array.
 *
 * @param builder The form builder.
 * @param name The name of the control (if within a group). For array elements,
 * this is ignored.
 * @param initial_value The initial value payload.
 * @param type The type of the signal payload.
 * @param validator An optional synchronous validator.
 * @param user_data User data for the validator.
 * @return ui_error_t
 */
ui_error_t ui_form_builder_control(ui_form_builder_t *builder, const char *name,
                                   union ui_signal_payload initial_value,
                                   enum ui_signal_type type,
                                   ui_validator_fn validator, void *user_data);

/**
 * @brief Finalizes the form construction and retrieves the root form group.
 * The builder must have exactly one root group that has been ended.
 *
 * @param builder The form builder.
 * @param out_root Pointer to store the finalized root form group.
 * @return ui_error_t
 */
ui_error_t ui_form_builder_build(ui_form_builder_t *builder,
                                 ui_form_group_t **out_root);

/**
 * @brief Destroys the form builder and frees its internal resources.
 * Note: The built form group and controls are allocated in the provided arena
 * and are not destroyed by this function.
 *
 * @param builder The form builder.
 * @return ui_error_t
 */
ui_error_t ui_form_builder_destroy(ui_form_builder_t *builder);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_FORM_BUILDER_H */
