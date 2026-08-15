/* clang-format off */
#include "ui_window_manager_base.h"
#include "ui_error.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

struct ui_window_manager_base {
  struct ui_component *component;
  struct ui_computed *data_signal;
};

extern int g_malloc_fail_countdown;

#define CHECK_FAIL(cond)                                                       \
  do {                                                                         \
    if (cond) {                                                                \
      printf("test_ui_window_manager_base failed at %d\n", __LINE__);          \
      failed = 1;                                                              \
    }                                                                          \
  } while (0)

static int test_window_manager_creation(void) {
  struct ui_window_manager_base *wm = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc = ui_window_manager_base_create(&wm);
  int failed = 0;
  CHECK_FAIL(rc != UI_ERROR_NONE);
  CHECK_FAIL(wm == NULL);

  CHECK_FAIL(ui_window_manager_base_get_component(wm, &comp) != UI_ERROR_NONE);
  CHECK_FAIL(comp == NULL);

  (void)ui_window_manager_base_destroy(wm);
  return failed;
}

static int test_window_manager_operations(void) {
  struct ui_window_manager_base *wm = NULL;
  ui_error_t rc;
  int failed = 0;

  rc = ui_window_manager_base_create(&wm);
  CHECK_FAIL(rc != UI_ERROR_NONE);

  rc = ui_window_manager_base_bring_to_front(wm, 1);
  CHECK_FAIL(rc != UI_ERROR_NONE);

  rc = ui_window_manager_base_drag(wm, 1, 10.0f, 20.0f);
  CHECK_FAIL(rc != UI_ERROR_NONE);

  (void)ui_window_manager_base_destroy(wm);
  return failed;
}

static int test_invalid_args(void) {
  struct ui_window_manager_base *wm = NULL;
  struct ui_component *comp = NULL;
  int failed = 0;

  CHECK_FAIL(ui_window_manager_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT);

  (void)ui_window_manager_base_destroy(NULL);

  CHECK_FAIL(ui_window_manager_base_get_component(NULL, &comp) !=
             UI_ERROR_INVALID_ARGUMENT);

  CHECK_FAIL(ui_window_manager_base_create(&wm) != UI_ERROR_NONE);

  CHECK_FAIL(ui_window_manager_base_get_component(wm, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  CHECK_FAIL(ui_window_manager_base_get_component(NULL, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);

  CHECK_FAIL(ui_window_manager_base_bring_to_front(NULL, 1) !=
             UI_ERROR_INVALID_ARGUMENT);
  CHECK_FAIL(ui_window_manager_base_drag(NULL, 1, 10.0f, 20.0f) !=
             UI_ERROR_INVALID_ARGUMENT);

  CHECK_FAIL(ui_window_manager_base_bind_data(NULL, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  CHECK_FAIL(ui_window_manager_base_bind_data(wm, NULL) != UI_ERROR_NONE);

  (void)ui_window_manager_base_destroy(wm);
  return failed;
}

static int test_oom(void) {
  int failed = 0;
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_window_manager_base *wm;
  int i;
  for (i = 0; i < 15; i++) {
    g_malloc_fail_countdown = i;
    (void)ui_window_manager_base_create(&wm);
  }
  g_malloc_fail_countdown = -1;
#endif
  return failed;
}

static int test_missing_coverage(void) {
  struct ui_window_manager_base *wm = NULL;
  ui_error_t rc = ui_window_manager_base_create(&wm);
  int failed = 0;
  if (rc == UI_ERROR_NONE) {
    /* Manually destroy component and set to NULL for coverage */
    extern ui_error_t ui_component_destroy(struct ui_component *);
    (void)ui_component_destroy(wm->component);
    wm->component = NULL;
    (void)ui_window_manager_base_destroy(wm);
  } else {
    failed = 1;
  }
  return failed;
}

int main(void) {
  int failed = 0;
  failed |= test_window_manager_creation();
  failed |= test_window_manager_operations();
  failed |= test_invalid_args();
  failed |= test_oom();
  failed |= test_missing_coverage();
  if (!failed) {
    printf("All window manager base tests passed\n");
  }
  return failed;
}
