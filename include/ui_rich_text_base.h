#ifndef UI_RICH_TEXT_BASE_H
#define UI_RICH_TEXT_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_event.h"
/* clang-format on */

struct ui_rich_text_base;

/** \brief ui_rich_text_format */
enum ui_rich_text_format {
  UI_RICH_TEXT_FORMAT_BOLD = 1,
  UI_RICH_TEXT_FORMAT_ITALIC = 2,
  UI_RICH_TEXT_FORMAT_UNDERLINE = 4,
  UI_RICH_TEXT_FORMAT_STRIKETHROUGH = 8
};

/**
 * @brief Creates a rich text editor base component.
 */
enum ui_error ui_rich_text_base_create(struct ui_rich_text_base **out_editor);

/**
 * @brief Destroys a rich text editor base component.
 */
void ui_rich_text_base_destroy(struct ui_rich_text_base *editor);

/**
 * @brief Gets the underlying component.
 *
 * @param editor The rich text editor.
 * @param out_component Output pointer.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_rich_text_base_get_component(struct ui_rich_text_base *editor,
                                struct ui_component **out_component);

/**
 * @brief Sets the content of the editor.
 */
enum ui_error ui_rich_text_base_set_text(struct ui_rich_text_base *editor,
                                         const char *text);

/**
 * @brief Gets the content of the editor. Returned string must be freed by the
 * caller.
 */
enum ui_error ui_rich_text_base_get_text(struct ui_rich_text_base *editor,
                                         char **out_text);

/**
 * @brief Toggles a specific format on the current selection.
 */
enum ui_error ui_rich_text_base_toggle_format(struct ui_rich_text_base *editor,
                                              enum ui_rich_text_format format);

/**
 * @brief Performs undo.
 */
enum ui_error ui_rich_text_base_undo(struct ui_rich_text_base *editor);

/**
 * @brief Performs redo.
 */
enum ui_error ui_rich_text_base_redo(struct ui_rich_text_base *editor);

/**
 * @brief Processes an input event (keyboard, mouse).
 */
enum ui_error ui_rich_text_base_process_event(struct ui_rich_text_base *editor,
                                              const struct ui_event *event);

/**
 * @brief Sets the IME composition text (called from window backends during IME
 * input).
 */
enum ui_error
ui_rich_text_base_set_ime_composition(struct ui_rich_text_base *editor,
                                      const char *composition_text);

/**
 * @brief Binds the text property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_rich_text_base_bind_text(struct ui_rich_text_base *widget,
                                          struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_RICH_TEXT_BASE_H */
