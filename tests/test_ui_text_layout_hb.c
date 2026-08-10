/* clang-format off */
#include "../include/ui_text_layout_hb.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  int failed = 0;

  if (ui_text_layout_hb_init() != UI_ERROR_UNSUPPORTED &&
      ui_text_layout_hb_init() != UI_ERROR_NONE)
    failed = 1;

  struct ui_text_layout *layout = (struct ui_text_layout *)1;
  struct ui_font *font = (struct ui_font *)1;

  if (ui_text_layout_shape_with_harfbuzz(NULL, font, 12.0f, "A", 100.0f,
                                         UI_TEXT_DIRECTION_LTR) !=
      UI_ERROR_INVALID_ARGUMENT)
    failed = 1;
  if (ui_text_layout_shape_with_harfbuzz(layout, NULL, 12.0f, "A", 100.0f,
                                         UI_TEXT_DIRECTION_LTR) !=
      UI_ERROR_INVALID_ARGUMENT)
    failed = 1;
  if (ui_text_layout_shape_with_harfbuzz(layout, font, 12.0f, NULL, 100.0f,
                                         UI_TEXT_DIRECTION_LTR) !=
      UI_ERROR_INVALID_ARGUMENT)
    failed = 1;

  ui_error_t rc = ui_text_layout_shape_with_harfbuzz(
      layout, font, 12.0f, "A", 100.0f, UI_TEXT_DIRECTION_LTR);
  if (rc != UI_ERROR_UNSUPPORTED && rc != UI_ERROR_NONE)
    failed = 1;

  return failed;
}
