/**
 * @file ui_tabs_base.h
 * @brief Defines the tabs base component and related operations.
 */
#ifndef UI_TABS_BASE_H
#define UI_TABS_BASE_H

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_event.h"
/* clang-format on */

struct ui_tabs_base;

/**
 * @brief Callback invoked when the active tab changes.
 */
typedef ui_error_t (*ui_tabs_on_change_t)(struct ui_tabs_base *tabs,
                                          int new_index, void *user_data);

/**
 * @brief Creates a new unstyled tabs base component.
 *
 * @param out_tabs Pointer to receive the allocated tabs base.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_tabs_base_create(struct ui_tabs_base **out_tabs);

/**
 * @brief Destroys a tabs base component.
 *
 * @param tabs The tabs component to destroy.
 */
ui_error_t ui_tabs_base_destroy(struct ui_tabs_base *tabs);

/**
 * @brief Adds a tab to the tabs group.
 * The component takes ownership of title_node and panel_node.
 * Automatically maps aria-controls and aria-labelledby.
 *
 * @param tabs The tabs component.
 * @param tab_id A unique string identifier for the tab.
 * @param title_node The DOM node representing the tab header/button.
 * @param panel_node The DOM node representing the tab content.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_tabs_base_add_tab(struct ui_tabs_base *tabs, const char *tab_id,
                                struct ui_dom_node *title_node,
                                struct ui_dom_node *panel_node);

/**
 * @brief Sets the active tab index.
 * Handles focus management and ARIA attribute updates.
 *
 * @param tabs The tabs component.
 * @param index The index of the tab to activate.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_tabs_base_set_active_index(struct ui_tabs_base *tabs, int index);

/**
 * @brief Gets the currently active tab index.
 *
 * @param tabs The tabs component.
 * @param out_index Pointer to receive the active index.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_tabs_base_get_active_index(const struct ui_tabs_base *tabs,
                                         int *out_index);

/**
 * @brief Sets the callback for tab changes.
 *
 * @param tabs The tabs component.
 * @param on_change The callback function.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_tabs_base_set_on_change(struct ui_tabs_base *tabs,
                                      ui_tabs_on_change_t on_change,
                                      void *user_data);

/**
 * @brief Processes an input event (e.g., keyboard navigation across headers).
 *
 * @param tabs The tabs component.
 * @param event The input event.
 * @param timestamp_ms Current time in milliseconds.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_tabs_base_process_event(struct ui_tabs_base *tabs,
                                      const struct ui_event *event,
                                      double timestamp_ms);

/**
 * @brief Gets the underlying UI component.
 *
 * @param tabs The tabs component.
 * @return The underlying component.
 */
ui_error_t ui_tabs_base_get_component(struct ui_tabs_base *tabs,
                                      struct ui_component **out_component);

/**
 * @brief Binds the active state/index to a signal.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_tabs_base_bind_active_index(struct ui_tabs_base *widget,
                                          struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TABS_BASE_H */
