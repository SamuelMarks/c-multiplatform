/* clang-format off */
#include <stdio.h>
#include "../include/ui_handle_manager.h"
#include "../include/ui_error.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

#if defined(__linux__) || defined(__APPLE__)
#include "../include/ui_atomic.h"
#include <pthread.h>
#include <unistd.h>

struct ui_handle_entry {
  void *data;
  ui_uint32 next_free;
  ui_uint32 generation;
};

struct ui_handle_manager {
  struct ui_handle_entry *entries;
  ui_uint32 capacity;
  ui_uint32 first_free;
  ui_uint32 active_count;
  ui_atomic_t lock;
};

struct thread_data {
  struct ui_handle_manager *manager;
  ui_uint64 handle;
  int success;
};

static void *thread_func(void *arg) {
  struct thread_data *td = (struct thread_data *)arg;
  void *data;
  /* This will spin-wait until the main thread releases the lock */
  if (ui_handle_manager_get(td->manager, td->handle, &data) == UI_ERROR_NONE) {
    td->success = 1;
  }
  return NULL;
}

static int run_multithread_test(void) {
  struct ui_handle_manager *manager = NULL;
  ui_uint64 handle;
  int test_val = 42;
  pthread_t thread;
  struct thread_data td;

  if (ui_handle_manager_create(2, &manager) != UI_ERROR_NONE)
    return 1;
  if (ui_handle_manager_alloc(manager, &test_val, &handle) != UI_ERROR_NONE)
    return 1;

  /* Lock it manually */
  ui_atomic_store(&manager->lock, 1);

  td.manager = manager;
  td.handle = handle;
  td.success = 0;
  pthread_create(&thread, NULL, thread_func, &td);

  /* Give the thread time to spin */
  usleep(10000);

  /* Unlock it */
  ui_atomic_store(&manager->lock, 0);

  pthread_join(thread, NULL);

  ui_handle_manager_destroy(manager);
  return td.success == 1 ? 0 : 1;
}
#endif

static int run_normal_tests(void) {
  struct ui_handle_manager *manager = NULL;
  enum ui_error rc;
  ui_uint64 handle1 = UI_INVALID_HANDLE;
  ui_uint64 handle2 = UI_INVALID_HANDLE;
  ui_uint64 handle3 = UI_INVALID_HANDLE;
  ui_uint64 out_of_bounds_handle = UI_INVALID_HANDLE;
  void *data_out = NULL;
  int test_val1 = 42;
  int test_val2 = 100;

  printf("Running normal handle manager tests...\n");

  /* Test Invalid Arguments */
  if (ui_handle_manager_create(0, &manager) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_handle_manager_create(2, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_handle_manager_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_handle_manager_create(3, &manager);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create handle manager\n");
    return 1;
  }

  if (ui_handle_manager_alloc(NULL, &test_val1, &handle1) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_handle_manager_alloc(manager, &test_val1, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_handle_manager_get(NULL, handle1, &data_out) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_handle_manager_get(manager, UI_INVALID_HANDLE, &data_out) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_handle_manager_get(manager, handle1, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_handle_manager_free(NULL, handle1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_handle_manager_free(manager, UI_INVALID_HANDLE) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_handle_manager_alloc(manager, &test_val1, &handle1);
  if (rc != UI_ERROR_NONE || handle1 == UI_INVALID_HANDLE) {
    printf("Failed to alloc handle 1\n");
    return 1;
  }

  rc = ui_handle_manager_alloc(manager, &test_val2, &handle2);
  if (rc != UI_ERROR_NONE || handle2 == UI_INVALID_HANDLE) {
    printf("Failed to alloc handle 2\n");
    return 1;
  }

  rc = ui_handle_manager_alloc(manager, NULL, &handle3);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_handle_manager_get(manager, handle3, &data_out);
  ui_handle_manager_free(manager, handle3);

  /* Allocate again to fill queue */
  ui_handle_manager_alloc(manager, &test_val1, &handle3);

  /* Queue should be full */
  rc = ui_handle_manager_alloc(manager, &test_val1, &handle3);
  if (rc != UI_ERROR_QUEUE_FULL) {
    printf("Manager did not return QUEUE_FULL on capacity reached\n");
    return 1;
  }

  /* Valid get */
  rc = ui_handle_manager_get(manager, handle1, &data_out);
  if (rc != UI_ERROR_NONE || data_out != &test_val1) {
    printf("Failed to get handle 1 data\n");
    return 1;
  }

  /* Valid free */
  rc = ui_handle_manager_free(manager, handle1);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to free handle 1\n");
    return 1;
  }

  /* Invalid get after free (ABA protection) */
  rc = ui_handle_manager_get(manager, handle1, &data_out);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Manager returned success on freed handle\n");
    return 1;
  }

  /* Double free protection */
  if (ui_handle_manager_free(manager, handle1) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Manager returned success on double freed handle\n");
    return 1;
  }

  /* Reallocate the slot and verify generation changed */
  rc = ui_handle_manager_alloc(manager, &test_val1, &handle3);
  if (rc != UI_ERROR_NONE || handle3 == handle1) {
    printf("Manager failed to reallocate or ABA generation failed\n");
    return 1;
  }

  /* Invalid handle index */
  out_of_bounds_handle = (100ULL << 32) | 100;
  if (ui_handle_manager_get(manager, out_of_bounds_handle, &data_out) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_handle_manager_free(manager, out_of_bounds_handle) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Final cleanup */
  ui_handle_manager_destroy(manager);

  return 0;
}

static int run_oom_tests(void) {
  struct ui_handle_manager *manager = NULL;
  enum ui_error rc;

  printf("Running handle manager OOM tests...\n");

  g_malloc_fail_countdown = 0;
  rc = ui_handle_manager_create(3, &manager);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  g_malloc_fail_countdown = 1;
  rc = ui_handle_manager_create(3, &manager);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  g_malloc_fail_countdown = -1;
  return 0;
}

int main(void) {
  int failed = 0;
  failed |= run_normal_tests();
  failed |= run_oom_tests();
#if defined(__linux__) || defined(__APPLE__)
  failed |= run_multithread_test();
#endif

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All handle manager tests passed.\n");
  return 0;
}
