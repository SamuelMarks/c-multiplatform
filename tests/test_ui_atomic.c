/* clang-format off */
#include "../include/ui_atomic.h"
#include "../include/ui_thread_pool.h"
#include <stdio.h>
/* clang-format on */

static enum ui_error thread_task_add(void *user_data) {
  ui_atomic_t *val = (ui_atomic_t *)user_data;
  long old_val = 0;
  ui_atomic_add(val, 1, &old_val);
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  ui_atomic_t val = 0;
  long old_val;
  void *ptr_val = NULL;
  void *dummy_ptr1 = (void *)0x1234;
  void *dummy_ptr2 = (void *)0x5678;

  printf("Running atomic tests...\n");

  /* Test store and load */
  ui_atomic_store(&val, 42);
  {
    long load_val = 0;
    ui_atomic_load(&val, &load_val);
    if (load_val != 42)
      return 1;
  }

  /* Test add */
  ui_atomic_add(&val, 10, &old_val);
  if (old_val != 42)
    return 1;
  {
    long load_val = 0;
    ui_atomic_load(&val, &load_val);
    if (load_val != 52)
      return 1;
  }

  /* Test cas success */
  {
    int swapped = 0;
    ui_atomic_cas(&val, 52, 100, &swapped);
    if (!swapped)
      return 1;
  }
  {
    long load_val = 0;
    ui_atomic_load(&val, &load_val);
    if (load_val != 100)
      return 1;
  }

  /* Test cas fail */
  {
    int swapped = 0;
    ui_atomic_cas(&val, 52, 200, &swapped);
    if (swapped)
      return 1;
  }
  {
    long load_val = 0;
    ui_atomic_load(&val, &load_val);
    if (load_val != 100)
      return 1;
  }

  /* Test ptr cas success */
  {
    int swapped = 0;
    ptr_val = dummy_ptr1;
    ui_atomic_ptr_cas((void *volatile *)&ptr_val, dummy_ptr1, dummy_ptr2,
                      &swapped);
    if (!swapped)
      return 1;
    if (ptr_val != dummy_ptr2)
      return 1;
  }

  /* Test ptr cas fail */
  {
    int swapped = 0;
    ptr_val = dummy_ptr1;
    ui_atomic_ptr_cas((void *volatile *)&ptr_val, dummy_ptr2, dummy_ptr1,
                      &swapped);
    if (swapped)
      return 1;
    if (ptr_val != dummy_ptr1)
      return 1;
  }

  /* Test concurrent add with thread pool */
  {
    struct ui_thread_pool *pool = NULL;
    int i;
    ui_atomic_store(&val, 0);

    /* Test across 100 logical tasks */
    if (ui_thread_pool_create(10, &pool) == UI_ERROR_NONE) {
      for (i = 0; i < 100; i++) {
        ui_thread_pool_schedule(pool, thread_task_add, (void *)&val);
      }
      ui_thread_pool_destroy(pool); /* waits for tasks */

      long final_val = 0;
      ui_atomic_load(&val, &final_val);
      if (final_val != 100) {
        printf("Concurrent add failed, expected 100, got %ld\n", final_val);
        return 1;
      }
    } else {
      printf("Warning: Thread pool create failed, skipping concurrent test.\n");
    }
  }

  return 0;
}

static int test_null_args(void) {
  ui_atomic_t val = 0;
  long old_val = 0;
  void *ptr_val = NULL;
  int swapped = 0;

  if (ui_atomic_add(NULL, 10, &old_val) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_atomic_add(&val, 10, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_atomic_cas(NULL, 10, 20, &swapped) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_atomic_cas(&val, 10, 20, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_atomic_ptr_cas(NULL, NULL, NULL, &swapped) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_atomic_ptr_cas((void *volatile *)&ptr_val, NULL, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_atomic_load(NULL, &old_val) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_atomic_load(&val, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_atomic_store(NULL, 10) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

int main(void) {
  int failed = 0;
  failed |= run_normal_tests();
  failed |= test_null_args();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All tests passed.\n");
  return 0;
}
