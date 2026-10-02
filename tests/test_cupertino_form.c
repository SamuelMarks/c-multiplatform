/**
 * @file test_cupertino_form.c
 * @brief Comprehensive tests for CupertinoFormSection, CupertinoFormRow, and
 * CupertinoTextFormFieldRow components.
 */

#include "greatest.h"
#include "ui_test_mock_mem.h"

/* clang-format off */
#include "cupertino/cupertino_form.h"
#include "ui_engine.h"
#include "ui_form_control.h"
#include "ui_form_validators.h"
#include "ui_reactor.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

TEST test_cupertino_form_section_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_form_section *section = NULL;
  struct ui_card_base *base = NULL;
  struct ui_component *content = NULL;
  ui_error_t rc;

  rc = ui_component_create(&content);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = cupertino_form_section_create(NULL, CUPERTINO_FORM_SECTION_PLAIN,
                                     &section);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_form_section_create(
      dummy_engine, (enum cupertino_form_section_style) - 1, &section);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_form_section_create(
      dummy_engine, CUPERTINO_FORM_SECTION_STYLE_COUNT, &section);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_form_section_create(dummy_engine, CUPERTINO_FORM_SECTION_PLAIN,
                                     NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_form_section_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_form_section_set_header(NULL, "Header");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_form_section_set_footer(NULL, "Footer");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_form_section_set_content(NULL, content);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_form_section_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid Plain creation */
  rc = cupertino_form_section_create(dummy_engine, CUPERTINO_FORM_SECTION_PLAIN,
                                     &section);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(section != NULL);
  ASSERT_EQ(CUPERTINO_FORM_SECTION_PLAIN, section->style);
  ASSERT_EQ_FMT(0.0f, section->corner_radius, "%f");
  ASSERT_EQ_FMT(0.0f, section->margin, "%f");

  rc = cupertino_form_section_get_base(section, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_form_section_get_base(section, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = cupertino_form_section_set_header(section, "ACCOUNT SETTINGS");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("ACCOUNT SETTINGS", section->header_text);

  rc = cupertino_form_section_set_header(section, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", section->header_text);

  rc = cupertino_form_section_set_footer(
      section, "Configure your primary account info.");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Configure your primary account info.", section->footer_text);

  rc = cupertino_form_section_set_footer(section, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", section->footer_text);

  rc = cupertino_form_section_set_content(section, content);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_form_section_destroy(section);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Valid Inset Grouped creation */
  rc = cupertino_form_section_create(
      dummy_engine, CUPERTINO_FORM_SECTION_INSET_GROUPED, &section);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(section != NULL);
  ASSERT_EQ(CUPERTINO_FORM_SECTION_INSET_GROUPED, section->style);
  ASSERT_EQ_FMT(10.0f, section->corner_radius, "%f");
  ASSERT_EQ_FMT(16.0f, section->margin, "%f");

  rc = cupertino_form_section_destroy(section);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_component_destroy(content);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_cupertino_form_row_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_form_row *row = NULL;
  struct ui_form_field_base *field = NULL;
  struct ui_component *child = NULL;
  ui_error_t rc;

  rc = ui_component_create(&child);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = cupertino_form_row_create(NULL, &row);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_form_row_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_form_row_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_form_row_set_prefix_label(NULL, "Name");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_form_row_set_helper_text(NULL, "Enter full name");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_form_row_set_error_text(NULL, "Name required");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_form_row_set_child(NULL, child);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_form_row_set_disabled(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_form_row_get_field(NULL, &field);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = cupertino_form_row_create(dummy_engine, &row);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(row != NULL);
  ASSERT_EQ(0, row->is_disabled);

  rc = cupertino_form_row_get_field(row, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_form_row_get_field(row, &field);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(field != NULL);

  rc = cupertino_form_row_set_prefix_label(row, "Username");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Username", row->prefix_label);

  rc = cupertino_form_row_set_prefix_label(row, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", row->prefix_label);

  rc = cupertino_form_row_set_helper_text(row, "Your public username");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Your public username", row->helper_text);

  rc = cupertino_form_row_set_helper_text(row, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", row->helper_text);

  rc = cupertino_form_row_set_error_text(row, "Username taken");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Username taken", row->error_text);

  rc = cupertino_form_row_set_error_text(row, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", row->error_text);

  rc = cupertino_form_row_set_child(row, child);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_form_row_set_disabled(row, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, row->is_disabled);

  rc = cupertino_form_row_set_disabled(row, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, row->is_disabled);

  rc = cupertino_form_row_destroy(row);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_component_destroy(child);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_cupertino_text_form_field_row_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_text_form_field_row *field_row = NULL;
  struct ui_control_value_accessor *cva = NULL;
  const char *text = NULL;
  union ui_signal_payload payload;
  ui_error_t rc;

  /* Null checks */
  rc = cupertino_text_form_field_row_create(NULL, &field_row, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_form_field_row_create(dummy_engine, NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_form_field_row_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_form_field_row_set_text(NULL, "test");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_form_field_row_get_text(NULL, &text);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_form_field_row_set_placeholder(NULL, "hint");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_form_field_row_bind_form_control(NULL, NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation with CVA */
  rc = cupertino_text_form_field_row_create(dummy_engine, &field_row, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(field_row != NULL);
  ASSERT(cva != NULL);
  ASSERT(cva->write_value != NULL);
  ASSERT(cva->set_disabled_state != NULL);

  rc = cupertino_text_form_field_row_get_text(field_row, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_form_field_row_get_text(field_row, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", text);

  rc = cupertino_text_form_field_row_set_text(field_row, "Johnny Appleseed");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Johnny Appleseed", field_row->text_value);

  rc = cupertino_text_form_field_row_get_text(field_row, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Johnny Appleseed", text);

  rc = cupertino_text_form_field_row_set_text(field_row, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", field_row->text_value);

  rc = cupertino_text_form_field_row_set_placeholder(field_row, "Required");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Required", field_row->placeholder);

  rc = cupertino_text_form_field_row_set_placeholder(field_row, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", field_row->placeholder);

  /* Form control binding */
  {
    struct ui_arena *arena = NULL;
    struct ui_form_control *ctrl = NULL;
    struct ui_reactor *reactor = NULL;
    union ui_signal_payload val;

    val.int_val = 0;
    rc = ui_arena_create(4096, &arena);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = ui_reactor_create(&reactor);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = ui_form_control_create(arena, val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                                UI_SIGNAL_MODE_SINGLE_THREADED, &ctrl);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = cupertino_text_form_field_row_bind_form_control(field_row, ctrl,
                                                         reactor);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(ctrl, field_row->form_control);
    ASSERT_EQ(reactor, field_row->reactor);

    rc = ui_reactor_destroy(reactor);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = ui_arena_destroy(arena);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* CVA methods */
  payload.ptr_val = (void *)"CVA Value";
  rc = cva->write_value(field_row, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("CVA Value", field_row->text_value);

  payload.ptr_val = NULL;
  rc = cva->write_value(field_row, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", field_row->text_value);

  rc = cva->write_value(NULL, payload);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cva->set_disabled_state(field_row, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, field_row->row->is_disabled);

  rc = cva->set_disabled_state(field_row, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, field_row->row->is_disabled);

  rc = cva->set_disabled_state(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_form_field_row_destroy(field_row);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Valid creation without out_cva */
  rc = cupertino_text_form_field_row_create(dummy_engine, &field_row, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(field_row != NULL);

  rc = cupertino_text_form_field_row_destroy(field_row);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_cupertino_form_oom(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_form_section *section = NULL;
  struct cupertino_form_row *row = NULL;
  struct cupertino_text_form_field_row *field_row = NULL;
  ui_error_t rc;

  /* Form Section OOM on root allocation */
  g_malloc_fail_countdown = 0;
  rc = cupertino_form_section_create(dummy_engine, CUPERTINO_FORM_SECTION_PLAIN,
                                     &section);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Form Section OOM on base card allocation */
  g_malloc_fail_countdown = 1;
  rc = cupertino_form_section_create(dummy_engine, CUPERTINO_FORM_SECTION_PLAIN,
                                     &section);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_malloc_fail_countdown = -1;

  /* Form Row OOM on root allocation */
  g_malloc_fail_countdown = 0;
  rc = cupertino_form_row_create(dummy_engine, &row);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Form Row OOM on inner field allocation */
  g_malloc_fail_countdown = 1;
  rc = cupertino_form_row_create(dummy_engine, &row);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_malloc_fail_countdown = -1;

  /* Text Form Field Row OOM on root allocation */
  g_malloc_fail_countdown = 0;
  rc = cupertino_text_form_field_row_create(dummy_engine, &field_row, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Text Form Field Row OOM during nested row/input creation */
  g_malloc_fail_countdown = 1;
  rc = cupertino_text_form_field_row_create(dummy_engine, &field_row, NULL);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 2;
  rc = cupertino_text_form_field_row_create(dummy_engine, &field_row, NULL);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_malloc_fail_countdown = -1;

  PASS();
}

extern int g_cupertino_form_mock_card_destroy_fail;
extern int g_cupertino_form_mock_card_set_title_fail;
extern int g_cupertino_form_mock_card_set_subtitle_fail;
extern int g_cupertino_form_mock_field_destroy_fail;
extern int g_cupertino_form_mock_field_set_label_fail;
extern int g_cupertino_form_mock_field_set_hint_fail;
extern int g_cupertino_form_mock_field_set_error_fail;
extern int g_cupertino_form_mock_field_bind_control_fail;
extern int g_cupertino_form_mock_field_set_has_value_fail;
extern int g_cupertino_form_mock_input_get_comp_fail;
extern int g_cupertino_form_mock_row_set_child_fail;
extern int g_cupertino_form_mock_input_set_text_fail;
extern int g_cupertino_form_mock_input_set_placeholder_fail;
extern int g_cupertino_form_mock_input_destroy_fail;
extern int g_cupertino_form_mock_row_destroy_fail;
extern int g_cupertino_form_mock_input_create_fail;
extern int g_cupertino_form_mock_field_set_control_fail;

TEST test_cupertino_form_coverage_branches(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_form_section *section = NULL;
  struct cupertino_form_row *row = NULL;
  struct cupertino_text_form_field_row *field_row = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_arena *arena = NULL;
  struct ui_reactor *reactor = NULL;
  struct ui_form_control *ctrl = NULL;
  ui_error_t rc;

  /* cupertino_form_section_destroy: section->base failure */
  rc = cupertino_form_section_create(dummy_engine, CUPERTINO_FORM_SECTION_PLAIN,
                                     &section);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_card_destroy_fail = 1;
  rc = cupertino_form_section_destroy(section);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_card_destroy_fail = 0;
  rc = cupertino_form_section_destroy(section);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* cupertino_form_section_destroy with NULL base */
  rc = cupertino_form_section_create(dummy_engine, CUPERTINO_FORM_SECTION_PLAIN,
                                     &section);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ui_card_base_destroy(section->base);
  section->base = NULL;
  rc = cupertino_form_section_destroy(section);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* cupertino_form_section_set_header: section->base error and NULL base */
  rc = cupertino_form_section_create(dummy_engine, CUPERTINO_FORM_SECTION_PLAIN,
                                     &section);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_card_set_title_fail = 1;
  rc = cupertino_form_section_set_header(section, "Test");
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_card_set_title_fail = 0;
  ui_card_base_destroy(section->base);
  section->base = NULL;
  rc = cupertino_form_section_set_header(section, "Header without base");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Header without base", section->header_text);

  /* cupertino_form_section_set_footer: section->base error and NULL base */
  ui_card_base_create(&section->base);
  g_cupertino_form_mock_card_set_subtitle_fail = 1;
  rc = cupertino_form_section_set_footer(section, "Footer");
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_card_set_subtitle_fail = 0;
  ui_card_base_destroy(section->base);
  section->base = NULL;
  rc = cupertino_form_section_set_footer(section, "Footer without base");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Footer without base", section->footer_text);
  rc = cupertino_form_section_destroy(section);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* cupertino_form_row_destroy: row->field failure */
  rc = cupertino_form_row_create(dummy_engine, &row);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_field_destroy_fail = 1;
  rc = cupertino_form_row_destroy(row);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_field_destroy_fail = 0;
  rc = cupertino_form_row_destroy(row);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* cupertino_form_row_destroy with NULL field */
  rc = cupertino_form_row_create(dummy_engine, &row);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ui_form_field_base_destroy(row->field);
  row->field = NULL;
  rc = cupertino_form_row_destroy(row);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* cupertino_form_row_set_prefix_label: row->field failure and NULL field */
  rc = cupertino_form_row_create(dummy_engine, &row);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_field_set_label_fail = 1;
  rc = cupertino_form_row_set_prefix_label(row, "Prefix");
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_field_set_label_fail = 0;
  ui_form_field_base_destroy(row->field);
  row->field = NULL;
  rc = cupertino_form_row_set_prefix_label(row, "Prefix without field");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Prefix without field", row->prefix_label);

  /* cupertino_form_row_set_helper_text: row->field failure and NULL field */
  ui_form_field_base_create(&row->field);
  g_cupertino_form_mock_field_set_hint_fail = 1;
  rc = cupertino_form_row_set_helper_text(row, "Helper");
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_field_set_hint_fail = 0;
  ui_form_field_base_destroy(row->field);
  row->field = NULL;
  rc = cupertino_form_row_set_helper_text(row, "Helper without field");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Helper without field", row->helper_text);

  /* cupertino_form_row_set_error_text: row->field failure and NULL field */
  ui_form_field_base_create(&row->field);
  g_cupertino_form_mock_field_set_error_fail = 1;
  rc = cupertino_form_row_set_error_text(row, "Error");
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_field_set_error_fail = 0;
  ui_form_field_base_destroy(row->field);
  row->field = NULL;
  rc = cupertino_form_row_set_error_text(row, "Error without field");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Error without field", row->error_text);
  rc = cupertino_form_row_destroy(row);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* cupertino_text_form_field_row_create error branches */
  /* 1. ui_input_base_create failure */
  g_cupertino_form_mock_input_create_fail = 1;
  rc = cupertino_text_form_field_row_create(dummy_engine, &field_row, &cva);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_input_create_fail = 0;

  /* 2. ui_input_base_get_component failure */
  g_cupertino_form_mock_input_get_comp_fail = 1;
  rc = cupertino_text_form_field_row_create(dummy_engine, &field_row, &cva);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_input_get_comp_fail = 0;

  /* 2b. ui_input_base_get_component failure with cleanup failures */
  g_cupertino_form_mock_input_get_comp_fail = 1;
  g_cupertino_form_mock_input_destroy_fail = 1;
  g_cupertino_form_mock_field_destroy_fail = 1;
  rc = cupertino_text_form_field_row_create(dummy_engine, &field_row, &cva);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_input_get_comp_fail = 0;
  g_cupertino_form_mock_input_destroy_fail = 0;
  g_cupertino_form_mock_field_destroy_fail = 0;

  /* 3. cupertino_form_row_set_child failure */
  g_cupertino_form_mock_field_set_control_fail = 1;
  rc = cupertino_text_form_field_row_create(dummy_engine, &field_row, &cva);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_field_set_control_fail = 0;

  /* 3b. cupertino_form_row_set_child failure with cleanup failures */
  g_cupertino_form_mock_field_set_control_fail = 1;
  g_cupertino_form_mock_input_destroy_fail = 1;
  g_cupertino_form_mock_field_destroy_fail = 1;
  rc = cupertino_text_form_field_row_create(dummy_engine, &field_row, &cva);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_field_set_control_fail = 0;
  g_cupertino_form_mock_input_destroy_fail = 0;
  g_cupertino_form_mock_field_destroy_fail = 0;

  /* cupertino_text_form_field_row_destroy: row destroy failure */
  rc = cupertino_text_form_field_row_create(dummy_engine, &field_row, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_field_destroy_fail = 1;
  rc = cupertino_text_form_field_row_destroy(field_row);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_field_destroy_fail = 0;
  rc = cupertino_text_form_field_row_destroy(field_row);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_text_form_field_row_create(dummy_engine, &field_row, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_input_set_text_fail = 1;
  rc = cupertino_text_form_field_row_set_text(field_row, "Text");
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_input_set_text_fail = 0;

  g_cupertino_form_mock_field_set_has_value_fail = 1;
  rc = cupertino_text_form_field_row_set_text(field_row, "Text");
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_field_set_has_value_fail = 0;

  /* cupertino_text_form_field_row_set_text with empty text sets has_value to 0
   */
  rc = cupertino_text_form_field_row_set_text(field_row, "");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", field_row->text_value);

  /* cupertino_text_form_field_row_set_text with NULL input and NULL row */
  ui_input_base_destroy(field_row->input);
  field_row->input = NULL;
  rc = cupertino_text_form_field_row_set_text(field_row, "No input");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("No input", field_row->text_value);

  /* cupertino_text_form_field_row_set_text with NULL field in row */
  ui_form_field_base_destroy(field_row->row->field);
  field_row->row->field = NULL;
  rc = cupertino_text_form_field_row_set_text(field_row, "No field");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  cupertino_form_row_destroy(field_row->row);
  field_row->row = NULL;
  rc = cupertino_text_form_field_row_set_text(field_row, "No row");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* cupertino_text_form_field_cva_set_disabled_state with NULL field_row->row
   */
  rc = cva->set_disabled_state(field_row, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_text_form_field_row_destroy(field_row);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* cupertino_text_form_field_row_set_placeholder error branches */
  rc = cupertino_text_form_field_row_create(dummy_engine, &field_row, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_input_set_placeholder_fail = 1;
  rc = cupertino_text_form_field_row_set_placeholder(field_row, "Holder");
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_input_set_placeholder_fail = 0;

  /* cupertino_text_form_field_row_set_placeholder with NULL input */
  ui_input_base_destroy(field_row->input);
  field_row->input = NULL;
  rc = cupertino_text_form_field_row_set_placeholder(field_row,
                                                     "No subcomponents");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("No subcomponents", field_row->placeholder);

  /* cupertino_text_form_field_row_destroy: input destroy failure */
  ui_input_base_create(&field_row->input);
  g_cupertino_form_mock_input_destroy_fail = 1;
  rc = cupertino_text_form_field_row_destroy(field_row);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_input_destroy_fail = 0;

  /* cupertino_text_form_field_row_destroy: NULL input and row */
  ui_input_base_destroy(field_row->input);
  field_row->input = NULL;
  cupertino_form_row_destroy(field_row->row);
  field_row->row = NULL;
  rc = cupertino_text_form_field_row_destroy(field_row);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* cupertino_text_form_field_row_bind_form_control error branches */
  rc = cupertino_text_form_field_row_create(dummy_engine, &field_row, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_arena_create(1024, &arena);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_reactor_create(&reactor);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  {
    union ui_signal_payload val;
    val.int_val = 0;
    rc = ui_form_control_create(arena, val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                                UI_SIGNAL_MODE_SINGLE_THREADED, &ctrl);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* NULL argument checks */
  rc = cupertino_text_form_field_row_bind_form_control(NULL, ctrl, reactor);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc =
      cupertino_text_form_field_row_bind_form_control(field_row, NULL, reactor);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_text_form_field_row_bind_form_control(field_row, ctrl, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Bind failure on row->field */
  g_cupertino_form_mock_field_bind_control_fail = 1;
  rc =
      cupertino_text_form_field_row_bind_form_control(field_row, ctrl, reactor);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_form_mock_field_bind_control_fail = 0;

  /* Bind with NULL row and NULL row->field */
  ui_form_field_base_destroy(field_row->row->field);
  field_row->row->field = NULL;
  rc =
      cupertino_text_form_field_row_bind_form_control(field_row, ctrl, reactor);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  cupertino_form_row_destroy(field_row->row);
  field_row->row = NULL;
  rc =
      cupertino_text_form_field_row_bind_form_control(field_row, ctrl, reactor);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_text_form_field_row_destroy(field_row);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_reactor_destroy(reactor);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_arena_destroy(arena);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();

  RUN_TEST(test_cupertino_form_section_lifecycle);
  RUN_TEST(test_cupertino_form_row_lifecycle);
  RUN_TEST(test_cupertino_text_form_field_row_lifecycle);
  RUN_TEST(test_cupertino_form_oom);
  RUN_TEST(test_cupertino_form_coverage_branches);

  GREATEST_MAIN_END();
}
