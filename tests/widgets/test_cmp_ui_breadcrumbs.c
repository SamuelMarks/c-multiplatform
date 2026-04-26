/* clang-format off */
#include "cmp_ui_breadcrumbs.h"
#include "cmp.h"
#include "cmp_log.h"
#include "greatest.h"
/* clang-format on */

TEST test_breadcrumbs_create_destroy(void) {
  cmp_ui_breadcrumbs_t *breadcrumbs = NULL;
  int rc;
  rc = cmp_ui_breadcrumbs_create(&breadcrumbs, 0xFF0000FF);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT(breadcrumbs != NULL);

  rc = cmp_ui_breadcrumbs_destroy(breadcrumbs);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_breadcrumbs_create_null_args(void) {
  int rc;
  rc = cmp_ui_breadcrumbs_create(NULL, 0xFF0000FF);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  PASS();
}

TEST test_breadcrumbs_destroy_null(void) {
  int rc;
  rc = cmp_ui_breadcrumbs_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);
  PASS();
}

TEST test_breadcrumbs_get_node(void) {
  cmp_ui_breadcrumbs_t *breadcrumbs = NULL;
  cmp_ui_node_t *node = NULL;
  int rc;

  rc = cmp_ui_breadcrumbs_create(&breadcrumbs, 0xFFFFFFFF);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_breadcrumbs_get_node(breadcrumbs, &node);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT(node != NULL);

  rc = cmp_ui_breadcrumbs_get_node(NULL, &node);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_breadcrumbs_get_node(breadcrumbs, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_breadcrumbs_destroy(breadcrumbs);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_breadcrumbs_add_segment(void) {
  cmp_ui_breadcrumbs_t *breadcrumbs = NULL;
  int rc;

  rc = cmp_ui_breadcrumbs_create(&breadcrumbs, 0xFFFFFFFF);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_breadcrumbs_add_segment(breadcrumbs, "Home");
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_breadcrumbs_add_segment(breadcrumbs, "Settings");
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_breadcrumbs_add_segment(NULL, "Fail");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_breadcrumbs_add_segment(breadcrumbs, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_breadcrumbs_destroy(breadcrumbs);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

SUITE(breadcrumbs_suite) {
  RUN_TEST(test_breadcrumbs_create_destroy);
  RUN_TEST(test_breadcrumbs_create_null_args);
  RUN_TEST(test_breadcrumbs_destroy_null);
  RUN_TEST(test_breadcrumbs_get_node);
  RUN_TEST(test_breadcrumbs_add_segment);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(breadcrumbs_suite);
  GREATEST_MAIN_END();
}
