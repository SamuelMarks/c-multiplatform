/* clang-format off */
#include "ui_aspect_ratio_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t test_aspect_ratio_creation(void) {
  struct ui_aspect_ratio_base *ar = NULL;
  ui_error_t rc = ui_aspect_ratio_base_create(&ar);
  assert(rc == UI_ERROR_NONE);
  assert(ar != NULL);
  {
    ui_error_t rc_cleanup = ui_aspect_ratio_base_destroy(ar);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  printf("test_aspect_ratio_creation passed\n");
  return UI_ERROR_NONE;
}

static ui_error_t test_aspect_ratio_set(void) {
  struct ui_aspect_ratio_base *ar = NULL;
  ui_error_t rc;

  rc = ui_aspect_ratio_base_create(&ar);
  assert(rc == UI_ERROR_NONE);

  rc = ui_aspect_ratio_base_set_ratio(ar, 16.0f / 9.0f);
  assert(rc == UI_ERROR_NONE);

  {
    ui_error_t rc_cleanup = ui_aspect_ratio_base_destroy(ar);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  printf("test_aspect_ratio_set passed\n");
  return UI_ERROR_NONE;
}

static void test_aspect_ratio_edge_cases(void) {
  struct ui_aspect_ratio_base *ar = NULL;
  struct ui_component *comp = NULL;
  int i;
  ui_error_t rc;

  /* NULL pointers */
  assert(ui_aspect_ratio_base_create(NULL) == UI_ERROR_INVALID_ARGUMENT);
  ui_aspect_ratio_base_destroy(NULL); /* Should not crash */

  assert(ui_aspect_ratio_base_set_ratio(NULL, 1.0f) ==
         UI_ERROR_INVALID_ARGUMENT);

  rc = ui_aspect_ratio_base_create(&ar);
  assert(rc == UI_ERROR_NONE);
  assert(ui_aspect_ratio_base_set_ratio(ar, -1.0f) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_aspect_ratio_base_set_ratio(ar, 0.0f) == UI_ERROR_INVALID_ARGUMENT);

  assert(ui_aspect_ratio_base_get_component(NULL, &comp) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_aspect_ratio_base_get_component(ar, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_aspect_ratio_base_get_component(ar, &comp) == UI_ERROR_NONE);

  struct ui_signal *signal = (struct ui_signal *)0x123;
  assert(ui_aspect_ratio_base_bind_ratio(NULL, signal) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_aspect_ratio_base_bind_ratio(ar, signal) == UI_ERROR_NONE);

  /* Set ratio fails if DOM attribute set fails */
  g_malloc_fail_countdown = 0;
  assert(ui_aspect_ratio_base_set_ratio(ar, 1.5f) == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  {
    ui_error_t rc_cleanup = ui_aspect_ratio_base_destroy(ar);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* OOM loop */
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_aspect_ratio_base_create(&ar);
    if (rc == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_aspect_ratio_base_destroy(ar);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      break;
    }
  }
  g_malloc_fail_countdown = -1;
  printf("test_aspect_ratio_edge_cases passed\n");
}

int main(void) {
  test_aspect_ratio_creation();
  test_aspect_ratio_set();
  test_aspect_ratio_edge_cases();

#ifdef UI_TEST_MOCK_ALLOC
  extern ui_error_t run_aspect_ratio_coverage(void);
  run_aspect_ratio_coverage();
#endif
  return 0;
}
