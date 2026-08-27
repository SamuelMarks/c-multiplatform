/* clang-format off */
#include "ui_command_palette_base.h"
/* clang-format on */

/*
 * @brief ui_command_palette_base_init.
 * @param palette Parameter palette.
 * @param component Parameter component.
 * @param autocomplete Parameter autocomplete.
 * @param dialog Parameter dialog.
 * @param list Parameter list.
 * @return Return value.
 */
ui_error_t ui_command_palette_base_init(
    struct ui_command_palette_base *palette, struct ui_component *component,
    struct ui_autocomplete_base *autocomplete, struct ui_dialog_base *dialog,
    struct ui_virtual_scroll_base *list) {
  if (!palette || !component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  palette->component = component;
  palette->autocomplete = autocomplete;
  palette->dialog = dialog;
  palette->list = list;
  palette->is_open = 0;

  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
ui_error_t
ui_command_palette_base_open(struct ui_command_palette_base *palette) {
  if (palette) {
    palette->is_open = 1;
  }
  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
ui_error_t
ui_command_palette_base_close(struct ui_command_palette_base *palette) {
  if (palette) {
    palette->is_open = 0;
  }
  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
ui_error_t
ui_command_palette_base_bind_open(struct ui_command_palette_base *widget,
                                  struct ui_signal *open_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->open_signal = open_signal;
  return UI_ERROR_NONE;
}

/* \brief ui_command_palette_base_get_animating_signal
 */
ui_error_t ui_command_palette_base_get_animating_signal(
    struct ui_command_palette_base *widget,
    struct ui_computed **out_animating) {
  if (!widget || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_animating = widget->animating_signal;
  return UI_ERROR_NONE;
}
