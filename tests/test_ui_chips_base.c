/* clang-format off */
#include "../include/ui_chips_base.h"
#include "../include/ui_error.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t dummy_cva_on_change(union ui_signal_payload new_value,
                                      void *user_data) {
  (void)new_value;
  (void)user_data;
  return UI_ERROR_NONE;
}

static ui_error_t dummy_cva_on_touched(void *user_data) {
  (void)user_data;
  return UI_ERROR_NONE;
}

static ui_error_t test_chips_basic(void) {
  ui_error_t rc;
  struct ui_chips_base *chips = NULL;
  size_t count = 0;
  const char *token = NULL;
  struct ui_control_value_accessor cva;

  /* Invalid arguments tests */
  rc = ui_chips_base_create(NULL, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_chips_base_destroy(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT && rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_chips_base_add(NULL, "test");

  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_chips_base_remove(NULL, 0);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_chips_base_remove_last(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_chips_base_get_count(NULL, &count);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_chips_base_get_token(NULL, 0, &token);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_chips_base_create(&chips, &cva);

  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_chips_base_add(chips, NULL);

  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_chips_base_get_count(chips, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_chips_base_get_token(chips, 0, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Empty state */
  rc = ui_chips_base_remove_last(chips);
  if (rc != UI_ERROR_NOT_FOUND)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_chips_base_remove(chips, 0);
  if (rc != UI_ERROR_OUT_OF_BOUNDS)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_chips_base_get_token(chips, 0, &token);
  if (rc != UI_ERROR_OUT_OF_BOUNDS)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* CVA methods */
  if (cva.register_on_change(NULL, dummy_cva_on_change, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  if (cva.register_on_touched(NULL, dummy_cva_on_touched, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  if (cva.set_disabled_state(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Explicitly trigger untouched dummy to cover line */
  dummy_cva_on_touched(NULL);

  rc = cva.register_on_change(chips, dummy_cva_on_change, NULL);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = cva.register_on_touched(chips, dummy_cva_on_touched, NULL);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = cva.set_disabled_state(chips, 1);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = cva.set_disabled_state(chips, 0);
  if (rc != UI_ERROR_NONE)
    return rc;

  {
    union ui_signal_payload payload;
    payload.ptr_val = NULL;
    rc = cva.write_value(chips, payload); /* does nothing but cover lines */
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  /* Functionality */
  rc = ui_chips_base_add(chips, "apple");
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_mock_strcpy_fail;
    g_mock_strcpy_fail = 1;
    rc = ui_chips_base_add(chips, "fail");
    if (rc != UI_ERROR_UNKNOWN)
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    g_mock_strcpy_fail = 0;
  }
#endif

  rc = ui_chips_base_add(chips, "banana");
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_chips_base_add(chips, "cherry");
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_chips_base_add(chips, "date");
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  /* Next add triggers capacity expansion (capacity goes 0->4->8) */
  rc = ui_chips_base_add(chips, "elderberry");
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_chips_base_get_count(chips, &count);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (count != 5)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_chips_base_remove(chips, 1);

  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc; /* banana */
  rc = ui_chips_base_get_count(chips, &count);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (count != 4)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_chips_base_get_token(chips, 1, &token);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (strcmp(token, "cherry") != 0)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_chips_base_remove_last(chips);

  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc; /* elderberry */
  rc = ui_chips_base_get_count(chips, &count);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (count != 3)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_chips_base_destroy(chips);
  if (rc != UI_ERROR_NONE)
    return rc;
  return UI_ERROR_NONE;
}

static ui_error_t test_chips_backspace(void) {
  ui_error_t rc;
  struct ui_chips_base *chips = NULL;
  int move_focus = 0;

  rc = ui_chips_base_handle_backspace(NULL, "", &move_focus);

  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_chips_base_create(&chips, NULL);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_chips_base_handle_backspace(chips, "", NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Empty chips, empty input -> shouldn't move focus to a chip since there are
   * none */
  rc = ui_chips_base_handle_backspace(chips, "", &move_focus);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (move_focus != 0)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_chips_base_add(chips, "token1");
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Has chips, but input is not empty */
  rc = ui_chips_base_handle_backspace(chips, "a", &move_focus);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (move_focus != 0)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Has chips, input is empty -> SHOULD move focus */
  rc = ui_chips_base_handle_backspace(chips, "", &move_focus);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (move_focus != 1)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_chips_base_handle_backspace(chips, NULL, &move_focus);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (move_focus != 1)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_chips_base_destroy(chips);
  if (rc != UI_ERROR_NONE)
    return rc;
  return UI_ERROR_NONE;
}

static ui_error_t run_oom_test_create_step(int i,
                                           struct ui_chips_base **out_chips,
                                           int *out_continue, int *out_break) {
  ui_error_t rc;
  g_malloc_fail_countdown = i;
  rc = ui_chips_base_create(out_chips, NULL);
  if (rc != UI_ERROR_NONE && rc != UI_ERROR_OUT_OF_MEMORY) {
    g_malloc_fail_countdown = -1;
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }
  if (rc == UI_ERROR_OUT_OF_MEMORY) {
    g_malloc_fail_countdown = -1;
    *out_continue = 1;
    return UI_ERROR_NONE;
  }
  if (rc == UI_ERROR_NONE) {
    g_malloc_fail_countdown = -1;
    {
      ui_error_t d_rc = ui_chips_base_destroy(*out_chips);
      if (d_rc != UI_ERROR_NONE)
        return d_rc;
    }
    *out_break = 1;
    return UI_ERROR_NONE;
  }
  g_malloc_fail_countdown = -1;
  return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
}

static ui_error_t run_oom_test_add_step(int i, struct ui_chips_base *chips,
                                        int *out_continue, int *out_break) {
  ui_error_t rc;
  g_malloc_fail_countdown = i;
  rc = ui_chips_base_add(chips, "token");
  if (rc != UI_ERROR_NONE && rc != UI_ERROR_OUT_OF_MEMORY) {
    g_malloc_fail_countdown = -1;
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }
  if (rc == UI_ERROR_OUT_OF_MEMORY) {
    g_malloc_fail_countdown = -1;
    *out_continue = 1;
    return UI_ERROR_NONE;
  }
  if (rc == UI_ERROR_NONE) {
    g_malloc_fail_countdown = -1;
    *out_break = 1;
    return UI_ERROR_NONE;
  }
  g_malloc_fail_countdown = -1;
  return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
}

static ui_error_t test_chips_oom(void) {
  struct ui_chips_base *chips = NULL;
  int i;
  ui_error_t rc;

  for (i = 0; i < 3; i++) {
    int do_continue = 0, do_break = 0;
    rc = run_oom_test_create_step(i, &chips, &do_continue, &do_break);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (do_continue)
      continue;
    if (do_break)
      break;
  }

  rc = ui_chips_base_create(&chips, NULL);
  if (rc != UI_ERROR_NONE)
    return rc;
  for (i = 0; i < 3; i++) {
    int do_continue = 0, do_break = 0;
    rc = run_oom_test_add_step(i, chips, &do_continue, &do_break);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (do_continue)
      continue;
    if (do_break)
      break;
  }
  rc = ui_chips_base_destroy(chips);
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
}

int main(void) {
  ui_error_t rc;
  printf("Running ui_chips_base tests...\n");
  rc = test_chips_basic();
  if (rc != UI_ERROR_NONE) {
    printf("Tests failed.\n");
    return 1;
  }
  rc = test_chips_backspace();
  if (rc != UI_ERROR_NONE) {
    printf("Tests failed.\n");
    return 1;
  }
  rc = test_chips_oom();
  if (rc != UI_ERROR_NONE) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All tests passed.\n");
  return 0;
}
