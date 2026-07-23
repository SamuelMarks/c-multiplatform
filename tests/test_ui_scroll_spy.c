/* clang-format off */
#include "../include/ui_scroll_spy.h"
#include "../include/ui_error.h"
#include "../include/ui_dom_node.h"
#include <stdio.h>
/* clang-format on */

static int test_scroll_spy_lifecycle(void) {
  struct ui_scroll_spy *spy = NULL;
  enum ui_error rc;

  rc = ui_scroll_spy_create(&spy);
  if (rc != UI_ERROR_NONE || spy == NULL)
    return 1;

  ui_scroll_spy_destroy(spy);
  return 0;
}

static int test_scroll_spy_targets(void) {
  struct ui_scroll_spy *spy = NULL;
  struct ui_dom_node *target1 = (struct ui_dom_node *)0x10;
  struct ui_dom_node *target2 = (struct ui_dom_node *)0x20;
  enum ui_error rc;

  rc = ui_scroll_spy_create(&spy);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_scroll_spy_set_root(spy, NULL, -10);
  if (rc != UI_ERROR_NONE)
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

  rc = ui_scroll_spy_remove_target(spy, target1);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_scroll_spy_destroy(spy);
  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_scroll_spy tests...\n");

  failed |= test_scroll_spy_lifecycle();
  failed |= test_scroll_spy_targets();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
