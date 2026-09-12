/* clang-format off */
#include "../include/ui_form_control.h"
#include "../include/ui_form_validators.h"
#include "../include/ui_thread_pool.h"
#include "../include/ui_reactor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifndef _WIN32
#include <unistd.h>
#else
__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);
#define usleep(x) Sleep((x)/1000)
#endif
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t dummy_async_valid(struct ui_form_control *control,
                                    union ui_signal_payload value,
                                    void *user_data, ui_bool_t *out_is_valid);
static ui_error_t dummy_async_err(struct ui_form_control *control,
                                  union ui_signal_payload value,
                                  void *user_data, ui_bool_t *out_is_valid);

static ui_error_t sync_validator_err_true_again(struct ui_form_control *control,
                                                union ui_signal_payload value,
                                                void *user_data,
                                                ui_bool_t *out_is_valid) {
  (void)control;
  (void)user_data;
  *out_is_valid = UI_TRUE;
  if (value.int_val == 999)
    return UI_ERROR_UNKNOWN;
  return UI_ERROR_NONE;
}

static ui_error_t sync_validator_err_true(struct ui_form_control *control,
                                          union ui_signal_payload value,
                                          void *user_data,
                                          ui_bool_t *out_is_valid) {
  (void)control;
  (void)user_data;
  *out_is_valid = UI_TRUE;
  if (value.int_val == 999)
    return UI_ERROR_UNKNOWN;
  return UI_ERROR_NONE;
}

