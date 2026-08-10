/* clang-format off */
#include "ui_scroll_dispatcher.h"
#include "ui_layout_observer.h"
#include <stdio.h>
/* clang-format on */

static int g_callback_called = 0;
static float g_last_scroll_x = -1.0f;
static float g_last_scroll_y = -1.0f;
static void *g_callback_user_data = NULL;

static ui_error_t test_callback(struct ui_scroll_dispatcher *dispatcher,
                                const struct ui_scroll_info *info,
                                void *user_data) {
  (void)dispatcher;
  g_callback_called++;
  if (info) {
    g_last_scroll_x = info->scroll_x;
    g_last_scroll_y = info->scroll_y;
    return UI_ERROR_NONE;
  }
  g_callback_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t test_callback_fail(struct ui_scroll_dispatcher *dispatcher,
                                     const struct ui_scroll_info *info,
                                     void *user_data) {
  (void)dispatcher;
  (void)info;
  (void)user_data;
  return UI_ERROR_UNKNOWN;
}

void test_ui_scroll_dispatcher_oom(void);
int main(void) {
  ui_error_t rc;
  struct ui_scroll_dispatcher *dispatcher = NULL;
  struct ui_layout_observer *layout_obs = NULL;
  struct ui_scroll_info info;
  int reg_id1 = -1;
  int reg_id2 = -1;
  int test_failed = 0;

  rc = ui_scroll_dispatcher_create(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Test failed: expected INVALID_ARGUMENT from create\n");
    test_failed = 1;
    goto cleanup;
  }

  rc = ui_scroll_dispatcher_create(&dispatcher);
  if (rc != UI_ERROR_NONE || !dispatcher) {
    printf("Test failed: create failed\n");
    test_failed = 1;
    goto cleanup;
  }

  rc = ui_scroll_dispatcher_register(NULL, test_callback, NULL, &reg_id1);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    test_failed = 1;
  rc = ui_scroll_dispatcher_register(dispatcher, NULL, NULL, &reg_id1);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    test_failed = 1;
  rc = ui_scroll_dispatcher_register(dispatcher, test_callback, NULL, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    test_failed = 1;
  if (test_failed) {
    printf("Test failed: expected INVALID_ARGUMENT from register\n");
    goto cleanup;
  }

  rc = ui_scroll_dispatcher_register(dispatcher, test_callback, (void *)0x1234,
                                     &reg_id1);
  if (rc != UI_ERROR_NONE || reg_id1 < 0) {
    printf("Test failed: register failed\n");
    test_failed = 1;
    goto cleanup;
  }

  rc = ui_scroll_dispatcher_register(dispatcher, test_callback, (void *)0x5678,
                                     &reg_id2);
  if (rc != UI_ERROR_NONE || reg_id2 < 0 || reg_id1 == reg_id2) {
    printf("Test failed: second register failed\n");
    test_failed = 1;
    goto cleanup;
  }

  info.scroll_x = 100.0f;
  info.scroll_y = 200.0f;
  info.delta_x = 10.0f;
  info.delta_y = 20.0f;
  info.source_id = 1;

  g_callback_called = 0;
  rc = ui_scroll_dispatcher_notify(NULL, &info);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    test_failed = 1;
  rc = ui_scroll_dispatcher_notify(dispatcher, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    test_failed = 1;
  if (test_failed) {
    printf("Test failed: expected INVALID_ARGUMENT from notify\n");
    goto cleanup;
  }

  rc = ui_scroll_dispatcher_notify(dispatcher, &info);
  if (rc != UI_ERROR_NONE || g_callback_called != 2 ||
      g_last_scroll_x != 100.0f) {
    printf("Test failed: notify failed\n");
    test_failed = 1;
    goto cleanup;
  }

  rc = ui_scroll_dispatcher_unregister(dispatcher, 999);
  if (rc != UI_ERROR_NOT_FOUND) {
    printf("Test failed: expected NOT_FOUND from unregister\n");
    test_failed = 1;
    goto cleanup;
  }

  rc = ui_scroll_dispatcher_unregister(dispatcher, reg_id1);
  if (rc != UI_ERROR_NONE) {
    printf("Test failed: unregister failed\n");
    test_failed = 1;
    goto cleanup;
  }

  g_callback_called = 0;
  g_last_scroll_y = -1.0f;
  info.scroll_y = 300.0f;
  rc = ui_scroll_dispatcher_notify(dispatcher, &info);
  if (rc != UI_ERROR_NONE || g_callback_called != 1 ||
      g_last_scroll_y != 300.0f) {
    printf("Test failed: unregister didn't remove callback\n");
    test_failed = 1;
    goto cleanup;
  }

  /* Test callback failure */
  {
    int fail_reg_id;
    ui_scroll_dispatcher_register(dispatcher, test_callback_fail, NULL,
                                  &fail_reg_id);
    rc = ui_scroll_dispatcher_notify(dispatcher, &info);
    if (rc != UI_ERROR_UNKNOWN) {
      printf("Test failed: expected UNKNOWN from failing callback\n");
      test_failed = 1;
      goto cleanup;
    }
    ui_scroll_dispatcher_unregister(dispatcher, fail_reg_id);
  }

  /* Layout observer binding test */
  rc = ui_layout_observer_create(&layout_obs);
  if (rc != UI_ERROR_NONE) {
    printf("Test failed: could not create layout observer\n");
    test_failed = 1;
    goto cleanup;
  }

  rc = ui_scroll_dispatcher_bind_layout_observer(NULL, layout_obs);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    test_failed = 1;
  rc = ui_scroll_dispatcher_bind_layout_observer(dispatcher, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    test_failed = 1;
  if (test_failed) {
    printf("Test failed: expected INVALID_ARGUMENT from bind\n");
    goto cleanup;
  }

  rc = ui_scroll_dispatcher_bind_layout_observer(dispatcher, layout_obs);
  if (rc != UI_ERROR_NONE) {
    printf("Test failed: bind failed\n");
    test_failed = 1;
    goto cleanup;
  }

  {
    int bp_id;
    rc =
        ui_layout_observer_add_breakpoint(layout_obs, 0, 1000, 0, 1000, &bp_id);
    if (rc != UI_ERROR_NONE) {
      printf("Test failed: could not add layout breakpoint\n");
      test_failed = 1;
      goto cleanup;
    }
  }

  g_callback_called = 0;
  g_last_scroll_x = -1.0f;
  g_last_scroll_y = -1.0f;
  rc = ui_layout_observer_notify_resize(layout_obs, 800, 600);
  if (rc != UI_ERROR_NONE) {
    printf("Test failed: layout notify resize failed\n");
    test_failed = 1;
    goto cleanup;
  }

  if (g_callback_called != 1 || g_last_scroll_x != 0.0f ||
      g_last_scroll_y != 0.0f) {
    printf(
        "Test failed: layout integration didn't fire generic scroll event\n");
    test_failed = 1;
    goto cleanup;
  }

  /* Test layout observer percolating callback failure */
  {
    int fail_reg_id;
    ui_scroll_dispatcher_register(dispatcher, test_callback_fail, NULL,
                                  &fail_reg_id);
    rc = ui_layout_observer_notify_resize(layout_obs, 1200, 1200);
    if (rc != UI_ERROR_UNKNOWN) {
      printf("Test failed: layout notify resize didn't percolate callback "
             "failure (got %d)\n",
             rc);
      test_failed = 1;
      goto cleanup;
    }
    ui_scroll_dispatcher_unregister(dispatcher, fail_reg_id);
  }

  rc = ui_scroll_dispatcher_destroy(dispatcher);
  dispatcher = NULL;
  if (rc != UI_ERROR_NONE) {
    printf("Test failed: destroy failed\n");
    test_failed = 1;
    goto cleanup;
  }

cleanup:
  if (layout_obs) {
    ui_layout_observer_destroy(layout_obs);
  }
  if (dispatcher) {
    (void)ui_scroll_dispatcher_destroy(dispatcher);
  }

  if (test_failed) {
    return 1;
  }

  test_ui_scroll_dispatcher_oom();
  printf("All scroll dispatcher tests passed.\n");
  return 0;
}
void test_ui_scroll_dispatcher_oom(void) {
  extern int g_malloc_fail_countdown;
  struct ui_scroll_dispatcher *dispatcher = NULL;
  int i;
  for (i = 0; i < 5; ++i) {
    g_malloc_fail_countdown = i;
    ui_scroll_dispatcher_create(&dispatcher);
  }
  g_malloc_fail_countdown = -1;

  ui_scroll_dispatcher_create(&dispatcher);
  if (dispatcher) {
    /* Fill up to capacity (INITIAL_SUBSCRIBER_CAPACITY = 8) */
    for (i = 0; i < 8; ++i) {
      int reg_id;
      ui_scroll_dispatcher_register(dispatcher, test_callback, NULL, &reg_id);
    }

    /* Force a successful realloc */
    {
      int reg_id;
      ui_scroll_dispatcher_register(dispatcher, test_callback, NULL, &reg_id);
    }

    /* force OOM during NEXT realloc (now capacity is 16, count is 9, so we need
     * to fill it to 16) */
    for (i = 9; i < 16; ++i) {
      int reg_id;
      ui_scroll_dispatcher_register(dispatcher, test_callback, NULL, &reg_id);
    }

    g_malloc_fail_countdown = 0;
    int r;
    ui_scroll_dispatcher_register(dispatcher, test_callback, NULL, &r);
    g_malloc_fail_countdown = -1;

    struct ui_layout_observer *obs = NULL;
    ui_layout_observer_create(&obs);
    for (i = 0; i < 5; ++i) {
      g_malloc_fail_countdown = i;
      ui_scroll_dispatcher_bind_layout_observer(dispatcher, obs);
    }
    g_malloc_fail_countdown = -1;
    ui_layout_observer_destroy(obs);

    (void)ui_scroll_dispatcher_destroy(dispatcher);
  }

  (void)ui_scroll_dispatcher_destroy(NULL);
  ui_scroll_dispatcher_unregister(NULL, 0);
}
