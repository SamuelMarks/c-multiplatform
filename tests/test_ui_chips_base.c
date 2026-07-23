/* clang-format off */
#include "../include/ui_chips_base.h"
#include "../include/ui_error.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static enum ui_error dummy_cva_on_change(union ui_signal_payload new_value,
                                         void *user_data) {
  (void)new_value;
  (void)user_data;
  return UI_ERROR_NONE;
}

static enum ui_error dummy_cva_on_touched(void *user_data) {
  (void)user_data;
  return UI_ERROR_NONE;
}

static int test_chips_basic(void) {
  struct ui_chips_base *chips = NULL;
  size_t count = 0;
  const char *token = NULL;
  struct ui_control_value_accessor cva;

  /* Invalid arguments tests */
  if (ui_chips_base_create(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_chips_base_destroy(NULL);

  if (ui_chips_base_add(NULL, "test") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chips_base_remove(NULL, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chips_base_remove_last(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chips_base_get_count(NULL, &count) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chips_base_get_token(NULL, 0, &token) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_chips_base_create(&chips, &cva) != UI_ERROR_NONE)
    return 1;

  if (ui_chips_base_add(chips, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chips_base_get_count(chips, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_chips_base_get_token(chips, 0, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Empty state */
  if (ui_chips_base_remove_last(chips) != UI_ERROR_NOT_FOUND)
    return 1;
  if (ui_chips_base_remove(chips, 0) != UI_ERROR_OUT_OF_BOUNDS)
    return 1;
  if (ui_chips_base_get_token(chips, 0, &token) != UI_ERROR_OUT_OF_BOUNDS)
    return 1;

  /* CVA methods */
  if (cva.register_on_change(NULL, dummy_cva_on_change, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (cva.register_on_touched(NULL, dummy_cva_on_touched, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (cva.set_disabled_state(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  cva.register_on_change(chips, dummy_cva_on_change, NULL);
  cva.register_on_touched(chips, dummy_cva_on_touched, NULL);
  cva.set_disabled_state(chips, 1);
  cva.set_disabled_state(chips, 0);

  {
    union ui_signal_payload payload;
    payload.ptr_val = NULL;
    cva.write_value(chips, payload); /* does nothing but cover lines */
  }

  /* Functionality */
  if (ui_chips_base_add(chips, "apple") != UI_ERROR_NONE)
    return 1;
  if (ui_chips_base_add(chips, "banana") != UI_ERROR_NONE)
    return 1;
  if (ui_chips_base_add(chips, "cherry") != UI_ERROR_NONE)
    return 1;
  if (ui_chips_base_add(chips, "date") != UI_ERROR_NONE)
    return 1;
  /* Next add triggers capacity expansion (capacity goes 0->4->8) */
  if (ui_chips_base_add(chips, "elderberry") != UI_ERROR_NONE)
    return 1;

  ui_chips_base_get_count(chips, &count);
  if (count != 5)
    return 1;

  if (ui_chips_base_remove(chips, 1) != UI_ERROR_NONE)
    return 1; /* banana */
  ui_chips_base_get_count(chips, &count);
  if (count != 4)
    return 1;

  ui_chips_base_get_token(chips, 1, &token);
  if (strcmp(token, "cherry") != 0)
    return 1;

  if (ui_chips_base_remove_last(chips) != UI_ERROR_NONE)
    return 1; /* elderberry */
  ui_chips_base_get_count(chips, &count);
  if (count != 3)
    return 1;

  ui_chips_base_destroy(chips);
  return 0;
}

static int test_chips_backspace(void) {
  struct ui_chips_base *chips = NULL;
  int move_focus = 0;

  if (ui_chips_base_handle_backspace(NULL, "", &move_focus) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_chips_base_create(&chips, NULL);
  if (ui_chips_base_handle_backspace(chips, "", NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Empty chips, empty input -> shouldn't move focus to a chip since there are
   * none */
  ui_chips_base_handle_backspace(chips, "", &move_focus);
  if (move_focus != 0)
    return 1;

  ui_chips_base_add(chips, "token1");

  /* Has chips, but input is not empty */
  ui_chips_base_handle_backspace(chips, "a", &move_focus);
  if (move_focus != 0)
    return 1;

  /* Has chips, input is empty -> SHOULD move focus */
  ui_chips_base_handle_backspace(chips, "", &move_focus);
  if (move_focus != 1)
    return 1;

  ui_chips_base_handle_backspace(chips, NULL, &move_focus);
  if (move_focus != 1)
    return 1;

  ui_chips_base_destroy(chips);
  return 0;
}

static int test_chips_oom(void) {
  struct ui_chips_base *chips = NULL;
  int i;
  enum ui_error rc;

  for (i = 0; i < 3; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_chips_base_create(&chips, NULL);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE) {
      ui_chips_base_destroy(chips);
      break;
    }
  }

  ui_chips_base_create(&chips, NULL);
  for (i = 0; i < 3; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_chips_base_add(chips, "token");
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE) {
      break;
    }
  }
  ui_chips_base_destroy(chips);

  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_chips_base tests...\n");
  failed |= test_chips_basic();
  failed |= test_chips_backspace();
  failed |= test_chips_oom();
  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All tests passed.\n");
  return 0;
}
