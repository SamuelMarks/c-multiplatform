/* clang-format off */
#include "ui_web_bridge.h"
#include "ui_bidi_manager.h"
/* clang-format on */

static enum ui_bidi_direction g_ui_bidi_dir = UI_BIDI_DIR_LTR;

ui_error_t ui_bidi_set_direction(enum ui_bidi_direction direction) {
  if (direction != UI_BIDI_DIR_LTR && direction != UI_BIDI_DIR_RTL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  g_ui_bidi_dir = direction;
#if defined(__EMSCRIPTEN__)
  /* Send to root element */
  ui_web_bridge_set_style(0, "dir",
                          direction == UI_BIDI_DIR_RTL ? "rtl" : "ltr");
#endif
  return UI_ERROR_NONE;
}

ui_error_t ui_bidi_get_direction(enum ui_bidi_direction *out_dir) {
  if (!out_dir) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_dir = g_ui_bidi_dir;
  return UI_ERROR_NONE;
}
ui_error_t ui_bidi_normalize_horizontal_key(enum ui_key_code key,
                                            enum ui_key_code *out_key) {
  if (!out_key) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (g_ui_bidi_dir == UI_BIDI_DIR_RTL) {
    if (key == UI_KEY_LEFT) {
      *out_key = UI_KEY_RIGHT;
      return UI_ERROR_NONE;
    }
    if (key == UI_KEY_RIGHT) {
      *out_key = UI_KEY_LEFT;
      return UI_ERROR_NONE;
    }
  }
  *out_key = key;
  return UI_ERROR_NONE;
}
