#include "f2/f2_spin_button.h"
#include <string.h>

CMP_API int CMP_CALL f2_spin_button_style_init(F2SpinButtonStyle *style) {
  if (!style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(style, 0, sizeof(*style));

  f2_text_field_style_init(&style->text_field_style);
  f2_button_style_init_standard(&style->button_style);

  /* Spin buttons typically have smaller up/down chevrons */
  style->button_style.min_width = 24.0f;
  style->button_style.min_height = 16.0f;
  style->button_style.padding_x = 4.0f;
  style->button_style.padding_y = 2.0f;

  return CMP_OK;
}

CMP_API int CMP_CALL f2_spin_button_init(
    F2SpinButton *spin_button, const CMPTextBackend *backend,
    const F2SpinButtonStyle *style, CMPAllocator *allocator, CMPScalar min_val,
    CMPScalar max_val, CMPScalar step, CMPScalar initial_val) {
  int res;
  if (!spin_button || !backend || !style || !allocator) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(spin_button, 0, sizeof(*spin_button));
  spin_button->style = *style;
  spin_button->min_value = min_val;
  spin_button->max_value = max_val;
  spin_button->step = step;
  spin_button->value = initial_val;

  res = f2_text_field_init(&spin_button->text_field, backend,
                           &style->text_field_style, allocator);
  if (res != CMP_OK)
    return res;

  res = f2_button_init(&spin_button->up_button, backend, &style->button_style,
                       "^", 1);
  if (res != CMP_OK)
    return res;

  res = f2_button_init(&spin_button->down_button, backend, &style->button_style,
                       "v", 1);
  if (res != CMP_OK)
    return res;

  return CMP_OK;
}

CMP_API int CMP_CALL f2_spin_button_set_value(F2SpinButton *spin_button,
                                              CMPScalar value) {
  if (!spin_button)
    return CMP_ERR_INVALID_ARGUMENT;
  if (value < spin_button->min_value)
    value = spin_button->min_value;
  if (value > spin_button->max_value)
    value = spin_button->max_value;
  spin_button->value = value;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_spin_button_set_on_change(
    F2SpinButton *spin_button, F2SpinButtonOnChange on_change, void *ctx) {
  if (!spin_button)
    return CMP_ERR_INVALID_ARGUMENT;
  spin_button->on_change = on_change;
  spin_button->on_change_ctx = ctx;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_spin_button_cleanup(F2SpinButton *spin_button) {
  if (!spin_button)
    return CMP_ERR_INVALID_ARGUMENT;
  /* cmp_text_field cleanup is unneeded/does not exist */
  return CMP_OK;
}
