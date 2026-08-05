#ifndef UI_BUTTON_GROUP_BASE_H
#define UI_BUTTON_GROUP_BASE_H

struct ui_signal;

/* clang-format off */
#include "ui_component.h"
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents an unstyled button group container.
 */
struct ui_button_group_base {
  struct ui_component base;
  int is_vertical;
  struct ui_signal *disabled_signal;
  struct ui_signal *text_signal;
};

/**
 * @brief Creates a new base button group component.
 *
 * @param out_group Pointer to output the initialized button group.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_button_group_base_create(struct ui_button_group_base **out_group);

/**
 * @brief Sets the orientation of the button group.
 *
 * @param group The button group component.
 * @param is_vertical 1 for vertical orientation, 0 for horizontal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_button_group_base_set_orientation(struct ui_button_group_base *group,
                                     int is_vertical);

/**
 * @brief Appends a button (or any component) to the group.
 *
 * @param group The button group component.
 * @param child The component to append as a child.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_button_group_base_append_button(struct ui_button_group_base *group,
                                   struct ui_component *child);

/**
 * @brief Binds the disabled state to a boolean signal.
 *
 * @param widget The widget.
 * @param disabled_signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_button_group_base_bind_disabled(struct ui_button_group_base *widget,
                                   struct ui_signal *disabled_signal);

/**
 * @brief Binds the text content to a string signal for dynamic
 * internationalization.
 *
 * @param widget The widget.
 * @param text_signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_button_group_base_bind_text(struct ui_button_group_base *widget,
                                          struct ui_signal *text_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_BUTTON_GROUP_BASE_H */
