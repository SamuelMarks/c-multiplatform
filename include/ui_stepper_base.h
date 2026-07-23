#ifndef UI_STEPPER_BASE_H
#define UI_STEPPER_BASE_H

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
/* clang-format on */

struct ui_stepper_base;

/**
 * @brief Mode of the stepper progression.
 */
enum ui_stepper_mode {
  UI_STEPPER_MODE_LINEAR,    /**< Must complete steps sequentially. */
  UI_STEPPER_MODE_NON_LINEAR /**< Can navigate freely to any step. */
};

/**
 * @brief Individual state of a step.
 */
enum ui_stepper_step_state {
  UI_STEPPER_STEP_STATE_DEFAULT,
  UI_STEPPER_STEP_STATE_ACTIVE,
  UI_STEPPER_STEP_STATE_COMPLETED,
  UI_STEPPER_STEP_STATE_ERROR
};

/**
 * @brief Callback to validate if a step can be navigated away from
 * successfully.
 *
 * @param stepper The stepper instance.
 * @param step_index The index of the step being validated.
 * @param user_data Opaque user data.
 * @return 1 if valid (can proceed), 0 if invalid (block progression).
 */
typedef int (*ui_stepper_validate_t)(struct ui_stepper_base *stepper,
                                     int step_index, void *user_data);

/**
 * @brief Creates a new unstyled stepper base component.
 *
 * @param out_stepper Pointer to receive the allocated stepper base.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_stepper_base_create(struct ui_stepper_base **out_stepper);

/**
 * @brief Destroys a stepper base component.
 *
 * @param stepper The stepper component to destroy.
 */
void ui_stepper_base_destroy(struct ui_stepper_base *stepper);

/**
 * @brief Sets the progression mode (linear or non-linear).
 * Default is UI_STEPPER_MODE_LINEAR.
 *
 * @param stepper The stepper.
 * @param mode The progression mode.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_stepper_base_set_mode(struct ui_stepper_base *stepper,
                                       enum ui_stepper_mode mode);

/**
 * @brief Sets the validation callback for linear progression.
 *
 * @param stepper The stepper.
 * @param hook The validation function.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_stepper_base_set_validate_hook(struct ui_stepper_base *stepper,
                                                ui_stepper_validate_t hook,
                                                void *user_data);

/**
 * @brief Adds a new step to the stepper.
 * The component takes ownership of header_node and content_node.
 *
 * @param stepper The stepper component.
 * @param step_id A unique string identifier for the step.
 * @param header_node The DOM node representing the step's header/label.
 * @param content_node The DOM node representing the step's content panel.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_stepper_base_add_step(struct ui_stepper_base *stepper,
                                       const char *step_id,
                                       struct ui_dom_node *header_node,
                                       struct ui_dom_node *content_node);

/**
 * @brief Attempts to set the active step by index.
 * In linear mode, this will invoke the validation hook for intermediate steps
 * if jumping ahead, or block if a previous step isn't complete.
 *
 * @param stepper The stepper.
 * @param index The target step index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_OPERATION_FAILED if blocked.
 */
enum ui_error ui_stepper_base_set_active_index(struct ui_stepper_base *stepper,
                                               int index);

/**
 * @brief Gets the currently active step index.
 *
 * @param stepper The stepper.
 * @param out_index Pointer to receive the active index.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_stepper_base_get_active_index(const struct ui_stepper_base *stepper,
                                 int *out_index);

/**
 * @brief Sets the explicit state of a step.
 * Useful for marking a step as "error" or "completed".
 *
 * @param stepper The stepper.
 * @param index The step index.
 * @param state The target state.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_stepper_base_set_step_state(struct ui_stepper_base *stepper,
                                             int index,
                                             enum ui_stepper_step_state state);

/**
 * @brief Gets the state of a specific step.
 *
 * @param stepper The stepper.
 * @param index The step index.
 * @param out_state Pointer to receive the state.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_stepper_base_get_step_state(const struct ui_stepper_base *stepper, int index,
                               enum ui_stepper_step_state *out_state);

/**
 * @brief Advances to the next step, evaluating validation hooks.
 *
 * @param stepper The stepper.
 * @return UI_ERROR_NONE on success, UI_ERROR_OPERATION_FAILED if validation
 * blocks.
 */
enum ui_error ui_stepper_base_next_step(struct ui_stepper_base *stepper);

/**
 * @brief Goes back to the previous step.
 *
 * @param stepper The stepper.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_stepper_base_prev_step(struct ui_stepper_base *stepper);

/**
 * @brief Gets the underlying UI component.
 *
 * @param stepper The stepper component.
 * @return The underlying component.
 */
enum ui_error
ui_stepper_base_get_component(struct ui_stepper_base *stepper,
                              struct ui_component **out_component);

/**
 * @brief Binds the active state/index to a signal.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_stepper_base_bind_active_index(struct ui_stepper_base *widget,
                                                struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_STEPPER_BASE_H */
