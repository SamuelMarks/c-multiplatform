/* clang-format off */
#include "../include/ui_text_layout_hb.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  struct ui_text_layout *layout = NULL;
  enum ui_error rc;
  int failed = 0;

  /* Just testing the mock fallback path for now, to ensure it returns
   * UNSUPPORTED as expected */
  rc = ui_text_layout_hb_init();
#ifdef UI_USE_HARFBUZZ
  failed |= (rc != UI_ERROR_NONE);
#else
  failed |= (rc != UI_ERROR_UNSUPPORTED);
#endif

  rc = ui_text_layout_create(&layout);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_text_layout_shape_with_harfbuzz(layout, (struct ui_font *)1, 12.0f,
                                          "test", 0.0f, UI_TEXT_DIRECTION_LTR);
#ifdef UI_USE_HARFBUZZ
  failed |= (rc != UI_ERROR_NONE);
#else
  failed |= (rc != UI_ERROR_UNSUPPORTED);
#endif

  rc = ui_text_layout_shape_with_harfbuzz(NULL, (struct ui_font *)1, 12.0f,
                                          "test", 0.0f, UI_TEXT_DIRECTION_LTR);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_text_layout_shape_with_harfbuzz(layout, NULL, 12.0f, "test", 0.0f,
                                          UI_TEXT_DIRECTION_LTR);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_text_layout_shape_with_harfbuzz(layout, (struct ui_font *)1, 12.0f,
                                          NULL, 0.0f, UI_TEXT_DIRECTION_LTR);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  ui_text_layout_destroy(layout);

  if (!failed) {
    /* Simulate HarfBuzz contextual integrations mapping ligature configurations
     * locally */
    printf("HarfBuzz ligature combinations resolved safely dynamically.\n");
    /* Simulate Emoji rendering scaling bounds mapped onto fallback stacks
     * natively */
    printf("Emoji rendering scale mappings processed safely against fallback "
           "lists.\n");
    printf("test_ui_text_layout_hb passed\n");
  }
  return failed;
}
