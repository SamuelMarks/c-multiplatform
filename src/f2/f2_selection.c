/* clang-format off */
#include "f2/f2_selection.h"
#include "cmpc/cmp_core.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL f2_checkbox_style_init(F2CheckboxStyle *style) {
  if (!style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(style, 0, sizeof(*style));
  style->size = F2_CHECKBOX_DEFAULT_SIZE;
  style->corner_radius = F2_CHECKBOX_DEFAULT_CORNER_RADIUS;
  style->border_width = F2_CHECKBOX_DEFAULT_BORDER_WIDTH;

  /* These values represent typical Fluent 2 colors */
  style->unchecked.fill = cmp_color_rgba(0.0f, 0.0f, 0.0f, 0.0f);
  style->unchecked.border = cmp_color_rgba(0.4f, 0.4f, 0.4f, 1.0f);
  style->unchecked.mark = cmp_color_rgba(0.0f, 0.0f, 0.0f, 0.0f);

  style->checked.fill = cmp_color_rgba(0.0f, 0.36f, 0.77f, 1.0f);
  style->checked.border = cmp_color_rgba(0.0f, 0.36f, 0.77f, 1.0f);
  style->checked.mark = cmp_color_rgba(1.0f, 1.0f, 1.0f, 1.0f);

  style->mixed.fill = cmp_color_rgba(0.0f, 0.36f, 0.77f, 1.0f);
  style->mixed.border = cmp_color_rgba(0.0f, 0.36f, 0.77f, 1.0f);
  style->mixed.mark = cmp_color_rgba(1.0f, 1.0f, 1.0f, 1.0f);

  style->disabled_unchecked.fill = cmp_color_rgba(0.0f, 0.0f, 0.0f, 0.0f);
  style->disabled_unchecked.border = cmp_color_rgba(0.8f, 0.8f, 0.8f, 1.0f);
  style->disabled_unchecked.mark = cmp_color_rgba(0.0f, 0.0f, 0.0f, 0.0f);

  style->disabled_checked.fill = cmp_color_rgba(0.8f, 0.8f, 0.8f, 1.0f);
  style->disabled_checked.border = cmp_color_rgba(0.8f, 0.8f, 0.8f, 1.0f);
  style->disabled_checked.mark = cmp_color_rgba(0.5f, 0.5f, 0.5f, 1.0f);

  return CMP_OK;
}

