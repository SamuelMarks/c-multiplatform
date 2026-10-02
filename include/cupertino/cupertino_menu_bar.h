/**
 * @file cupertino_menu_bar.h
 * @brief macOS Global Application Menu Bar engine conforming to macOS HIG.
 */

#ifndef CUPERTINO_CUPERTINO_MENU_BAR_H
#define CUPERTINO_CUPERTINO_MENU_BAR_H

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_MENU_BAR_HEIGHT 24.0f
#define CUPERTINO_MENU_BAR_MAX_MENUS 16
#define CUPERTINO_MENU_BAR_MAX_ITEMS 32

/**
 * @enum cupertino_menu_item_state
 * @brief Selection / checkmark state for menu items.
 */
enum cupertino_menu_item_state {
  CUPERTINO_MENU_ITEM_STATE_OFF = 0, /**< No checkmark. */
  CUPERTINO_MENU_ITEM_STATE_ON,      /**< Checkmark displayed. */
  CUPERTINO_MENU_ITEM_STATE_MIXED    /**< Mixed / dash state. */
};

/**
 * @struct cupertino_menu_item
 * @brief Single item entry within a menu.
 */
struct cupertino_menu_item {
  char title[64];
  char shortcut[32];
  int is_enabled;
  int is_separator;
  enum cupertino_menu_item_state state;
};

/**
 * @struct cupertino_menu
 * @brief Top-level menu column containing items.
 */
struct cupertino_menu {
  char title[64];
  int item_count;
  struct cupertino_menu_item items[CUPERTINO_MENU_BAR_MAX_ITEMS];
};

/**
 * @struct cupertino_menu_bar_descriptor
 * @brief Configuration descriptor for macOS menu bar.
 */
struct cupertino_menu_bar_descriptor {
  const char *app_name; /**< Application name for the Application Menu. */
};

/**
 * @struct cupertino_menu_bar
 * @brief macOS Menu Bar instance.
 */
struct cupertino_menu_bar {
  char app_name[64];
  int menu_count;
  struct cupertino_menu menus[CUPERTINO_MENU_BAR_MAX_MENUS];
  int active_menu_index; /**< Index of open dropdown menu (-1 if closed). */
  float width;
  float height;
};

/**
 * @brief Creates a new macOS menu bar instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_bar Pointer to receive newly created menu bar instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_bar_create(
    struct ui_engine *engine, const struct cupertino_menu_bar_descriptor *desc,
    struct cupertino_menu_bar **out_bar);

/**
 * @brief Destroys a macOS menu bar instance.
 *
 * @param bar Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_menu_bar_destroy(struct cupertino_menu_bar *bar);

/**
 * @brief Adds a new top-level menu (e.g. 'File', 'Edit') to the menu bar.
 *
 * @param bar Target menu bar.
 * @param title Title of the menu column.
 * @param out_menu_index Pointer to receive index of the added menu.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_bar_add_menu(
    struct cupertino_menu_bar *bar, const char *title, int *out_menu_index);

/**
 * @brief Adds an item or separator to a specific menu column.
 *
 * @param bar Target menu bar.
 * @param menu_index Index of the target menu.
 * @param title Item title (or NULL/empty for separator).
 * @param shortcut Keyboard shortcut string (or NULL).
 * @param is_separator 1 if separator line, 0 for regular item.
 * @param out_item_index Pointer to receive item index within menu.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_bar_add_item(
    struct cupertino_menu_bar *bar, int menu_index, const char *title,
    const char *shortcut, int is_separator, int *out_item_index);

/**
 * @brief Updates enabled/disabled state of a menu item.
 *
 * @param bar Target menu bar.
 * @param menu_index Target menu index.
 * @param item_index Target item index.
 * @param is_enabled 1 to enable, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_bar_set_item_enabled(
    struct cupertino_menu_bar *bar, int menu_index, int item_index,
    int is_enabled);

/**
 * @brief Updates checkmark state of a menu item.
 *
 * @param bar Target menu bar.
 * @param menu_index Target menu index.
 * @param item_index Target item index.
 * @param state State (OFF, ON, MIXED).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_bar_set_item_state(
    struct cupertino_menu_bar *bar, int menu_index, int item_index,
    enum cupertino_menu_item_state state);

/**
 * @brief Opens a specific top-level menu dropdown.
 *
 * @param bar Target menu bar.
 * @param menu_index Index of menu to open.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_menu_bar_open_menu(struct cupertino_menu_bar *bar, int menu_index);

/**
 * @brief Closes any currently open dropdown menu.
 *
 * @param bar Target menu bar.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_menu_bar_close(struct cupertino_menu_bar *bar);

/**
 * @brief Gets index of currently open dropdown menu.
 *
 * @param bar Target menu bar.
 * @param out_menu_index Pointer to receive open menu index (-1 if none open).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_bar_get_active_menu(
    const struct cupertino_menu_bar *bar, int *out_menu_index);

/**
 * @brief Gets menu bar bounding dimensions.
 *
 * @param bar Target menu bar.
 * @param out_width Pointer to receive width.
 * @param out_height Pointer to receive height (24pt).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_bar_get_dimensions(
    const struct cupertino_menu_bar *bar, float *out_width, float *out_height);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_MENU_BAR_H */
