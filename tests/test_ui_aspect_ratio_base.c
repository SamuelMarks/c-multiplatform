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
      return rc_cleanup;
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
      return rc_cleanup;
    }
  }
  printf("test_aspect_ratio_set passed\n");
  return UI_ERROR_NONE;
}

static void test_aspect_ratio_edge_cases(void) {
  struct ui_aspect_ratio_base *ar = NULL;
  struct ui_component *comp = NULL;
  struct ui_signal *signal = (struct ui_signal *)0x123;
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

  assert(ui_aspect_ratio_base_bind_ratio(NULL, signal) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_aspect_ratio_base_bind_ratio(ar, signal) == UI_ERROR_NONE);

  /* Set ratio fails if DOM attribute set fails */
  g_malloc_fail_countdown = 0;
  assert(ui_aspect_ratio_base_set_ratio(ar, 1.5f) == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  {
    ui_error_t rc_cleanup = ui_aspect_ratio_base_destroy(ar);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  /* OOM loop */
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_aspect_ratio_base_create(&ar);
    if (rc == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_aspect_ratio_base_destroy(ar);
        assert(rc_cleanup == UI_ERROR_NONE);
      }
      break;
    }
  }
  g_malloc_fail_countdown = -1;
  printf("test_aspect_ratio_edge_cases passed\n");
}

#ifdef UI_TEST_MOCK_ALLOC
static ui_error_t run_aspect_ratio_coverage(void) {
  struct ui_aspect_ratio_base *ar = NULL;
  ui_error_t rc;
  extern int g_aspect_ratio_mock_fail;

  g_aspect_ratio_mock_fail = 1;
  rc = ui_aspect_ratio_base_create(&ar);
  if (rc != UI_ERROR_UNKNOWN) {
    return UI_ERROR_UNKNOWN;
  }
  g_aspect_ratio_mock_fail = 0;

  rc = ui_aspect_ratio_base_create(&ar);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  g_aspect_ratio_mock_fail = 2;
  rc = ui_aspect_ratio_base_destroy(ar);
  if (rc != UI_ERROR_UNKNOWN) {
    return UI_ERROR_UNKNOWN;
  }
  g_aspect_ratio_mock_fail = 0;

  return UI_ERROR_NONE;
}
#endif

int main(void) {
  ui_error_t rc;
  rc = test_aspect_ratio_creation();
  assert(rc == UI_ERROR_NONE);
  rc = test_aspect_ratio_set();
  assert(rc == UI_ERROR_NONE);
  test_aspect_ratio_edge_cases();

#ifdef UI_TEST_MOCK_ALLOC
  rc = run_aspect_ratio_coverage();
  assert(rc == UI_ERROR_NONE);
#endif
  return 0;
}
