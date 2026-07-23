/* clang-format off */
#include "ui_focus_ring.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

static enum ui_error test_focus_ring(void) {
  ui_color_t primary = UI_COLOR_ARGB(255, 10, 20, 30);
  struct ui_focus_ring ring;
  enum ui_error err;

  err = ui_focus_ring_get_standard(primary, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "ui_focus_ring_get_standard(NULL) failed\n");
    exit(1);
    return UI_ERROR_NONE;
  }

  err = ui_focus_ring_get_standard(primary, &ring);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_focus_ring_get_standard failed\n");
    exit(1);
    return UI_ERROR_NONE;
  }

  if (ring.color != primary || ring.offset != 2.0f) {
    fprintf(stderr, "ui_focus_ring_get_standard returned bad data\n");
    exit(1);
  }
}

int main(void) {
  test_focus_ring();
  printf("test_ui_focus_ring passed\n");
  return 0;
}
