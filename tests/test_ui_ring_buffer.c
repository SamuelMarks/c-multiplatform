/* clang-format off */
#include "../include/ui_ring_buffer.h"
#include "../include/ui_thread_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t thread_task_push_mp(void *user_data) {
  struct ui_ring_buffer *rb = (struct ui_ring_buffer *)user_data;
  int item = 1;
  int i;
  for (i = 0; i < 5000; i++) {
    while (ui_ring_buffer_push_mp(rb, &item) == UI_ERROR_QUEUE_FULL) {
      /* wait */
    }
  }
  return UI_ERROR_NONE;
}

static ui_error_t thread_task_push_spsc(void *user_data) {
  struct ui_ring_buffer *rb = (struct ui_ring_buffer *)user_data;
  int i;
  for (i = 0; i < 20; i++) {
    while (ui_ring_buffer_push(rb, &i) == UI_ERROR_QUEUE_FULL) {
      /* spin wait */
    }
  }
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  struct ui_ring_buffer *rb = NULL;
  ui_error_t rc;
  int test_val;
  int i;

  printf("Running normal ring buffer tests...\n");

  rc = ui_ring_buffer_create(sizeof(int), 3, &rb);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Pop empty */
  rc = ui_ring_buffer_pop(rb, &test_val);
  if (rc != UI_ERROR_QUEUE_EMPTY)
    return 1;

  /* Push up to capacity */
  for (i = 1; i <= 3; i++) {
    rc = ui_ring_buffer_push(rb, &i);
    if (rc != UI_ERROR_NONE)
      return 1;
  }

  /* Push full */
  test_val = 4;
  rc = ui_ring_buffer_push(rb, &test_val);
  if (rc != UI_ERROR_QUEUE_FULL)
    return 1;

  rc = ui_ring_buffer_push_mp(rb, &test_val);
  if (rc != UI_ERROR_QUEUE_FULL)
    return 1;

  /* Pop one */
  rc = ui_ring_buffer_pop(rb, &test_val);
  if (rc != UI_ERROR_NONE || test_val != 1)
    return 1;

  /* Push one */
  test_val = 5;
  rc = ui_ring_buffer_push(rb, &test_val);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Pop remaining */
  rc = ui_ring_buffer_pop(rb, &test_val);
  if (rc != UI_ERROR_NONE || test_val != 2)
    return 1;

  rc = ui_ring_buffer_pop(rb, &test_val);
  if (rc != UI_ERROR_NONE || test_val != 3)
    return 1;

  rc = ui_ring_buffer_pop(rb, &test_val);
  if (rc != UI_ERROR_NONE || test_val != 5)
    return 1;

  /* Empty again */
  rc = ui_ring_buffer_pop(rb, &test_val);
  if (rc != UI_ERROR_QUEUE_EMPTY)
    return 1;

  (void)ui_ring_buffer_destroy(rb);
  rb = NULL;

  /* Test wrap-around math at exact byte boundaries */
  {
    /* 10 bytes capacity */
    char bytes[10] = "0123456789";
    char out_byte;
    rc = ui_ring_buffer_create(1, 10, &rb);
    if (rc != UI_ERROR_NONE)
      return 1;
    /* push 5 */
    for (i = 0; i < 5; i++)
      (void)ui_ring_buffer_push(rb, &bytes[i]);
    /* pop 5 */
    for (i = 0; i < 5; i++)
      (void)ui_ring_buffer_pop(rb, &out_byte);
    /* buffer head is now at 5. push 7 to cause wrap around */
    for (i = 0; i < 7; i++)
      (void)ui_ring_buffer_push(rb, &bytes[i]);
    /* pop 7 */
    for (i = 0; i < 7; i++) {
      (void)ui_ring_buffer_pop(rb, &out_byte);
      if (out_byte != bytes[i])
        return 1;
    }
    (void)ui_ring_buffer_destroy(rb);
    rb = NULL;
  }

  /* Test single-producer, single-consumer thread safety */
  {
    struct ui_thread_pool *pool = NULL;
    rc = ui_ring_buffer_create(sizeof(int), 10, &rb);
    if (rc == UI_ERROR_NONE &&
        ui_thread_pool_create(1, &pool) == UI_ERROR_NONE) {
      (void)ui_thread_pool_schedule(pool, thread_task_push_spsc, rb);
      for (i = 0; i < 20; i++) {
        while (ui_ring_buffer_pop(rb, &test_val) == UI_ERROR_QUEUE_EMPTY) {
          /* wait */
        }
        if (test_val != i)
          return 1;
      }
      (void)ui_thread_pool_destroy(pool);
    }
    if (rb) {
      (void)ui_ring_buffer_destroy(rb);
      rb = NULL;
    }
  }

  /* Test multi-producer, single-consumer spinlock safety */
  {
    struct ui_thread_pool *pool = NULL;
    int pop_count = 0;
    rc = ui_ring_buffer_create(sizeof(int), 20, &rb);
    if (rc == UI_ERROR_NONE &&
        ui_thread_pool_create(10, &pool) == UI_ERROR_NONE) {
      for (i = 0; i < 10; i++) {
        (void)ui_thread_pool_schedule(pool, thread_task_push_mp, rb);
      }
      while (pop_count < 50000) {
        if (ui_ring_buffer_pop(rb, &test_val) == UI_ERROR_NONE) {
          pop_count++;
        }
      }
      (void)ui_thread_pool_destroy(pool);
    }
    if (rb) {
      (void)ui_ring_buffer_destroy(rb);
      rb = NULL;
    }
  }

  /* Invalid args */
  if (ui_ring_buffer_create(0, 10, &rb) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ring_buffer_create(10, 0, &rb) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ring_buffer_create(10, 10, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  (void)ui_ring_buffer_create(sizeof(int), 3, &rb);
  if (ui_ring_buffer_push(NULL, &test_val) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ring_buffer_push(rb, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ring_buffer_push_mp(NULL, &test_val) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ring_buffer_push_mp(rb, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ring_buffer_pop(NULL, &test_val) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ring_buffer_pop(rb, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ring_buffer_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  (void)ui_ring_buffer_destroy(rb);

  return 0;
}

static int run_oom_tests(void) {
  struct ui_ring_buffer *rb = NULL;
  ui_error_t rc;

  printf("Running ring buffer OOM tests...\n");

  g_malloc_fail_countdown = 0;
  rc = ui_ring_buffer_create(sizeof(int), 10, &rb);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  g_malloc_fail_countdown = 1;
  rc = ui_ring_buffer_create(sizeof(int), 10, &rb);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  g_malloc_fail_countdown = -1;
  return 0;
}

int main(void) {
  int failed = 0;
  failed |= run_normal_tests();
  failed |= run_oom_tests();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All tests passed.\n");
  return 0;
}
