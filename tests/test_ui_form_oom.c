/* clang-format off */
#include "ui_arena.h"
#include "ui_form_control.h"
#include "ui_signal.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int ui_mock_alloc_fail_countdown;
extern int ui_mock_alloc_fail_enabled;
#endif

int main(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_arena *arena;
  struct ui_form_control *control;
  union ui_signal_payload init_val;
  enum ui_error rc;
  int i;

  init_val.int_val = 0;

  for (i = 0; i < 20; i++) {
    ui_mock_alloc_fail_enabled = 1;
    ui_mock_alloc_fail_countdown = i;

    rc = ui_arena_create(1024, &arena);
    if (rc == UI_ERROR_NONE) {
      rc = ui_form_control_create(arena, init_val, UI_SIGNAL_TYPE_INT32, NULL,
                                  NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                                  &control);
      if (rc == UI_ERROR_NONE) {
        ui_form_control_destroy(control);
      }
      ui_arena_destroy(arena);
    }

    ui_mock_alloc_fail_enabled = 0;
  }
#endif
  printf("test_ui_form_oom passed\\n");
  return 0;
}