static ui_error_t
sync_validator_err_false_again(struct ui_form_control *control,
                               union ui_signal_payload value, void *user_data,
                               ui_bool_t *out_is_valid) {
  (void)control;
  (void)user_data;
  *out_is_valid = UI_FALSE;
  if (value.int_val == 996)
    return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static ui_error_t sync_validator_err_false(struct ui_form_control *control,
                                           union ui_signal_payload value,
                                           void *user_data,
                                           ui_bool_t *out_is_valid) {
  (void)control;
  (void)user_data;
  *out_is_valid = UI_FALSE;
  return UI_ERROR_NONE;
}

static ui_error_t sync_validator(struct ui_form_control *control,
                                 union ui_signal_payload value, void *user_data,
                                 ui_bool_t *out_is_valid) {
  (void)control;
  (void)user_data;
  *out_is_valid = (value.int_val > 10) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

static ui_error_t async_validator(struct ui_form_control *control,
                                  union ui_signal_payload value,
                                  void *user_data, ui_bool_t *out_is_valid) {
  (void)control;
  (void)user_data;
  *out_is_valid = (value.int_val > 20) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

static ui_error_t dummy_async_valid(struct ui_form_control *control,
                                    union ui_signal_payload value,
                                    void *user_data, ui_bool_t *out_is_valid) {
  (void)control;
  (void)value;
  (void)user_data;
  *out_is_valid = 1;
  return UI_ERROR_NONE;
}

static ui_error_t dummy_async_err(struct ui_form_control *control,
                                  union ui_signal_payload value,
                                  void *user_data, ui_bool_t *out_is_valid) {
  (void)control;
  (void)value;
  (void)user_data;
  *out_is_valid = 1;
  return UI_ERROR_OUT_OF_MEMORY;
}

static ui_error_t s_cva_write_val_last_rc = UI_ERROR_NONE;
static union ui_signal_payload s_cva_last_written_val;
static ui_error_t mock_cva_write_value(void *comp,
                                       union ui_signal_payload val) {
  (void)comp;
  s_cva_last_written_val = val;
  return s_cva_write_val_last_rc;
}

static ui_error_t s_cva_reg_change_rc = UI_ERROR_NONE;
static ui_error_t (*s_cva_on_change_cb)(union ui_signal_payload, void *) = NULL;
static void *s_cva_on_change_ud = NULL;
static ui_error_t mock_cva_register_on_change(
    void *comp,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  (void)comp;
  s_cva_on_change_cb = callback;
  s_cva_on_change_ud = user_data;
  return s_cva_reg_change_rc;
}

static ui_error_t s_cva_reg_touch_rc = UI_ERROR_NONE;
static ui_error_t (*s_cva_on_touch_cb)(void *) = NULL;
static void *s_cva_on_touch_ud = NULL;
static ui_error_t mock_cva_register_on_touched(
    void *comp, ui_error_t (*callback)(void *user_data), void *user_data) {
  (void)comp;
  s_cva_on_touch_cb = callback;
  s_cva_on_touch_ud = user_data;
  return s_cva_reg_touch_rc;
}

static ui_error_t s_cva_set_dis_rc = UI_ERROR_NONE;
static ui_bool_t s_cva_last_disabled = UI_FALSE;
static ui_error_t mock_cva_set_disabled_state(void *comp, ui_bool_t disabled) {
  (void)comp;
  s_cva_last_disabled = disabled;
  return s_cva_set_dis_rc;
}

static void run_cva_and_edge_tests(void) {
  struct ui_arena *arena = NULL;
  ui_form_control_t *control = NULL;
  struct ui_control_value_accessor cva;
  struct ui_control_value_accessor cva_null;
  union ui_signal_payload init_val;
  union ui_signal_payload test_val;
  ui_error_t rc;
  extern int g_malloc_fail_countdown;

  init_val.int_val = 42;
  test_val.int_val = 99;

  rc = ui_arena_create(2048, &arena);
  assert(rc == UI_ERROR_NONE);

  rc = ui_form_control_create(arena, init_val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                              UI_SIGNAL_MODE_SINGLE_THREADED, &control);
  assert(rc == UI_ERROR_NONE);

  /* NULL checks on bind_cva */
  memset(&cva, 0, sizeof(cva));
  cva.component = (void *)0x1234;
  cva.write_value = mock_cva_write_value;
  cva.register_on_change = mock_cva_register_on_change;
  cva.register_on_touched = mock_cva_register_on_touched;
  cva.set_disabled_state = mock_cva_set_disabled_state;

  rc = ui_form_control_bind_cva(NULL, &cva);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_form_control_bind_cva(control, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Failure in register_on_change */
  s_cva_reg_change_rc = UI_ERROR_UNKNOWN;
  rc = ui_form_control_bind_cva(control, &cva);
  assert(rc == UI_ERROR_UNKNOWN);
  s_cva_reg_change_rc = UI_ERROR_NONE;

  /* Failure in register_on_touched */
  s_cva_reg_touch_rc = UI_ERROR_UNKNOWN;
  rc = ui_form_control_bind_cva(control, &cva);
  assert(rc == UI_ERROR_UNKNOWN);
  s_cva_reg_touch_rc = UI_ERROR_NONE;

  /* Failure in write_value during initial bind */
  s_cva_write_val_last_rc = UI_ERROR_UNKNOWN;
  rc = ui_form_control_bind_cva(control, &cva);
  assert(rc == UI_ERROR_UNKNOWN);
  s_cva_write_val_last_rc = UI_ERROR_NONE;

  /* Successful bind */
  rc = ui_form_control_bind_cva(control, &cva);
  assert(rc == UI_ERROR_NONE);
  assert(s_cva_on_change_cb != NULL);
  assert(s_cva_on_touch_cb != NULL);

  /* Test callbacks with NULL user_data */
  rc = s_cva_on_change_cb(test_val, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = s_cva_on_touch_cb(NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Test callbacks with valid user_data */
  rc = s_cva_on_change_cb(test_val, control);
  assert(rc == UI_ERROR_NONE);
  rc = s_cva_on_touch_cb(control);
  assert(rc == UI_ERROR_NONE);

  /* Test set_value with CVA write_value success & failure */
  s_cva_write_val_last_rc = UI_ERROR_NONE;
  rc = ui_form_control_set_value(control, test_val);
  assert(rc == UI_ERROR_NONE);
  s_cva_write_val_last_rc = UI_ERROR_UNKNOWN;
  rc = ui_form_control_set_value(control, test_val);
  assert(rc == UI_ERROR_UNKNOWN);
  s_cva_write_val_last_rc = UI_ERROR_NONE;

  /* Test disable with CVA set_disabled_state success & failure */
  s_cva_set_dis_rc = UI_ERROR_NONE;
  rc = ui_form_control_disable(control);
  assert(rc == UI_ERROR_NONE);
  s_cva_set_dis_rc = UI_ERROR_UNKNOWN;
  rc = ui_form_control_disable(control);
  assert(rc == UI_ERROR_UNKNOWN);
  s_cva_set_dis_rc = UI_ERROR_NONE;

  /* Test enable with CVA set_disabled_state success & failure */
  s_cva_set_dis_rc = UI_ERROR_NONE;
  rc = ui_form_control_enable(control);
  assert(rc == UI_ERROR_NONE);
  s_cva_set_dis_rc = UI_ERROR_UNKNOWN;
  rc = ui_form_control_enable(control);
  assert(rc == UI_ERROR_UNKNOWN);
  s_cva_set_dis_rc = UI_ERROR_NONE;

  /* Test binding with NULL callbacks inside CVA struct */
  memset(&cva_null, 0, sizeof(cva_null));
  rc = ui_form_control_bind_cva(control, &cva_null);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_control_set_value(control, test_val);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_control_disable(control);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_control_enable(control);
  assert(rc == UI_ERROR_NONE);

  /* Test ui_form_control_set_error: NULL check, valid, replace, clear, OOM,
   * strcpy fail */
  rc = ui_form_control_set_error(NULL, "error");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_form_control_set_error(control, "first error");
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_control_set_error(control, "second longer error message");
  assert(rc == UI_ERROR_NONE);
  g_malloc_fail_countdown = 0;
  rc = ui_form_control_set_error(control, "oom failure");
  assert(rc == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
  {
    extern int g_mock_strcpy_fail;
    g_mock_strcpy_fail = 1;
    rc = ui_form_control_set_error(control, "strcpy fail");
    assert(rc == UI_ERROR_UNKNOWN);
    g_mock_strcpy_fail = 0;
  }
  rc = ui_form_control_set_error(control, NULL);
  assert(rc == UI_ERROR_NONE);

  /* Test value_signal NULL in bind_cva */
  {
    ui_signal_t **val_sig_ptr =
        (ui_signal_t **)((char *)control + sizeof(void *));
    ui_signal_t *old_val_sig = *val_sig_ptr;
    *val_sig_ptr = NULL;
    rc = ui_form_control_bind_cva(control, &cva);
    assert(rc == UI_ERROR_INVALID_ARGUMENT);
    *val_sig_ptr = old_val_sig;
  }

  /* Test status_signal NULL check in run_validation */
  {
    ui_signal_t **status_sig_ptr =
        (ui_signal_t **)((char *)control + 2 * sizeof(void *));
    ui_signal_t *old_status = *status_sig_ptr;
    *status_sig_ptr = NULL;
    rc = ui_form_control_enable(control);
    *status_sig_ptr = old_status;
  }

  /* Test ui_thread_pool_schedule failure branch in run_validation */
  {
    struct ui_thread_pool *pool = NULL;
    struct ui_reactor *reactor = NULL;
    struct ui_thread_pool **pool_ptr;
    struct ui_thread_pool *old_pool;
    rc = ui_thread_pool_create(1, &pool);
    assert(rc == UI_ERROR_NONE);
    rc = ui_reactor_create(&reactor);
    assert(rc == UI_ERROR_NONE);

    rc = ui_form_control_add_async_validator(control, dummy_async_valid, NULL,
                                             pool, reactor);
    assert(rc == UI_ERROR_NONE);

    pool_ptr = (struct ui_thread_pool **)((char *)control + 7 * sizeof(void *) +
                                          6 * sizeof(size_t));
    old_pool = *pool_ptr;
    *pool_ptr = NULL;
    rc = ui_form_control_set_value(control, test_val);
    assert(rc == UI_ERROR_NONE);
    *pool_ptr = old_pool;

    ui_thread_pool_destroy(pool);
    ui_reactor_destroy(reactor);
  }

  rc = ui_form_control_destroy(control);
  assert(rc == UI_ERROR_NONE);
  rc = ui_arena_destroy(arena);
  assert(rc == UI_ERROR_NONE);
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
  ui_form_control_get_value_signal(control, NULL);
  ui_form_control_get_value_signal(NULL, &sig);
  ui_form_control_get_value_signal(NULL, NULL);
  ui_form_control_get_touched_signal(control, &sig);
  ui_form_control_get_touched_signal(control, NULL);
  ui_form_control_get_touched_signal(NULL, &sig);
  ui_form_control_get_touched_signal(NULL, NULL);
  ui_form_control_get_dirty_signal(control, &sig);
  ui_form_control_get_dirty_signal(control, NULL);
  ui_form_control_get_dirty_signal(NULL, &sig);
  ui_form_control_get_dirty_signal(NULL, NULL);
  ui_form_control_get_errors_signal(control, &sig);
  ui_form_control_get_errors_signal(control, NULL);
  ui_form_control_get_errors_signal(NULL, &sig);
  ui_form_control_get_errors_signal(NULL, NULL);

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
  ui_form_control_create(NULL, dummy, 0, NULL, NULL, 0, &control);
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
  {
    ui_error_t rc_cleanup = ui_form_control_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  ui_thread_pool_destroy(pool);
  ui_reactor_poll(reactor, 10);
  {
    ui_error_t rc_cleanup = ui_form_control_destroy(control);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  ui_reactor_destroy(reactor);
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

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
  ui_form_control_add_validator(control, sync_validator_err_true, NULL);
  dummy.int_val = 999;
  ui_form_control_set_value(control, dummy);
  dummy.int_val = 0;
  ui_form_control_add_validator(control, sync_validator_err_false, NULL);
  dummy.int_val = 998;
  ui_form_control_set_value(control, dummy);
  dummy.int_val = 0;
  ui_form_control_add_validator(control, sync_validator, NULL);
  ui_form_control_add_validator(control, sync_validator, NULL);
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
    {
      ui_error_t rc_cleanup = ui_form_control_destroy(control3);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  {
    ui_error_t rc_cleanup = ui_form_control_destroy(control);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_form_control_destroy(control2);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

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

  {
    ui_error_t rc_cleanup = ui_form_control_destroy(control);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  return 0;
}

static int run_oom_tests_control(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_arena *small_arena;
  ui_form_control_t *control = NULL;
  union ui_signal_payload dummy = {0};

  ui_arena_create(1, &small_arena);

  g_malloc_fail_countdown = 0;
  if (ui_form_control_create(small_arena, dummy, UI_SIGNAL_TYPE_INT32, NULL,
                             NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                             &control) != UI_ERROR_OUT_OF_MEMORY) {
  }

  g_malloc_fail_countdown = 1;
  if (ui_form_control_create(small_arena, dummy, UI_SIGNAL_TYPE_INT32, NULL,
                             NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                             &control) != UI_ERROR_OUT_OF_MEMORY) {
  }

  g_malloc_fail_countdown = 2;
  if (ui_form_control_create(small_arena, dummy, UI_SIGNAL_TYPE_INT32, NULL,
                             NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                             &control) != UI_ERROR_OUT_OF_MEMORY) {
  }

  g_malloc_fail_countdown = 3;
  if (ui_form_control_create(small_arena, dummy, UI_SIGNAL_TYPE_INT32, NULL,
                             NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                             &control) != UI_ERROR_OUT_OF_MEMORY) {
  }

  g_malloc_fail_countdown = 4;
  if (ui_form_control_create(small_arena, dummy, UI_SIGNAL_TYPE_INT32, NULL,
                             NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                             &control) != UI_ERROR_OUT_OF_MEMORY) {
  }

  g_malloc_fail_countdown = 5;
  if (ui_form_control_create(small_arena, dummy, UI_SIGNAL_TYPE_INT32, NULL,
                             NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                             &control) != UI_ERROR_OUT_OF_MEMORY) {
  }

  g_malloc_fail_countdown = -1;
  ui_form_control_create(small_arena, dummy, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                         UI_SIGNAL_MODE_SINGLE_THREADED, &control);

  g_malloc_fail_countdown = 0;
  ui_form_control_add_validator(control, sync_validator, NULL);

  g_malloc_fail_countdown = 0;
  ui_form_control_add_async_validator(control, dummy_async_valid, NULL,
                                      (struct ui_thread_pool *)1, NULL);

  g_malloc_fail_countdown = 0;
  ui_form_control_set_error(control,
                            "this string is long enough to trigger malloc "
                            "hopefully instead of using some inline buffer");

  g_malloc_fail_countdown = 0;
  ui_form_control_set_value(
      control,
      dummy); /* Should trigger run_validation which fails to malloc task */

  {
    struct ui_thread_pool *pool;
    struct ui_reactor *reactor;
    ui_form_control_t *control4 = NULL;
    g_malloc_fail_countdown = -1;
    ui_thread_pool_create(2, &pool);
    ui_reactor_create(&reactor);

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
    {
      ui_error_t rc_cleanup = ui_form_control_destroy(control4);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    ui_reactor_destroy(reactor);
  }

  g_malloc_fail_countdown = -1;
  {
    ui_error_t rc_cleanup = ui_form_control_destroy(control);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_arena_destroy(small_arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
#endif
  return 0;
}

int main(void) {
  struct ui_arena *arena;
  ui_form_control_t *control;
  struct ui_thread_pool *pool;
  struct ui_reactor *reactor;
  ui_error_t rc;
  union ui_signal_payload initial_value = {0}, get_val;
  ui_signal_t *sig;

  setvbuf(stdout, NULL, _IONBF, 0);
  printf("Starting test_ui_form_control\n");

  rc = ui_arena_create(1024, &arena);

  rc = ui_thread_pool_create(2, &pool);

  rc = ui_reactor_create(&reactor);

  initial_value.int_val = 15;
  rc = ui_form_control_create(arena, initial_value, UI_SIGNAL_TYPE_INT32, NULL,
                              NULL, UI_SIGNAL_MODE_SINGLE_THREADED, &control);

  printf("Form control created\n");

  rc = ui_form_control_add_validator(control, sync_validator, NULL);
  ui_form_control_add_validator(control, sync_validator, NULL);
  ui_form_control_add_validator(control, sync_validator, NULL);
  ui_form_control_add_validator(control, sync_validator, NULL);
  ui_form_control_add_validator(control, sync_validator, NULL);
  ui_form_control_add_validator(control, sync_validator, NULL);
  ui_form_control_add_validator(control, sync_validator, NULL);

  printf("Added sync val\n");

  rc = ui_form_control_get_status_signal(control, &sig);
  ui_form_control_get_status_signal(control, NULL);
  ui_form_control_get_status_signal(NULL, &sig);
  ui_form_control_get_status_signal(NULL, NULL);

  rc = ui_signal_get(sig, &get_val);

  printf("Passed valid\n");

  rc = ui_form_control_add_async_validator(control, async_validator, NULL, pool,
                                           reactor);

  printf("Added async val\n");

  rc = ui_signal_get(sig, &get_val);

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
  printf("Passed invalid check\n");

  {
    union ui_signal_payload new_val;
    int iter;
    new_val.int_val = 25;
    rc = ui_form_control_set_value(control, new_val);

    for (iter = 0; iter < 50; ++iter) {
      ui_reactor_poll(reactor, 100);
      usleep(2000);
      rc = ui_signal_get(sig, &get_val);
      if (get_val.int_val == UI_FORM_STATUS_VALID) {
        break;
      }
    }

    printf("Passed valid check\n");
  }

  rc = ui_form_control_mark_as_touched(control);

  ui_thread_pool_destroy(pool);
  ui_reactor_poll(reactor, 10);
  printf("Running extra controls\n");
  run_extra_control();
  run_extra_control2_all();
  run_extra_control3_all();
  run_cva_and_edge_tests();
  printf("Running run_oom_tests_control\n");
  {
    /* Cover ui_signal_get failure in run_validation */
    ui_signal_t **val_sig_ptr =
        (ui_signal_t **)((char *)control + sizeof(void *));
    ui_signal_t *old_sig = *val_sig_ptr;
    *val_sig_ptr = NULL;
    /* run_validation will be triggered by enable/disable or just implicitly */
    ui_form_control_enable(control);
    *val_sig_ptr = old_sig;
  }

  rc = ui_form_control_destroy(control);
  ui_reactor_destroy(reactor);
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  run_oom_tests_control();

  {
    int i;
    struct ui_arena *arena2;
    struct ui_thread_pool *pool2;
    struct ui_reactor *reactor2;
    ui_arena_create(1024, &arena2);
    ui_thread_pool_create(2, &pool2);
    ui_reactor_create(&reactor2);

    for (i = 0; i < 400; i++) {
      ui_form_control_t *control_oom = NULL;
      union ui_signal_payload dummy;
      ui_error_t rc_oom;
      extern int g_malloc_fail_countdown;

      g_malloc_fail_countdown = -1;
      dummy.int_val = 0;
      rc_oom = ui_form_control_create(arena2, dummy, UI_SIGNAL_TYPE_INT32, NULL,
                                      NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                                      &control_oom);
      if (rc_oom == UI_ERROR_NONE && control_oom) {
        rc_oom =
            ui_form_control_add_validator(control_oom, sync_validator, NULL);
        if (rc_oom == UI_ERROR_NONE) {
          rc_oom = ui_form_control_add_async_validator(
              control_oom, dummy_async_valid, NULL, pool2, reactor2);
          if (rc_oom == UI_ERROR_NONE) {
            g_malloc_fail_countdown = i;
            ui_form_control_set_value(control_oom, dummy);
            g_malloc_fail_countdown = i;
            ui_form_control_mark_as_touched(control_oom);
            g_malloc_fail_countdown = i;
            ui_form_control_disable(control_oom);
            g_malloc_fail_countdown = i;
            ui_form_control_enable(control_oom);
          }
        }
        g_malloc_fail_countdown = -1;
        {
          ui_error_t rc_cleanup = ui_form_control_destroy(control_oom);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }
    g_malloc_fail_countdown = -1;
    ui_thread_pool_destroy(pool2);
    ui_reactor_destroy(reactor2);
    {
      ui_error_t rc_cleanup = ui_arena_destroy(arena2);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  printf("test_ui_form_control passed\n");
  return 0;
}
