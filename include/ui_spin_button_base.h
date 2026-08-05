#ifndef UI_SPIN_BUTTON_BASE_H
#define UI_SPIN_BUTTON_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_event.h"
#include "ui_control_value_accessor.h"
/* clang-format on */

struct ui_spin_button_base;

/**
 * @brief Callback invoked when the spin button value changes.
 */
typedef ui_error_t (*ui_spin_button_on_change_t)(
    struct ui_spin_button_base *spin_button, double value, void *user_data);

/**
 * @brief Creates a new unstyled spin button base component.
 *
 * CSS Custom Properties (Variable Hooks):
 * - `--spin-button-bg`: Background color.
 * - `--spin-button-color`: Text color.
 * - `--spin-button-border`: Border.
 * - `--spin-button-border-radius`: Border radius.
 * - `--spin-button-disabled-opacity`: Opacity when disabled.
 *
 * @param out_spin_button Pointer to receive the allocated spin button base.
 * @param out_cva Optional pointer to receive the CVA interface.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_spin_button_base_create(struct ui_spin_button_base **out_spin_button,
                           struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a spin button base component.
 *
 * @param spin_button The spin button to destroy.
 */
ui_error_t ui_spin_button_base_destroy(struct ui_spin_button_base *spin_button);

/**
 * @brief Sets the minimum value.
 *
 * @param spin_button The spin button.
 * @param min The minimum value.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_spin_button_base_set_min(struct ui_spin_button_base *spin_button,
                                       double min);

/**
 * @brief Sets the maximum value.
 *
 * @param spin_button The spin button.
 * @param max The maximum value.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_spin_button_base_set_max(struct ui_spin_button_base *spin_button,
                                       double max);

/**
 * @brief Sets the current value. Clamps to min/max.
 *
 * @param spin_button The spin button.
 * @param value The current value.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_set_value(struct ui_spin_button_base *spin_button,
                              double value);

/**
 * @brief Gets the current value.
 *
 * @param spin_button The spin button.
 * @return The current value.
 */
ui_error_t
ui_spin_button_base_get_value(const struct ui_spin_button_base *spin_button,
                              double *out_val);

/**
 * @brief Sets the step increment.
 *
 * @param spin_button The spin button.
 * @param step The step increment.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_spin_button_base_set_step(struct ui_spin_button_base *spin_button,
                                        double step);

/**
 * @brief Sets the disabled state.
 *
 * @param spin_button The spin button.
 * @param disabled 1 to disable, 0 to enable.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_set_disabled(struct ui_spin_button_base *spin_button,
                                 int disabled);

/**
 * @brief Sets the change handler.
 *
 * @param spin_button The spin button.
 * @param on_change The callback to invoke when the value changes.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_set_on_change(struct ui_spin_button_base *spin_button,
                                  ui_spin_button_on_change_t on_change,
                                  void *user_data);

/**
 * @brief Increments the value by one step.
 *
 * @param spin_button The spin button.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_increment(struct ui_spin_button_base *spin_button);

/**
 * @brief Decrements the value by one step.
 *
 * @param spin_button The spin button.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_decrement(struct ui_spin_button_base *spin_button);

/**
 * @brief Starts continuous increment (e.g. holding the up button).
 *
 * @param spin_button The spin button.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_spin_button_base_start_continuous_increment(
    struct ui_spin_button_base *spin_button);

/**
 * @brief Starts continuous decrement (e.g. holding the down button).
 *
 * @param spin_button The spin button.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_spin_button_base_start_continuous_decrement(
    struct ui_spin_button_base *spin_button);

/**
 * @brief Stops continuous increment/decrement.
 *
 * @param spin_button The spin button.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_stop_continuous(struct ui_spin_button_base *spin_button);

/**
 * @brief Integrates timer tick for continuous holding.
 *
 * @param spin_button The spin button.
 * @param delta_ms The time passed since last tick in milliseconds.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_spin_button_base_on_tick(struct ui_spin_button_base *spin_button,
                                       double delta_ms);

/**
 * @brief Processes an incoming input event (e.g., Up/Down arrows).
 *
 * @param spin_button The spin button.
 * @param event The input event.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_process_event(struct ui_spin_button_base *spin_button,
                                  const struct ui_event *event);

/**
 * @brief Gets the underlying component instance.
 *
 * @param spin_button The spin button.
 * @return The underlying component.
 */
ui_error_t
ui_spin_button_base_get_component(struct ui_spin_button_base *spin_button,
                                  struct ui_component **out_component);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SPIN_BUTTON_BASE_H */
