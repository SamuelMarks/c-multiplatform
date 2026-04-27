/* clang-format off */
#include "cmp.h"
#include "cmp_ui_breadcrumbs.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_breadcrumbs_suite);

TEST test_ui_breadcrumbs_lifecycle(void) {
  cmp_ui_breadcrumbs_t *breadcrumbs = NULL;
  cmp_ui_node_t *node = NULL;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_breadcrumbs_create(NULL, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_breadcrumbs_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_breadcrumbs_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_breadcrumbs_add_segment(NULL, "a"));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_breadcrumbs_create(&breadcrumbs, 0xFFFFFFFF));
  ASSERT_NEQ(NULL, breadcrumbs);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_breadcrumbs_get_node(breadcrumbs, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_breadcrumbs_get_node(breadcrumbs, &node));
  ASSERT_NEQ(NULL, node);

  /* Add segments */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_breadcrumbs_add_segment(breadcrumbs, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_breadcrumbs_add_segment(breadcrumbs, "Home"));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_breadcrumbs_add_segment(breadcrumbs, "Settings"));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_breadcrumbs_destroy(breadcrumbs));

  PASS();
}

SUITE(cmp_ui_breadcrumbs_suite) { RUN_TEST(test_ui_breadcrumbs_lifecycle); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_breadcrumbs_suite);
  GREATEST_MAIN_END();
}