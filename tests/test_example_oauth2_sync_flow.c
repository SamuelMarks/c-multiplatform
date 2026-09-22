/**
 * @file test_example_oauth2_sync_flow.c
 * @brief Integration runner and OOM test for oauth2_sync_flow example.
 */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OMIT_MAIN 1
#include "../examples/oauth2_sync_flow/main.c"
#undef OMIT_MAIN

extern int g_malloc_fail_countdown;
extern int g_malloc_called;

int main(void) {
  int i;
  int rc;
  printf("Running OOM loop for oauth2_sync_flow...\n");
  for (i = 1; i < 5; i++) {
    g_malloc_called = 0;
    g_malloc_fail_countdown = i;
    rc = example_oauth2_sync_flow_main();
    if (rc != 0) {
      /* Expected failure on simulated memory exhaustion */
    }
    if (g_malloc_fail_countdown > 0) {
      break;
    }
  }
  g_malloc_fail_countdown = -1;
  /* Run once successfully */
  rc = example_oauth2_sync_flow_main();
  if (rc != 0) {
    return rc;
  }
  return 0;
}
/* clang-format on */
