/* clang-format off */
#include "cmp.h"
#include "cmp_ui_modal_drawer.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_modal_drawer_suite);

TEST test_ui_modal_drawer_lifecycle(void) {
  cmp_ui_modal_drawer_t *drawer = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;
  int index;
  int i;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_modal_drawer_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_modal_drawer_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_modal_drawer_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_modal_drawer_set_open(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_modal_drawer_add_item(NULL, "icon", "label", &index));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_modal_drawer_set_selected(NULL, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_modal_drawer_bind_a11y(NULL, tree));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_modal_drawer_create(&drawer));
  ASSERT_NEQ(NULL, drawer);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_modal_drawer_get_node(drawer, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_modal_drawer_get_node(drawer, &node));
  ASSERT_NEQ(NULL, node);

  /* Set open */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_modal_drawer_set_open(drawer, 1));

  /* Add items */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_modal_drawer_add_item(drawer, "icon", NULL, &index));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_modal_drawer_add_item(drawer, "home", "Home", &index));
  ASSERT_EQ(0, index);

  /* Add more to trigger realloc */
  for (i = 0; i < 10; i++) {
    ASSERT_EQ(CMP_SUCCESS,
              cmp_ui_modal_drawer_add_item(drawer, "icon", "Item", &index));
  }

  /* Set selected */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_modal_drawer_set_selected(drawer, -1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_modal_drawer_set_selected(drawer, 100));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_modal_drawer_set_selected(drawer, 1));

  /* A11y test */
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_create(&tree));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_modal_drawer_bind_a11y(drawer, tree));
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_destroy(tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_modal_drawer_destroy(drawer));

  PASS();
}

SUITE(cmp_ui_modal_drawer_suite) { RUN_TEST(test_ui_modal_drawer_lifecycle); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_modal_drawer_suite);
  GREATEST_MAIN_END();
}