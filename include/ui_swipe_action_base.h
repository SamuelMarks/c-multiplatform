#ifndef UI_SWIPE_ACTION_BASE_H
#define UI_SWIPE_ACTION_BASE_H

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

struct ui_component;

/**
 * @brief Represents the state of a swipeable list item.
 */
enum ui_swipe_action_state {
  UI_SWIPE_ACTION_IDLE,
  UI_SWIPE_ACTION_SWIPING,
  UI_SWIPE_ACTION_REVEALED_LEFT,
  UI_SWIPE_ACTION_REVEALED_RIGHT
};

/**
 * @brief Base logic for a swipe action item.
 */
struct ui_swipe_action_base {
  struct ui_component *component;
  enum ui_swipe_action_state state;
  float offset_x;
  float threshold;
  struct ui_signal *disabled_signal;
  struct ui_signal *text_signal;
};

/**
 * @brief Initializes a base swipe action component.
 *
 * @param swipe_action Pointer to the swipe action base struct.
 * @param component The UI component to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_swipe_action_base_init(struct ui_swipe_action_base *swipe_action,
                                     struct ui_component *component);

/**
 * @brief Updates the swipe position.
 *
 * @param swipe_action Pointer to the swipe action base struct.
 * @param delta_x The change in X position (positive means swiping right).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_swipe_action_base_update(struct ui_swipe_action_base *swipe_action,
                            float delta_x);

/**
 * @brief Commits the swipe gesture.
 *
 * Depending on the accumulated offset and the threshold, the component will
 * either spring back to idle or transition to a revealed state.
 *
 * @param swipe_action Pointer to the swipe action base struct.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_swipe_action_base_commit(struct ui_swipe_action_base *swipe_action);

/**
 * @brief Resets the swipe action to idle state.
 *
 * @param swipe_action Pointer to the swipe action base struct.
 */
ui_error_t
ui_swipe_action_base_reset(struct ui_swipe_action_base *swipe_action);

/**
 * @brief Binds the disabled state to a boolean signal.
 *
 * @param widget The widget.
 * @param disabled_signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_swipe_action_base_bind_disabled(struct ui_swipe_action_base *widget,
                                   struct ui_signal *disabled_signal);

/**
 * @brief Binds the text content to a string signal for dynamic
 * internationalization.
 *
 * @param widget The widget.
 * @param text_signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_swipe_action_base_bind_text(struct ui_swipe_action_base *widget,
                                          struct ui_signal *text_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SWIPE_ACTION_BASE_H */
