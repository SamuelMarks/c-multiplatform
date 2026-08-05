/* clang-format off */
#include "../include/ui_scroll_spy.h"
#include "../include/ui_error.h"
#include "../include/ui_dom_node.h"
#include "../include/ui_signal.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_scroll_spy_lifecycle(void) {
  struct ui_scroll_spy *spy = NULL;
  ui_error_t rc;

  /* Test NULL out_spy */
  rc = ui_scroll_spy_create(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test OOM */
  g_malloc_fail_countdown = 0;
  rc = ui_scroll_spy_create(&spy);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  rc = ui_scroll_spy_create(&spy);
  if (rc != UI_ERROR_NONE || spy == NULL)
    return 1;

  /* Destroy with NULL */
  rc = ui_scroll_spy_destroy(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    (void)ui_scroll_spy_destroy(spy);
    return 1;
  }

  rc = ui_scroll_spy_destroy(spy);
  if (rc != UI_ERROR_NONE)
    return 1;

  return 0;
}

static int test_scroll_spy_targets(void) {
  struct ui_scroll_spy *spy = NULL;
  struct ui_dom_node *target1 = (struct ui_dom_node *)0x10;
  struct ui_dom_node *target2 = (struct ui_dom_node *)0x20;
  struct ui_dom_node *target3 = (struct ui_dom_node *)0x30;
  ui_error_t rc;
  int i;

  rc = ui_scroll_spy_create(&spy);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test evaluate before observer is set */
  rc = ui_scroll_spy_evaluate(spy);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test set_root with NULL spy */
  rc = ui_scroll_spy_set_root(NULL, NULL, 0);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_scroll_spy_set_root(spy, NULL, -10);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test OOM during observer creation in set_root */
  /* Actually observer creation requires allocs which we mock. But we can't
   * reliably mock IO's inner allocs without deeper integration, wait IO mock
   * fails if we set g_malloc_fail_countdown here */

  /* Test add_target invalid args */
  rc = ui_scroll_spy_add_target(NULL, target1, 1);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_scroll_spy_add_target(spy, NULL, 1);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_scroll_spy_add_target(spy, target1, 1);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_scroll_spy_add_target(spy, target2, 2);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_scroll_spy_evaluate(spy);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Call set_root again to test existing observer destruction */
  rc = ui_scroll_spy_set_root(spy, NULL, -20);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Fill up targets */
  for (i = 2; i < 64; i++) {
    rc = ui_scroll_spy_add_target(spy, target3, i);
    if (rc != UI_ERROR_NONE)
      return 1;
  }
  /* Next one should fail out of bounds */
  rc = ui_scroll_spy_add_target(spy, target3, 64);
  if (rc != UI_ERROR_OUT_OF_BOUNDS)
    return 1;

  /* Test OOM in set_root */
  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    ui_scroll_spy_set_root(spy, NULL, -10);
  }
  g_malloc_fail_countdown = -1;

  /* Test remove_target */
  rc = ui_scroll_spy_remove_target(NULL, target1);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_scroll_spy_remove_target(spy, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_scroll_spy_remove_target(spy, target1);
  if (rc != UI_ERROR_NONE)
    return 1;
  /* Remove again, should fail */
  rc = ui_scroll_spy_remove_target(spy, target1);
  if (rc != UI_ERROR_NOT_FOUND)
    return 1;

  (void)ui_scroll_spy_destroy(spy);
  return 0;
}

static int test_scroll_spy_signal(void) {
  struct ui_scroll_spy *spy = NULL;
  ui_signal_t *sig = NULL;
  union ui_signal_payload init_val;
  struct ui_dom_node *target1 = (struct ui_dom_node *)0x10;
  ui_error_t rc;

  rc = ui_scroll_spy_create(&spy);
  if (rc != UI_ERROR_NONE)
    return 1;

  init_val.int_val = -1;
  rc = ui_signal_create(NULL, init_val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_scroll_spy_bind_active_section(NULL, sig);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_scroll_spy_bind_active_section(spy, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_scroll_spy_bind_active_section(spy, sig);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Add root, target, and evaluate to trigger on_intersection and hit
   * ui_signal_set */
  rc = ui_scroll_spy_set_root(spy, NULL, 0);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_scroll_spy_add_target(spy, target1, 1);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_scroll_spy_evaluate(spy);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* test evaluate invalid args */
  rc = ui_scroll_spy_evaluate(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  (void)ui_signal_destroy(sig);
  (void)ui_scroll_spy_destroy(spy);
  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_scroll_spy tests...\n");

  failed |= test_scroll_spy_lifecycle();
  failed |= test_scroll_spy_targets();
  failed |= test_scroll_spy_signal();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