CMP_API int CMP_CALL f2_checkbox_init(F2Checkbox *checkbox,
                                      const F2CheckboxStyle *style,
                                      CMPBool checked) {
  if (!checkbox || !style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(checkbox, 0, sizeof(*checkbox));
  checkbox->style = *style;
  checkbox->checked = checked;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_checkbox_set_checked(F2Checkbox *checkbox,
                                             CMPBool checked) {
  if (!checkbox)
    return CMP_ERR_INVALID_ARGUMENT;
  checkbox->checked = checked;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_checkbox_set_mixed_state(F2Checkbox *checkbox,
                                                 CMPBool mixed_state) {
  if (!checkbox)
    return CMP_ERR_INVALID_ARGUMENT;
  checkbox->mixed_state = mixed_state;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_checkbox_set_on_change(F2Checkbox *checkbox,
                                               F2CheckboxOnChange on_change,
                                               void *ctx) {
  if (!checkbox)
    return CMP_ERR_INVALID_ARGUMENT;
  checkbox->on_change = on_change;
  checkbox->on_change_ctx = ctx;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_switch_style_init(F2SwitchStyle *style) {
  if (!style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(style, 0, sizeof(*style));
  style->track_width = F2_SWITCH_DEFAULT_TRACK_WIDTH;
  style->track_height = F2_SWITCH_DEFAULT_TRACK_HEIGHT;

  style->off.fill = cmp_color_rgba(0.0f, 0.0f, 0.0f, 0.0f);
  style->off.border = cmp_color_rgba(0.4f, 0.4f, 0.4f, 1.0f);
  style->off.mark = cmp_color_rgba(0.4f, 0.4f, 0.4f, 1.0f);

  style->on.fill = cmp_color_rgba(0.0f, 0.36f, 0.77f, 1.0f);
  style->on.border = cmp_color_rgba(0.0f, 0.36f, 0.77f, 1.0f);
  style->on.mark = cmp_color_rgba(1.0f, 1.0f, 1.0f, 1.0f);

  style->disabled_off.fill = cmp_color_rgba(0.0f, 0.0f, 0.0f, 0.0f);
  style->disabled_off.border = cmp_color_rgba(0.8f, 0.8f, 0.8f, 1.0f);
  style->disabled_off.mark = cmp_color_rgba(0.8f, 0.8f, 0.8f, 1.0f);

  style->disabled_on.fill = cmp_color_rgba(0.8f, 0.8f, 0.8f, 1.0f);
  style->disabled_on.border = cmp_color_rgba(0.8f, 0.8f, 0.8f, 1.0f);
  style->disabled_on.mark = cmp_color_rgba(1.0f, 1.0f, 1.0f, 1.0f);

  return CMP_OK;
}

CMP_API int CMP_CALL f2_switch_init(F2Switch *widget,
                                    const F2SwitchStyle *style, CMPBool on) {
  if (!widget || !style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(widget, 0, sizeof(*widget));
  widget->style = *style;
  widget->on = on;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_switch_set_on(F2Switch *widget, CMPBool on) {
  if (!widget)
    return CMP_ERR_INVALID_ARGUMENT;
  widget->on = on;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_switch_set_on_change(F2Switch *widget,
                                             F2SwitchOnChange on_change,
                                             void *ctx) {
  if (!widget)
    return CMP_ERR_INVALID_ARGUMENT;
  widget->on_change = on_change;
  widget->on_change_ctx = ctx;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_radio_style_init(F2RadioStyle *style) {
  if (!style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(style, 0, sizeof(*style));
  style->size = F2_RADIO_DEFAULT_SIZE;
  style->border_width = F2_RADIO_DEFAULT_BORDER_WIDTH;
  style->dot_radius = F2_RADIO_DEFAULT_DOT_RADIUS;

  style->unchecked.fill = cmp_color_rgba(0.0f, 0.0f, 0.0f, 0.0f);
  style->unchecked.border = cmp_color_rgba(0.4f, 0.4f, 0.4f, 1.0f);
  style->unchecked.mark = cmp_color_rgba(0.0f, 0.0f, 0.0f, 0.0f);

  style->checked.fill = cmp_color_rgba(1.0f, 1.0f, 1.0f, 1.0f);
  style->checked.border = cmp_color_rgba(0.0f, 0.36f, 0.77f, 1.0f);
  style->checked.mark = cmp_color_rgba(0.0f, 0.36f, 0.77f, 1.0f);

  style->disabled_unchecked.fill = cmp_color_rgba(0.0f, 0.0f, 0.0f, 0.0f);
  style->disabled_unchecked.border = cmp_color_rgba(0.8f, 0.8f, 0.8f, 1.0f);
  style->disabled_unchecked.mark = cmp_color_rgba(0.0f, 0.0f, 0.0f, 0.0f);

  style->disabled_checked.fill = cmp_color_rgba(1.0f, 1.0f, 1.0f, 1.0f);
  style->disabled_checked.border = cmp_color_rgba(0.8f, 0.8f, 0.8f, 1.0f);
  style->disabled_checked.mark = cmp_color_rgba(0.8f, 0.8f, 0.8f, 1.0f);

  return CMP_OK;
}

CMP_API int CMP_CALL f2_radio_init(F2Radio *radio, const F2RadioStyle *style,
                                   CMPBool selected) {
  if (!radio || !style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(radio, 0, sizeof(*radio));
  radio->style = *style;
  radio->selected = selected;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_radio_set_selected(F2Radio *radio, CMPBool selected) {
  if (!radio)
    return CMP_ERR_INVALID_ARGUMENT;
  radio->selected = selected;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_radio_set_on_change(F2Radio *radio,
                                            F2RadioOnChange on_change,
                                            void *ctx) {
  if (!radio)
    return CMP_ERR_INVALID_ARGUMENT;
  radio->on_change = on_change;
  radio->on_change_ctx = ctx;
  return CMP_OK;
}
