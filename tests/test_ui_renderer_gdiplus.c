/* clang-format off */
#include "../include/ui_renderer.h"
#include "../include/ui_error.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

#if defined(_WIN32)
extern enum ui_error ui_renderer_native_init(struct ui_renderer *renderer);

int main(void) {
  struct ui_renderer renderer;

  if (ui_renderer_native_init(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "Expected UI_ERROR_INVALID_ARGUMENT for NULL renderer\n");
    return 1;
  }

  /* Ignore other failures in tests because we might not have a full environment
   * setup */
  printf("test_ui_renderer_gdiplus passed\n");
  return 0;
}
#else
int main(void) {
  printf("test_ui_renderer_gdiplus skipped (not WIN32)\n");
  return 0;
}
#endif
