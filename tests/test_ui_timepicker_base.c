/* clang-format off */
#include "ui_timepicker_base.h"
#include "ui_error.h"
#include "ui_control_value_accessor.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#define ACCUM_ERR(failed, expr) failed |= ((expr) != UI_ERROR_NONE)
#define ACCUM_FAIL(failed, expr) failed |= (expr)

static int g_change_called = 0;
static int g_touched_called = 0;
static int g_last_seconds = -1;

static int g_mock_cb_fail = 0;

static ui_error_t on_change(union ui_signal_payload new_value,
                            void *user_data) {
  (void)user_data;
  if (g_mock_cb_fail == 1)
    return UI_ERROR_UNKNOWN;
  g_change_called++;
  g_last_seconds = new_value.int_val;
  return UI_ERROR_NONE;
}

static ui_error_t on_touched(void *user_data) {
  (void)user_data;
  if (g_mock_cb_fail == 2)
    return UI_ERROR_UNKNOWN;
  g_touched_called++;
  return UI_ERROR_NONE;
}

static int test_normal(void) {
  struct ui_timepicker_base *tp = NULL;
  struct ui_control_value_accessor cva;
  int h, m;
  enum ui_timepicker_format fmt;
  enum ui_timepicker_period per;
  char *str = NULL;
  union ui_signal_payload val;
  int failed = 0;
  memset(&val, 0, sizeof(val));

  failed |=
      (ui_timepicker_base_create(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT);
  ui_timepicker_base_destroy(NULL);

  ACCUM_ERR(failed, ui_timepicker_base_create(&tp, &cva));

  failed |=
      (ui_timepicker_base_set_time(NULL, 10, 30) != UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_timepicker_base_get_time(NULL, &h, &m) != UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_timepicker_base_get_time(tp, NULL, &m) != UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_timepicker_base_get_time(tp, &h, NULL) != UI_ERROR_INVALID_ARGUMENT);

  failed |= (ui_timepicker_base_get_formatted_time(NULL, &h, &m, &per) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_timepicker_base_get_formatted_time(tp, NULL, &m, &per) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_timepicker_base_get_formatted_time(tp, &h, NULL, &per) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_timepicker_base_get_formatted_time(tp, &h, &m, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);

  failed |= (ui_timepicker_base_set_format(NULL, UI_TIMEPICKER_FORMAT_12H) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_timepicker_base_get_format(NULL, &fmt) != UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_timepicker_base_get_format(tp, NULL) != UI_ERROR_INVALID_ARGUMENT);

  failed |= (ui_timepicker_base_get_time_string(NULL, &str) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_timepicker_base_get_time_string(tp, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);

  /* Set time clamping */
  ACCUM_ERR(failed, ui_timepicker_base_set_time(tp, -5, -10));
  ACCUM_ERR(failed, ui_timepicker_base_get_time(tp, &h, &m));
  failed |= (h != 0 || m != 0);

  ACCUM_ERR(failed, ui_timepicker_base_set_time(tp, 25, 65));
  ACCUM_ERR(failed, ui_timepicker_base_get_time(tp, &h, &m));
  failed |= (h != 23 || m != 59);

  ACCUM_ERR(failed, ui_timepicker_base_set_time(tp, 10, 30));

  /* Formats */
  ACCUM_ERR(failed, ui_timepicker_base_get_format(tp, &fmt));
  failed |= (fmt != UI_TIMEPICKER_FORMAT_24H);

  ACCUM_ERR(failed, ui_timepicker_base_get_time_string(tp, &str));
  failed |= (strcmp(str, "10:30") != 0);
  free(str);
  str = NULL;

  ACCUM_ERR(failed,
            ui_timepicker_base_set_format(tp, UI_TIMEPICKER_FORMAT_12H));
  ACCUM_ERR(failed, ui_timepicker_base_get_format(tp, &fmt));
  failed |= (fmt != UI_TIMEPICKER_FORMAT_12H);

  ACCUM_ERR(failed, ui_timepicker_base_get_time_string(tp, &str));
  failed |= (strcmp(str, "10:30 AM") != 0);
  free(str);
  str = NULL;

  /* Formatted time tests */
  ui_timepicker_base_set_time(tp, 0, 0);
  ui_timepicker_base_get_formatted_time(tp, &h, &m, &per);
  failed |= (h != 12 || per != UI_TIMEPICKER_PERIOD_AM);

  ui_timepicker_base_set_time(tp, 12, 0);
  ui_timepicker_base_get_formatted_time(tp, &h, &m, &per);
  failed |= (h != 12 || per != UI_TIMEPICKER_PERIOD_PM);

  ui_timepicker_base_set_time(tp, 14, 0);
  ui_timepicker_base_get_formatted_time(tp, &h, &m, &per);
  failed |= (h != 2 || per != UI_TIMEPICKER_PERIOD_PM);

  ui_timepicker_base_set_time(tp, 9, 0);
  ui_timepicker_base_get_formatted_time(tp, &h, &m, &per);
  failed |= (h != 9 || per != UI_TIMEPICKER_PERIOD_AM);

  ACCUM_ERR(failed, ui_timepicker_base_get_time_string(tp, &str));
  failed |= (strcmp(str, "09:00 AM") != 0);
  free(str);
  str = NULL;

  /* Test CVA */
  failed |= (cva.register_on_change(NULL, on_change, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (cva.register_on_touched(NULL, on_touched, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (cva.write_value(NULL, val) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (cva.set_disabled_state(NULL, 1) != UI_ERROR_INVALID_ARGUMENT);

  ACCUM_ERR(failed, cva.register_on_change(tp, on_change, NULL));
  ACCUM_ERR(failed, cva.register_on_touched(tp, on_touched, NULL));

  /* Trigger changes */
  g_change_called = 0;
  g_touched_called = 0;
  ui_timepicker_base_set_time(tp, 11, 15);
  failed |= (g_change_called != 1 || g_touched_called != 1);
  failed |= (g_last_seconds != (11 * 3600 + 15 * 60));

  /* Test cb failures */
  g_mock_cb_fail = 2; /* touched fails */
  failed |= (ui_timepicker_base_set_time(tp, 12, 15) != UI_ERROR_UNKNOWN);
  g_mock_cb_fail = 1; /* change fails */
  failed |= (ui_timepicker_base_set_time(tp, 12, 16) != UI_ERROR_UNKNOWN);
  g_mock_cb_fail = 0;

  /* No change */
  ui_timepicker_base_set_time(tp, 12, 16);
  failed |= (g_change_called != 1 || g_touched_called != 2);

  /* CVA write value */
  val.int_val = -100;
  ACCUM_ERR(failed, cva.write_value(tp, val));
  ui_timepicker_base_get_time(tp, &h, &m);
  failed |= (h != 0 || m != 0);

  val.int_val = 14 * 3600 + 45 * 60;
  ACCUM_ERR(failed, cva.write_value(tp, val));
  ui_timepicker_base_get_time(tp, &h, &m);
  failed |= (h != 14 || m != 45);

  /* CVA disabled state */
  ACCUM_ERR(failed, cva.set_disabled_state(tp, 1));

  ui_timepicker_base_destroy(tp);
  return failed;
}

static int test_oom(void) {
  int failed = 0;
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_timepicker_base *tp;
  char *str = NULL;

  g_malloc_fail_countdown = 0;
  failed |= (ui_timepicker_base_create(&tp, NULL) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  ui_timepicker_base_create(&tp, NULL);

  g_malloc_fail_countdown = 0;
  failed |=
      (ui_timepicker_base_get_time_string(tp, &str) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  ui_timepicker_base_destroy(tp);
#endif
  return failed;
}

static int run_string_fail_tests(void) { return 0; }

int main(void) {
  int failed = 0;
  failed |= test_normal();
  failed |= test_oom();
  failed |= run_string_fail_tests();
  if (!failed) {
    printf("All ui_timepicker_base tests passed.\n");
  }
  return failed;
}
