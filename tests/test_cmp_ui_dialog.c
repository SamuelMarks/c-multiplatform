/* clang-format off */
#include "cmp.h"
#include "cmp_ui_dialog.h"
#include "greatest.h"
/* clang-format on */

TEST test_ui_dialog_lifecycle(void) {
  cmp_ui_dialog_t *dialog = NULL;
  int res;

  res = cmp_ui_dialog_create(&dialog, "Test Title", "Test Content");
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, dialog);

  res = cmp_ui_dialog_destroy(dialog);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_ui_dialog_null_args(void) {
  cmp_ui_dialog_t *dialog = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;
  int res;

  res = cmp_ui_dialog_create(NULL, "Title", "Content");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_dialog_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_dialog_create(&dialog, NULL, NULL);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_dialog_get_node(NULL, &node);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_dialog_get_node(dialog, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_dialog_set_visible(NULL, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_dialog_bind_a11y(NULL, tree);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_dialog_bind_a11y(dialog, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_destroy(tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_dialog_destroy(dialog);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_ui_dialog_operations(void) {
  cmp_ui_dialog_t *dialog = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;
  int res;

  res = cmp_ui_dialog_create(&dialog, "Alert", "This is an alert");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_dialog_get_node(dialog, &node);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, node);

  res = cmp_ui_dialog_set_visible(dialog, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_dialog_set_visible(dialog, 0);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_dialog_bind_a11y(dialog, tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_destroy(tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_dialog_destroy(dialog);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(ui_dialog_suite) {
  RUN_TEST(test_ui_dialog_lifecycle);
  RUN_TEST(test_ui_dialog_null_args);
  RUN_TEST(test_ui_dialog_operations);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_dialog_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
