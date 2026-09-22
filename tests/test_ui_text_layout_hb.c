/* clang-format off */
#include "../include/ui_text_layout_hb.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  int failed;
  ui_error_t rc;
  struct ui_text_layout *layout;
  struct ui_font *font;

  failed = 0;
  layout = (struct ui_text_layout *)1;
  font = (struct ui_font *)1;

  rc = ui_text_layout_hb_init();
  if (rc != UI_ERROR_UNSUPPORTED && rc != UI_ERROR_NONE) {
    failed = 1;
  }

  rc = ui_text_layout_shape_with_harfbuzz(NULL, font, 12.0f, "A", 100.0f,
                                          UI_TEXT_DIRECTION_LTR);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

  rc = ui_text_layout_shape_with_harfbuzz(layout, NULL, 12.0f, "A", 100.0f,
                                          UI_TEXT_DIRECTION_LTR);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

  rc = ui_text_layout_shape_with_harfbuzz(layout, font, 12.0f, NULL, 100.0f,
                                          UI_TEXT_DIRECTION_LTR);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

  rc = ui_text_layout_shape_with_harfbuzz(layout, font, 12.0f, "A", 100.0f,
                                          (enum ui_text_direction)999);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

  rc = ui_text_layout_shape_with_harfbuzz(layout, font, 12.0f, "A", 100.0f,
                                          (enum ui_text_direction) - 1);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    failed = 1;
  }

  rc = ui_text_layout_shape_with_harfbuzz(layout, font, 12.0f, "A", 100.0f,
                                          UI_TEXT_DIRECTION_LTR);
  if (rc != UI_ERROR_UNSUPPORTED && rc != UI_ERROR_NONE) {
    failed = 1;
  }

  rc = ui_text_layout_shape_with_harfbuzz(layout, font, 0.0f, "A", 0.0f,
                                          UI_TEXT_DIRECTION_LTR);
  if (rc != UI_ERROR_UNSUPPORTED && rc != UI_ERROR_NONE) {
    failed = 1;
  }

  rc = ui_text_layout_shape_with_harfbuzz(layout, font, 12.0f, "A", 0.0f,
                                          UI_TEXT_DIRECTION_RTL);
  if (rc != UI_ERROR_UNSUPPORTED && rc != UI_ERROR_NONE) {
    failed = 1;
  }

  rc = ui_text_layout_shape_with_harfbuzz(layout, font, 0.0f, "A", 100.0f,
                                          UI_TEXT_DIRECTION_RTL);
  if (rc != UI_ERROR_UNSUPPORTED && rc != UI_ERROR_NONE) {
    failed = 1;
  }

  return failed;
}
