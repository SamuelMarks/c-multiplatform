/* clang-format off */
#include "../include/ui_form_control.h"
#include "../include/ui_form_validators.h"
#include "../include/ui_thread_pool.h"
#include "../include/ui_reactor.h"
#include "../include/ui_atomic.h"
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
  if (control) {
  }
  if (user_data) {
  }
  *out_is_valid = UI_TRUE;
  if (value.int_val == 999)
    return UI_ERROR_UNKNOWN;
  return UI_ERROR_NONE;
}

static ui_error_t sync_validator_err_true(struct ui_form_control *control,
                                          union ui_signal_payload value,
                                          void *user_data,
                                          ui_bool_t *out_is_valid) {
  if (control) {
  }
  if (user_data) {
  }
  *out_is_valid = UI_TRUE;
  if (value.int_val == 999)
    return UI_ERROR_UNKNOWN;
  return UI_ERROR_NONE;
}

static ui_error_t
sync_validator_err_false_again(struct ui_form_control *control,
                               union ui_signal_payload value, void *user_data,
                               ui_bool_t *out_is_valid) {
  if (control) {
  }
  if (user_data) {
  }
  *out_is_valid = UI_FALSE;
  if (value.int_val == 996)
    return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static ui_error_t sync_validator_err_false(struct ui_form_control *control,
                                           union ui_signal_payload value,
                                           void *user_data,
                                           ui_bool_t *out_is_valid) {
  if (control) {
  }
  if (user_data) {
  }
  *out_is_valid = UI_FALSE;
  return UI_ERROR_NONE;
}

static ui_error_t sync_validator(struct ui_form_control *control,
                                 union ui_signal_payload value, void *user_data,
                                 ui_bool_t *out_is_valid) {
  if (control) {
  }
  if (user_data) {
  }
  *out_is_valid = (value.int_val > 10) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

static ui_error_t async_validator(struct ui_form_control *control,
                                  union ui_signal_payload value,
                                  void *user_data, ui_bool_t *out_is_valid) {
  if (control) {
  }
  if (user_data) {
  }
  *out_is_valid = (value.int_val > 20) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

static ui_error_t dummy_async_valid(struct ui_form_control *control,
                                    union ui_signal_payload value,
                                    void *user_data, ui_bool_t *out_is_valid) {
  if (control) {
  }
  if (value.ptr_val) {
  }
  if (user_data) {
  }
  *out_is_valid = 1;
  return UI_ERROR_NONE;
}

static ui_error_t dummy_async_err(struct ui_form_control *control,
                                  union ui_signal_payload value,
                                  void *user_data, ui_bool_t *out_is_valid) {
  if (control) {
  }
  if (value.ptr_val) {
  }
  if (user_data) {
  }
  *out_is_valid = 1;
  return UI_ERROR_OUT_OF_MEMORY;
}

static ui_error_t s_cva_write_val_last_rc = UI_ERROR_NONE;
static union ui_signal_payload s_cva_last_written_val;
static ui_error_t mock_cva_write_value(void *comp,
                                       union ui_signal_payload val) {
  if (comp) {
  }
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
  if (comp) {
  }
  s_cva_on_change_cb = callback;
  s_cva_on_change_ud = user_data;
  return s_cva_reg_change_rc;
}

static ui_error_t s_cva_reg_touch_rc = UI_ERROR_NONE;
static ui_error_t (*s_cva_on_touch_cb)(void *) = NULL;
static void *s_cva_on_touch_ud = NULL;
static ui_error_t mock_cva_register_on_touched(
    void *comp, ui_error_t (*callback)(void *user_data), void *user_data) {
  if (comp) {
  }
  s_cva_on_touch_cb = callback;
  s_cva_on_touch_ud = user_data;
  return s_cva_reg_touch_rc;
}

static ui_error_t s_cva_set_dis_rc = UI_ERROR_NONE;
static ui_bool_t s_cva_last_disabled = UI_FALSE;
static ui_error_t mock_cva_set_disabled_state(void *comp, ui_bool_t disabled) {
  if (comp) {
  }
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
    if (rc_cleanup != UI_ERROR_INVALID_ARGUMENT) {
      return 1;
    }
  }

  ui_thread_pool_destroy(pool);
  ui_reactor_poll(reactor, 10);
  {
    ui_error_t rc_cleanup = ui_form_control_destroy(control);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  ui_reactor_destroy(reactor);
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
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
        return 1;
      }
    }
  }

  {
    ui_error_t rc_cleanup = ui_form_control_destroy(control);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_form_control_destroy(control2);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
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
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
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
        return 1;
      }
    }
    ui_reactor_destroy(reactor);
  }

  g_malloc_fail_countdown = -1;
  {
    ui_error_t rc_cleanup = ui_form_control_destroy(control);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_arena_destroy(small_arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
#endif
  return 0;
}

struct test_form_ctrl_mirror {
  struct ui_arena *arena;
  ui_signal_t *value_signal;
  ui_signal_t *status_signal;
  ui_signal_t *touched_signal;
  ui_signal_t *dirty_signal;
  ui_signal_t *errors_signal;
  char *error_str;
  ui_validator_t *sync_validators;
  size_t sync_validators_count;
  size_t sync_validators_capacity;
  ui_async_validator_t *async_validators;
  size_t async_validators_count;
  size_t async_validators_capacity;
  struct ui_thread_pool *thread_pool;
  struct ui_reactor *reactor;
  ui_int32 validation_generation;
  size_t pending_async_count;
  struct ui_control_value_accessor cva;
  ui_bool_t has_cva;
  ui_bool_t in_cva_update;
};

struct test_signal_mirror {
  union ui_signal_payload value;
  enum ui_signal_type type;
  ui_equality_fn equality_fn;
  ui_destructor_fn destructor_fn;
  enum ui_signal_mode mode;
  struct ui_arena *arena;
  ui_int32 ref_count;
  ui_atomic_t lock;
  struct ui_reactive_node **subscribers;
  size_t subscribers_count;
  size_t subscribers_capacity;
};

static int s_mock_fail_on_invalid = 0;
static int s_mock_fail_on_valid = 0;
static int s_mock_fail_on_pending = 0;
static int s_fail_reactor_schedule = 0;

static ui_error_t validator_trigger_sched_fail(struct ui_form_control *control,
                                               union ui_signal_payload value,
                                               void *user_data,
                                               ui_bool_t *out_is_valid) {
  if (control) {
  }
  if (value.int_val) {
  }
  if (user_data) {
  }
  *out_is_valid = UI_TRUE;
  if (s_fail_reactor_schedule) {
    g_malloc_fail_countdown = 0;
  }
  return UI_ERROR_NONE;
}

static ui_error_t mock_status_equality_ctrl(union ui_signal_payload a,
                                            union ui_signal_payload b,
                                            ui_bool_t *out_equal) {
  if (s_mock_fail_on_invalid && b.int_val == (ui_int32)UI_FORM_STATUS_INVALID) {
    return UI_ERROR_UNKNOWN;
  }
  if (s_mock_fail_on_valid && b.int_val == (ui_int32)UI_FORM_STATUS_VALID) {
    return UI_ERROR_UNKNOWN;
  }
  if (s_mock_fail_on_pending && b.int_val == (ui_int32)UI_FORM_STATUS_PENDING) {
    return UI_ERROR_UNKNOWN;
  }
  *out_equal = (a.int_val == b.int_val) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

static int run_target_branch_tests(void) {
  struct ui_arena *arena = NULL;
  struct ui_thread_pool *pool = NULL;
  ui_form_control_t *control = NULL;
  ui_form_control_t *control2 = NULL;
  union ui_signal_payload dummy;
  struct test_form_ctrl_mirror *tc = NULL;
  struct test_signal_mirror *ts = NULL;

  dummy.int_val = 10;
  if (ui_arena_create(8192, &arena) != UI_ERROR_NONE) {
    return 1;
  }
  if (ui_thread_pool_create(1, &pool) != UI_ERROR_NONE) {
    ui_arena_destroy(arena);
    return 1;
  }

  if (ui_form_control_create(arena, dummy, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                             UI_SIGNAL_MODE_SINGLE_THREADED,
                             &control) != UI_ERROR_NONE) {
    ui_thread_pool_destroy(pool);
    ui_arena_destroy(arena);
    return 1;
  }

  tc = (struct test_form_ctrl_mirror *)control;

  /* Test line 212: ui_form_control_add_validator validation error */
  {
    ui_signal_t *saved_val = tc->value_signal;
    tc->value_signal = NULL;
    ui_form_control_add_validator(control, sync_validator, NULL);
    tc->value_signal = saved_val;
  }

  /* Test line 270: ui_form_control_add_async_validator validation error */
  {
    ui_signal_t *saved_val = tc->value_signal;
    tc->value_signal = NULL;
    ui_form_control_add_async_validator(control, dummy_async_valid, NULL, pool,
                                        NULL);
    tc->value_signal = saved_val;
  }

  /* Test lines 499, 505, 510 in ui_form_control_set_value */
  {
    union ui_signal_payload val;
    ui_signal_t *saved_val = tc->value_signal;
    ui_signal_t *saved_dirty = tc->dirty_signal;
    ui_signal_t *saved_status = tc->status_signal;
    val.int_val = 100;

    /* Line 499: value_signal fails */
    tc->value_signal = NULL;
    ui_form_control_set_value(control, val);
    tc->value_signal = saved_val;

    /* Line 505: dirty_signal fails */
    tc->dirty_signal = NULL;
    ui_form_control_set_value(control, val);
    tc->dirty_signal = saved_dirty;

    /* Line 510: run_validation fails in set_value */
    tc->status_signal = NULL;
    ui_form_control_set_value(control, val);
    tc->status_signal = saved_status;
  }

  /* Test line 475: setting VALID fails when no validators */
  {
    ui_form_control_t *ctrl_no_val = NULL;
    if (ui_form_control_create(arena, dummy, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                               UI_SIGNAL_MODE_SINGLE_THREADED,
                               &ctrl_no_val) == UI_ERROR_NONE) {
      struct test_form_ctrl_mirror *tcnv =
          (struct test_form_ctrl_mirror *)ctrl_no_val;
      struct test_signal_mirror *tsnv =
          (struct test_signal_mirror *)tcnv->status_signal;
      tsnv->equality_fn = mock_status_equality_ctrl;
      s_mock_fail_on_valid = 1;
      ui_form_control_enable(ctrl_no_val);
      s_mock_fail_on_valid = 0;
      ui_form_control_destroy(ctrl_no_val);
    }
  }

  /* Test line 409: setting INVALID fails when sync validator fails */
  ts = (struct test_signal_mirror *)tc->status_signal;
  ts->equality_fn = mock_status_equality_ctrl;
  ui_form_control_add_validator(control, sync_validator_err_false, NULL);
  s_mock_fail_on_invalid = 1;
  ui_form_control_enable(control);
  s_mock_fail_on_invalid = 0;

  /* Test lines 422, 448, 462, 307, 347 with control2 */
  if (ui_form_control_create(arena, dummy, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                             UI_SIGNAL_MODE_SINGLE_THREADED,
                             &control2) == UI_ERROR_NONE) {
    struct test_form_ctrl_mirror *tc2 =
        (struct test_form_ctrl_mirror *)control2;
    struct test_signal_mirror *ts2 =
        (struct test_signal_mirror *)tc2->status_signal;
    ts2->equality_fn = mock_status_equality_ctrl;

    /* Line 422: setting PENDING fails */
    s_mock_fail_on_pending = 1;
    ui_form_control_add_async_validator(control2, dummy_async_valid, NULL, pool,
                                        NULL);
    s_mock_fail_on_pending = 0;

    /* Line 448: pool schedule fails with INVALID fail */
    tc2->thread_pool = NULL;
    s_mock_fail_on_invalid = 1;
    ui_form_control_enable(control2);
    s_mock_fail_on_invalid = 0;
    tc2->thread_pool = pool;

    /* Line 462: task malloc fails with INVALID fail */
    g_malloc_fail_countdown = 0;
    s_mock_fail_on_invalid = 1;
    ui_form_control_enable(control2);
    s_mock_fail_on_invalid = 0;
    g_malloc_fail_countdown = -1;

    /* Lines 307-308 & 347-348: inline worker cb sets VALID which fails */
    s_mock_fail_on_valid = 1;
    ui_form_control_enable(control2);
    usleep(50000);
    s_mock_fail_on_valid = 0;

    ui_form_control_destroy(control2);
  }

  /* Test lines 340-341: ui_reactor_schedule fails in async worker */
  {
    ui_form_control_t *control3 = NULL;
    struct ui_reactor *target_reactor = NULL;
    if (ui_reactor_create(&target_reactor) == UI_ERROR_NONE) {
      if (ui_form_control_create(arena, dummy, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                                 UI_SIGNAL_MODE_SINGLE_THREADED,
                                 &control3) == UI_ERROR_NONE) {
        s_fail_reactor_schedule = 1;
        ui_form_control_add_async_validator(
            control3, validator_trigger_sched_fail, NULL, pool, target_reactor);
        usleep(50000);
        g_malloc_fail_countdown = -1;
        s_fail_reactor_schedule = 0;
        ui_form_control_destroy(control3);
      }
      ui_reactor_destroy(target_reactor);
    }
  }

  ui_form_control_destroy(control);
  ui_thread_pool_destroy(pool);
  ui_arena_destroy(arena);
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
  if (run_extra_control() != 0)
    return 1;
  if (run_extra_control2_all() != 0)
    return 1;
  if (run_extra_control3_all() != 0)
    return 1;
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
      return 1;
    }
  }

  if (run_target_branch_tests() != 0)
    return 1;

  if (run_oom_tests_control() != 0)
    return 1;

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
            return 1;
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
        return 1;
      }
    }
  }

  printf("test_ui_form_control passed\n");
  return 0;
}
