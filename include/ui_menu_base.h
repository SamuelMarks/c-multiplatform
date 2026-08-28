/**
 * @file ui_menu_base.h
 * @brief Base menu component handling popup contextual and dropdown menus.
 */

#ifndef UI_MENU_BASE_H
#define UI_MENU_BASE_H

/**
 * @brief Opaque structure representing a signal.
 */
struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_event.h"
#include "ui_component.h"
#include "ui_overlay_director.h"
/* clang-format on */

/**
 * @brief Opaque structure representing a menu base component.
 */
struct ui_menu_base;

/**
 * @brief Callback invoked when a menu item is triggered.
 *
 * @param menu The menu component.
 * @param item_id The unique identifier of the triggered item.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_menu_on_action_t)(struct ui_menu_base *menu,
                                          const char *item_id, void *user_data);

/**
 * @brief Creates a new unstyled menu component.
 *
 * @param out_menu Pointer to receive the allocated menu base.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_menu_base_create(struct ui_menu_base **out_menu);

/**
 * @brief Destroys a menu component.
 *
 * @param menu The menu component to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_menu_base_destroy(struct ui_menu_base *menu);

/**
 * @brief Adds an item to the menu.
 * The component takes ownership of the label_node.
 * If submenu is provided, this item will act as a cascading trigger.
 *
 * @param menu The menu component.
 * @param item_id A unique string identifier for the item.
 * @param label_node The DOM node representing the item's content.
 * @param submenu Optional. Another menu to open as a cascading sub-menu.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_menu_base_add_item(struct ui_menu_base *menu, const char *item_id,
                                 struct ui_dom_node *label_node,
                                 struct ui_menu_base *submenu);

/**
 * @brief Sets the callback for item activation.
 *
 * @param menu The menu component.
 * @param on_action The callback function.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_menu_base_set_on_action(struct ui_menu_base *menu,
                                      ui_menu_on_action_t on_action,
                                      void *user_data);

/**
 * @brief Opens the menu at specific screen coordinates (e.g., for a context
 * menu).
 *
 * @param menu The menu component.
 * @param director The overlay director to mount the menu into.
 * @param x The X screen coordinate.
 * @param y The Y screen coordinate.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_menu_base_open_at(struct ui_menu_base *menu,
                                struct ui_overlay_director *director, int x,
                                int y);

/**
 * @brief Closes the menu and any open sub-menus.
 *
 * @param menu The menu component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_menu_base_close(struct ui_menu_base *menu);

/**
 * @brief Checks if the menu is open.
 *
 * @param menu The menu component.
 * @param out_is_open Pointer to receive 1 if open, 0 if closed.
 * @return ui_error_t `UI_ERROR_NONE` on success.
 */
ui_error_t ui_menu_base_is_open(const struct ui_menu_base *menu,
                                int *out_is_open);

/**
 * @brief Intercepts a potential context menu event (right-click).
 * If the event is a right-click, this opens the menu at the cursor coordinates.
 *
 * @param menu The menu component.
 * @param director The overlay director to mount into if opened.
 * @param event The input event to check.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_menu_base_intercept_context_menu(struct ui_menu_base *menu,
                                    struct ui_overlay_director *director,
                                    const struct ui_event *event);

/**
 * @brief Processes keyboard/pointer events for menu navigation (Up/Down,
 * Left/Right).
 *
 * @param menu The menu component.
 * @param event The input event.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_menu_base_process_event(struct ui_menu_base *menu,
                                      const struct ui_event *event);

/**
 * @brief Gets the underlying UI component.
 *
 * @param menu The menu component.
 * @param out_component Pointer to receive the underlying component.
 * @return ui_error_t `UI_ERROR_NONE` on success.
 */
ui_error_t ui_menu_base_get_component(struct ui_menu_base *menu,
                                      struct ui_component **out_component);

/**
 * @brief Binds the active state/index to a signal.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_menu_base_bind_active_index(struct ui_menu_base *widget,
                                          struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_MENU_BASE_H */
