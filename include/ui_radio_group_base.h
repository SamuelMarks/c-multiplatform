#ifndef UI_RADIO_GROUP_BASE_H
#define UI_RADIO_GROUP_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_event.h"
#include "ui_control_value_accessor.h"
/* clang-format on */

struct ui_radio_group_base;
struct ui_toggle_base;

/**
 * @brief Creates a new radio group manager.
 *
 * @param out_group Pointer to receive the allocated radio group.
 * @param out_cva Optional pointer to receive the CVA interface.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_radio_group_base_create(struct ui_radio_group_base **out_group,
                           struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a radio group manager.
 * Note: This does not destroy the individual ui_toggle_base components.
 *
 * @param group The radio group manager to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_radio_group_base_destroy(struct ui_radio_group_base *group);

/**
 * @brief Adds a toggle (radio button) to the group.
 *
 * @param group The radio group manager.
 * @param toggle The toggle component to add.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_radio_group_base_add_toggle(struct ui_radio_group_base *group,
                                          struct ui_toggle_base *toggle);

/**
 * @brief Removes a toggle from the group.
 *
 * @param group The radio group manager.
 * @param toggle The toggle component to remove.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if it was not in the
 * group.
 */
ui_error_t ui_radio_group_base_remove_toggle(struct ui_radio_group_base *group,
                                             struct ui_toggle_base *toggle);

/**
 * @brief Manually sets the active (checked) toggle in the group.
 * Unchecks all other toggles in this group.
 *
 * @param group The radio group manager.
 * @param toggle The toggle to set as active. If NULL, unchecks all.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_radio_group_base_set_active(struct ui_radio_group_base *group,
                                          struct ui_toggle_base *toggle);

/**
 * @brief Retrieves the currently active (checked) toggle in the group.
 *
 * @param group The radio group manager.
 * @return The active toggle, or NULL if none are active.
 */
ui_error_t
ui_radio_group_base_get_active(const struct ui_radio_group_base *group,
                               struct ui_toggle_base **out_toggle);

/**
 * @brief Callback invoked when the active toggle in the radio group changes.
 */
typedef ui_error_t (*ui_radio_group_on_change_t)(
    struct ui_radio_group_base *group, struct ui_toggle_base *active_toggle,
    void *user_data);

/**
 * @brief Sets the change handler for the radio group.
 *
 * @param group The radio group manager.
 * @param on_change The callback to invoke when the active radio changes.
 * @param user_data Opaque user data passed to the callback.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_radio_group_base_set_on_change(struct ui_radio_group_base *group,
                                  ui_radio_group_on_change_t on_change,
                                  void *user_data);

/**
 * @brief Processes an input event for keyboard routing (Arrow keys) to cycle
 * selection. Typically, this is called when the group container or an active
 * radio receives key events.
 *
 * @param group The radio group manager.
 * @param event The input event.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_radio_group_base_process_event(struct ui_radio_group_base *group,
                                             const struct ui_event *event);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_RADIO_GROUP_BASE_H */
