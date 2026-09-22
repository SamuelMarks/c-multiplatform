/* clang-format off */
#include "../include/ui_handle_manager.h"
#include <stdio.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static void test_handle_manager_basic(void) {
  struct ui_handle_manager *mgr = NULL;
  ui_uint64 handle1, handle2;
  void *data1 = (void *)0x1234;
  void *data2 = (void *)0x5678;
  void *out_data;
  ui_error_t rc;

  /* Invalid args */
  rc = ui_handle_manager_create(0, &mgr);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_handle_manager_create(10, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_handle_manager_create(2, &mgr);
  assert(rc == UI_ERROR_NONE);

  rc = ui_handle_manager_alloc(NULL, data1, &handle1);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_handle_manager_alloc(mgr, data1, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Alloc 1 */
  rc = ui_handle_manager_alloc(mgr, data1, &handle1);
  assert(rc == UI_ERROR_NONE);

  /* Alloc 2 */
  rc = ui_handle_manager_alloc(mgr, data2, &handle2);
  assert(rc == UI_ERROR_NONE);

  /* Queue full */
  rc = ui_handle_manager_alloc(mgr, data1, &handle1);
  assert(rc == UI_ERROR_QUEUE_FULL);

  /* Get invalid */
  rc = ui_handle_manager_get(NULL, handle1, &out_data);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_handle_manager_get(mgr, UI_INVALID_HANDLE, &out_data);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_handle_manager_get(mgr, handle1, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_handle_manager_get(mgr, 99999, &out_data);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Get valid */
  rc = ui_handle_manager_get(mgr, handle2, &out_data);
  assert(rc == UI_ERROR_NONE);
  assert(out_data == data2);

  /* Forged handle (unallocated index but correct generation) */
  rc = ui_handle_manager_get(mgr, (((ui_uint64)1) << 32) | 9, &out_data);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_handle_manager_free(mgr, (((ui_uint64)1) << 32) | 9);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Free invalid */
  rc = ui_handle_manager_free(NULL, handle2);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_handle_manager_free(mgr, UI_INVALID_HANDLE);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_handle_manager_free(mgr, 99999);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Free valid */
  rc = ui_handle_manager_free(mgr, handle2);
  assert(rc == UI_ERROR_NONE);

  /* Free already freed */
  rc = ui_handle_manager_free(mgr, handle2);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Get already freed */
  rc = ui_handle_manager_get(mgr, handle2, &out_data);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Test NULL data allocation */
  {
    ui_uint64 handle_null;
    rc = ui_handle_manager_alloc(mgr, NULL, &handle_null);
    assert(rc == UI_ERROR_NONE);
    rc = ui_handle_manager_get(mgr, handle_null, &out_data);
    assert(rc == UI_ERROR_INVALID_ARGUMENT);
    rc = ui_handle_manager_free(mgr, handle_null);
    assert(rc == UI_ERROR_INVALID_ARGUMENT);
  }

  /* Destroy */
  rc = ui_handle_manager_destroy(NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_handle_manager_destroy(mgr);
  assert(rc == UI_ERROR_NONE);
}

static void run_oom_tests_handle_manager(void) {
  struct ui_handle_manager *mgr;
  int i;
  ui_error_t rc;

  for (i = 0; i < 50; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_handle_manager_create(10, &mgr);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_handle_manager_destroy(mgr);
        assert(rc_cleanup == UI_ERROR_NONE);
      }
      break;
    }
  }
}

#ifdef UI_TEST_MOCK_ALLOC
#ifndef UI_SINGLE_THREADED
extern int g_handle_manager_mock_cas_fail;
extern int g_handle_manager_mock_store_fail;
#endif
#endif

static void test_handle_manager_atomic_failures(void) {
#ifdef UI_TEST_MOCK_ALLOC
#ifndef UI_SINGLE_THREADED
  struct ui_handle_manager *mgr = NULL;
  ui_uint64 handle1, handle2;
  void *data1 = (void *)0x1234;
  void *out_data = NULL;
  ui_error_t rc;

  /* 1. ui_handle_manager_create: atomic_store fails */
  g_handle_manager_mock_store_fail = 1;
  rc = ui_handle_manager_create(2, &mgr);
  assert(rc == UI_ERROR_UNKNOWN);
  assert(mgr == NULL);
  g_handle_manager_mock_store_fail = 0;

  rc = ui_handle_manager_create(2, &mgr);
  assert(rc == UI_ERROR_NONE);

  /* 2. ui_handle_manager_alloc: spin_lock fails */
  g_handle_manager_mock_cas_fail = 1;
  rc = ui_handle_manager_alloc(mgr, data1, &handle1);
  assert(rc == UI_ERROR_UNKNOWN);
  g_handle_manager_mock_cas_fail = 0;

  /* 3. ui_handle_manager_alloc: success path spin_unlock fails */
  g_handle_manager_mock_store_fail = 1;
  rc = ui_handle_manager_alloc(mgr, data1, &handle1);
  assert(rc == UI_ERROR_UNKNOWN);
  g_handle_manager_mock_store_fail = 0;

  /* Destroy mgr and create fresh one for remaining tests */
  rc = ui_handle_manager_destroy(mgr);
  assert(rc == UI_ERROR_NONE);
  rc = ui_handle_manager_create(2, &mgr);
  assert(rc == UI_ERROR_NONE);

  /* Alloc real handle1 and handle2 to fill queue (capacity 2) */
  rc = ui_handle_manager_alloc(mgr, data1, &handle1);
  assert(rc == UI_ERROR_NONE);
  rc = ui_handle_manager_alloc(mgr, data1, &handle2);
  assert(rc == UI_ERROR_NONE);

  /* 4. ui_handle_manager_alloc: queue full spin_unlock fails */
  g_handle_manager_mock_store_fail = 1;
  rc = ui_handle_manager_alloc(mgr, data1, &handle1);
  assert(rc == UI_ERROR_UNKNOWN);
  g_handle_manager_mock_store_fail = 0;

  /* 5. ui_handle_manager_get: spin_lock fails */
  g_handle_manager_mock_cas_fail = 1;
  rc = ui_handle_manager_get(mgr, handle1, &out_data);
  assert(rc == UI_ERROR_UNKNOWN);
  g_handle_manager_mock_cas_fail = 0;

  /* 6. ui_handle_manager_get: valid handle spin_unlock fails */
  g_handle_manager_mock_store_fail = 1;
  rc = ui_handle_manager_get(mgr, handle1, &out_data);
  assert(rc == UI_ERROR_UNKNOWN);
  g_handle_manager_mock_store_fail = 0;

  /* 7. ui_handle_manager_get: stale handle spin_unlock fails */
  g_handle_manager_mock_store_fail = 1;
  rc = ui_handle_manager_get(mgr, (((ui_uint64)999) << 32) | 0, &out_data);
  assert(rc == UI_ERROR_UNKNOWN);
  g_handle_manager_mock_store_fail = 0;

  /* 8. ui_handle_manager_free: spin_lock fails */
  g_handle_manager_mock_cas_fail = 1;
  rc = ui_handle_manager_free(mgr, handle1);
  assert(rc == UI_ERROR_UNKNOWN);
  g_handle_manager_mock_cas_fail = 0;

  /* 9. ui_handle_manager_free: stale handle spin_unlock fails */
  g_handle_manager_mock_store_fail = 1;
  rc = ui_handle_manager_free(mgr, (((ui_uint64)999) << 32) | 0);
  assert(rc == UI_ERROR_UNKNOWN);
  g_handle_manager_mock_store_fail = 0;

  /* 10. ui_handle_manager_free: valid handle spin_unlock fails */
  g_handle_manager_mock_store_fail = 1;
  rc = ui_handle_manager_free(mgr, handle1);
  assert(rc == UI_ERROR_UNKNOWN);
  g_handle_manager_mock_store_fail = 0;

  rc = ui_handle_manager_destroy(mgr);
  assert(rc == UI_ERROR_NONE);
#endif
#endif
}

int main(void) {
  test_handle_manager_basic();
  test_handle_manager_atomic_failures();
  run_oom_tests_handle_manager();
  printf("All ui_handle_manager tests passed.\n");
  return 0;
}
