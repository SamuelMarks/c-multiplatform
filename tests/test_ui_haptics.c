/* clang-format off */
#include "ui_haptics.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  ui_error_t rc;

  /* Test triggering different haptic feedback types.
     Since this is a stub, we expect UI_ERROR_UNSUPPORTED. */

  rc = ui_haptics_trigger(UI_HAPTIC_FEEDBACK_LIGHT);
  if (rc != UI_ERROR_UNSUPPORTED) {
    printf("Expected UI_ERROR_UNSUPPORTED for LIGHT haptics\n");
    return 1;
  }

  rc = ui_haptics_trigger(UI_HAPTIC_FEEDBACK_HEAVY);
  if (rc != UI_ERROR_UNSUPPORTED) {
    printf("Expected UI_ERROR_UNSUPPORTED for HEAVY haptics\n");
    return 1;
  }

  printf("test_ui_haptics passed\n");
  return 0;
}
