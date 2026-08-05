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
 * @brief Node for a secondary action in the speed dial.
 */
struct ui_speed_dial_action {
  int id;
  struct ui_fab_base *fab;
  struct ui_speed_dial_action *next;
};

/**
 * @brief Base state for a Speed Dial widget.
 */
struct ui_speed_dial_base {
  struct ui_component *component;
  int is_open;
  struct ui_speed_dial_action *actions;
  struct ui_signal *disabled_signal;
  struct ui_signal *text_signal;
};

/**
 * @brief Initializes a base speed dial component.
 *
 * @param speed_dial Pointer to the speed dial base struct.
 * @param component The UI component to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_speed_dial_base_init(struct ui_speed_dial_base *speed_dial,
                                   struct ui_component *component);

/**
 * @brief Adds a secondary action to the speed dial.
 *
 * @param speed_dial Pointer to the speed dial base struct.
 * @param id The unique identifier for the action.
 * @param fab Pointer to the FAB component for this action.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_speed_dial_base_add_action(struct ui_speed_dial_base *speed_dial,
                                         int id, struct ui_fab_base *fab);

/**
 * @brief Toggles the open/closed state of the speed dial.
 *
 * @param speed_dial Pointer to the speed dial base struct.
 */
ui_error_t ui_speed_dial_base_toggle(struct ui_speed_dial_base *speed_dial);

/**
 * @brief Cleans up resources allocated by the speed dial base.
 *
 * @param speed_dial Pointer to the speed dial base struct.
 */
ui_error_t ui_speed_dial_base_cleanup(struct ui_speed_dial_base *speed_dial);

/**
 * @brief Binds the disabled state to a boolean signal.
 *
 * @param widget The widget.
 * @param disabled_signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_speed_dial_base_bind_disabled(struct ui_speed_dial_base *widget,
                                            struct ui_signal *disabled_signal);

/**
 * @brief Binds the text content to a string signal for dynamic
 * internationalization.
 *
 * @param widget The widget.
 * @param text_signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_speed_dial_base_bind_text(struct ui_speed_dial_base *widget,
                                        struct ui_signal *text_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SPEED_DIAL_BASE_H */
