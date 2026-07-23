#ifndef UI_TRAY_MANAGER_H
#define UI_TRAY_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_menu_base.h"
#include "ui_image_base.h"
/* clang-format on */

/**
 * @brief Opaque handle representing a system tray / app indicator instance.
 */
struct ui_tray_manager;

/**
 * @brief Creates a new system tray manager instance.
 *
 * @param out_tray Pointer to receive the new tray manager instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_tray_manager_create(struct ui_tray_manager **out_tray);

/**
 * @brief Destroys a tray manager instance and removes it from the OS taskbar.
 *
 * @param tray The tray manager to destroy.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_tray_manager_destroy(struct ui_tray_manager *tray);

/**
 * @brief Sets the icon of the tray using an unstyled image base.
 *
 * @param tray The tray manager.
 * @param image The image component holding the icon pixels.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_tray_manager_set_icon(struct ui_tray_manager *tray,
                                       struct ui_image_base *image);

/**
 * @brief Sets the tooltip text shown when hovering over the tray icon.
 *
 * @param tray The tray manager.
 * @param tooltip The tooltip text.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_tray_manager_set_tooltip(struct ui_tray_manager *tray,
                                          const char *tooltip);

/**
 * @brief Binds a C-Multiplatform menu to the native OS context menu of the tray
 * icon.
 *
 * @param tray The tray manager.
 * @param menu The menu component to be displayed on right-click.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_tray_manager_set_context_menu(struct ui_tray_manager *tray,
                                               struct ui_menu_base *menu);

/**
 * @brief Shows the tray icon in the system taskbar/menubar.
 *
 * @param tray The tray manager.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_tray_manager_show(struct ui_tray_manager *tray);

/**
 * @brief Hides the tray icon from the system taskbar/menubar.
 *
 * @param tray The tray manager.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_tray_manager_hide(struct ui_tray_manager *tray);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TRAY_MANAGER_H */
