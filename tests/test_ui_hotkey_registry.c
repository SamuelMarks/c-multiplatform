/* clang-format off */
#include "../include/ui_hotkey_registry.h"
#include "../include/ui_error.h"
#include "../include/ui_event.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static enum ui_error mock_hotkey_callback(void *user_data) {
  int *called = (int *)user_data;
  *called = 1;
  return UI_ERROR_NONE;
}

static int test_hotkey_lifecycle(void) {
  struct ui_hotkey_registry *registry = NULL;
  enum ui_error rc;

  rc = ui_hotkey_registry_create(&registry);
  if (rc != UI_ERROR_NONE || registry == NULL)
    return 1;

  ui_hotkey_registry_destroy(registry);
  return 0;
}

static int test_hotkey_registration(void) {
  struct ui_hotkey_registry *registry = NULL;
  enum ui_error rc;
  int id1;
  int called = 0;
  struct ui_hotkey_chord chord;
  struct ui_event ev;
  int handled;

  rc = ui_hotkey_registry_create(&registry);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Ctrl+Shift+P */
  chord.key_code = 'P';
  chord.modifiers = UI_MODIFIER_CTRL | UI_MODIFIER_SHIFT;

  rc = ui_hotkey_registry_register(registry, chord, mock_hotkey_callback,
                                   &called, &id1);
  if (rc != UI_ERROR_NONE)
    return 1;

  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = 'P';
  ev.event_data.keyboard.modifiers = UI_MODIFIER_CTRL | UI_MODIFIER_SHIFT;

  rc = ui_hotkey_registry_process_event(registry, &ev, &handled);
  if (rc != UI_ERROR_NONE || !handled || !called)
    return 1;

  called = 0;
  rc = ui_hotkey_registry_unregister(registry, id1);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_hotkey_registry_process_event(registry, &ev, &handled);
  if (rc != UI_ERROR_NONE || handled || called)
    return 1;

  ui_hotkey_registry_destroy(registry);
  return 0;
}

static int test_edge_cases(void) {
  struct ui_hotkey_registry *r = NULL;
  struct ui_hotkey_chord chord = {'A', 0};
  struct ui_event ev;
  int id;
  int handled;

  if (ui_hotkey_registry_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_hotkey_registry_destroy(NULL);

  ui_hotkey_registry_create(&r);

  if (ui_hotkey_registry_register(NULL, chord, mock_hotkey_callback, NULL,
                                  &id) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_hotkey_registry_register(r, chord, NULL, NULL, &id) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_hotkey_registry_register(r, chord, mock_hotkey_callback, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_hotkey_registry_unregister(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Register a real entry so we can fail to match it later */
  if (ui_hotkey_registry_register(r, chord, mock_hotkey_callback, NULL, &id) !=
      UI_ERROR_NONE)
    return 1;

  /* Unregister non-existent ID when there is an active entry */
  if (ui_hotkey_registry_unregister(r, 999) != UI_ERROR_NONE)
    return 1;

  /* Keyboard event that matches keycode but NOT modifiers */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = 'A';
  ev.event_data.keyboard.modifiers = UI_MODIFIER_SHIFT;
  if (ui_hotkey_registry_process_event(r, &ev, &handled) != UI_ERROR_NONE ||
      handled != 0)
    return 1;

  ev.type = UI_EVENT_KEY_DOWN;
  if (ui_hotkey_registry_process_event(NULL, &ev, &handled) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_hotkey_registry_process_event(r, NULL, &handled) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_hotkey_registry_process_event(r, &ev, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Non-keyboard event */
  ev.type = UI_EVENT_MOUSE_DOWN;
  if (ui_hotkey_registry_process_event(r, &ev, &handled) != UI_ERROR_NONE ||
      handled != 0)
    return 1;

  /* Keyboard event that doesn't match */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = 'B';
  ev.event_data.keyboard.modifiers = 0;
  if (ui_hotkey_registry_process_event(r, &ev, &handled) != UI_ERROR_NONE ||
      handled != 0)
    return 1;

  ui_hotkey_registry_destroy(r);
  return 0;
}

static int test_oom_and_capacity(void) {
  struct ui_hotkey_registry *r = NULL;
  struct ui_hotkey_chord chord = {'C', 0};
  int id;
  int i;

  g_malloc_fail_countdown = 0;
  if (ui_hotkey_registry_create(&r) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  ui_hotkey_registry_create(&r);

  /* Fill up capacity (MAX_HOTKEYS is 128) */
  for (i = 0; i < 128; i++) {
    if (ui_hotkey_registry_register(r, chord, mock_hotkey_callback, NULL,
                                    &id) != UI_ERROR_NONE)
      return 1;
  }

  /* One more should fail with OUT_OF_BOUNDS */
  if (ui_hotkey_registry_register(r, chord, mock_hotkey_callback, NULL, &id) !=
      UI_ERROR_OUT_OF_BOUNDS)
    return 1;

  ui_hotkey_registry_destroy(r);
  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_hotkey_registry tests...\n");

  failed |= test_hotkey_lifecycle();
  failed |= test_hotkey_registration();
  failed |= test_edge_cases();
  failed |= test_oom_and_capacity();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
