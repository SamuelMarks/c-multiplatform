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
  if (user_data) {
  }
  return g_change_rc;
}

static ui_error_t on_touched(void *user_data) {
  g_touched_called++;
  if (user_data) {
  }
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
  assert(ui_pin_input_base_on_input(pin_input, 0, "5") ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_pin_input_base_on_backspace(pin_input, 0) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_pin_input_base_on_paste(pin_input, "1") ==
         UI_ERROR_INVALID_ARGUMENT);
  g_change_rc = UI_ERROR_NONE;

  /* Fail touched */
  g_touched_rc = UI_ERROR_INVALID_ARGUMENT;
  assert(ui_pin_input_base_on_input(pin_input, 0, "5") ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_pin_input_base_on_backspace(pin_input, 0) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_pin_input_base_on_paste(pin_input, "1") ==
         UI_ERROR_INVALID_ARGUMENT);
  g_touched_rc = UI_ERROR_NONE;

  {
    ui_error_t rc_cleanup = ui_pin_input_base_destroy(pin_input);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
  {
    ui_error_t rc_cleanup = ui_pin_input_base_destroy(NULL);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
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

  {
    ui_error_t rc_cleanup = ui_pin_input_base_destroy(dummy);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
}

static void test_pin_input_oom(void) {
  struct ui_pin_input_base *pin_input;
  int i;
  for (i = 0; i < 100; i++) {
    g_malloc_fail_countdown = i;
    if (ui_pin_input_base_create(&pin_input, 4, NULL) == UI_ERROR_NONE) {
      ui_error_t rc_cleanup = ui_pin_input_base_destroy(pin_input);
      assert(rc_cleanup == UI_ERROR_NONE);
    }
  }
  g_malloc_fail_countdown = -1;

  if (ui_pin_input_base_create(&pin_input, 4, NULL) == UI_ERROR_NONE) {
    struct ui_control_value_accessor cva;
    ui_pin_input_base_get_component(pin_input,
                                    NULL); /* Ensure not null internally */
    {
      ui_error_t rc_cleanup = ui_pin_input_base_destroy(pin_input);
      assert(rc_cleanup == UI_ERROR_NONE);
    }
  }

  {
    struct ui_control_value_accessor cva;
    if (ui_pin_input_base_create(&pin_input, 4, &cva) == UI_ERROR_NONE) {
      for (i = 0; i < 5; i++) {
        ui_error_t rc_input;
        g_malloc_fail_countdown = i;
        rc_input = ui_pin_input_base_on_input(pin_input, 0, "1");
        assert(rc_input == UI_ERROR_NONE || rc_input == UI_ERROR_OUT_OF_MEMORY);
        g_malloc_fail_countdown = -1;
      }
      for (i = 0; i < 5; i++) {
        ui_error_t rc_bs;
        g_malloc_fail_countdown = i;
        rc_bs = ui_pin_input_base_on_backspace(pin_input, 1);
        assert(rc_bs == UI_ERROR_NONE || rc_bs == UI_ERROR_OUT_OF_MEMORY);
        g_malloc_fail_countdown = -1;
      }
      for (i = 0; i < 5; i++) {
        ui_error_t rc_paste;
        g_malloc_fail_countdown = i;
        rc_paste = ui_pin_input_base_on_paste(pin_input, "123");
        assert(rc_paste == UI_ERROR_NONE || rc_paste == UI_ERROR_OUT_OF_MEMORY);
        g_malloc_fail_countdown = -1;
      }
      for (i = 0; i < 5; i++) {
        ui_error_t rc_dis;
        g_malloc_fail_countdown = i;
        rc_dis = cva.set_disabled_state(pin_input, 1);
        assert(rc_dis == UI_ERROR_NONE || rc_dis == UI_ERROR_OUT_OF_MEMORY);
        g_malloc_fail_countdown = -1;
      }
      {
        ui_error_t rc_cleanup = ui_pin_input_base_destroy(pin_input);
        assert(rc_cleanup == UI_ERROR_NONE);
      }
    }
  }
}

#ifdef UI_TEST_MOCK_ALLOC
extern int g_pin_input_mock_comp_destroy_fail;
extern int g_pin_input_mock_set_style_fail;

static void test_pin_input_mocks(void) {
  struct ui_pin_input_base *pin_input = NULL;
  ui_error_t rc;

  /* Creation style failure */
  g_pin_input_mock_set_style_fail = 1;
  rc = ui_pin_input_base_create(&pin_input, 4, NULL);
  g_pin_input_mock_set_style_fail = 0;
  assert(rc != UI_ERROR_NONE);
  assert(pin_input == NULL);

  /* Component destroy failure in destroy */
  rc = ui_pin_input_base_create(&pin_input, 4, NULL);
  assert(rc == UI_ERROR_NONE);
  assert(pin_input != NULL);
  g_pin_input_mock_comp_destroy_fail = 1;
  rc = ui_pin_input_base_destroy(pin_input);
  g_pin_input_mock_comp_destroy_fail = 0;
  assert(rc != UI_ERROR_NONE);
}
#endif

int main(void) {
  struct ui_pin_input_base *unregistered = NULL;
  test_pin_input_creation_and_events();
  test_pin_input_nulls();
  test_pin_input_oom();
#ifdef UI_TEST_MOCK_ALLOC
  test_pin_input_mocks();
#endif
  /* Events without CVA */
  assert(ui_pin_input_base_create(&unregistered, 4, NULL) == UI_ERROR_NONE);
  assert(ui_pin_input_base_on_input(unregistered, 0, "1") == UI_ERROR_NONE);
  assert(ui_pin_input_base_on_backspace(unregistered, 0) == UI_ERROR_NONE);
  assert(ui_pin_input_base_on_paste(unregistered, "12") == UI_ERROR_NONE);
  {
    ui_error_t rc_cleanup = ui_pin_input_base_destroy(unregistered);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  printf("test_ui_pin_input_base passed\n");
  return 0;
}
