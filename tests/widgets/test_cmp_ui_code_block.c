/* clang-format off */
#include "cmp_ui_code_block.h"
#include "cmp.h"
#include "cmp_log.h"
#include "greatest.h"
/* clang-format on */

TEST test_code_block_create_destroy(void) {
  cmp_ui_code_block_t *block = NULL;
  int rc;
  rc = cmp_ui_code_block_create(&block, "int main() {}", "c");
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT(block != NULL);

  rc = cmp_ui_code_block_destroy(block);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_code_block_create_null_args(void) {
  cmp_ui_code_block_t *block = NULL;
  int rc;

  rc = cmp_ui_code_block_create(NULL, "int main() {}", "c");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_code_block_create(&block, NULL, NULL);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_code_block_destroy(block);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_code_block_destroy_null(void) {
  int rc;
  rc = cmp_ui_code_block_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);
  PASS();
}

TEST test_code_block_get_node(void) {
  cmp_ui_code_block_t *block = NULL;
  cmp_ui_node_t *node = NULL;
  int rc;

  rc = cmp_ui_code_block_create(&block, "int main() {}", "c");
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_code_block_get_node(block, &node);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT(node != NULL);

  rc = cmp_ui_code_block_get_node(NULL, &node);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_code_block_get_node(block, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_code_block_destroy(block);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_code_block_set_code(void) {
  cmp_ui_code_block_t *block = NULL;
  int rc;

  rc = cmp_ui_code_block_create(&block, "old code", "c");
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_code_block_set_code(block, "new code");
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_code_block_set_code(block, NULL);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_code_block_set_code(NULL, "Fail");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_code_block_destroy(block);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

SUITE(code_block_suite) {
  RUN_TEST(test_code_block_create_destroy);
  RUN_TEST(test_code_block_create_null_args);
  RUN_TEST(test_code_block_destroy_null);
  RUN_TEST(test_code_block_get_node);
  RUN_TEST(test_code_block_set_code);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(code_block_suite);
  GREATEST_MAIN_END();
}
