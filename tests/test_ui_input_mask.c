/* clang-format off */
#include "../include/ui_input_mask.h"
#include "../include/ui_input_base.h"
#include "../include/ui_error.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

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
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_input_mask tests...\n");

  failed |= test_mask_formatting();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
