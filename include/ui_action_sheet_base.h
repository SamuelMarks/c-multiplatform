/**
 * @file ui_action_sheet_base.h
 * @brief Action sheet base component.
 */
#ifndef UI_ACTION_SHEET_BASE_H
#define UI_ACTION_SHEET_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_bottom_sheet_base.h"
#include "ui_focus_manager.h"
#include "ui_keyboard_responder.h"
#include "ui_event.h"
#include "ui_signal.h"
#include "ui_computed.h"
/* clang-format on */

struct ui_action_sheet_base;

/**
 * @brief Callback invoked when the action sheet is closed.
 *
 * @param sheet The action sheet.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_action_sheet_on_close_t)(
    struct ui_action_sheet_base *sheet, void *user_data);

/**
 * @brief Creates a new unstyled action sheet base component.
 *
 * @param out_sheet Pointer to receive the allocated component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_action_sheet_base_create(struct ui_action_sheet_base **out_sheet);
/**
 * @brief Destroys an action sheet base component.
 *
 * @param sheet The action sheet to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_action_sheet_base_destroy(struct ui_action_sheet_base *sheet);

/**
 * @brief Adds an action component to the action sheet.
 *
 * @param sheet The action sheet.
 * @param action_comp The action component to add.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_action_sheet_base_add_action(struct ui_action_sheet_base *sheet,
                                           struct ui_component *action_comp);

/**
 * @brief Sets the cancel action component for the action sheet.
 *
 * @param sheet The action sheet.
 * @param cancel_comp The cancel component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_action_sheet_base_set_cancel_action(struct ui_action_sheet_base *sheet,
                                       struct ui_component *cancel_comp);

/**
 * @brief Sets the open state of the action sheet.
 *
 * @param sheet The action sheet.
 * @param is_open 1 to open, 0 to close.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_action_sheet_base_set_open(struct ui_action_sheet_base *sheet,
                                         int is_open);
/**
 * @brief Checks if the action sheet is open.
 *
 * @param sheet The action sheet.
 * @param out_is_open Pointer to receive the open state (1 if open, 0
 * otherwise).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_action_sheet_base_is_open(const struct ui_action_sheet_base *sheet,
                             int *out_is_open);

/**
 * @brief Sets the overlay director for the action sheet.
 *
 * @param sheet The action sheet.
 * @param director The overlay director.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_action_sheet_base_set_overlay_director(struct ui_action_sheet_base *sheet,
                                          struct ui_overlay_director *director);

/**
 * @brief Sets the on close callback for the action sheet.
 *
 * @param sheet The action sheet.
 * @param on_close The callback function.
 * @param user_data Opaque user data to pass to the callback.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_action_sheet_base_set_on_close(struct ui_action_sheet_base *sheet,
                                  ui_action_sheet_on_close_t on_close,
                                  void *user_data);

/**
 * @brief Attaches focus and keyboard responders to the action sheet.
 *
 * @param sheet The action sheet.
 * @param focus_manager The focus manager.
 * @param keyboard_responder The keyboard responder.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_action_sheet_base_attach_focus_and_keyboard(
    struct ui_action_sheet_base *sheet, struct ui_focus_manager *focus_manager,
    struct ui_keyboard_responder *keyboard_responder);

/**
 * @brief Processes an event for the action sheet.
 *
 * @param sheet The action sheet.
 * @param event The event to process.
 * @param timestamp_ms The timestamp in milliseconds.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_action_sheet_base_process_event(struct ui_action_sheet_base *sheet,
                                   const struct ui_event *event,
                                   double timestamp_ms);

/**
 * @brief Updates the action sheet animation state.
 *
 * @param sheet The action sheet.
 * @param timestamp_ms The timestamp in milliseconds.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_action_sheet_base_update(struct ui_action_sheet_base *sheet,
                                       double timestamp_ms);

/**
 * @brief Retrieves the underlying component of the action sheet.
 *
 * @param sheet The action sheet.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_action_sheet_base_get_component(struct ui_action_sheet_base *sheet,
                                   struct ui_component **out_component);

/**
 * @brief Binds the action sheet's open state to a signal.
 *
 * @param sheet The action sheet.
 * @param open_signal The boolean signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_action_sheet_base_bind_open(struct ui_action_sheet_base *sheet,
                                          struct ui_signal *open_signal);

/**
 * @brief Retrieves the computed signal indicating if the action sheet is
 * animating.
 *
 * @param sheet The action sheet.
 * @param out_animating Pointer to receive the computed signal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_action_sheet_base_get_animating_signal(struct ui_action_sheet_base *sheet,
                                          struct ui_computed **out_animating);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_ACTION_SHEET_BASE_H */
