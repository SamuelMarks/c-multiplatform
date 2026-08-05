/* clang-format off */
#include "../include/ui_text_layout_hb.h"
#include "ui_internal_mem.h"

#ifdef UI_USE_HARFBUZZ
#include <hb.h>
#endif
/* clang-format on */

ui_error_t ui_text_layout_hb_init(void) {
#ifdef UI_USE_HARFBUZZ
  return UI_ERROR_NONE;
#else
  return UI_ERROR_UNSUPPORTED;
#endif
}

/** \brief ui_text_layout_shape_with_harfbuzz */
ui_error_t ui_text_layout_shape_with_harfbuzz(
    struct ui_text_layout *layout, struct ui_font *font, float font_size,
    const char *text, float max_width, enum ui_text_direction direction) {
  if (!layout || !font || !text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#ifdef UI_USE_HARFBUZZ
  return UI_ERROR_NONE;
#else
  (void)font_size;
  (void)max_width;
  (void)direction;
  return UI_ERROR_UNSUPPORTED;
#endif
}
