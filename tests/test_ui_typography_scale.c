/* clang-format off */
#include "../include/ui_typography_scale.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

static int test_typography_scale(void) {
  struct ui_typography_metrics metrics;
  int i;
  int failed = 0;

  failed |=
      (ui_typography_scale_get_metrics(UI_TYPOGRAPHY_SCALE_BODY_SMALL, NULL) !=
       UI_ERROR_INVALID_ARGUMENT);

  for (i = UI_TYPOGRAPHY_SCALE_DISPLAY_LARGE;
       i <= UI_TYPOGRAPHY_SCALE_BODY_SMALL; i++) {
    failed |= (ui_typography_scale_get_metrics((enum ui_typography_scale)i,
                                               &metrics) != UI_ERROR_NONE);
  }

  failed |=
      (ui_typography_scale_get_metrics((enum ui_typography_scale)999,
                                       &metrics) != UI_ERROR_INVALID_ARGUMENT);

  return failed;
}

int main(void) {
  int failed = 0;
  failed |= test_typography_scale();
  return failed;
}
