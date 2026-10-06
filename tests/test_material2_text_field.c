/**
 * @file test_material2_text_field.c
 * @brief Unit tests for Material Design 2 Text Field component.
 */

/* clang-format off */
#include "material2/md2_text_field.h"
#include "ui_error.h"
#include "ui_component.h"
#include "ui_engine.h"
#include "ui_test_mock_mem.h"
#include <greatest.h>
/* clang-format on */

TEST test_md2_text_field_create_destroy(void) {
  struct md2_text_field *field = NULL;
  struct ui_engine *engine = NULL;
  ui_error_t rc;

  struct ui_engine_config config;
  memset(&config, 0, sizeof(config));
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_text_field_create(engine, MD2_TEXT_FIELD_FILLED, &field);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, field);

  rc = md2_text_field_destroy(field);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  ui_engine_destroy(engine);
  PASS();
}

TEST test_md2_text_field_invalid_args(void) {
  struct md2_text_field *field = NULL;
  struct ui_form_field_base *base = NULL;
  struct ui_engine *engine = NULL;
  ui_error_t rc;

  struct ui_engine_config config;
  memset(&config, 0, sizeof(config));
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  /* Null checks */
  rc = md2_text_field_create(NULL, MD2_TEXT_FIELD_FILLED, &field);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_create(engine, MD2_TEXT_FIELD_FILLED, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_create(engine, MD2_TEXT_FIELD_FILLED, &field);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_text_field_get_base(field, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_set_label(NULL, "label");
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_set_hint(NULL, "hint");
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_set_error(NULL, "error");
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_set_prefix(NULL, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_set_suffix(NULL, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_text_field_set_control(NULL, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  md2_text_field_destroy(field);
  ui_engine_destroy(engine);

  PASS();
}

TEST test_md2_text_field_properties(void) {
  struct md2_text_field *field = NULL;
  struct ui_engine *engine = NULL;
  struct ui_form_field_base *base = NULL;
  struct ui_component *prefix = NULL;
  struct ui_component *suffix = NULL;
  struct ui_component *control = NULL;
  ui_error_t rc;

  struct ui_engine_config config;
  memset(&config, 0, sizeof(config));
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_text_field_create(engine, MD2_TEXT_FIELD_OUTLINED, &field);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_text_field_get_base(field, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = md2_text_field_set_label(field, "My Label");
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_text_field_set_hint(field, "My Hint");
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_text_field_set_error(field, "My Error");
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = ui_component_create(&prefix);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  rc = md2_text_field_set_prefix(field, prefix);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = ui_component_create(&suffix);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  rc = md2_text_field_set_suffix(field, suffix);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = ui_component_create(&control);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  rc = md2_text_field_set_control(field, control);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  md2_text_field_destroy(field);
  ui_engine_destroy(engine);

  PASS();
}

TEST test_md2_text_field_oom(void) {
  struct md2_text_field *field = NULL;
  struct ui_engine *engine = NULL;
  ui_error_t rc;
  int i;

  struct ui_engine_config config;
  memset(&config, 0, sizeof(config));
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  for (i = 0; i < 500; ++i) {
    g_malloc_fail_countdown = i;
    rc = md2_text_field_create(engine, MD2_TEXT_FIELD_FILLED, &field);
    if (rc == UI_ERROR_NONE) {
      md2_text_field_destroy(field);
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  ui_engine_destroy(engine);
  PASS();
}

SUITE(material2_text_field_suite) {
  RUN_TEST(test_md2_text_field_create_destroy);
  RUN_TEST(test_md2_text_field_invalid_args);
  RUN_TEST(test_md2_text_field_properties);
  RUN_TEST(test_md2_text_field_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(material2_text_field_suite);
  GREATEST_MAIN_END();
}
