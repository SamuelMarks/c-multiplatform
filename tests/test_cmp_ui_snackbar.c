/* clang-format off */
#include "cmp.h"
#include "cmp_ui_snackbar.h"
#include "greatest.h"
/* clang-format on */

TEST test_ui_snackbar_lifecycle(void) {
  cmp_ui_snackbar_t *snackbar = NULL;
  int res;

  res = cmp_ui_snackbar_create(&snackbar, "Message", "Undo");
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, snackbar);

  res = cmp_ui_snackbar_destroy(snackbar);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_ui_snackbar_null_args(void) {
  cmp_ui_snackbar_t *snackbar = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;
  int res;

  res = cmp_ui_snackbar_create(NULL, "Msg", "Undo");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_snackbar_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_snackbar_create(&snackbar, NULL, NULL);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_snackbar_get_node(NULL, &node);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_snackbar_get_node(snackbar, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_snackbar_set_message(NULL, "msg");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_snackbar_set_action(NULL, "undo");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_snackbar_bind_a11y(NULL, tree);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_snackbar_bind_a11y(snackbar, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_destroy(tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_snackbar_destroy(snackbar);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_ui_snackbar_operations(void) {
  cmp_ui_snackbar_t *snackbar = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;
  int res;

  res = cmp_ui_snackbar_create(&snackbar, "Item archived", "Undo");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_snackbar_get_node(snackbar, &node);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, node);

  res = cmp_ui_snackbar_set_message(snackbar, "Item deleted");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_snackbar_set_action(snackbar, "Recover");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_snackbar_set_action(snackbar, NULL);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_snackbar_bind_a11y(snackbar, tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_destroy(tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_snackbar_destroy(snackbar);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(ui_snackbar_suite) {
  RUN_TEST(test_ui_snackbar_lifecycle);
  RUN_TEST(test_ui_snackbar_null_args);
  RUN_TEST(test_ui_snackbar_operations);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_snackbar_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
