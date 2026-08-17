/**
 * @file ui_toolbar_base.h
 * @brief Defines the base logic for a toolbar UI component.
 */
#ifndef UI_TOOLBAR_BASE_H
#define UI_TOOLBAR_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Toolbar modes for layout behavior.
 */
enum ui_toolbar_mode {
  UI_TOOLBAR_MODE_STATIC,  /**< Toolbar scrolls normally with the document. */
  UI_TOOLBAR_MODE_STICKY,  /**< Toolbar sticks to the top when scrolled past. */
  UI_TOOLBAR_MODE_FLOATING /**< Toolbar floats above the content. */
};

/**
 * @brief Toolbar alignment for its children.
 */
enum ui_toolbar_alignment {
  UI_TOOLBAR_ALIGN_ROW,   /**< Align items horizontally. */
  UI_TOOLBAR_ALIGN_COLUMN /**< Align items vertically. */
};

/**
 * @brief Represents a toolbar base component.
 */
struct ui_toolbar_base;

/**
 * @brief Creates a new unstyled toolbar base component.
 *
 * @param out_toolbar Pointer to receive the allocated toolbar base.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY on allocation
 * failure.
 */
ui_error_t ui_toolbar_base_create(struct ui_toolbar_base **out_toolbar);

/**
 * @brief Destroys a toolbar base component and frees its title if allocated.
 *
 * @param toolbar The toolbar to destroy.
 */
ui_error_t ui_toolbar_base_destroy(struct ui_toolbar_base *toolbar);

/**
 * @brief Sets the title of the toolbar.
 *
 * Uses safe CRT functions (`strcpy_s` on MSVC) to copy the string.
 *
 * @param toolbar The toolbar.
 * @param title The title string to set (must be null-terminated).
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer,
 * UI_ERROR_OUT_OF_MEMORY on allocation failure.
 */
ui_error_t ui_toolbar_base_set_title(struct ui_toolbar_base *toolbar,
                                     const char *title);

/**
 * @brief Gets the title of the toolbar.
 *
 * @param toolbar The toolbar.
 * @param out_title Pointer to receive the title string pointer (owned by
 * toolbar).
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_toolbar_base_get_title(const struct ui_toolbar_base *toolbar,
                                     const char **out_title);

/**
 * @brief Sets the layout mode of the toolbar.
 *
 * @param toolbar The toolbar.
 * @param mode The mode to set.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_toolbar_base_set_mode(struct ui_toolbar_base *toolbar,
                                    enum ui_toolbar_mode mode);

/**
 * @brief Gets the layout mode of the toolbar.
 *
 * @param toolbar The toolbar.
 * @param out_mode Pointer to receive the mode.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_toolbar_base_get_mode(const struct ui_toolbar_base *toolbar,
                                    enum ui_toolbar_mode *out_mode);

/**
 * @brief Sets the alignment of the toolbar contents.
 *
 * @param toolbar The toolbar.
 * @param alignment The alignment to set (row or column).
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_toolbar_base_set_alignment(struct ui_toolbar_base *toolbar,
                                         enum ui_toolbar_alignment alignment);

/**
 * @brief Gets the alignment of the toolbar contents.
 *
 * @param toolbar The toolbar.
 * @param out_alignment Pointer to receive the alignment.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t
ui_toolbar_base_get_alignment(const struct ui_toolbar_base *toolbar,
                              enum ui_toolbar_alignment *out_alignment);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_toolbar_base_bind_data(struct ui_toolbar_base *widget,
                                     struct ui_signal *signal);

#ifdef __cplusplus
}
#endif

#endif /* UI_TOOLBAR_BASE_H */
