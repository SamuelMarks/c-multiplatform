/* clang-format off */
#include "../include/ui_input_mask.h"
#include "../include/ui_input_base.h"
#include "../include/ui_component.h"
#include "../include/ui_gesture.h"
#include "../include/ui_error.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

struct ui_input_mask {
  struct ui_input_base *input;
  char pattern[64];
  char raw_value[64];
  char formatted_value[64];
  int is_processing;
};

struct ui_input_base {
  struct ui_component *component;
  struct ui_gesture_recognizer *gesture_recognizer;
  char *text;
  char *placeholder;
  int disabled;
  int cursor_position;
  ui_input_on_change_t on_change;
  void *user_data;
  ui_error_t (*on_touched)(void *user_data);
  void *on_touched_user_data;
};

extern int g_malloc_fail_countdown;

static int test_mask_formatting(void) {
  struct ui_input_base *input = NULL;
  struct ui_input_mask *mask = NULL;
  ui_error_t rc;
  const char *raw;

  rc = ui_input_base_create(&input);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* test null checks */
  if (ui_input_mask_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  g_malloc_fail_countdown = 0;
  if (ui_input_mask_create(&mask) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  rc = ui_input_mask_create(&mask);
  if (rc != UI_ERROR_NONE)
    return 1;

  if (ui_input_mask_bind(NULL, input) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_input_mask_bind(mask, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* set text first so that bind initial formats something */
  ui_input_base_set_text(input, "xyz");

  rc = ui_input_mask_bind(mask, input);
  if (rc != UI_ERROR_NONE)
    return 1;

  if (ui_input_mask_set_pattern(NULL, "") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_input_mask_set_pattern(mask, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_input_mask_set_pattern(mask, "(999) 999-9999");
  if (rc != UI_ERROR_NONE)
    return 1;

  if (ui_input_mask_process_text(NULL, "123") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_input_mask_process_text(mask, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Simulate user typing text */
  ui_input_mask_process_text(mask, "1234567890x");

  {
    const char *tmp_text;
    if (ui_input_base_get_text(input, &tmp_text) != UI_ERROR_NONE ||
        strcmp(tmp_text, "(123) 456-7890") != 0) {
      return 1;
    }
  }

  if (ui_input_mask_get_raw_value(NULL, &raw) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_input_mask_get_raw_value(mask, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_input_mask_get_raw_value(mask, &raw);
  if (rc != UI_ERROR_NONE || strcmp(raw, "1234567890") != 0) {
    return 1;
  }

  /* Test alpha and alphanumeric */
  ui_input_mask_set_pattern(mask, "aA**");
  ui_input_mask_process_text(mask, "ab123");
  ui_input_mask_process_text(mask, "12ab!@"); /* Test skip invalid */

  /* Test literal match consume */
  ui_input_mask_set_pattern(mask, "(999)");
  ui_input_mask_process_text(mask,
                             "(123)"); /* '( )' are literal, 123 are digits */

  /* Simulate user input via events to trigger on_input_change via the base
   * component */
  {
    struct ui_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = UI_EVENT_KEY_DOWN;
    ev.event_data.keyboard.key_code = '0';
    ui_input_base_process_event(input, &ev, 0.0);
  }

  /* Test is_processing recursive guard and invalid argument in on_input_change
   */
  {
    mask->is_processing = 1;
    if (input->on_change(input, "0", mask) != UI_ERROR_NONE)
      return 1;
    mask->is_processing = 0;
    if (input->on_change(NULL, "0", mask) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }

  if (ui_input_mask_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test when mask is not attached to input */
  {
    struct ui_input_mask *unattached = NULL;
    ui_input_mask_create(&unattached);
    ui_input_mask_set_pattern(unattached, "99-99");
    ui_input_mask_process_text(unattached, "1234");
    ui_input_mask_destroy(unattached);
  }
  /* Test string exceeding MAX_MASK_LEN */
  {
    char long_text[300];
    char long_pattern[300];
    int i;
    for (i = 0; i < 299; i++) {
      long_text[i] = '1';
      long_pattern[i] = '9';
    }
    long_text[299] = '\0';
    long_pattern[299] = '\0';
    ui_input_mask_set_pattern(mask, long_pattern);
    ui_input_mask_process_text(mask, long_text);
  }

  ui_input_mask_destroy(mask);

  /* Also test mask destroy when mask->input is NULL */
  ui_input_mask_create(&mask);
  /* Test when mask is not attached to input */
  {
    struct ui_input_mask *unattached = NULL;
    ui_input_mask_create(&unattached);
    ui_input_mask_set_pattern(unattached, "99-99");
    ui_input_mask_process_text(unattached, "1234");
    ui_input_mask_destroy(unattached);
  }
  /* Test string exceeding MAX_MASK_LEN */
  {
    char long_text[300];
    char long_pattern[300];
    int i;
    for (i = 0; i < 299; i++) {
      long_text[i] = '1';
      long_pattern[i] = '9';
    }
    long_text[299] = '\0';
    long_pattern[299] = '\0';
    ui_input_mask_set_pattern(mask, long_pattern);
    ui_input_mask_process_text(mask, long_text);
  }

  ui_input_mask_destroy(mask);

  {
    ui_error_t rc_cleanup = ui_input_base_destroy(input);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  return 0;
}

#ifdef UI_TEST_MOCK_ALLOC
static int test_mask_mock_failures(void) {
  struct ui_input_base *input = NULL;
  struct ui_input_mask *mask = NULL;
  ui_error_t rc;
  extern int g_input_mask_mock_fail;

  rc = ui_input_base_create(&input);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_input_mask_create(&mask);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* 1. ui_input_base_set_on_change fails in bind */
  g_input_mask_mock_fail = 1;
  rc = ui_input_mask_bind(mask, input);
  if (rc != UI_ERROR_UNKNOWN)
    return 1;
  g_input_mask_mock_fail = 0;

  /* 2. ui_input_base_get_text fails in bind */
  g_input_mask_mock_fail = 3;
  rc = ui_input_mask_bind(mask, input);
  if (rc != UI_ERROR_UNKNOWN)
    return 1;
  g_input_mask_mock_fail = 0;

  /* Bind properly */
  rc = ui_input_mask_bind(mask, input);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* 3. ui_input_base_set_text fails in process_text */
  g_input_mask_mock_fail = 4;
  rc = ui_input_mask_process_text(mask, "123");
  if (rc != UI_ERROR_UNKNOWN)
    return 1;
  g_input_mask_mock_fail = 0;

  /* 4. set_pattern triggers process_text which fails */
  g_input_mask_mock_fail = 4;
  rc = ui_input_mask_set_pattern(mask, "999");
  if (rc != UI_ERROR_UNKNOWN)
    return 1;
  g_input_mask_mock_fail = 0;

  /* 5. destroy fails when ui_input_base_set_on_change fails */
  g_input_mask_mock_fail = 2;
  rc = ui_input_mask_destroy(mask);
  if (rc != UI_ERROR_UNKNOWN)
    return 1;
  g_input_mask_mock_fail = 0;

  rc = ui_input_base_destroy(input);
  if (rc != UI_ERROR_NONE)
    return 1;

  return 0;
}
#endif

int main(void) {
  int failed = 0;
  printf("Running ui_input_mask tests...\n");

  failed |= test_mask_formatting();
#ifdef UI_TEST_MOCK_ALLOC
  failed |= test_mask_mock_failures();
#endif

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
