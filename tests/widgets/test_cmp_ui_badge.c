/* clang-format off */
#include "cmp_ui_badge.h"
#include "cmp.h"
#include "cmp_log.h"
#include "greatest.h"
/* clang-format on */

TEST test_badge_create_destroy(void) {
  cmp_ui_badge_t *badge = NULL;
  int rc;
  rc = cmp_ui_badge_create(&badge, "New", 0xFF0000FF, 0xFFFFFFFF);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT(badge != NULL);

  rc = cmp_ui_badge_destroy(badge);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_badge_create_null_args(void) {
  cmp_ui_badge_t *badge = NULL;
  int rc;

  rc = cmp_ui_badge_create(NULL, "New", 0xFF0000FF, 0xFFFFFFFF);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_badge_create(&badge, NULL, 0xFF0000FF, 0xFFFFFFFF);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_badge_destroy(badge);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_badge_destroy_null(void) {
  int rc;
  rc = cmp_ui_badge_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);
  PASS();
}

TEST test_badge_get_node(void) {
  cmp_ui_badge_t *badge = NULL;
  cmp_ui_node_t *node = NULL;
  int rc;

  rc = cmp_ui_badge_create(&badge, "New", 0xFF0000FF, 0xFFFFFFFF);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_badge_get_node(badge, &node);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT(node != NULL);

  rc = cmp_ui_badge_get_node(NULL, &node);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_badge_get_node(badge, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_badge_destroy(badge);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_badge_set_text(void) {
  cmp_ui_badge_t *badge = NULL;
  int rc;

  rc = cmp_ui_badge_create(&badge, "Old", 0xFF0000FF, 0xFFFFFFFF);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_badge_set_text(badge, "New");
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_badge_set_text(badge, NULL);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_badge_set_text(NULL, "Fail");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_badge_destroy(badge);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_badge_bind_a11y(void) {
  cmp_ui_badge_t *badge = NULL;
  cmp_a11y_tree_t *tree = NULL;
  int rc;

  rc = cmp_ui_badge_create(&badge, "A11y Test", 0xFF0000FF, 0xFFFFFFFF);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_a11y_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_badge_bind_a11y(badge, tree);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_badge_bind_a11y(NULL, tree);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_badge_bind_a11y(badge, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_a11y_tree_destroy(tree);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_badge_destroy(badge);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

SUITE(badge_suite) {
  RUN_TEST(test_badge_create_destroy);
  RUN_TEST(test_badge_create_null_args);
  RUN_TEST(test_badge_destroy_null);
  RUN_TEST(test_badge_get_node);
  RUN_TEST(test_badge_set_text);
  RUN_TEST(test_badge_bind_a11y);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(badge_suite);
  GREATEST_MAIN_END();
}