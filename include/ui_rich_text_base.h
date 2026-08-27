/**
 * @file ui_rich_text_base.h
 */
#ifndef UI_RICH_TEXT_BASE_H
#define UI_RICH_TEXT_BASE_H

/**
 * \file ui_rich_text_base.h
 * \brief UI Rich Text Base component.
 *
 * This file contains definitions for a rich text editor component,
 * supporting formatted text runs, undo/redo history, and IME composition.
 */

/** \brief Forward declaration of ui_computed */
struct ui_computed;

/** \brief Forward declaration of ui_signal */
struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_event.h"
/* clang-format on */

/**
 * @brief Opaque handle representing a rich text editor base component.
 */
struct ui_rich_text_base;

/**
 * @brief Bitmask flags for rich text formatting.
 */
enum ui_rich_text_format {
  UI_RICH_TEXT_FORMAT_BOLD = 1,         /**< Bold format */
  UI_RICH_TEXT_FORMAT_ITALIC = 2,       /**< Italic format */
  UI_RICH_TEXT_FORMAT_UNDERLINE = 4,    /**< Underline format */
  UI_RICH_TEXT_FORMAT_STRIKETHROUGH = 8 /**< Strikethrough format */
};

/**
 * @brief Creates a rich text editor base component.
 *
 * @param out_editor Pointer to receive the allocated component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_create(struct ui_rich_text_base **out_editor);

/**
 * @brief Destroys a rich text editor base component.
 *
 * @param editor The rich text editor.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_destroy(struct ui_rich_text_base *editor);

/**
 * @brief Gets the underlying component.
 *
 * @param editor The rich text editor.
 * @param out_component Output pointer.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_get_component(struct ui_rich_text_base *editor,
                                           struct ui_component **out_component);

/**
 * @brief Sets the content of the editor.
 *
 * @param editor The rich text editor.
 * @param text The text to set.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_set_text(struct ui_rich_text_base *editor,
                                      const char *text);

/**
 * @brief Gets the content of the editor.
 *
 * Returned string must be freed by the caller using C_MULTIPLATFORM_FREE.
 *
 * @param editor The rich text editor.
 * @param out_text Pointer to receive the allocated string.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_get_text(struct ui_rich_text_base *editor,
                                      char **out_text);

/**
 * @brief Toggles a specific format on the current selection.
 *
 * @param editor The rich text editor.
 * @param format The format flag to toggle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_toggle_format(struct ui_rich_text_base *editor,
                                           enum ui_rich_text_format format);

/**
 * @brief Performs undo.
 *
 * @param editor The rich text editor.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_undo(struct ui_rich_text_base *editor);

/**
 * @brief Performs redo.
 *
 * @param editor The rich text editor.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_redo(struct ui_rich_text_base *editor);

/**
 * @brief Processes an input event (keyboard, mouse).
 *
 * @param editor The rich text editor.
 * @param event The input event.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_process_event(struct ui_rich_text_base *editor,
                                           const struct ui_event *event);

/**
 * @brief Sets the IME composition text (called from window backends during IME
 * input).
 *
 * @param editor The rich text editor.
 * @param composition_text The composition string.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_rich_text_base_set_ime_composition(struct ui_rich_text_base *editor,
                                      const char *composition_text);

/**
 * @brief Binds the text property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_bind_text(struct ui_rich_text_base *widget,
                                       struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_RICH_TEXT_BASE_H */
