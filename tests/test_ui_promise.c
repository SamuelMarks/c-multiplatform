/* clang-format off */
#include "../include/ui_promise.h"
#include "../include/ui_execution_context.h"
#include "../include/ui_thread_pool.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
#ifdef UI_TEST_MOCK_ALLOC
extern int g_promise_mock_destroy_fail;
#endif

static ui_error_t test_resolve_cb(void *result, void *user_data,
                                  void **out_result) {
  int *val = (int *)user_data;
  if (val)
    *val = (int)(size_t)result;
  if (out_result)
    *out_result = result;
  return UI_ERROR_NONE;
}

static ui_error_t test_reject_cb(ui_error_t error, void *user_data,
                                 void **out_result) {
  ui_error_t *err = (ui_error_t *)user_data;
  if (err)
    *err = error;
  if (out_result)
    *out_result = NULL;
  return UI_ERROR_NONE;
}

static ui_error_t test_finally_cb(void *user_data) {
  int *val = (int *)user_data;
  if (val)
    *val += 1;
  return UI_ERROR_NONE;
}

static ui_error_t async_task_resolve(void *user_data) {
  struct ui_promise *promise = (struct ui_promise *)user_data;
  ui_promise_resolve(promise, (void *)100);
  return UI_ERROR_NONE;
}

static ui_error_t test_fail_resolve_cb(void *result, void *user_data,
                                       void **out_result) {
  if (result) {
  }
  if (user_data) {
  }
  if (out_result)
    *out_result = NULL;
  return UI_ERROR_UNKNOWN;
}

static ui_error_t test_fail_reject_cb(ui_error_t error, void *user_data,
                                      void **out_result) {
  if (user_data) {
  }
  if (error != UI_ERROR_NONE) {
    /* handled rejection */
  }
  if (out_result)
    *out_result = NULL;
  return UI_ERROR_UNKNOWN;
}

