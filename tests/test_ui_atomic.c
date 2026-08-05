/* clang-format off */
#include "../include/ui_atomic.h"
#include "../include/ui_thread_pool.h"
#include <stdio.h>
/* clang-format on */

static ui_error_t thread_task_add(void *user_data) {
  ui_error_t rc;
  ui_atomic_t *val = (ui_atomic_t *)user_data;
  long old_val = 0;
  rc = ui_atomic_add(val, 1, &old_val);
  if (rc != UI_ERROR_NONE)
    return rc;
  return UI_ERROR_NONE;
}

static ui_error_t run_normal_tests(void) {
  ui_error_t rc;
  ui_atomic_t val = 0;
  long old_val;
  void *ptr_val = NULL;
  void *dummy_ptr1 = (void *)0x1234;
  void *dummy_ptr2 = (void *)0x5678;

  printf("Running atomic tests...\n");

  /* Test store and load */
  rc = ui_atomic_store(&val, 42);
  if (rc != UI_ERROR_NONE)
    return rc;
  {
    long load_val = 0;
    rc = ui_atomic_load(&val, &load_val);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (load_val != 42)
      return UI_ERROR_UNKNOWN;
  }

  /* Test add */
  rc = ui_atomic_add(&val, 10, &old_val);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (old_val != 42)
    return UI_ERROR_UNKNOWN;
  {
    long load_val = 0;
    rc = ui_atomic_load(&val, &load_val);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (load_val != 52)
      return UI_ERROR_UNKNOWN;
  }

  /* Test cas success */
  {
    int swapped = 0;
    rc = ui_atomic_cas(&val, 52, 100, &swapped);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (!swapped)
      return UI_ERROR_UNKNOWN;
  }
  {
    long load_val = 0;
    rc = ui_atomic_load(&val, &load_val);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (load_val != 100)
      return UI_ERROR_UNKNOWN;
  }

  /* Test cas fail */
  {
    int swapped = 0;
    rc = ui_atomic_cas(&val, 52, 200, &swapped);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (swapped)
      return UI_ERROR_UNKNOWN;
  }
  {
    long load_val = 0;
    rc = ui_atomic_load(&val, &load_val);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (load_val != 100)
      return UI_ERROR_UNKNOWN;
  }

  /* Test ptr cas success */
  {
    int swapped = 0;
    ptr_val = dummy_ptr1;
    rc = ui_atomic_ptr_cas((void *volatile *)&ptr_val, dummy_ptr1, dummy_ptr2,
                           &swapped);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (!swapped)
      return UI_ERROR_UNKNOWN;
    if (ptr_val != dummy_ptr2)
      return UI_ERROR_UNKNOWN;
  }

  /* Test ptr cas fail */
  {
    int swapped = 0;
    ptr_val = dummy_ptr1;
    rc = ui_atomic_ptr_cas((void *volatile *)&ptr_val, dummy_ptr2, dummy_ptr1,
                           &swapped);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (swapped)
      return UI_ERROR_UNKNOWN;
    if (ptr_val != dummy_ptr1)
      return UI_ERROR_UNKNOWN;
  }

  /* Test concurrent add with thread pool */
  {
    struct ui_thread_pool *pool = NULL;
    int i;
    rc = ui_atomic_store(&val, 0);
    if (rc != UI_ERROR_NONE)
      return rc;

    /* Test across 100 logical tasks */
    rc = ui_thread_pool_create(10, &pool);
    if (rc != UI_ERROR_NONE) {
      printf("Warning: Thread pool create failed, skipping concurrent test.\n");
      return rc;
    }
    for (i = 0; i < 100; i++) {
      rc = ui_thread_pool_schedule(pool, thread_task_add, (void *)&val);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    rc = ui_thread_pool_destroy(pool); /* waits for tasks */
    if (rc != UI_ERROR_NONE)
      return rc;

    long final_val = 0;
    rc = ui_atomic_load(&val, &final_val);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (final_val != 100) {
      printf("Concurrent add failed, expected 100, got %ld\n", final_val);
      return UI_ERROR_UNKNOWN;
    }
  }

  return UI_ERROR_NONE;
}

static ui_error_t test_null_args(void) {
  ui_error_t rc;
  ui_atomic_t val = 0;
  long old_val = 0;
  void *ptr_val = NULL;
  int swapped = 0;

  rc = ui_atomic_add(NULL, 10, &old_val);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_atomic_add(&val, 10, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_atomic_cas(NULL, 10, 20, &swapped);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_atomic_cas(&val, 10, 20, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_atomic_ptr_cas(NULL, NULL, NULL, &swapped);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_atomic_ptr_cas((void *volatile *)&ptr_val, NULL, NULL, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_atomic_load(NULL, &old_val);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_atomic_load(&val, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_atomic_store(NULL, 10);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  return UI_ERROR_NONE;
}

int main(void) {
  int failed = 0;
  failed |= (run_normal_tests() != UI_ERROR_NONE);
  failed |= (test_null_args() != UI_ERROR_NONE);

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All tests passed.\n");
  return 0;
}
