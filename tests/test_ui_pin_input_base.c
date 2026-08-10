/* clang-format off */
#include "ui_pin_input_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int g_change_called = 0;
static const char *g_change_value = NULL;
static int g_touched_called = 0;

static ui_error_t g_change_rc = UI_ERROR_NONE;
static ui_error_t g_touched_rc = UI_ERROR_NONE;

static ui_error_t on_change(union ui_signal_payload new_value,
                            void *user_data) {
  g_change_called++;
  g_change_value = (const char *)new_value.ptr_val;
  (void)user_data;
  return g_change_rc;
}

static ui_error_t on_touched(void *user_data) {
  g_touched_called++;
  (void)user_data;
  return g_touched_rc;
}

static void test_pin_input_creation_and_events(void) {
  struct ui_pin_input_base *pin_input = NULL;
  struct ui_control_value_accessor cva;
  struct ui_component *comp = NULL;
  union ui_signal_payload payload;

  /* Invalid creations */
  assert(ui_pin_input_base_create(NULL, 4, NULL) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_pin_input_base_create(&pin_input, 0, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_pin_input_base_create(&pin_input, -1, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);

  assert(ui_pin_input_base_create(&pin_input, 4, &cva) == UI_ERROR_NONE);

  assert(ui_pin_input_base_get_component(pin_input, &comp) == UI_ERROR_NONE);
  assert(comp != NULL);

  assert(cva.register_on_change(NULL, on_change, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  cva.register_on_change(pin_input, on_change, NULL);
  cva.register_on_touched(pin_input, on_touched, NULL);

  /* Set value */
  payload.ptr_val = "12345";
  cva.write_value(pin_input, payload);

  payload.ptr_val = "12";
  cva.write_value(pin_input, payload);

  payload.ptr_val = NULL;
  cva.write_value(pin_input, payload);

  /* Input */
  g_change_called = 0;
  g_touched_called = 0;
  ui_pin_input_base_on_input(pin_input, 0, "5");
  assert(g_change_called == 1);
  assert(g_touched_called == 1);
  assert(g_change_value != NULL && g_change_value[0] == '5');

  /* Out of bounds input */
  ui_pin_input_base_on_input(pin_input, 4, "5");
  ui_pin_input_base_on_input(pin_input, -1, "5");

  /* Backspace */
  ui_pin_input_base_on_backspace(pin_input, 0);
  assert(g_change_value[0] == '\0');

  /* Out of bounds backspace */
  ui_pin_input_base_on_backspace(pin_input, 4);
  ui_pin_input_base_on_backspace(pin_input, -1);

  /* Paste */
  ui_pin_input_base_on_paste(pin_input, "12345");
  assert(g_change_value[0] == '1');
  assert(g_change_value[3] == '4');
  assert(g_change_value[4] == '\0');

  /* Disable */
  cva.set_disabled_state(pin_input, 0);
  cva.set_disabled_state(pin_input, 1);

  /* Events while disabled */
  g_change_called = 0;
  ui_pin_input_base_on_input(pin_input, 0, "5");
  ui_pin_input_base_on_backspace(pin_input, 0);
  ui_pin_input_base_on_paste(pin_input, "1");
  assert(g_change_called == 0);

  /* Trigger callback errors */
  cva.set_disabled_state(pin_input, 0); /* RE-ENABLE! */

  /* Fail change */
  g_change_rc = UI_ERROR_INVALID_ARGUMENT;
  (void)ui_pin_input_base_on_input(pin_input, 0, "5");
  (void)ui_pin_input_base_on_backspace(pin_input, 0);
  (void)ui_pin_input_base_on_paste(pin_input, "1");
  g_change_rc = UI_ERROR_NONE;

  /* Fail touched */
  g_touched_rc = UI_ERROR_INVALID_ARGUMENT;
  (void)ui_pin_input_base_on_input(pin_input, 0, "5");
  (void)ui_pin_input_base_on_backspace(pin_input, 0);
  (void)ui_pin_input_base_on_paste(pin_input, "1");
  g_touched_rc = UI_ERROR_NONE;

  (void)ui_pin_input_base_destroy(pin_input);
  (void)ui_pin_input_base_destroy(NULL);
}

static void test_pin_input_nulls(void) {
  struct ui_control_value_accessor cva;
  struct ui_component *comp;
  union ui_signal_payload payload;
  struct ui_pin_input_base *dummy = NULL;

  ui_pin_input_base_create(&dummy, 4, &cva);
  payload.ptr_val = NULL;

  /* CVA functions with NULL */
  assert(cva.write_value(NULL, payload) == UI_ERROR_INVALID_ARGUMENT);
  assert(cva.register_on_change(NULL, on_change, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(cva.register_on_touched(NULL, on_touched, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(cva.set_disabled_state(NULL, 1) == UI_ERROR_INVALID_ARGUMENT);

  assert(ui_pin_input_base_get_component(NULL, &comp) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_pin_input_base_get_component(dummy, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);

  assert(ui_pin_input_base_on_input(NULL, 0, "5") == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_pin_input_base_on_input(dummy, 0, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);

  assert(ui_pin_input_base_on_backspace(NULL, 0) == UI_ERROR_INVALID_ARGUMENT);

  assert(ui_pin_input_base_on_paste(NULL, "5") == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_pin_input_base_on_paste(dummy, NULL) == UI_ERROR_INVALID_ARGUMENT);

  (void)ui_pin_input_base_destroy(dummy);
}

static void test_pin_input_oom(void) {
  struct ui_pin_input_base *pin_input;
  int i;
  for (i = 0; i < 100; i++) {
    g_malloc_fail_countdown = i;
    if (ui_pin_input_base_create(&pin_input, 4, NULL) == UI_ERROR_NONE) {
      (void)ui_pin_input_base_destroy(pin_input);
    }
  }
  g_malloc_fail_countdown = -1;

  if (ui_pin_input_base_create(&pin_input, 4, NULL) == UI_ERROR_NONE) {
    struct ui_control_value_accessor cva;
    ui_pin_input_base_get_component(pin_input,
                                    NULL); /* Ensure not null internally */
    /* Access internal CVA just for testing disabled state OOM */
    /* But set_disabled_state is a normal function internally
       `ui_pin_input_base_set_disabled_state`? No, it is exposed via CVA. We
       need the cva. */
    (void)ui_pin_input_base_destroy(pin_input);
  }

  {
    struct ui_control_value_accessor cva;
    if (ui_pin_input_base_create(&pin_input, 4, &cva) == UI_ERROR_NONE) {
      for (i = 0; i < 5; i++) {
        g_malloc_fail_countdown = i;
        (void)ui_pin_input_base_on_input(pin_input, 0, "1");
        g_malloc_fail_countdown = -1;
      }
      for (i = 0; i < 5; i++) {
        g_malloc_fail_countdown = i;
        (void)ui_pin_input_base_on_backspace(pin_input, 1);
        g_malloc_fail_countdown = -1;
      }
      for (i = 0; i < 5; i++) {
        g_malloc_fail_countdown = i;
        (void)ui_pin_input_base_on_paste(pin_input, "123");
        g_malloc_fail_countdown = -1;
      }
      for (i = 0; i < 5; i++) {
        g_malloc_fail_countdown = i;
        (void)cva.set_disabled_state(pin_input, 1);
        g_malloc_fail_countdown = -1;
      }
      (void)ui_pin_input_base_destroy(pin_input);
    }
  }
}

/* Trick: Need to access static CVA functions, but they are static.
   Wait, they are assigned to out_cva! So we can test them via cva structure!
   The test_pin_input_nulls above calls them directly. We can't do that.
   I will fix this. */

int main(void) {
  test_pin_input_creation_and_events();
  test_pin_input_nulls();
  test_pin_input_oom();
  /* Events without CVA */
  struct ui_pin_input_base *unregistered = NULL;
  ui_pin_input_base_create(&unregistered, 4, NULL);
  ui_pin_input_base_on_input(unregistered, 0, "1");
  ui_pin_input_base_on_backspace(unregistered, 0);
  ui_pin_input_base_on_paste(unregistered, "12");
  (void)ui_pin_input_base_destroy(unregistered);

  printf("test_ui_pin_input_base passed\n");
  return 0;
}
