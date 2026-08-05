/* clang-format off */
#include "../include/ui_hotkey_registry.h"
#include <stdio.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t mock_hotkey_callback(void *user_data) {
  int *called = (int *)user_data;
  *called = 1;
  return UI_ERROR_NONE;
}

static ui_error_t mock_hotkey_callback_fail(void *user_data) {
  int *called = (int *)user_data;
  *called = 1;
  return UI_ERROR_UNKNOWN;
}

static void test_hotkey_registry_basic(void) {
  struct ui_hotkey_registry *reg = NULL;
  struct ui_hotkey_chord chord;
  struct ui_event ev;
  int id1, id2;
  int called = 0;
  int handled = 0;
  ui_error_t rc;

  /* Invalid args */
  rc = ui_hotkey_registry_create(NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_hotkey_registry_create(&reg);
  assert(rc == UI_ERROR_NONE);

  rc = ui_hotkey_registry_destroy(NULL);
  assert(rc == UI_ERROR_NONE);

  chord.key_code = 'A';
  chord.modifiers = UI_MODIFIER_CTRL;

  /* Register Invalid */
  rc = ui_hotkey_registry_register(NULL, chord, mock_hotkey_callback, &called,
                                   &id1);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_hotkey_registry_register(reg, chord, NULL, &called, &id1);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_hotkey_registry_register(reg, chord, mock_hotkey_callback, &called,
                                   NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Register 1 */
  rc = ui_hotkey_registry_register(reg, chord, mock_hotkey_callback, &called,
                                   &id1);
  assert(rc == UI_ERROR_NONE);

  /* Unregister Invalid */
  rc = ui_hotkey_registry_unregister(NULL, id1);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Process Event Invalid */
  rc = ui_hotkey_registry_process_event(NULL, &ev, &handled);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_hotkey_registry_process_event(reg, NULL, &handled);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_hotkey_registry_process_event(reg, &ev, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Process Event - Wrong type */
  ev.type = UI_EVENT_KEY_UP;
  ev.event_data.keyboard.key_code = 'A';
  ev.event_data.keyboard.modifiers = UI_MODIFIER_CTRL;
  rc = ui_hotkey_registry_process_event(reg, &ev, &handled);
  assert(rc == UI_ERROR_NONE);
  assert(handled == 0);

  /* Process Event - Wrong key */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = 'B';
  ev.event_data.keyboard.modifiers = UI_MODIFIER_CTRL;
  rc = ui_hotkey_registry_process_event(reg, &ev, &handled);
  assert(rc == UI_ERROR_NONE);
  assert(handled == 0);
  assert(called == 0);

  /* Process Event - Wrong modifier */
  ev.event_data.keyboard.key_code = 'A';
  ev.event_data.keyboard.modifiers = UI_MODIFIER_ALT;
  rc = ui_hotkey_registry_process_event(reg, &ev, &handled);
  assert(rc == UI_ERROR_NONE);
  assert(handled == 0);
  assert(called == 0);

  /* Process Event - Match */
  ev.event_data.keyboard.key_code = 'A';
  ev.event_data.keyboard.modifiers = UI_MODIFIER_CTRL;
  rc = ui_hotkey_registry_process_event(reg, &ev, &handled);
  assert(rc == UI_ERROR_NONE);
  assert(handled == 1);
  assert(called == 1);

  /* Register 2 */
  chord.key_code = 'B';
  rc = ui_hotkey_registry_register(reg, chord, mock_hotkey_callback_fail,
                                   &called, &id2);
  assert(rc == UI_ERROR_NONE);

  /* Process Event - Match with error */
  called = 0;
  ev.event_data.keyboard.key_code = 'B';
  rc = ui_hotkey_registry_process_event(reg, &ev, &handled);
  assert(rc == UI_ERROR_UNKNOWN);
  assert(called == 1);

  /* Unregister 1 */
  rc = ui_hotkey_registry_unregister(reg, id1);
  assert(rc == UI_ERROR_NONE);

  /* Process Event - Unregistered */
  called = 0;
  ev.event_data.keyboard.key_code = 'A';
  rc = ui_hotkey_registry_process_event(reg, &ev, &handled);
  assert(rc == UI_ERROR_NONE);
  assert(handled == 0);
  assert(called == 0);

  /* Unregister already unregistered */
  rc = ui_hotkey_registry_unregister(reg, id1);
  assert(rc == UI_ERROR_NONE);

  (void)ui_hotkey_registry_destroy(reg);
}

static void test_hotkey_registry_full(void) {
  struct ui_hotkey_registry *reg = NULL;
  struct ui_hotkey_chord chord;
  int id;
  ui_error_t rc;
  int i;

  ui_hotkey_registry_create(&reg);
  chord.key_code = 'A';
  chord.modifiers = 0;

  for (i = 0; i < 128; i++) { /* MAX_HOTKEYS */
    rc = ui_hotkey_registry_register(reg, chord, mock_hotkey_callback, NULL,
                                     &id);
    assert(rc == UI_ERROR_NONE);
  }

  rc = ui_hotkey_registry_register(reg, chord, mock_hotkey_callback, NULL, &id);
  assert(rc == UI_ERROR_OUT_OF_BOUNDS);

  (void)ui_hotkey_registry_destroy(reg);
}

static void run_oom_tests_hotkey_registry(void) {
  struct ui_hotkey_registry *reg;
  int i;
  ui_error_t rc;

  for (i = 0; i < 50; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_hotkey_registry_create(&reg);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE) {
      (void)ui_hotkey_registry_destroy(reg);
      break;
    }
  }
}

int main(void) {
  test_hotkey_registry_basic();
  test_hotkey_registry_full();
  run_oom_tests_hotkey_registry();
  printf("All ui_hotkey_registry tests passed.\n");
  return 0;
}
