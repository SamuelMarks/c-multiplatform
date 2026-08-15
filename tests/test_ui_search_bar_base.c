/* clang-format off */
#include <stdio.h>
#include <string.h>

#include "../include/ui_search_bar_base.h"
#include "../include/ui_error.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

struct ui_component {
  int id;
};

static int mock_change_called = 0;
static const char *mock_change_last_query = NULL;
static void *mock_change_user_data = NULL;

static int mock_touched_called = 0;
static void *mock_touched_user_data = NULL;

static ui_error_t mock_on_change(union ui_signal_payload new_value,
                                 void *user_data) {
  mock_change_called = 1;
  mock_change_last_query = (const char *)new_value.ptr_val;
  mock_change_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t mock_on_touched(void *user_data) {
  mock_touched_called = 1;
  mock_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

static int test_search_bar_init(void) {
  struct ui_search_bar_base sb;
  struct ui_component comp;
  struct ui_control_value_accessor cva;
  ui_error_t err;

  err = ui_search_bar_base_init(NULL, &comp, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  err = ui_search_bar_base_init(&sb, NULL, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  err = ui_search_bar_base_init(&sb, &comp, NULL);
  if (err != UI_ERROR_NONE)
    return 1;
  if (sb.component != &comp)
    return 1;
  if (sb.query != NULL)
    return 1;
  if (sb.is_loading != 0)
    return 1;

  err = ui_search_bar_base_init(&sb, &comp, &cva);
  if (err != UI_ERROR_NONE)
    return 1;
  if (cva.write_value == NULL)
    return 1;
  if (cva.register_on_change == NULL)
    return 1;
  if (cva.register_on_touched == NULL)
    return 1;
  if (cva.set_disabled_state == NULL)
    return 1;

  return 0;
}

static int test_search_bar_operations(void) {
  struct ui_search_bar_base sb;
  struct ui_component comp;
  ui_error_t err;

  (void)ui_search_bar_base_init(&sb, &comp, NULL);

  err = ui_search_bar_base_set_query(NULL, "hello");
  if (err != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  err = ui_search_bar_base_set_query(&sb, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  err = ui_search_bar_base_set_query(&sb, "hello world");
  if (err != UI_ERROR_NONE)
    return 1;
  if (strcmp(sb.query, "hello world") != 0)
    return 1;

  /* Test setting when query already exists */
  err = ui_search_bar_base_set_query(&sb, "new query");
  if (err != UI_ERROR_NONE)
    return 1;
  if (strcmp(sb.query, "new query") != 0)
    return 1;

  err = ui_search_bar_base_set_loading(NULL, 1);
  if (err != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  err = ui_search_bar_base_set_loading(&sb, 1);
  if (err != UI_ERROR_NONE)
    return 1;
  if (sb.is_loading != 1)
    return 1;

  err = ui_search_bar_base_set_loading(&sb, 0);
  if (err != UI_ERROR_NONE)
    return 1;
  if (sb.is_loading != 0)
    return 1;

  err = ui_search_bar_base_cleanup(NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* cleanup with null query */
  struct ui_search_bar_base sb2;
  (void)ui_search_bar_base_init(&sb2, &comp, NULL);
  (void)ui_search_bar_base_cleanup(&sb2);

  err = ui_search_bar_base_cleanup(&sb);
  if (err != UI_ERROR_NONE)
    return 1;
  if (sb.query != NULL)
    return 1;

  /* Test OOM */
  (void)ui_search_bar_base_init(&sb, &comp, NULL);
  g_malloc_fail_countdown = 0;
  err = ui_search_bar_base_set_query(&sb, "oom text");
  if (err != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  extern int g_mock_strcpy_fail;
  g_mock_strcpy_fail = 1;
  {
    ui_error_t _ign = ui_search_bar_base_set_query(&sb, "strcpy fail");
    (void)_ign;
  }
  g_mock_strcpy_fail = 0;

  (void)ui_search_bar_base_cleanup(&sb);

  return 0;
}

static int test_search_bar_cva(void) {
  struct ui_search_bar_base sb;
  struct ui_component comp;
  struct ui_control_value_accessor cva;
  union ui_signal_payload val;
  ui_error_t err;
  void *ud1 = (void *)0x111;
  void *ud2 = (void *)0x222;

  (void)ui_search_bar_base_init(&sb, &comp, &cva);

  /* Test invalid args to CVA functions */
  err = cva.write_value(NULL, val);
  if (err != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  err = cva.register_on_change(NULL, mock_on_change, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  err = cva.register_on_touched(NULL, mock_on_touched, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  err = cva.set_disabled_state(NULL, 1);
  if (err != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test valid args */
  err = cva.register_on_change(&sb, mock_on_change, ud1);
  if (err != UI_ERROR_NONE)
    return 1;

  err = cva.register_on_touched(&sb, mock_on_touched, ud2);
  if (err != UI_ERROR_NONE)
    return 1;

  err = cva.set_disabled_state(&sb, 1);
  if (err != UI_ERROR_NONE)
    return 1;
  if (sb.is_disabled != 1)
    return 1;

  /* Test write_value with string */
  val.ptr_val = (void *)"cva test";
  mock_change_called = 0;
  mock_touched_called = 0;
  err = cva.write_value(&sb, val);
  if (err != UI_ERROR_NONE)
    return 1;
  if (strcmp(sb.query, "cva test") != 0)
    return 1;
  if (!mock_change_called)
    return 1;
  if (strcmp(mock_change_last_query, "cva test") != 0)
    return 1;
  if (mock_change_user_data != ud1)
    return 1;
  if (!mock_touched_called)
    return 1;
  if (mock_touched_user_data != ud2)
    return 1;

  /* Test write_value with NULL string */
  val.ptr_val = NULL;
  err = cva.write_value(&sb, val);
  if (err != UI_ERROR_NONE)
    return 1;
  if (strcmp(sb.query, "") != 0)
    return 1;

  (void)ui_search_bar_base_cleanup(&sb);
  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_search_bar_base tests...\n");

  failed |= test_search_bar_init();
  failed |= test_search_bar_operations();
  failed |= test_search_bar_cva();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
