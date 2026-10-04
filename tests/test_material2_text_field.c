/**
 * @file test_material2_text_field.c
 * @brief Unit tests for Material Design 2 Text Field component.
 */

/* clang-format off */
#include "material2/md2_text_field.h"
#include "ui_error.h"
#include <greatest.h>
/* clang-format on */

TEST test_md2_text_field_lifecycle(void) {
  struct md2_text_field *field = NULL;
  struct ui_form_field_base *base = NULL;
  ui_error_t rc;

  rc = md2_text_field_create(NULL, MD2_TEXT_FIELD_FILLED, &field);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, field);

  rc = md2_text_field_get_base(field, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = md2_text_field_destroy(field);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_text_field_create(NULL, MD2_TEXT_FIELD_OUTLINED, &field);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, field);

  rc = md2_text_field_destroy(field);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  PASS();
}

TEST test_md2_text_field_invalid_args(void) {
  struct md2_text_field *field = NULL;
  struct ui_form_field_base *base = NULL;
  ui_error_t rc;

  rc = md2_text_field_create(NULL, MD2_TEXT_FIELD_FILLED, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_create(NULL, MD2_TEXT_FIELD_FILLED, &field);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_text_field_get_base(field, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_set_label(NULL, "Label");
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_set_hint(NULL, "Hint");
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_set_error(NULL, "Error");
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_set_prefix(NULL, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_set_suffix(NULL, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_set_control(NULL, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  md2_text_field_destroy(field);
  PASS();
}

TEST test_md2_text_field_setters(void) {
  struct md2_text_field *field = NULL;
  ui_error_t rc;

  rc = md2_text_field_create(NULL, MD2_TEXT_FIELD_FILLED, &field);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_text_field_set_label(field, "Label");
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_text_field_set_hint(field, "Hint");
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_text_field_set_error(field, "Error");
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_text_field_set_error(field, NULL);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  md2_text_field_destroy(field);
  PASS();
}

SUITE(material2_text_field_suite) {
  RUN_TEST(test_md2_text_field_lifecycle);
  RUN_TEST(test_md2_text_field_invalid_args);
  RUN_TEST(test_md2_text_field_setters);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(material2_text_field_suite);
  GREATEST_MAIN_END();
}