static int run_normal_tests(void) {
  struct ui_promise *promise = NULL;
  struct ui_promise *chained = NULL;
  struct ui_promise *chained2 = NULL;
  ui_error_t rc;
  enum ui_promise_state state;
  int test_val = 0;
  ui_error_t test_err = UI_ERROR_NONE;
  int finally_called = 0;
  int i;

  printf("Running normal promise tests...\n");

  /* Test resolution */
  rc = ui_promise_create(&promise);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_promise_get_state(promise, &state);
  if (state != UI_PROMISE_PENDING)
    return 1;

  rc = ui_promise_then(promise, test_resolve_cb, test_reject_cb, &test_val,
                       NULL);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_promise_resolve(promise, (void *)42);
  if (rc != UI_ERROR_NONE)
    return 1;

  if (test_val != 42)
    return 1;

  ui_promise_get_state(promise, &state);
  if (state != UI_PROMISE_FULFILLED)
    return 1;

  /* Test then on already resolved promise */
  test_val = 0;
  ui_promise_then(promise, test_resolve_cb, test_reject_cb, &test_val, NULL);
  if (test_val != 42)
    return 1;

  /* Test ignoring second resolution */
  ui_promise_resolve(promise, (void *)99);
  test_val = 0;
  ui_promise_then(promise, test_resolve_cb, test_reject_cb, &test_val, NULL);
  if (test_val != 42)
    return 1;

  {
    ui_error_t rc_cleanup = ui_promise_destroy(promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  /* Test rejection */
  rc = ui_promise_create(&promise);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_promise_then(promise, test_resolve_cb, test_reject_cb, &test_err,
                       NULL);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_promise_reject(promise, UI_ERROR_INVALID_ARGUMENT);
  if (rc != UI_ERROR_NONE)
    return 1;

  if (test_err != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_promise_get_state(promise, &state);
  if (state != UI_PROMISE_REJECTED)
    return 1;

  /* Test then on already rejected promise */
  test_err = UI_ERROR_NONE;
  ui_promise_then(promise, test_resolve_cb, test_reject_cb, &test_err, NULL);
  if (test_err != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  {
    ui_error_t rc_cleanup = ui_promise_destroy(promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  /* Test catch and finally */
  {
    ui_error_t rc_cleanup = ui_promise_create(&promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  finally_called = 0;
  ui_promise_catch(promise, test_reject_cb, &test_err, &chained);
  ui_promise_finally(chained, test_finally_cb, &finally_called, NULL);

  ui_promise_reject(promise, UI_ERROR_OUT_OF_MEMORY);
  if (test_err != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  if (finally_called != 1)
    return 1;
  {
    ui_error_t rc_cleanup = ui_promise_destroy(promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  /* Test memory bounds: 500 chained promises */
  {
    ui_error_t rc_cleanup = ui_promise_create(&promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  chained = promise;
  for (i = 0; i < 500; i++) {
    ui_promise_then(chained, test_resolve_cb, NULL, NULL, &chained2);
    chained = chained2;
  }
  ui_promise_finally(chained, test_finally_cb, &finally_called, NULL);
  finally_called = 0;
  ui_promise_resolve(promise, (void *)123);
  if (finally_called != 1)
    return 1;
  {
    ui_error_t rc_cleanup = ui_promise_destroy(promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  /* Test invalid state ignores */
  {
    ui_error_t rc_cleanup = ui_promise_create(&promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  ui_promise_reject(promise, UI_ERROR_INVALID_ARGUMENT);
  /* rejecting an already rejected promise should return UI_ERROR_NONE and do
   * nothing */
  if (ui_promise_reject(promise, UI_ERROR_OUT_OF_MEMORY) != UI_ERROR_NONE)
    return 1;
  {
    ui_error_t rc_cleanup = ui_promise_destroy(promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  /* Test then bubbling (no resolve handler, but chained) */
  {
    ui_error_t rc_cleanup = ui_promise_create(&promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  ui_promise_then(promise, NULL, NULL, NULL, &chained);
  ui_promise_then(chained, test_resolve_cb, NULL, &test_val, NULL);
  /* Test empty handler with no chained promise */
  ui_promise_then(promise, NULL, NULL, NULL, NULL);
  test_val = 0;
  ui_promise_resolve(promise, (void *)1234);
  if (test_val != 1234)
    return 1;
  {
    ui_error_t rc_cleanup = ui_promise_destroy(promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  /* Test catch bubbling (no reject handler, but chained) */
  {
    ui_error_t rc_cleanup = ui_promise_create(&promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  ui_promise_then(promise, NULL, NULL, NULL, &chained);
  ui_promise_catch(chained, test_reject_cb, &test_err, NULL);
  /* Test empty catch with no chained promise */
  ui_promise_then(promise, NULL, NULL, NULL, NULL);
  test_err = UI_ERROR_NONE;
  ui_promise_reject(promise, UI_ERROR_OUT_OF_BOUNDS);
  if (test_err != UI_ERROR_OUT_OF_BOUNDS)
    return 1;
  {
    ui_error_t rc_cleanup = ui_promise_destroy(promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  /* Test handler failure chaining (resolve) */
  {
    ui_error_t rc_cleanup = ui_promise_create(&promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  ui_promise_then(promise, test_fail_resolve_cb, NULL, NULL, &chained);
  ui_promise_catch(chained, test_reject_cb, &test_err, NULL);
  test_err = UI_ERROR_NONE;
  ui_promise_resolve(promise, (void *)1);
  if (test_err != UI_ERROR_UNKNOWN)
    return 1;
  {
    ui_error_t rc_cleanup = ui_promise_destroy(promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  /* Test handler failure chaining (reject) */
  {
    ui_error_t rc_cleanup = ui_promise_create(&promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  ui_promise_catch(promise, test_fail_reject_cb, NULL, &chained);
  ui_promise_catch(chained, test_reject_cb, &test_err, NULL);
  test_err = UI_ERROR_NONE;
  ui_promise_reject(promise, UI_ERROR_OUT_OF_MEMORY);
  if (test_err != UI_ERROR_UNKNOWN)
    return 1;
  {
    ui_error_t rc_cleanup = ui_promise_destroy(promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  /* Test finally bubbling */
  {
    ui_error_t rc_cleanup = ui_promise_create(&promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  ui_promise_finally(promise, test_finally_cb, &finally_called, &chained);
  ui_promise_then(chained, test_resolve_cb, NULL, &test_val, NULL);
  test_val = 0;
  finally_called = 0;
  ui_promise_resolve(promise, (void *)5678);
  if (finally_called != 1)
    return 1;
  if (test_val != 5678)
    return 1;
  {
    ui_error_t rc_cleanup = ui_promise_destroy(promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  {
    ui_error_t rc_cleanup = ui_promise_create(&promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  ui_promise_finally(promise, test_finally_cb, &finally_called, &chained);
  ui_promise_catch(chained, test_reject_cb, &test_err, NULL);
  test_err = UI_ERROR_NONE;
  finally_called = 0;
  ui_promise_reject(promise, UI_ERROR_NOT_FOUND);
  if (finally_called != 1)
    return 1;
  if (test_err != UI_ERROR_NOT_FOUND)
    return 1;
  {
    ui_error_t rc_cleanup = ui_promise_destroy(promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  if (ui_promise_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_promise_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_promise_then(NULL, NULL, NULL, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_promise_resolve(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_promise_reject(NULL, UI_ERROR_NONE) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_promise_get_state(NULL, &state) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  {
    ui_error_t rc_cleanup = ui_promise_create(&promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  if (ui_promise_get_state(promise, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    ui_error_t rc_cleanup = ui_promise_destroy(promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  return 0;
}

static int run_async_tests(void) {
  struct ui_execution_context *ctx = NULL;
  struct ui_promise *promise = NULL;
  int test_val = 0;

  printf("Running async promise tests...\n");

  /* Test resolution asynchronously (next tick) in single-threaded context */
  ui_execution_context_create(&ctx);
  {
    ui_error_t rc_cleanup = ui_promise_create(&promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  ui_promise_then(promise, test_resolve_cb, NULL, &test_val, NULL);

  ui_execution_context_schedule(ctx, async_task_resolve, promise);

  if (test_val == 100)
    return 1; /* should not be resolved yet */

  ui_execution_context_tick(ctx); /* process scheduled tasks */

  if (test_val != 100)
    return 1; /* should be resolved now */

  {
    ui_error_t rc_cleanup = ui_promise_destroy(promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_execution_context_destroy(ctx);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  return 0;
}

static int run_oom_tests(void) {
  struct ui_promise *promise = NULL;
  struct ui_promise *chained = NULL;
  ui_error_t rc;

  printf("Running promise OOM tests...\n");

  g_malloc_fail_countdown = 0;
  rc = ui_promise_create(&promise);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  g_malloc_fail_countdown = -1;
  rc = ui_promise_create(&promise);
  if (rc != UI_ERROR_NONE)
    return 1;
  {
    ui_error_t rc_cleanup = ui_promise_destroy(promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  promise = NULL;

  /* Test chained promise create fail */
  {
    ui_error_t rc_cleanup = ui_promise_create(&promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  g_malloc_fail_countdown = 0;
  rc =
      ui_promise_then(promise, test_resolve_cb, test_reject_cb, NULL, &chained);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  /* Test node alloc fail when chained is requested */
  g_malloc_fail_countdown = 1;
  rc =
      ui_promise_then(promise, test_resolve_cb, test_reject_cb, NULL, &chained);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  /* Test node alloc fail when chained is NOT requested */
  g_malloc_fail_countdown = 0;
  rc = ui_promise_then(promise, test_resolve_cb, test_reject_cb, NULL, NULL);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  {
    ui_error_t rc_cleanup = ui_promise_destroy(promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  return 0;
}

static ui_error_t test_fail_finally_cb(void *user_data) {
  if (user_data) {
  }
  return UI_ERROR_UNKNOWN;
}

static int test_bubbling_failure(void) {
  struct ui_promise *p1 = NULL, *p2 = NULL;
  ui_error_t rc;

  /* Test finally returning error (resolve) */
  ui_promise_create(&p1);
  ui_promise_finally(p1, test_fail_finally_cb, NULL, &p2);
  rc = ui_promise_resolve(p1, NULL);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("fail1 %d\n", rc);
    return 1;
  }
  ui_promise_destroy(p1);

  /* Test finally returning error (reject) */
  ui_promise_create(&p1);
  ui_promise_finally(p1, test_fail_finally_cb, NULL, &p2);
  rc = ui_promise_reject(p1, UI_ERROR_NOT_FOUND);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("fail4 %d\n", rc);
    return 1;
  }
  ui_promise_destroy(p1);

  /* Test then on already resolved, callback returns error */
  ui_promise_create(&p1);
  ui_promise_resolve(p1, NULL);
  rc = ui_promise_then(p1, test_fail_resolve_cb, NULL, NULL, NULL);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("fail2 %d\n", rc);
    return 1;
  }
  ui_promise_destroy(p1);

  /* Test catch on already rejected, callback returns error */
  ui_promise_create(&p1);
  ui_promise_reject(p1, UI_ERROR_NOT_FOUND);
  rc = ui_promise_catch(p1, test_fail_reject_cb, NULL, NULL);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("fail3 %d\n", rc);
    return 1;
  }
  ui_promise_destroy(p1);

  return 0;
}

static int test_more_bubbling_and_mock(void) {
  struct ui_promise *p1 = NULL, *p2 = NULL, *p3 = NULL;
  ui_error_t rc;

  /* Case 1: on_finally with fulfilled, chained resolve fails */
  ui_promise_create(&p1);
  ui_promise_finally(p1, test_finally_cb, NULL, &p2);
  ui_promise_then(p2, test_fail_resolve_cb, NULL, NULL, NULL);
  rc = ui_promise_resolve(p1, (void *)1);
  if (rc != UI_ERROR_UNKNOWN)
    return 1;
  ui_promise_destroy(p1);

  /* Case 2: on_finally with rejected, chained reject fails */
  ui_promise_create(&p1);
  ui_promise_finally(p1, test_finally_cb, NULL, &p2);
  ui_promise_catch(p2, test_fail_reject_cb, NULL, NULL);
  rc = ui_promise_reject(p1, UI_ERROR_NOT_FOUND);
  if (rc != UI_ERROR_UNKNOWN)
    return 1;
  ui_promise_destroy(p1);

  /* Case 3: fulfilled, on_resolve succeeds, chained resolve fails */
  ui_promise_create(&p1);
  ui_promise_then(p1, test_resolve_cb, NULL, NULL, &p2);
  ui_promise_then(p2, test_fail_resolve_cb, NULL, NULL, NULL);
  rc = ui_promise_resolve(p1, (void *)1);
  if (rc != UI_ERROR_UNKNOWN)
    return 1;
  ui_promise_destroy(p1);

  /* Case 4: fulfilled, on_resolve fails, chained reject fails */
  ui_promise_create(&p1);
  ui_promise_then(p1, test_fail_resolve_cb, NULL, NULL, &p2);
  ui_promise_catch(p2, test_fail_reject_cb, NULL, NULL);
  rc = ui_promise_resolve(p1, (void *)1);
  if (rc != UI_ERROR_UNKNOWN)
    return 1;
  ui_promise_destroy(p1);

  /* Case 5: fulfilled, on_resolve is NULL, bubble up resolve fails */
  ui_promise_create(&p1);
  ui_promise_catch(p1, test_reject_cb, NULL, &p2);
  ui_promise_then(p2, test_fail_resolve_cb, NULL, NULL, NULL);
  rc = ui_promise_resolve(p1, (void *)1);
  if (rc != UI_ERROR_UNKNOWN)
    return 1;
  ui_promise_destroy(p1);

  /* Case 6: rejected, on_reject succeeds, chained resolve fails */
  ui_promise_create(&p1);
  ui_promise_catch(p1, test_reject_cb, NULL, &p2);
  ui_promise_then(p2, test_fail_resolve_cb, NULL, NULL, NULL);
  rc = ui_promise_reject(p1, UI_ERROR_NOT_FOUND);
  if (rc != UI_ERROR_UNKNOWN)
    return 1;
  ui_promise_destroy(p1);

  /* Case 7: rejected, on_reject fails, chained reject fails */
  ui_promise_create(&p1);
  ui_promise_catch(p1, test_fail_reject_cb, NULL, &p2);
  ui_promise_catch(p2, test_fail_reject_cb, NULL, NULL);
  rc = ui_promise_reject(p1, UI_ERROR_NOT_FOUND);
  if (rc != UI_ERROR_UNKNOWN)
    return 1;
  ui_promise_destroy(p1);

  /* Case 8: rejected, on_reject is NULL, bubble up reject fails */
  ui_promise_create(&p1);
  ui_promise_then(p1, test_resolve_cb, NULL, NULL, &p2);
  ui_promise_catch(p2, test_fail_reject_cb, NULL, NULL);
  rc = ui_promise_reject(p1, UI_ERROR_NOT_FOUND);
  if (rc != UI_ERROR_UNKNOWN)
    return 1;
  ui_promise_destroy(p1);

#ifdef UI_TEST_MOCK_ALLOC
  /* Case 9: add_callback OOM on node allocation, chained destroy fails */
  ui_promise_create(&p1);
  g_malloc_fail_countdown = 1;
  g_promise_mock_destroy_fail = 1;
  rc = ui_promise_then(p1, test_resolve_cb, NULL, NULL, &p2);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_UNKNOWN)
    return 1;
  ui_promise_destroy(p1);

  /* Case 10: ui_promise_destroy with multiple chained promises that fail */
  ui_promise_create(&p1);
  ui_promise_then(p1, test_resolve_cb, NULL, NULL, &p2);
  ui_promise_then(p1, test_resolve_cb, NULL, NULL, &p3);
  g_promise_mock_destroy_fail = 2;
  rc = ui_promise_destroy(p1);
  if (rc != UI_ERROR_UNKNOWN)
    return 1;

  /* Case 11: node allocation OOM with out_promise == NULL */
  ui_promise_create(&p1);
  g_malloc_fail_countdown = 0;
  rc = ui_promise_then(p1, test_resolve_cb, NULL, NULL, NULL);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  ui_promise_destroy(p1);
#endif

  return 0;
}

int main(void) {
  int failed = 0;
  failed |= run_normal_tests();
  failed |= run_async_tests();
  failed |= run_oom_tests();
  failed |= test_bubbling_failure();
  failed |= test_more_bubbling_and_mock();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All tests passed.\n");
  return 0;
}
