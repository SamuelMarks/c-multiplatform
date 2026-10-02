/**
 * @file test_cupertino_text_field.c
 * @brief Comprehensive tests for Cupertino Text Field & Text View components.
 */

#include "greatest.h"
#include "ui_test_mock_mem.h"

/* clang-format off */
#include "cupertino/cupertino_text_field.h"
#include "ui_engine.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

TEST test_cupertino_text_field_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_text_field *field = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_input_base *base = NULL;
  const char *text = NULL;
  int is_visible = 0;
  union ui_signal_payload payload;
  ui_error_t rc;

  /* Null checks */
  rc = cupertino_text_field_create(NULL, &field, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_field_create(dummy_engine, NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_field_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_field_set_text(NULL, "text");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_field_get_text(NULL, &text);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_field_set_placeholder(NULL, "hint");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_field_set_clear_button_mode(NULL,
                                                  CUPERTINO_OVERLAY_ALWAYS);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_field_is_clear_button_visible(NULL, &is_visible);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_field_trigger_clear(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_field_set_secure_text_entry(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_field_set_editing(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_field_set_prefix(NULL, "$");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_field_set_suffix(NULL, ".00");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_field_set_disabled(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_field_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid single-line creation */
  rc = cupertino_text_field_create(dummy_engine, &field, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(field != NULL);
  ASSERT(cva != NULL);
  ASSERT_EQ(0, field->is_multiline);

  rc = cupertino_text_field_get_text(field, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_field_get_text(field, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", text);

  rc = cupertino_text_field_get_base(field, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_field_get_base(field, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Text & Placeholder operations */
  rc = cupertino_text_field_set_text(field, "Hello Cupertino");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_text_field_get_text(field, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Hello Cupertino", text);

  rc = cupertino_text_field_set_text(field, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_text_field_get_text(field, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", text);

  rc = cupertino_text_field_set_placeholder(field,
                                            "Search or enter website name");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Search or enter website name", field->placeholder);

  rc = cupertino_text_field_set_placeholder(field, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", field->placeholder);

  /* Prefix & Suffix operations */
  rc = cupertino_text_field_set_prefix(field, "https://");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("https://", field->prefix_text);

  rc = cupertino_text_field_set_prefix(field, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", field->prefix_text);

  rc = cupertino_text_field_set_suffix(field, ".com");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ(".com", field->suffix_text);

  rc = cupertino_text_field_set_suffix(field, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", field->suffix_text);

  /* Secure text entry */
  rc = cupertino_text_field_set_secure_text_entry(field, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, field->is_secure_text_entry);

  rc = cupertino_text_field_set_secure_text_entry(field, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, field->is_secure_text_entry);

  /* Clear button modes & visibility evaluation */
  rc = cupertino_text_field_set_clear_button_mode(
      field, CUPERTINO_OVERLAY_VISIBILITY_COUNT);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_field_is_clear_button_visible(field, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Empty text -> always hidden regardless of mode */
  rc = cupertino_text_field_set_clear_button_mode(field,
                                                  CUPERTINO_OVERLAY_ALWAYS);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_text_field_is_clear_button_visible(field, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_visible);

  /* Set text */
  rc = cupertino_text_field_set_text(field, "Active input");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mode: ALWAYS */
  rc = cupertino_text_field_is_clear_button_visible(field, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_visible);

  /* Mode: NEVER */
  rc = cupertino_text_field_set_clear_button_mode(field,
                                                  CUPERTINO_OVERLAY_NEVER);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_text_field_is_clear_button_visible(field, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_visible);

  /* Mode: WHILE_EDITING */
  rc = cupertino_text_field_set_clear_button_mode(
      field, CUPERTINO_OVERLAY_WHILE_EDITING);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_text_field_set_editing(field, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_text_field_is_clear_button_visible(field, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_visible);

  rc = cupertino_text_field_set_editing(field, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_text_field_is_clear_button_visible(field, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_visible);

  /* Mode: UNLESS_EDITING */
  rc = cupertino_text_field_set_clear_button_mode(
      field, CUPERTINO_OVERLAY_UNLESS_EDITING);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_text_field_is_clear_button_visible(field, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_visible);

  rc = cupertino_text_field_set_editing(field, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_text_field_is_clear_button_visible(field, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_visible);

  /* Mode: invalid mode enum triggers default: *out_visible = 0 */
  field->clear_button_mode = (enum cupertino_overlay_visibility_mode)999;
  rc = cupertino_text_field_is_clear_button_visible(field, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_visible);

  /* Trigger clear */
  rc = cupertino_text_field_trigger_clear(field);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_text_field_get_text(field, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", text);

  /* Destroy with field->input == NULL */
  {
    struct ui_input_base *saved_in = field->input;
    field->input = NULL;
    rc = cupertino_text_field_destroy(field);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ui_input_base_destroy(saved_in);
    field = NULL;
  }

  /* Recreate field for subsequent tests */
  rc = cupertino_text_field_create(dummy_engine, &field, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Disabled state */
  rc = cupertino_text_field_set_disabled(field, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_text_field_set_disabled(field, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* CVA methods */
  payload.ptr_val = (void *)"Written via CVA";
  rc = cva->write_value(field, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_text_field_get_text(field, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Written via CVA", text);

  payload.ptr_val = NULL;
  rc = cva->write_value(field, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_text_field_get_text(field, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", text);

  rc = cva->write_value(NULL, payload);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cva->set_disabled_state(field, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cva->set_disabled_state(field, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cva->set_disabled_state(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_field_destroy(field);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_cupertino_text_view_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_text_field *view = NULL;
  struct ui_control_value_accessor *cva = NULL;
  const char *text = NULL;
  ui_error_t rc;

  /* Null checks */
  rc = cupertino_text_view_create(NULL, 3, 10, &view, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_view_create(dummy_engine, 3, 10, NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid multi-line creation */
  rc = cupertino_text_view_create(dummy_engine, 3, 10, &view, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(view != NULL);
  ASSERT(cva != NULL);
  ASSERT_EQ(1, view->is_multiline);
  ASSERT_EQ((size_t)3, view->min_lines);
  ASSERT_EQ((size_t)10, view->max_lines);

  rc = cupertino_text_field_set_text(view, "Line 1\nLine 2\nLine 3");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_text_field_get_text(view, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Line 1\nLine 2\nLine 3", text);

  rc = cupertino_text_field_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Valid creation without out_cva */
  rc = cupertino_text_view_create(dummy_engine, 1, 5, &view, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(view != NULL);

  rc = cupertino_text_field_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_cupertino_text_field_oom(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_text_field *field = NULL;
  int is_visible = 0;
  ui_error_t rc;

  /* Root allocation failure */
  g_malloc_fail_countdown = 0;
  rc = cupertino_text_field_create(dummy_engine, &field, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Wrapped input base allocation failure */
  g_malloc_fail_countdown = 1;
  rc = cupertino_text_field_create(dummy_engine, &field, NULL);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_malloc_fail_countdown = -1;

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_text_field_mock_destroy_fail;
  extern int g_cupertino_text_field_mock_set_placeholder_fail;
  extern int g_cupertino_text_field_mock_get_text_fail;
  extern int g_cupertino_text_field_mock_set_type_fail;

  rc = cupertino_text_field_create(dummy_engine, &field, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock set_placeholder failure */
  g_cupertino_text_field_mock_set_placeholder_fail = 1;
  rc = cupertino_text_field_set_placeholder(field, "Test");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_text_field_mock_set_placeholder_fail = 0;

  /* Mock get_text failure in is_clear_button_visible */
  g_cupertino_text_field_mock_get_text_fail = 1;
  rc = cupertino_text_field_is_clear_button_visible(field, &is_visible);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_text_field_mock_get_text_fail = 0;

  /* Mock get_text returning NULL text in is_clear_button_visible */
  {
    extern int g_cupertino_text_field_mock_get_text_null;
    g_cupertino_text_field_mock_get_text_null = 1;
    rc = cupertino_text_field_is_clear_button_visible(field, &is_visible);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(0, is_visible);
    g_cupertino_text_field_mock_get_text_null = 0;
  }

  /* Mock set_type failure */
  g_cupertino_text_field_mock_set_type_fail = 1;
  rc = cupertino_text_field_set_secure_text_entry(field, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_text_field_mock_set_type_fail = 0;

  /* Mock destroy failure */
  g_cupertino_text_field_mock_destroy_fail = 1;
  rc = cupertino_text_field_destroy(field);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_text_field_mock_destroy_fail = 0;

  rc = cupertino_text_field_destroy(field);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#endif

  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();

  RUN_TEST(test_cupertino_text_field_lifecycle);
  RUN_TEST(test_cupertino_text_view_lifecycle);
  RUN_TEST(test_cupertino_text_field_oom);

  GREATEST_MAIN_END();
}
