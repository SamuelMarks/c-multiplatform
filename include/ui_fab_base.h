/**
 * @file ui_fab_base.h
 * @brief Floating Action Button (FAB) component definitions.
 */

#ifndef UI_FAB_BASE_H
#define UI_FAB_BASE_H

struct ui_signal;

/* clang-format off */
#include "ui_error.h"
#include "ui_button_base.h"
#include "ui_ripple_base.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief States of a Floating Action Button (FAB) speed dial.
 */
enum ui_fab_state {
  UI_FAB_STATE_COLLAPSED, /**< Speed dial is hidden */
  UI_FAB_STATE_EXPANDING, /**< Speed dial is animating open */
  UI_FAB_STATE_EXPANDED,  /**< Speed dial is fully visible */
  UI_FAB_STATE_COLLAPSING /**< Speed dial is animating closed */
};

struct ui_fab_base;
struct ui_fab_action;

/**
 * @brief Creates a new unstyled FAB base component.
 *
 * @param out_fab Pointer to receive the allocated FAB base.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY on allocation
 * failure.
 */
ui_error_t ui_fab_base_create(struct ui_fab_base **out_fab);

/**
 * @brief Destroys a FAB base component and frees all internal actions.
 *
 * @param fab The FAB to destroy.
 */
ui_error_t ui_fab_base_destroy(struct ui_fab_base *fab);

/**
 * @brief Sets the main button of the FAB.
 *
 * The FAB takes ownership of the main button.
 *
 * @param fab The FAB.
 * @param button The main button component.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_fab_base_set_main_button(struct ui_fab_base *fab,
                                       struct ui_button_base *button);

/**
 * @brief Gets the main button of the FAB.
 *
 * @param fab The FAB.
 * @param out_button Pointer to receive the main button.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_fab_base_get_main_button(const struct ui_fab_base *fab,
                                       struct ui_button_base **out_button);

/**
 * @brief Adds a speed-dial action to the FAB.
 *
 * The FAB takes ownership of the action button.
 *
 * @param fab The FAB.
 * @param action_button The button representing the action.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer,
 * UI_ERROR_OUT_OF_MEMORY on allocation failure.
 */
ui_error_t ui_fab_base_add_action(struct ui_fab_base *fab,
                                  struct ui_button_base *action_button);

/**
 * @brief Gets the number of speed-dial actions.
 *
 * @param fab The FAB.
 * @param out_count Pointer to receive the count.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_fab_base_get_action_count(const struct ui_fab_base *fab,
                                        size_t *out_count);

/**
 * @brief Gets a speed-dial action button by index.
 *
 * @param fab The FAB.
 * @param index The index of the action.
 * @param out_button Pointer to receive the action button.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer,
 * UI_ERROR_OUT_OF_BOUNDS if index is invalid.
 */
ui_error_t ui_fab_base_get_action(const struct ui_fab_base *fab, size_t index,
                                  struct ui_button_base **out_button);

/**
 * @brief Toggles the expanded state of the FAB speed dial.
 *
 * @param fab The FAB.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_fab_base_toggle(struct ui_fab_base *fab);

/**
 * @brief Gets the current state of the FAB speed dial.
 *
 * @param fab The FAB.
 * @param out_state Pointer to receive the state.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_fab_base_get_state(const struct ui_fab_base *fab,
                                 enum ui_fab_state *out_state);

/**
 * @brief Advances the FAB animation state by the specified delta time.
 *
 * Updates the expansion progress and internal ripple states.
 *
 * @param fab The FAB.
 * @param dt_ms Delta time in milliseconds.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_fab_base_tick(struct ui_fab_base *fab, float dt_ms);

/**
 * @brief Gets the current expansion progress [0.0, 1.0].
 *
 * 0.0 means fully collapsed, 1.0 means fully expanded.
 *
 * @param fab The FAB.
 * @param out_progress Pointer to receive the progress.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_fab_base_get_expansion_progress(const struct ui_fab_base *fab,
                                              float *out_progress);

/**
 * @brief Retrieves the ripple state for rendering touch feedback on the main
 * FAB.
 *
 * @param fab The FAB.
 * @param out_ripple_state Pointer to receive the ripple state.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t
ui_fab_base_get_ripple_state(const struct ui_fab_base *fab,
                             struct ui_ripple_state **out_ripple_state);

/**
 * @brief Starts a ripple animation on the FAB.
 *
 * @param fab The FAB.
 * @param origin_x X coordinate of the ripple origin.
 * @param origin_y Y coordinate of the ripple origin.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_fab_base_start_ripple(struct ui_fab_base *fab, float origin_x,
                                    float origin_y);

/**
 * @brief Binds the disabled state to a boolean signal.
 *
 * @param widget The widget.
 * @param disabled_signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_fab_base_bind_disabled(struct ui_fab_base *widget,
                                     struct ui_signal *disabled_signal);

/**
 * @brief Binds the text content to a string signal for dynamic
 * internationalization.
 *
 * @param widget The widget.
 * @param text_signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_fab_base_bind_text(struct ui_fab_base *widget,
                                 struct ui_signal *text_signal);

#ifdef __cplusplus
}
#endif

#endif /* UI_FAB_BASE_H */
