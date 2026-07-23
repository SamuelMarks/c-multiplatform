#ifndef UI_COMMAND_PALETTE_BASE_H
#define UI_COMMAND_PALETTE_BASE_H
struct ui_computed;
struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

struct ui_component;
struct ui_autocomplete_base;
struct ui_dialog_base;
struct ui_virtual_scroll_base;

/**
 * @brief Base state for a Command Palette widget.
 */
struct ui_command_palette_base {
  struct ui_component *component;
  struct ui_autocomplete_base *autocomplete;
  struct ui_dialog_base *dialog;
  struct ui_virtual_scroll_base *list;
  int is_open;
  struct ui_signal *open_signal;
  struct ui_computed *animating_signal;
};

/**
 * @brief Initializes a base command palette component.
 *
 * @param palette Pointer to the command palette base struct.
 * @param component The UI component to bind to.
 * @param autocomplete Pointer to the autocomplete component.
 * @param dialog Pointer to the dialog component.
 * @param list Pointer to the virtualized list component.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_command_palette_base_init(
    struct ui_command_palette_base *palette, struct ui_component *component,
    struct ui_autocomplete_base *autocomplete, struct ui_dialog_base *dialog,
    struct ui_virtual_scroll_base *list);

/**
 * @brief Opens the command palette.
 *
 * @param palette Pointer to the command palette base struct.
 */
enum ui_error
ui_command_palette_base_open(struct ui_command_palette_base *palette);

/**
 * @brief Closes the command palette.
 *
 * @param palette Pointer to the command palette base struct.
 */
enum ui_error
ui_command_palette_base_close(struct ui_command_palette_base *palette);

/**
 * @brief Binds the open state to a signal.
 *
 * @param widget The widget.
 * @param open_signal The boolean signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_command_palette_base_bind_open(struct ui_command_palette_base *widget,
                                  struct ui_signal *open_signal);

/**
 * @brief Retrieves the computed signal indicating if the widget is animating.
 *
 * @param widget The widget.
 * @param out_animating Pointer to receive the computed signal.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_command_palette_base_get_animating_signal(
    struct ui_command_palette_base *widget, struct ui_computed **out_animating);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_COMMAND_PALETTE_BASE_H */
