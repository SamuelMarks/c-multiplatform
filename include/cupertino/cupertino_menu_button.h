/**
 * @file cupertino_menu_button.h
 * @brief Cupertino Pop-up and Pull-down Menus conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_MENU_BUTTON_H
#define CUPERTINO_CUPERTINO_MENU_BUTTON_H

/* clang-format off */
#include "cupertino/cupertino_button.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;
struct ui_overlay_director;

/**
 * @enum cupertino_menu_button_mode
 * @brief Mode of menu button: Pop-up or Pull-down.
 */
enum cupertino_menu_button_mode {
  CUPERTINO_MENU_BUTTON_PULL_DOWN =
      0, /**< Performs secondary actions; button label stays constant. */
  CUPERTINO_MENU_BUTTON_POP_UP /**< Displays current selection with checkmark;
                                  replaces button label. */
};

/**
 * @def CUPERTINO_MENU_BUTTON_MAX_ITEMS
 * @brief Maximum menu items supported in a single pop-up / pull-down menu.
 */
#define CUPERTINO_MENU_BUTTON_MAX_ITEMS 32

/**
 * @struct cupertino_menu_button_item
 * @brief Single item in a pop-up or pull-down menu.
 */
struct cupertino_menu_button_item {
  char title[128];      /**< Display title of menu item. */
  char icon_symbol[64]; /**< Optional SF Symbol name for item. */
  int is_destructive;   /**< Non-zero to style in SystemRed. */
  int is_disabled;      /**< Non-zero to disable interaction and dim. */
  void (*on_select)(
      void *user_data); /**< Optional callback when item is selected. */
  void *user_data;      /**< User data passed to on_select callback. */
};

/**
 * @struct cupertino_menu_button_descriptor
 * @brief Configuration descriptor for creating a Cupertino pop-up or pull-down
 * menu button.
 */
struct cupertino_menu_button_descriptor {
  enum cupertino_menu_button_mode mode; /**< Pop-up or Pull-down mode. */
  enum cupertino_button_style style;    /**< Visual button style. */
  char initial_title[128]; /**< Initial title displayed on button. */
  int is_disabled;         /**< Non-zero if button is initially disabled. */
};

/**
 * @struct cupertino_menu_button
 * @brief Cupertino Pop-up & Pull-down Menu instance.
 */
struct cupertino_menu_button {
  struct cupertino_button *button; /**< Underlying wrapped Cupertino button. */
  enum cupertino_menu_button_mode mode; /**< Mode (Pop-up or Pull-down). */
  char default_title[128];              /**< Original default title. */
  char current_title[128];              /**< Currently displayed title. */
  int is_open;        /**< Non-zero if menu popup is currently presented. */
  int selected_index; /**< Currently selected item index (or -1 if none). */
  size_t item_count;  /**< Total items in menu. */
  struct cupertino_menu_button_item
      items[CUPERTINO_MENU_BUTTON_MAX_ITEMS]; /**< Items table. */
};

/**
 * @brief Creates a new Cupertino Pop-up or Pull-down menu button.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Configuration descriptor.
 * @param out_menu Pointer to receive newly created menu button instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_button_create(
    struct ui_engine *engine,
    const struct cupertino_menu_button_descriptor *desc,
    struct cupertino_menu_button **out_menu);

/**
 * @brief Destroys a Cupertino Pop-up or Pull-down menu button.
 *
 * @param menu Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_menu_button_destroy(struct cupertino_menu_button *menu);

/**
 * @brief Adds an item to the menu button.
 *
 * @param menu Target menu button.
 * @param item Item configuration to add.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_menu_button_add_item(struct cupertino_menu_button *menu,
                               const struct cupertino_menu_button_item *item);

/**
 * @brief Gets the number of items in the menu.
 *
 * @param menu Target menu button.
 * @param out_count Pointer to receive item count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_button_get_item_count(
    const struct cupertino_menu_button *menu, size_t *out_count);

/**
 * @brief Gets an item configuration by index.
 *
 * @param menu Target menu button.
 * @param index Item index.
 * @param out_item Pointer to receive item details.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_button_get_item(
    const struct cupertino_menu_button *menu, size_t index,
    struct cupertino_menu_button_item *out_item);

/**
 * @brief Selects an item by index. In pop-up mode, this updates the button's
 * title.
 *
 * @param menu Target menu button.
 * @param index Item index to select.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_button_select_item(
    struct cupertino_menu_button *menu, size_t index);

/**
 * @brief Gets the currently selected index (-1 if none selected).
 *
 * @param menu Target menu button.
 * @param out_index Pointer to receive selected index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_menu_button_get_selected_index(
    const struct cupertino_menu_button *menu, int *out_index);

/**
 * @brief Opens or closes the menu popup.
 *
 * @param menu Target menu button.
 * @param is_open Non-zero to open, 0 to close.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_menu_button_set_open(struct cupertino_menu_button *menu, int is_open);

/**
 * @brief Queries whether the menu popup is currently open.
 *
 * @param menu Target menu button.
 * @param out_is_open Pointer to receive open status.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_button_is_open(
    const struct cupertino_menu_button *menu, int *out_is_open);

/**
 * @brief Gets the currently displayed button label title.
 *
 * @param menu Target menu button.
 * @param out_title Pointer to receive string pointer of current title.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_menu_button_get_current_title(
    const struct cupertino_menu_button *menu, const char **out_title);

/**
 * @brief Gets the underlying cupertino_button instance.
 *
 * @param menu Target menu button.
 * @param out_button Pointer to receive underlying cupertino_button.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_button_get_button(
    struct cupertino_menu_button *menu, struct cupertino_button **out_button);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_MENU_BUTTON_H */
