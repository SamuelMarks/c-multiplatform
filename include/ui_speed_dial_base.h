#ifndef UI_SPEED_DIAL_BASE_H
#define UI_SPEED_DIAL_BASE_H

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

struct ui_component;
struct ui_fab_base;

/**
 * @struct ui_speed_dial_action
 * @brief Node for a secondary action in the speed dial.
 */
struct ui_speed_dial_action {
  /** @brief Unique identifier for this action. */
  int id;
  /** @brief The FAB component associated with this action. */
  struct ui_fab_base *fab;
  /** @brief Pointer to the next action in the list. */
  struct ui_speed_dial_action *next;
};

/**
 * @struct ui_speed_dial_base
 * @brief Base state for a Speed Dial widget.
 */
struct ui_speed_dial_base {
  /** @brief Pointer to the associated UI component. */
  struct ui_component *component;
  /** @brief Non-zero if the speed dial menu is currently open. */
  int is_open;
  /** @brief Linked list of secondary actions. */
  struct ui_speed_dial_action *actions;
  /** @brief The signal controlling the disabled state. */
  struct ui_signal *disabled_signal;
  /** @brief The signal containing the text content. */
  struct ui_signal *text_signal;
};

/**
 * @brief Initializes a base speed dial component.
 *
 * @param speed_dial Pointer to the speed dial base struct.
 * @param component The UI component to bind to.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_speed_dial_base_init(struct ui_speed_dial_base *speed_dial,
                                   struct ui_component *component);

/**
 * @brief Adds a secondary action to the speed dial.
 *
 * @param speed_dial Pointer to the speed dial base struct.
 * @param id The unique identifier for the action.
 * @param fab Pointer to the FAB component for this action.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_speed_dial_base_add_action(struct ui_speed_dial_base *speed_dial,
                                         int id, struct ui_fab_base *fab);

/**
 * @brief Toggles the open/closed state of the speed dial.
 *
 * @param speed_dial Pointer to the speed dial base struct.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_speed_dial_base_toggle(struct ui_speed_dial_base *speed_dial);

/**
 * @brief Cleans up resources allocated by the speed dial base.
 *
 * @param speed_dial Pointer to the speed dial base struct.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_speed_dial_base_cleanup(struct ui_speed_dial_base *speed_dial);

/**
 * @brief Binds the disabled state to a boolean signal.
 *
 * @param widget The widget.
 * @param disabled_signal The signal to bind to.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_speed_dial_base_bind_disabled(struct ui_speed_dial_base *widget,
                                            struct ui_signal *disabled_signal);

/**
 * @brief Binds the text content to a string signal for dynamic
 * internationalization.
 *
 * @param widget The widget.
 * @param text_signal The signal to bind to.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_speed_dial_base_bind_text(struct ui_speed_dial_base *widget,
                                        struct ui_signal *text_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SPEED_DIAL_BASE_H */
