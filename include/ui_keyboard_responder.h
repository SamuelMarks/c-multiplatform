/**
 * @file ui_keyboard_responder.h
 * @brief Keyboard responder mapping keystrokes to actions within components.
 */

#ifndef UI_KEYBOARD_RESPONDER_H
#define UI_KEYBOARD_RESPONDER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_event.h"
#include "ui_dom_node.h"
/* clang-format on */

/**
 * @brief Opaque structure representing a keyboard responder.
 */
struct ui_keyboard_responder;

/**
 * @brief Creates a keyboard responder instance.
 *
 * @param out_responder Pointer to receive the new responder.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_keyboard_responder_create(struct ui_keyboard_responder **out_responder);

/**
 * @brief Destroys a keyboard responder instance.
 *
 * @param responder The responder.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_keyboard_responder_destroy(struct ui_keyboard_responder *responder);

/**
 * @brief Binds a specific key to an action for a specific role or tag.
 *
 * @param responder The responder.
 * @param role_or_tag The HTML tag name (e.g., "button") or ARIA role (e.g.,
 * "checkbox") to match against the focused node.
 * @param key_code The key code to bind (e.g., UI_KEY_SPACE).
 * @param callback The function to invoke when the key is pressed on a matching
 * node.
 * @param user_data Optional user data to pass to the callback.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_keyboard_responder_bind_key(
    struct ui_keyboard_responder *responder, const char *role_or_tag,
    enum ui_key_code key_code,
    ui_error_t (*callback)(struct ui_dom_node *node, void *user_data),
    void *user_data);

/**
 * @brief Evaluates an incoming event and dispatches to a bound callback if
 * conditions are met.
 *
 * @param responder The responder.
 * @param focused_node The currently focused DOM node.
 * @param event The incoming event.
 * @param out_handled Set to 1 if the event was handled by a binding, 0
 * otherwise.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_keyboard_responder_handle_event(
    struct ui_keyboard_responder *responder, struct ui_dom_node *focused_node,
    const struct ui_event *event, int *out_handled);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_KEYBOARD_RESPONDER_H */
