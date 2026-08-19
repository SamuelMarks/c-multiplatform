/**
 * @file ui_bottom_nav_base.h
 * @brief Bottom Navigation base component.
 */

#ifndef UI_BOTTOM_NAV_BASE_H
#define UI_BOTTOM_NAV_BASE_H

/** @brief Forward declaration of ui_signal. */
struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
/* clang-format on */

/** @brief Opaque handle to a bottom nav component. */
struct ui_bottom_nav_base;

/** @brief Opaque handle to a bottom nav item component. */
struct ui_bottom_nav_item_base;

/**
 * @brief Creates a new unstyled bottom navigation component.
 *
 * @param out_nav Pointer to receive the allocated bottom nav base.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_bottom_nav_base_create(struct ui_bottom_nav_base **out_nav);

/**
 * @brief Destroys a bottom navigation component.
 *
 * @param nav The bottom nav to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_bottom_nav_base_destroy(struct ui_bottom_nav_base *nav);

/**
 * @brief Gets the underlying component for the bottom navigation.
 *
 * @param nav The bottom nav.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_bottom_nav_base_get_component(struct ui_bottom_nav_base *nav,
                                 struct ui_component **out_component);

/**
 * @brief Appends an item to the bottom navigation.
 *
 * @param nav The bottom nav.
 * @param item The item to append.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_bottom_nav_base_append_item(struct ui_bottom_nav_base *nav,
                                          struct ui_bottom_nav_item_base *item);

/**
 * @brief Creates a new unstyled bottom navigation item component.
 *
 * @param out_item Pointer to receive the allocated item base.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_bottom_nav_item_base_create(struct ui_bottom_nav_item_base **out_item);

/**
 * @brief Destroys a bottom navigation item component.
 *
 * @param item The item to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_bottom_nav_item_base_destroy(struct ui_bottom_nav_item_base *item);

/**
 * @brief Gets the underlying component for the bottom navigation item.
 *
 * @param item The item.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_bottom_nav_item_base_get_component(struct ui_bottom_nav_item_base *item,
                                      struct ui_component **out_component);

/**
 * @brief Sets the active (selected) state of the bottom navigation item.
 *
 * @param item The item.
 * @param active True if active, false otherwise.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_bottom_nav_item_base_set_active(struct ui_bottom_nav_item_base *item,
                                   int active);

/**
 * @brief Gets the active (selected) state of the bottom navigation item.
 *
 * @param item The item.
 * @param out_active Pointer to receive the active state.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_bottom_nav_item_base_get_active(struct ui_bottom_nav_item_base *item,
                                   int *out_active);

/**
 * @brief Binds the active state/index to a signal.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_bottom_nav_base_bind_active_index(struct ui_bottom_nav_base *widget,
                                     struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_BOTTOM_NAV_BASE_H */
