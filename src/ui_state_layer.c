/**
 * @file ui_state_layer.c
 * @brief ui_state_layer.c implementation.
 */
/* clang-format off */
#include "ui_state_layer.h"
/* clang-format on */

/**
 * @brief ui_state_layer_get_color.
 * @param base_color Parameter base_color.
 * @param on_color Parameter on_color.
 * @param state Parameter state.
 * @param out_color Parameter out_color.
 * @return Return value.
 */
ui_error_t ui_state_layer_get_color(ui_color_t base_color, ui_color_t on_color,
                                    enum ui_state_layer_type state,
                                    ui_color_t *out_color) {
  float opacity = 0.0f;
  ui_uint8 r_base, g_base, b_base;
  ui_uint8 r_on, g_on, b_on;
  ui_uint8 r_out, g_out, b_out;

  if (!out_color) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /*
   * Common Material 3 state layer opacities:
   * Hover: 0.08
   * Focus: 0.12
   * Pressed: 0.12
   * Dragged: 0.16
   */
  switch (state) {
  case UI_STATE_LAYER_HOVER:
    opacity = 0.08f;
    break;
  case UI_STATE_LAYER_FOCUS:
  case UI_STATE_LAYER_PRESSED:
    opacity = 0.12f;
    break;
  case UI_STATE_LAYER_DRAGGED:
    opacity = 0.16f;
    break;
  default:
    return UI_ERROR_INVALID_ARGUMENT;
  }

  r_base = UI_COLOR_RED(base_color);
  g_base = UI_COLOR_GREEN(base_color);
  b_base = UI_COLOR_BLUE(base_color);

  r_on = UI_COLOR_RED(on_color);
  g_on = UI_COLOR_GREEN(on_color);
  b_on = UI_COLOR_BLUE(on_color);

  r_out = (ui_uint8)((r_on * opacity) + (r_base * (1.0f - opacity)));
  g_out = (ui_uint8)((g_on * opacity) + (g_base * (1.0f - opacity)));
  b_out = (ui_uint8)((b_on * opacity) + (b_base * (1.0f - opacity)));

  *out_color = UI_COLOR_ARGB(UI_COLOR_ALPHA(base_color), r_out, g_out, b_out);
  return UI_ERROR_NONE;
}
