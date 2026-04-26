/* clang-format off */
#include "cmp_ui_diff.h"
#include "cmp.h"
#include "cmp_log.h"
#include "greatest.h"
/* clang-format on */

TEST test_diff_create_destroy(void) {
  cmp_ui_diff_t *diff = NULL;
  int rc;
  rc = cmp_ui_diff_create(&diff, "old", "new");
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT(diff != NULL);

  rc = cmp_ui_diff_destroy(diff);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_diff_create_null_args(void) {
  cmp_ui_diff_t *diff = NULL;
  int rc;

  rc = cmp_ui_diff_create(NULL, "old", "new");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_diff_create(&diff, NULL, NULL);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_diff_destroy(diff);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_diff_destroy_null(void) {
  int rc;
  rc = cmp_ui_diff_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);
  PASS();
}

TEST test_diff_get_node(void) {
  cmp_ui_diff_t *diff = NULL;
  cmp_ui_node_t *node = NULL;
  int rc;

  rc = cmp_ui_diff_create(&diff, "old", "new");
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_diff_get_node(diff, &node);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT(node != NULL);

  rc = cmp_ui_diff_get_node(NULL, &node);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_diff_get_node(diff, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_diff_destroy(diff);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

SUITE(diff_suite) {
  RUN_TEST(test_diff_create_destroy);
  RUN_TEST(test_diff_create_null_args);
  RUN_TEST(test_diff_destroy_null);
  RUN_TEST(test_diff_get_node);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(diff_suite);
  GREATEST_MAIN_END();
}