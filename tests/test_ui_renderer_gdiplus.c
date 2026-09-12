/* clang-format off */
#include "../include/ui_renderer.h"
#include "../include/ui_error.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

int main(void) {
  struct ui_renderer renderer;

  if (ui_renderer_gdiplus_init(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "Expected UI_ERROR_INVALID_ARGUMENT for NULL renderer\n");
    return 1;
  }

#if defined(_WIN32)
  if (ui_renderer_native_init(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "Expected UI_ERROR_INVALID_ARGUMENT for NULL renderer in "
                    "native_init\n");
    return 1;
  }
#else
  if (ui_renderer_gdiplus_init(&renderer) != UI_ERROR_UNKNOWN) {
    fprintf(stderr, "Expected UI_ERROR_UNKNOWN for stub on non-Windows\n");
    return 1;
  }
#endif

  printf("test_ui_renderer_gdiplus passed\n");
  return 0;
}
