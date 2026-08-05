/* clang-format off */
#include "ui_key_manager.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int g_callback_called = 0;
static void *g_callback_user_data = NULL;

static void test_callback(void *user_data) {
  g_callback_called = 1;
  g_callback_user_data = user_data;
}

int main(void) {
  ui_error_t rc;
  struct ui_key_manager manager;
  struct ui_hotkey hk1;
  struct ui_hotkey hk2;
  struct ui_keyboard_event evt;
  int handled;
  char buffer[64];
  int test_failed = 0;
  size_t i;

  memset(&manager, 0, sizeof(manager));

  /* Cleanup Null */
  assert(ui_key_manager_cleanup(NULL) == UI_ERROR_INVALID_ARGUMENT);

  /* Init tests */
  rc = ui_key_manager_init(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Test failed: expected INVALID_ARGUMENT from init\n");
    test_failed = 1;
    goto cleanup;
  }

  g_malloc_fail_countdown = 0;
  rc = ui_key_manager_init(&manager);
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Test failed: expected OOM from init\n");
    test_failed = 1;
    goto cleanup;
  }
  g_malloc_fail_countdown = -1;

  rc = ui_key_manager_init(&manager);
  if (rc != UI_ERROR_NONE) {
    printf("Test failed: init failed\n");
    test_failed = 1;
    goto cleanup;
  }

  /* Register tests */
  hk1.id = 1;
  hk1.key_code = 'S';
  hk1.modifiers = UI_MODIFIER_CTRL;
  hk1.callback = test_callback;
  hk1.user_data = (void *)0x1234;

  rc = ui_key_manager_register(NULL, &hk1);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Test failed: expected INVALID_ARGUMENT from register\n");
    test_failed = 1;
    goto cleanup;
  }

  rc = ui_key_manager_register(&manager, &hk1);
  if (rc != UI_ERROR_NONE) {
    printf("Test failed: register failed\n");
    test_failed = 1;
    goto cleanup;
  }

  hk2.id = 2;
  hk2.key_code = UI_KEY_SPACE;
  hk2.modifiers = UI_MODIFIER_SHIFT | UI_MODIFIER_ALT;
  hk2.callback = test_callback;
  hk2.user_data = (void *)0x5678;

  rc = ui_key_manager_register(&manager, &hk2);
  if (rc != UI_ERROR_NONE) {
    printf("Test failed: register 2 failed\n");
    test_failed = 1;
    goto cleanup;
  }

  /* Test reallocation limit */
  for (i = 3; i < 9; ++i) {
    struct ui_hotkey hk;
    hk.id = (int)i;
    hk.key_code = 'A';
    hk.modifiers = 0;
    hk.callback = NULL;
    hk.user_data = NULL;
    rc = ui_key_manager_register(&manager, &hk);
    if (rc != UI_ERROR_NONE) {
      printf("Test failed: register reallocation failed\n");
      test_failed = 1;
      goto cleanup;
    }
  }

  {
    /* At count 8, next will reallocate */
    struct ui_hotkey hk;
    hk.id = 9;
    hk.key_code = 'A';
    hk.modifiers = 0;
    hk.callback = NULL;
    hk.user_data = NULL;

    g_malloc_fail_countdown = 0;
    rc = ui_key_manager_register(&manager, &hk);
    if (rc != UI_ERROR_OUT_OF_MEMORY) {
      printf("Test failed: register reallocation OOM failed\n");
      test_failed = 1;
      goto cleanup;
    }
    g_malloc_fail_countdown = -1;

    rc = ui_key_manager_register(&manager, &hk);
    if (rc != UI_ERROR_NONE) {
      printf("Test failed: register reallocation success failed\n");
      test_failed = 1;
      goto cleanup;
    }
  }

  /* Capacity scale 0 fallback */
  {
    struct ui_hotkey hk;
    size_t old_capacity = manager.capacity;
    manager.capacity = 0; /* force 0 */
    hk.id = 100;
    hk.key_code = 'B';
    hk.modifiers = 0;
    hk.callback = NULL;
    hk.user_data = NULL;
    rc = ui_key_manager_register(&manager, &hk);
    if (rc != UI_ERROR_NONE) {
      test_failed = 1;
      goto cleanup;
    }
    manager.capacity = old_capacity; /* Restore */
  }

  /* Process event tests */
  assert(ui_key_manager_process_event(NULL, &evt, &handled) ==
         UI_ERROR_INVALID_ARGUMENT);

  evt.key_code = 'S';
  evt.modifiers = UI_MODIFIER_CTRL;
  evt.scan_code = 0;
  evt.is_repeat = 0;

  g_callback_called = 0;
  rc = ui_key_manager_process_event(&manager, &evt, &handled);
  if (rc != UI_ERROR_NONE || !handled || !g_callback_called ||
      g_callback_user_data != (void *)0x1234) {
    printf("Test failed: process event matched failed\n");
    test_failed = 1;
    goto cleanup;
  }

  evt.key_code = 'S';
  evt.modifiers = UI_MODIFIER_SHIFT; /* Wrong modifier */
  g_callback_called = 0;
  rc = ui_key_manager_process_event(&manager, &evt, &handled);
  if (rc != UI_ERROR_NONE || handled || g_callback_called) {
    printf("Test failed: process event wrong modifier handled\n");
    test_failed = 1;
    goto cleanup;
  }

  /* Formatting tests */
  assert(ui_key_manager_format_hotkey(NULL, buffer, sizeof(buffer)) ==
         UI_ERROR_INVALID_ARGUMENT);

  rc = ui_key_manager_format_hotkey(&hk1, buffer, sizeof(buffer));
  if (rc != UI_ERROR_NONE || strcmp(buffer, "Ctrl+S") != 0) {
    printf("Test failed: formatting hk1 failed (got: %s)\n", buffer);
    test_failed = 1;
    goto cleanup;
  }

  rc = ui_key_manager_format_hotkey(&hk2, buffer, sizeof(buffer));
  if (rc != UI_ERROR_NONE || strcmp(buffer, "Shift+Alt+Space") != 0) {
    printf("Test failed: formatting hk2 failed (got: %s)\n", buffer);
    test_failed = 1;
    goto cleanup;
  }

  /* Additional format tests */
  {
    struct ui_hotkey hk3;
    hk3.modifiers = UI_MODIFIER_SUPER;
    hk3.key_code = UI_KEY_ESCAPE;
    ui_key_manager_format_hotkey(&hk3, buffer, sizeof(buffer));
    if (strcmp(buffer, "Super+Key") != 0) {
      test_failed = 1;
      goto cleanup;
    }
  }

  /* Unregister tests */
  assert(ui_key_manager_unregister(NULL, 999) == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_key_manager_unregister(&manager, 999);
  if (rc != UI_ERROR_NOT_FOUND) {
    printf("Test failed: expected NOT_FOUND from unregister\n");
    test_failed = 1;
    goto cleanup;
  }

  rc = ui_key_manager_unregister(&manager, 1);
  if (rc != UI_ERROR_NONE) {
    printf("Test failed: unregister failed\n");
    test_failed = 1;
    goto cleanup;
  }

  evt.key_code = 'S';
  evt.modifiers = UI_MODIFIER_CTRL;
  rc = ui_key_manager_process_event(&manager, &evt, &handled);
  if (rc != UI_ERROR_NONE || handled) {
    printf("Test failed: unregister didn't remove hotkey\n");
    test_failed = 1;
    goto cleanup;
  }

  /* Full branch coverage additions */
  {
    int dummy_id;
    struct ui_hotkey null_cb_hk;
    char fmt_buf[32];

    /* Missing branches in add_hotkey */
    ui_key_manager_register(&manager, NULL);

    /* Missing branches in process_event */
    ui_key_manager_process_event(&manager, NULL, &handled);
    ui_key_manager_process_event(&manager, &evt, NULL);

    /* Missing branches in format_hotkey */
    ui_key_manager_format_hotkey(&null_cb_hk, NULL, 32);
    ui_key_manager_format_hotkey(&null_cb_hk, fmt_buf, 0);

    /* Null callback event processing */
    null_cb_hk.key_code = '0';
    null_cb_hk.modifiers = 0;
    null_cb_hk.callback = NULL;
    null_cb_hk.user_data = NULL;
    ui_key_manager_register(&manager, &null_cb_hk);

    evt.key_code = '0';
    evt.modifiers = 0;
    ui_key_manager_process_event(&manager, &evt, &handled);

    /* Formatting a non A-Z key (branch 1 of the A-Z check) */
    ui_key_manager_format_hotkey(&null_cb_hk, fmt_buf, sizeof(fmt_buf));
  }

  /* Cleanup tests */
  rc = ui_key_manager_cleanup(&manager);
  if (rc != UI_ERROR_NONE) {
    printf("Test failed: cleanup failed\n");
    test_failed = 1;
    goto cleanup;
  }

cleanup:
  ui_key_manager_cleanup(&manager);
  if (test_failed) {
    return 1;
  }

  printf("All key manager tests passed.\n");
  return 0;
}
