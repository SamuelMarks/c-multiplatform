/* clang-format off */
#include "../include/ui_form_control.h"
#include "../include/ui_form_validators.h"
#include "../include/ui_thread_pool.h"
#include "../include/ui_reactor.h"
#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define usleep(x) Sleep((x)/1000)
#endif
/* clang-format on */

extern int g_malloc_fail_countdown;

static enum ui_error dummy_async_valid(struct ui_form_control *control,
                                       union ui_signal_payload value,
                                       void *user_data,
                                       ui_bool_t *out_is_valid);
static enum ui_error dummy_async_err(struct ui_form_control *control,
                                     union ui_signal_payload value,
                                     void *user_data, ui_bool_t *out_is_valid);

static enum ui_error sync_validator(struct ui_form_control *control,
                                    union ui_signal_payload value,
                                    void *user_data, ui_bool_t *out_is_valid) {
  (void)control;
  (void)user_data;
  *out_is_valid = (value.int_val > 10) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

static enum ui_error async_validator(struct ui_form_control *control,
                                     union ui_signal_payload value,
                                     void *user_data, ui_bool_t *out_is_valid) {
  (void)control;
  (void)user_data;
  *out_is_valid = (value.int_val > 20) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

static enum ui_error dummy_async_valid(struct ui_form_control *control,
                                       union ui_signal_payload value,
                                       void *user_data,
                                       ui_bool_t *out_is_valid) {
  (void)control;
  (void)value;
  (void)user_data;
  *out_is_valid = 1;
  return UI_ERROR_NONE;
}

static enum ui_error dummy_async_err(struct ui_form_control *control,
                                     union ui_signal_payload value,
                                     void *user_data, ui_bool_t *out_is_valid) {
  (void)control;
  (void)value;
  (void)user_data;
  *out_is_valid = 1;
  return UI_ERROR_OUT_OF_MEMORY;
}

static int run_extra_control(void) {
  struct ui_arena *arena;
  ui_form_control_t *control;
  struct ui_thread_pool *pool;
  struct ui_reactor *reactor;
  union ui_signal_payload dummy = {0};
  ui_signal_t *sig;

  ui_arena_create(1024, &arena);
  ui_thread_pool_create(2, &pool);
  ui_reactor_create(&reactor);
  ui_form_control_create(arena, dummy, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                         UI_SIGNAL_MODE_SINGLE_THREADED, &control);

  ui_form_control_disable(control);
  ui_form_control_set_value(control, dummy);
  ui_form_control_enable(control);
  ui_form_control_patch_value(control, dummy);

  ui_form_control_set_error(control, "hello");
  ui_form_control_set_error(control, "world");
  ui_form_control_set_error(control, NULL);

  ui_form_control_get_value_signal(control, &sig);
  ui_form_control_get_touched_signal(control, &sig);
  ui_form_control_get_dirty_signal(control, &sig);
  ui_form_control_get_errors_signal(control, &sig);

  ui_form_control_add_async_validator(control, dummy_async_valid, NULL, pool,
                                      reactor);

  {
    int iter;
    for (iter = 0; iter < 50; ++iter) {
      ui_reactor_poll(reactor, 10);
      usleep(2000);
    }
  }

  ui_form_control_add_async_validator(control, dummy_async_err, NULL, pool,
                                      reactor);
  union ui_signal_payload diff_val = {0};
  diff_val.int_val = 99;
  ui_form_control_set_value(control, diff_val);
  {
    int iter;
    for (iter = 0; iter < 50; ++iter) {
      ui_reactor_poll(reactor, 10);
      usleep(2000);
    }
  }

  ui_form_control_add_async_validator(control, dummy_async_valid, NULL, pool,
                                      NULL);

  ui_form_control_create(NULL, dummy, 0, NULL, NULL, 0, NULL);
  ui_form_control_create(arena, dummy, 0, NULL, NULL, 0, NULL);
  ui_form_control_add_validator(NULL, NULL, NULL);
  ui_form_control_add_validator(control, NULL, NULL);
  ui_form_control_add_async_validator(NULL, NULL, NULL, NULL, NULL);
  ui_form_control_add_async_validator(control, NULL, NULL, pool, reactor);
  ui_form_control_add_async_validator(control, dummy_async_valid, NULL, NULL,
                                      reactor);
  ui_form_control_set_value(NULL, dummy);
  ui_form_control_mark_as_touched(NULL);
  ui_form_control_disable(NULL);
  ui_form_control_enable(NULL);
  ui_form_control_get_value_signal(NULL, NULL);
  ui_form_control_get_status_signal(NULL, NULL);
  ui_form_control_get_touched_signal(NULL, NULL);
  ui_form_control_get_dirty_signal(NULL, NULL);
  ui_form_control_get_errors_signal(NULL, NULL);
  ui_form_control_set_error(NULL, NULL);
  ui_form_control_destroy(NULL);

  ui_thread_pool_destroy(pool);
  ui_reactor_poll(reactor, 10);
  ui_form_control_destroy(control);
  ui_reactor_destroy(reactor);
  ui_arena_destroy(arena);

  return 0;
}

static int run_extra_control2_all(void) {
  struct ui_arena *arena;
  ui_form_control_t *control;
  ui_form_control_t *control2;
  ui_form_control_t *control3;
  union ui_signal_payload dummy = {0};

  ui_arena_create(1024, &arena);

  ui_form_control_create(arena, dummy, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                         UI_SIGNAL_MODE_SINGLE_THREADED, &control);
  ui_form_control_add_validator(control, sync_validator, NULL);
  ui_form_control_add_validator(control, sync_validator, NULL);
  ui_form_control_add_validator(control, sync_validator, NULL);
  ui_form_control_add_validator(control, sync_validator, NULL);
  ui_form_control_add_validator(control, sync_validator, NULL);

  ui_form_control_add_async_validator(control, dummy_async_valid, NULL,
                                      (void *)1, (void *)1);
  ui_form_control_add_async_validator(control, dummy_async_valid, NULL,
                                      (void *)1, (void *)1);
  ui_form_control_add_async_validator(control, dummy_async_valid, NULL,
                                      (void *)1, (void *)1);
  ui_form_control_add_async_validator(control, dummy_async_valid, NULL,
                                      (void *)1, (void *)1);
  ui_form_control_add_async_validator(control, dummy_async_valid, NULL,
                                      (void *)1, (void *)1);

  ui_form_control_create(arena, dummy, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                         UI_SIGNAL_MODE_SINGLE_THREADED, &control2);
  {
    struct ui_thread_pool *pool;
    ui_thread_pool_create(2, &pool);
    ui_form_control_add_async_validator(control2, dummy_async_valid, NULL, pool,
                                        NULL);
    ui_form_control_set_value(control2, dummy);

    ui_form_control_create(arena, dummy, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                           UI_SIGNAL_MODE_SINGLE_THREADED, &control3);
    ui_form_control_add_async_validator(control3, dummy_async_valid, NULL, pool,
                                        NULL);
    ui_form_control_set_value(control3, dummy);

    ui_thread_pool_destroy(pool);
    ui_form_control_destroy(control3);
  }

  ui_form_control_destroy(control);
  ui_form_control_destroy(control2);
  ui_arena_destroy(arena);

  return 0;
}

static int run_extra_control3_all(void) {
  struct ui_arena *arena;
  ui_form_control_t *control;
  union ui_signal_payload dummy = {0};

  ui_arena_create(1024, &arena);
  ui_form_control_create(arena, dummy, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                         UI_SIGNAL_MODE_SINGLE_THREADED, &control);

  {
    struct ui_thread_pool *pool;
    ui_thread_pool_create(2, &pool);

    ui_form_control_add_async_validator(control, dummy_async_err, NULL, pool,
                                        NULL);
    ui_form_control_set_value(control, dummy);

    usleep(50000);
    ui_thread_pool_destroy(pool);
  }

  ui_form_control_set_error(
      control,
      "this is a long error string that will be freed when we set it to null");
  ui_form_control_set_error(control, "another one");

  ui_form_control_destroy(control);
  ui_arena_destroy(arena);

  return 0;
}

static int run_oom_tests_control(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_arena *small_arena;
  ui_form_control_t *control;
  union ui_signal_payload dummy = {0};

  ui_arena_create(1, &small_arena);

  g_malloc_fail_countdown = 0;
  if (ui_form_control_create(small_arena, dummy, UI_SIGNAL_TYPE_INT32, NULL,
                             NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                             &control) != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed OOM test 1\n");
    return 1;
  }

  g_malloc_fail_countdown = 1;
  if (ui_form_control_create(small_arena, dummy, UI_SIGNAL_TYPE_INT32, NULL,
                             NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                             &control) != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed OOM test 2\n");
    return 1;
  }

  g_malloc_fail_countdown = 2;
  if (ui_form_control_create(small_arena, dummy, UI_SIGNAL_TYPE_INT32, NULL,
                             NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                             &control) != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed OOM test 3\n");
    return 1;
  }

  g_malloc_fail_countdown = 3;
  if (ui_form_control_create(small_arena, dummy, UI_SIGNAL_TYPE_INT32, NULL,
                             NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                             &control) != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed OOM test 4\n");
    return 1;
  }

  g_malloc_fail_countdown = 4;
  if (ui_form_control_create(small_arena, dummy, UI_SIGNAL_TYPE_INT32, NULL,
                             NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                             &control) != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed OOM test 5\n");
    return 1;
  }

  g_malloc_fail_countdown = 5;
  if (ui_form_control_create(small_arena, dummy, UI_SIGNAL_TYPE_INT32, NULL,
                             NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                             &control) != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed OOM test 6\n");
    return 1;
  }

  g_malloc_fail_countdown = -1;
  ui_form_control_create(small_arena, dummy, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                         UI_SIGNAL_MODE_SINGLE_THREADED, &control);

  g_malloc_fail_countdown = 0;
  if (ui_form_control_add_validator(control, sync_validator, NULL) !=
      UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed OOM test 7\n");
    return 1;
  }

  g_malloc_fail_countdown = 0;
  if (ui_form_control_add_async_validator(control, dummy_async_valid, NULL,
                                          (struct ui_thread_pool *)1,
                                          NULL) != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed OOM test 8\n");
    return 1;
  }

  g_malloc_fail_countdown = 0;
  if (ui_form_control_set_error(
          control,
          "this string is long enough to trigger malloc hopefully "
          "instead of using some inline buffer") != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed OOM test 9\n");
    return 1;
  }

  g_malloc_fail_countdown = 0;
  ui_form_control_set_value(
      control,
      dummy); /* Should trigger run_validation which fails to malloc task */

  {
    struct ui_thread_pool *pool;
    struct ui_reactor *reactor;
    g_malloc_fail_countdown = -1;
    ui_thread_pool_create(2, &pool);
    ui_reactor_create(&reactor);

    ui_form_control_t *control4;
    ui_form_control_create(small_arena, dummy, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                           UI_SIGNAL_MODE_SINGLE_THREADED, &control4);

    ui_form_control_add_async_validator(control4, dummy_async_valid, NULL, pool,
                                        reactor);

    g_malloc_fail_countdown = 0;
    ui_form_control_set_value(control4, dummy);

    g_malloc_fail_countdown = 1;
    ui_form_control_set_value(control4, dummy);

    g_malloc_fail_countdown = -1;
    ui_thread_pool_destroy(pool);
    ui_reactor_poll(reactor, 10);
    ui_form_control_destroy(control4);
    ui_reactor_destroy(reactor);
  }

  g_malloc_fail_countdown = -1;
  ui_form_control_destroy(control);
  ui_arena_destroy(small_arena);
#endif
  return 0;
}

int main(void) {
  setvbuf(stdout, NULL, _IONBF, 0);
  printf("Starting test_ui_form_control\n");
  struct ui_arena *arena;
  ui_form_control_t *control;
  struct ui_thread_pool *pool;
  struct ui_reactor *reactor;
  enum ui_error rc;
  union ui_signal_payload initial_value = {0}, get_val;
  ui_signal_t *sig;
  int failed = 0;

  rc = ui_arena_create(1024, &arena);
  if (rc != UI_ERROR_NONE) {
    printf("Failed init 1\n");
    return 1;
  }

  rc = ui_thread_pool_create(2, &pool);
  if (rc != UI_ERROR_NONE) {
    printf("Failed init 2\n");
    return 1;
  }

  rc = ui_reactor_create(&reactor);
  if (rc != UI_ERROR_NONE) {
    printf("Failed init 3\n");
    return 1;
  }

  initial_value.int_val = 15;
  rc = ui_form_control_create(arena, initial_value, UI_SIGNAL_TYPE_INT32, NULL,
                              NULL, UI_SIGNAL_MODE_SINGLE_THREADED, &control);
  if (rc != UI_ERROR_NONE) {
    printf("Failed form control create 2\n");
    return 1;
  }
  printf("Form control created\n");

  rc = ui_form_control_add_validator(control, sync_validator, NULL);
  if (rc != UI_ERROR_NONE) {
    printf("Failed sync val\n");
    return 1;
  }
  printf("Added sync val\n");

  rc = ui_form_control_get_status_signal(control, &sig);
  if (rc != UI_ERROR_NONE) {
    printf("Failed get sig\n");
    return 1;
  }
  rc = ui_signal_get(sig, &get_val);
  if (get_val.int_val != UI_FORM_STATUS_VALID) {
    printf("Failed valid get_val\n");
    return 1;
  }
  printf("Passed valid\n");

  rc = ui_form_control_add_async_validator(control, async_validator, NULL, pool,
                                           reactor);
  if (rc != UI_ERROR_NONE) {
    printf("Failed async val\n");
    return 1;
  }
  printf("Added async val\n");

  rc = ui_signal_get(sig, &get_val);
  if (get_val.int_val != UI_FORM_STATUS_PENDING) {
    printf("Failed pending get_val\n");
    return 1;
  }
  printf("Passed pending\n");

  {
    int iter;
    for (iter = 0; iter < 50; ++iter) {
      ui_reactor_poll(reactor, 100);
      usleep(2000);
      rc = ui_signal_get(sig, &get_val);
      if (get_val.int_val == UI_FORM_STATUS_INVALID) {
        break;
      }
    }
  }

  rc = ui_signal_get(sig, &get_val);
  if (get_val.int_val != UI_FORM_STATUS_INVALID) {
    printf("Failed invalid check %d\n", get_val.int_val);
  } else {
    printf("Passed invalid check\n");
  }

  {
    union ui_signal_payload new_val;
    new_val.int_val = 25;
    rc = ui_form_control_set_value(control, new_val);

    int iter;
    for (iter = 0; iter < 50; ++iter) {
      ui_reactor_poll(reactor, 100);
      usleep(2000);
      rc = ui_signal_get(sig, &get_val);
      if (get_val.int_val == UI_FORM_STATUS_VALID) {
        break;
      }
    }

    if (get_val.int_val != UI_FORM_STATUS_VALID) {
      printf("Failed valid check %d\n", get_val.int_val);
    } else {
      printf("Passed valid check\n");
    }
  }

  rc = ui_form_control_mark_as_touched(control);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_thread_pool_destroy(pool);
  ui_reactor_poll(reactor, 10);
  rc = ui_form_control_destroy(control);
  ui_reactor_destroy(reactor);
  ui_arena_destroy(arena);

  printf("Running extra controls\n");
  failed = 0;
  if (run_extra_control() != 0) {
    printf("Failed run_extra_control\n");
    failed = 1;
  }
  if (run_extra_control2_all() != 0) {
    printf("Failed run_extra_control2_all\n");
    failed = 1;
  }
  if (run_extra_control3_all() != 0) {
    printf("Failed run_extra_control3_all\n");
    failed = 1;
  }
  printf("Running run_oom_tests_control\n");
  if (run_oom_tests_control() != 0) {
    printf("Failed run_oom_tests_control\n");
    failed = 1;
  }

  if (failed)
    return 1;

  printf("test_ui_form_control passed\n");
  return 0;
}
