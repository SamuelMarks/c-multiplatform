/**
 * @file md2_navigation_drawer.h
 * @brief Material Design 2 Navigation Drawer component.
 */

#ifndef MATERIAL2_MD2_NAVIGATION_DRAWER_H
#define MATERIAL2_MD2_NAVIGATION_DRAWER_H

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_sidenav_base.h"
#include "ui_overlay_director.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum md2_navigation_drawer_type
 * @brief Specifies the visual and behavioral type of the Material 2 navigation
 * drawer.
 */
enum md2_navigation_drawer_type {
  MD2_NAVIGATION_DRAWER_STANDARD = 0, /**< Standard side drawer. */
  MD2_NAVIGATION_DRAWER_MODAL = 1     /**< Modal drawer with scrim overlay. */
};

/**
 * @struct md2_navigation_drawer
 * @brief Opaque handle to a Material 2 navigation drawer instance.
 */
struct md2_navigation_drawer;

/**
 * @brief Callback invoked when the modal drawer is dismissed.
 *
 * @param drawer The navigation drawer instance.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
typedef ui_error_t (*md2_navigation_drawer_on_close_t)(
    struct md2_navigation_drawer *drawer, void *user_data);

/**
 * @brief Creates a new Material Design 2 navigation drawer.
 *
 * @param type The type of drawer (standard or modal).
 * @param out_drawer Pointer to receive the allocated drawer instance.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_navigation_drawer_create(enum md2_navigation_drawer_type type,
                             struct md2_navigation_drawer **out_drawer);

/**
 * @brief Destroys a Material Design 2 navigation drawer instance.
 *
 * @param drawer The drawer to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_navigation_drawer_destroy(struct md2_navigation_drawer *drawer);

/**
 * @brief Sets the content rendered inside the drawer sheet.
 *
 * @param drawer The drawer.
 * @param content The component to render inside the drawer.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_navigation_drawer_set_drawer_content(struct md2_navigation_drawer *drawer,
                                         struct ui_component *content);

/**
 * @brief Sets the main application content displayed beside or under the
 * drawer.
 *
 * @param drawer The drawer.
 * @param content The main content component.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_navigation_drawer_set_main_content(
    struct md2_navigation_drawer *drawer, struct ui_component *content);

/**
 * @brief Toggles the open/closed state of the drawer.
 *
 * @param drawer The drawer.
 * @param is_open 1 to open, 0 to close.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_navigation_drawer_set_open(
    struct md2_navigation_drawer *drawer, int is_open);

/**
 * @brief Checks if the drawer is currently open.
 *
 * @param drawer The drawer.
 * @param out_is_open Pointer to receive the open state (1 if open, 0 if
 * closed).
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_navigation_drawer_is_open(
    const struct md2_navigation_drawer *drawer, int *out_is_open);

/**
 * @brief Sets the overlay director used to mount the scrim for modal drawers.
 *
 * @param drawer The drawer.
 * @param director The overlay director instance.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_navigation_drawer_set_overlay_director(
    struct md2_navigation_drawer *drawer, struct ui_overlay_director *director);

/**
 * @brief Sets a callback invoked when the drawer is dismissed (e.g., clicking
 * scrim).
 *
 * @param drawer The drawer.
 * @param on_close The callback.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_navigation_drawer_set_on_close(
    struct md2_navigation_drawer *drawer,
    md2_navigation_drawer_on_close_t on_close, void *user_data);

/**
 * @brief Retrieves the underlying component wrapper.
 *
 * @param drawer The drawer.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_navigation_drawer_get_component(
    struct md2_navigation_drawer *drawer, struct ui_component **out_component);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL2_MD2_NAVIGATION_DRAWER_H */
