/* clang-format off */
#include "cmp_ui_action_button.h"
#include "cmp.h"
#include "cmp_log.h"
#include "greatest.h"
/* clang-format on */

TEST test_action_button_create_destroy(void) {
  cmp_ui_action_button_t *btn = NULL;
  int rc;
  rc = cmp_ui_action_button_create(&btn, "Action Button",
                                   CMP_UI_ACTION_BUTTON_STYLE_FILLED);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT(btn != NULL);

  rc = cmp_ui_action_button_destroy(btn);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_action_button_create_null_args(void) {
  cmp_ui_action_button_t *btn = NULL;
  int rc;

  rc = cmp_ui_action_button_create(NULL, "Test",
                                   CMP_UI_ACTION_BUTTON_STYLE_ELEVATED);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_action_button_create(&btn, NULL,
                                   CMP_UI_ACTION_BUTTON_STYLE_ELEVATED);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_action_button_destroy(btn);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_action_button_destroy_null(void) {
  int rc;
  rc = cmp_ui_action_button_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);
  PASS();
}

TEST test_action_button_get_node(void) {
  cmp_ui_action_button_t *btn = NULL;
  cmp_ui_node_t *node = NULL;
  int rc;

  rc = cmp_ui_action_button_create(&btn, "Action Button",
                                   CMP_UI_ACTION_BUTTON_STYLE_FILLED);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_action_button_get_node(btn, &node);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT(node != NULL);

  rc = cmp_ui_action_button_get_node(NULL, &node);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_action_button_get_node(btn, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_action_button_destroy(btn);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_action_button_set_label(void) {
  cmp_ui_action_button_t *btn = NULL;
  int rc;

  rc = cmp_ui_action_button_create(&btn, "Test",
                                   CMP_UI_ACTION_BUTTON_STYLE_FILLED);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_action_button_set_label(btn, "New Label");
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_action_button_set_label(btn, NULL);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_action_button_set_label(NULL, "Fail");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_action_button_destroy(btn);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_action_button_bind_a11y(void) {
  cmp_ui_action_button_t *btn = NULL;
  cmp_a11y_tree_t *tree = NULL;
  int rc;

  rc = cmp_ui_action_button_create(&btn, "A11y Test",
                                   CMP_UI_ACTION_BUTTON_STYLE_FILLED);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_a11y_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_action_button_bind_a11y(btn, tree);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_action_button_bind_a11y(NULL, tree);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_action_button_bind_a11y(btn, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_a11y_tree_destroy(tree);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_action_button_destroy(btn);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

SUITE(action_button_suite) {
  RUN_TEST(test_action_button_create_destroy);
  RUN_TEST(test_action_button_create_null_args);
  RUN_TEST(test_action_button_destroy_null);
  RUN_TEST(test_action_button_get_node);
  RUN_TEST(test_action_button_set_label);
  RUN_TEST(test_action_button_bind_a11y);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(action_button_suite);
  GREATEST_MAIN_END();
}