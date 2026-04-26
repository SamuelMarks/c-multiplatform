/* clang-format off */
#include "cmp_ui_accordion.h"
#include "cmp.h"
#include "cmp_log.h"
#include "greatest.h"
/* clang-format on */

TEST test_accordion_create_destroy(void) {
  cmp_ui_accordion_t *accordion = NULL;
  int rc;
  rc = cmp_ui_accordion_create(&accordion, "Test Accordion", 0xFFFFFFFF);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT(accordion != NULL);

  rc = cmp_ui_accordion_destroy(accordion);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_accordion_create_null_args(void) {
  cmp_ui_accordion_t *accordion = NULL;
  int rc;

  rc = cmp_ui_accordion_create(NULL, "Test", 0);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_accordion_create(&accordion, NULL, 0);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_accordion_destroy(accordion);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_accordion_destroy_null(void) {
  int rc;
  rc = cmp_ui_accordion_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);
  PASS();
}

TEST test_accordion_get_node(void) {
  cmp_ui_accordion_t *accordion = NULL;
  cmp_ui_node_t *node = NULL;
  int rc;

  rc = cmp_ui_accordion_create(&accordion, "Test Accordion", 0xFFFFFFFF);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_accordion_get_node(accordion, &node);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT(node != NULL);

  rc = cmp_ui_accordion_get_node(NULL, &node);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_accordion_get_node(accordion, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_accordion_destroy(accordion);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_accordion_set_expanded(void) {
  cmp_ui_accordion_t *accordion = NULL;
  int rc;

  rc = cmp_ui_accordion_create(&accordion, "Test Accordion", 0xFFFFFFFF);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_accordion_set_expanded(accordion, 1);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_accordion_set_expanded(accordion, 0);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_accordion_set_expanded(NULL, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_accordion_destroy(accordion);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

SUITE(accordion_suite) {
  RUN_TEST(test_accordion_create_destroy);
  RUN_TEST(test_accordion_create_null_args);
  RUN_TEST(test_accordion_destroy_null);
  RUN_TEST(test_accordion_get_node);
  RUN_TEST(test_accordion_set_expanded);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(accordion_suite);
  GREATEST_MAIN_END();
}
