#ifndef UI_MENUBAR_BASE_H
#define UI_MENUBAR_BASE_H

struct ui_signal;

/* clang-format off */
#include "ui_component.h"
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents an unstyled desktop-style menubar component.
 */
struct ui_menubar_base {
  struct ui_component base;
  struct ui_signal *active_index_signal;
};

/**
 * @brief Creates a new base menubar component.
 *
 * @param out_menubar Pointer to output the initialized menubar.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_menubar_base_create(struct ui_menubar_base **out_menubar);

/**
 * @brief Appends a top-level menu item to the menubar.
 *
 * @param menubar The menubar component.
 * @param item The menu item to append.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_menubar_base_append_item(struct ui_menubar_base *menubar,
                                          struct ui_component *item);

/**
 * @brief Binds the active state/index to a signal.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_menubar_base_bind_active_index(struct ui_menubar_base *widget,
                                                struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_MENUBAR_BASE_H */
