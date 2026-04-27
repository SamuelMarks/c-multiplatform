/* clang-format off */
#include "cmp.h"
#include "cmp_ui_badge.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_badge_suite);

TEST test_ui_badge_lifecycle(void) {
  cmp_ui_badge_t *badge = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_badge_create(NULL, "99+", 0xFF000000, 0xFFFFFFFF));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_badge_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_badge_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_badge_set_text(NULL, "1"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_badge_bind_a11y(NULL, tree));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_badge_create(&badge, "99+", 0xFF000000, 0xFFFFFFFF));
  ASSERT_NEQ(NULL, badge);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_badge_get_node(badge, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_badge_get_node(badge, &node));
  ASSERT_NEQ(NULL, node);

  /* Set text */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_badge_set_text(badge, "1"));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_badge_set_text(badge, NULL));

  /* A11y tests */
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_create(&tree));

  /* Need mock layout to succeed */
  ASSERT_EQ(CMP_MALLOC(sizeof(cmp_layout_node_t), (void **)&node->layout),
            CMP_SUCCESS);
  node->layout->id = 1;

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_badge_bind_a11y(badge, tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_destroy(tree));

  /* Clean up mock layout to prevent leak, not normally part of badge struct
   * directly managed */
  CMP_FREE(node->layout);
  node->layout = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_badge_destroy(badge));

  PASS();
}

TEST test_ui_badge_null_text_init(void) {
  cmp_ui_badge_t *badge = NULL;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_badge_create(&badge, NULL, 0xFF000000, 0xFFFFFFFF));
  ASSERT_NEQ(NULL, badge);
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_badge_destroy(badge));

  PASS();
}

SUITE(cmp_ui_badge_suite) {
  RUN_TEST(test_ui_badge_lifecycle);
  RUN_TEST(test_ui_badge_null_text_init);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_badge_suite);
  GREATEST_MAIN_END();
}