/**
 * @file ui_theme_manager.h
 * @brief Defines the theme manager and built-in theme modes.
 */
#ifndef UI_THEME_MANAGER_H
#define UI_THEME_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_signal.h"
/* clang-format on */

/**
 * @brief Built-in theme modes.
 */
enum ui_theme_mode {
  UI_THEME_MODE_SYSTEM,
  UI_THEME_MODE_LIGHT,
  UI_THEME_MODE_DARK
};

/**
 * @brief Forward declaration of theme manager.
 */
struct ui_theme_manager;
struct ui_arena;

/**
 * @brief Creates a theme manager.
 *
 * @param arena The memory arena to use for allocation.
 * @param out_manager Output pointer for the created manager.
 * @return ui_error_t
 */
ui_error_t ui_theme_manager_create(struct ui_arena *arena,
                                   struct ui_theme_manager **out_manager);

/**
 * @brief Destroys a theme manager.
 *
 * @param manager The theme manager.
 * @return ui_error_t
 */
ui_error_t ui_theme_manager_destroy(struct ui_theme_manager *manager);

/**
 * @brief Sets the current theme mode.
 *
 * @param manager The theme manager.
 * @param mode The desired mode.
 * @return ui_error_t
 */
ui_error_t ui_theme_manager_set_mode(struct ui_theme_manager *manager,
                                     enum ui_theme_mode mode);

/**
 * @brief Gets the current theme mode.
 *
 * @param manager The theme manager.
 * @param out_mode Output pointer for the current mode.
 * @return ui_error_t
 */
ui_error_t ui_theme_manager_get_mode(struct ui_theme_manager *manager,
                                     enum ui_theme_mode *out_mode);

/**
 * @brief Gets the signal emitted when the theme mode changes.
 * The payload is an int32 containing the ui_theme_mode.
 *
 * @param manager The theme manager.
 * @param out_signal Output pointer for the signal.
 * @return ui_error_t
 */
ui_error_t ui_theme_manager_get_change_signal(struct ui_theme_manager *manager,
                                              ui_signal_t **out_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_THEME_MANAGER_H */
