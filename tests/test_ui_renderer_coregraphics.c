/* clang-format off */
#include "../include/ui_renderer.h"
#include "../include/ui_error.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

#if defined(__APPLE__)
extern ui_error_t ui_renderer_native_init(struct ui_renderer *renderer);

int main(void) {
  struct ui_renderer renderer;

  if (ui_renderer_native_init(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "Expected UI_ERROR_INVALID_ARGUMENT for NULL renderer\n");
    return 1;
  }

  if (ui_renderer_native_init(&renderer) != UI_ERROR_NONE) {
    fprintf(stderr, "Expected UI_ERROR_NONE for CoreGraphics init\n");
    return 1;
  }

  /* Cleanup */
  if (renderer.vtable && renderer.vtable->destroy) {
    renderer.vtable->destroy(renderer.ctx);
  }

  printf("test_ui_renderer_coregraphics passed\n");
  return 0;
}
#else
int main(void) {
  printf("test_ui_renderer_coregraphics skipped (not APPLE)\n");
  return 0;
}
#endif
