/* clang-format off */
#include "ui_breakpoint_observer.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static void test_invalid_args(void) {
  struct ui_breakpoint_observer *observer = NULL;
  struct ui_signal *sig = NULL;

  assert(ui_breakpoint_observer_create(NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  ui_breakpoint_observer_destroy(NULL); /* Should not crash */

  assert(ui_breakpoint_observer_create(NULL, &observer) == UI_ERROR_NONE);

  assert(ui_breakpoint_observer_get_signal(NULL, UI_BREAKPOINT_SMALL, &sig) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_breakpoint_observer_get_signal(observer, UI_BREAKPOINT_SMALL,
                                           NULL) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_breakpoint_observer_get_signal(observer, UI_BREAKPOINT_COUNT,
                                           &sig) == UI_ERROR_INVALID_ARGUMENT);

  assert(ui_breakpoint_observer_tick(NULL, 100.0f, 0.0) ==
         UI_ERROR_INVALID_ARGUMENT);

  ui_breakpoint_observer_destroy(observer);
}

static void test_oom(void) {
  int i;
  enum ui_error rc;

  for (i = 0; i < 20; i++) {
    struct ui_breakpoint_observer *observer = NULL;
    g_malloc_fail_countdown = i;
    rc = ui_breakpoint_observer_create(NULL, &observer);
    if (rc == UI_ERROR_NONE) {
      ui_breakpoint_observer_destroy(observer);
      break;
    } else {
      assert(rc == UI_ERROR_OUT_OF_MEMORY);
    }
  }
  g_malloc_fail_countdown = -1;
}

static void test_widths(void) {
  struct ui_breakpoint_observer *o = NULL;
  ui_breakpoint_observer_create(NULL, &o);

  ui_breakpoint_observer_tick(o, 400.0, 100.0);        /* xsmall */
  ui_breakpoint_observer_tick(o, 400.0, 100.0 + 60.0); /* trigger debounce */

  ui_breakpoint_observer_tick(o, 600.0, 200.0); /* small lower bound */
  ui_breakpoint_observer_tick(o, 600.0, 200.0 + 60.0);

  ui_breakpoint_observer_tick(o, 900.0, 300.0); /* medium lower bound */
  ui_breakpoint_observer_tick(o, 900.0, 300.0 + 60.0);

  ui_breakpoint_observer_tick(o, 1200.0, 400.0); /* large lower bound */
  ui_breakpoint_observer_tick(o, 1200.0, 400.0 + 60.0);

  ui_breakpoint_observer_tick(o, 1536.0, 500.0); /* xlarge lower bound */
  ui_breakpoint_observer_tick(o, 1536.0, 500.0 + 60.0);

  ui_breakpoint_observer_destroy(o);
}
int main(void) {
  test_widths();
  struct ui_breakpoint_observer *observer = NULL;
  struct ui_signal *small_sig;
  struct ui_signal *large_sig;
  union ui_signal_payload val_small, val_large;
  enum ui_error rc;

  test_invalid_args();
  test_oom();

  rc = ui_breakpoint_observer_create(NULL, &observer);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "Failed to create observer\n");
    return 1;
  }

  rc = ui_breakpoint_observer_get_signal(observer, UI_BREAKPOINT_SMALL,
                                         &small_sig);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_breakpoint_observer_get_signal(observer, UI_BREAKPOINT_LARGE,
                                         &large_sig);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* First tick initiates */
  ui_breakpoint_observer_tick(observer, 800.0f, 0.0);

  /* Debouncing wait */
  ui_breakpoint_observer_tick(observer, 800.0f, 60.0);

  ui_signal_get(small_sig, &val_small);
  ui_signal_get(large_sig, &val_large);

  if (!val_small.bool_val || val_large.bool_val) {
    fprintf(stderr, "Failed first breakpoint evaluation: small=%d, large=%d\n",
            val_small.bool_val, val_large.bool_val);
    return 1;
  }

  /* Resize triggers debounce */
  ui_breakpoint_observer_tick(observer, 1300.0f, 70.0);

  /* Before debounce ends, value should remain small */
  ui_breakpoint_observer_tick(observer, 1300.0f, 80.0);
  ui_signal_get(small_sig, &val_small);
  if (!val_small.bool_val) {
    fprintf(stderr, "Debounce failed to hold state\n");
    return 1;
  }

  /* After debounce ends */
  ui_breakpoint_observer_tick(observer, 1300.0f, 130.0);
  ui_signal_get(small_sig, &val_small);
  ui_signal_get(large_sig, &val_large);

  if (val_small.bool_val || !val_large.bool_val) {
    fprintf(stderr, "Failed second breakpoint evaluation: small=%d, large=%d\n",
            val_small.bool_val, val_large.bool_val);
    return 1;
  }

  /* Wait beyond debounce threshold with no width change */
  ui_breakpoint_observer_tick(observer, 1300.0f, 200.0);

  ui_breakpoint_observer_destroy(observer);
  printf("test_ui_breakpoint_observer passed\n");
  return 0;
}
