/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_aria_live_lifecycle(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_aria_live_t *live = NULL;
  int res;
  cmp_a11y_tree_create(&tree);

  res = cmp_aria_live_create(tree, &live);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, live);

  res = cmp_aria_live_destroy(live);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_aria_live_null_args(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_aria_live_t *live = NULL;
  int res;
  cmp_a11y_tree_create(&tree);

  res = cmp_aria_live_create(NULL, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_aria_live_create(tree, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_aria_live_create(NULL, &live);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_aria_live_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_aria_live_create(tree, &live);

  res = cmp_aria_live_set_mode(NULL, 1, CMP_ARIA_LIVE_POLITE);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_aria_live_announce(NULL, 1, "Hello");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_aria_live_announce(live, 1, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_aria_live_destroy(live);
  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_aria_live_operations(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_aria_live_t *live = NULL;
  int res;
  cmp_a11y_tree_create(&tree);

  cmp_aria_live_create(tree, &live);

  res = cmp_aria_live_set_mode(live, 10, CMP_ARIA_LIVE_POLITE);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_aria_live_set_mode(live, 20, CMP_ARIA_LIVE_ASSERTIVE);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Update existing node */
  res = cmp_aria_live_set_mode(live, 10, CMP_ARIA_LIVE_OFF);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_aria_live_announce(live, 10, "Ignored");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_aria_live_announce(live, 20, "Alert!");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_aria_live_announce(live, 30, "Unknown Node");
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_aria_live_destroy(live);
  cmp_a11y_tree_destroy(tree);
  PASS();
}

SUITE(aria_live_suite) {
  RUN_TEST(test_aria_live_lifecycle);
  RUN_TEST(test_aria_live_null_args);
  RUN_TEST(test_aria_live_operations);
}

#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(aria_live_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
