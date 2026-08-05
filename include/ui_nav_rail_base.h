#ifndef UI_NAV_RAIL_BASE_H
#define UI_NAV_RAIL_BASE_H

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
/* clang-format on */

struct ui_nav_rail_base;
struct ui_nav_rail_item_base;

/**
 * @brief Creates a new unstyled navigation rail component.
 *
 * @param out_rail Pointer to receive the allocated nav rail base.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_nav_rail_base_create(struct ui_nav_rail_base **out_rail);

/**
 * @brief Destroys a navigation rail component.
 *
 * @param rail The rail to destroy.
 */
ui_error_t ui_nav_rail_base_destroy(struct ui_nav_rail_base *rail);

/**
 * @brief Gets the underlying component for the navigation rail.
 *
 * @param rail The rail.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_nav_rail_base_get_component(struct ui_nav_rail_base *rail,
                                          struct ui_component **out_component);

/**
 * @brief Appends an item to the navigation rail.
 *
 * @param rail The rail.
 * @param item The item to append.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_nav_rail_base_append_item(struct ui_nav_rail_base *rail,
                                        struct ui_nav_rail_item_base *item);

/**
 * @brief Creates a new unstyled navigation rail item component.
 *
 * @param out_item Pointer to receive the allocated item base.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_nav_rail_item_base_create(struct ui_nav_rail_item_base **out_item);

/**
 * @brief Destroys a navigation rail item component.
 *
 * @param item The item to destroy.
 */
ui_error_t ui_nav_rail_item_base_destroy(struct ui_nav_rail_item_base *item);

/**
 * @brief Gets the underlying component for the navigation rail item.
 *
 * @param item The item.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_nav_rail_item_base_get_component(struct ui_nav_rail_item_base *item,
                                    struct ui_component **out_component);

/**
 * @brief Sets the active (selected) state of the navigation item.
 *
 * @param item The item.
 * @param active True if active, false otherwise.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_nav_rail_item_base_set_active(struct ui_nav_rail_item_base *item,
                                            int active);

/**
 * @brief Gets the active (selected) state of the navigation item.
 *
 * @param item The item.
 * @param out_active Pointer to receive the active state.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_nav_rail_item_base_get_active(struct ui_nav_rail_item_base *item,
                                            int *out_active);

/**
 * @brief Binds the active state/index to a signal.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_nav_rail_base_bind_active_index(struct ui_nav_rail_base *widget,
                                              struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_NAV_RAIL_BASE_H */
