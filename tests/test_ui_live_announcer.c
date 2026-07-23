/* clang-format off */
#include "../include/ui_live_announcer.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_live_announcer_lifecycle(void) {
  struct ui_live_announcer *announcer = NULL;
  enum ui_error rc;

  rc = ui_live_announcer_create(&announcer);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_live_announce(announcer, "Form submitted successfully.",
                        UI_LIVE_POLITE);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_live_announce(announcer, "Error: Invalid username.",
                        UI_LIVE_ASSERTIVE);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_live_announcer_clear(announcer);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_live_announcer_destroy(announcer);
  return 0;
}

static int test_live_announcer_edge_cases(void) {
  struct ui_live_announcer *announcer;
  ui_live_announcer_create(&announcer);

  ui_live_announcer_create(NULL);
  ui_live_announce(NULL, "a", UI_LIVE_POLITE);
  ui_live_announce(announcer, NULL, UI_LIVE_POLITE);
  ui_live_announcer_clear(NULL);
  ui_live_announcer_destroy(NULL);

  g_malloc_fail_countdown = 0;
  ui_live_announcer_create(&announcer);

  ui_live_announcer_create(&announcer);
  g_malloc_fail_countdown = 0;
  ui_live_announce(announcer, "a", UI_LIVE_POLITE);
  g_malloc_fail_countdown = 1;
  ui_live_announce(announcer, "b", UI_LIVE_POLITE);

  ui_live_announcer_destroy(announcer);
  g_malloc_fail_countdown = -1;
  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_live_announcer tests...\n");

  failed |= test_live_announcer_lifecycle();
  failed |= test_live_announcer_edge_cases();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
