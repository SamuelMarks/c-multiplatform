/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_code_block_lifecycle(void) {
  cmp_code_block_t *block = NULL;
  int res;

  res = cmp_code_block_create(&block);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, block);

  res = cmp_code_block_destroy(block);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_code_block_null_args(void) {
  cmp_code_block_t *block = NULL;
  int res;

  res = cmp_code_block_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_code_block_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_code_block_create(&block);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_code_block_toggle_fold(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_code_block_is_expanded(NULL, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_code_block_is_expanded(block, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_code_block_add_reference_highlight(NULL, 0, 10, "ref1");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_code_block_add_reference_highlight(block, 0, 10, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_code_block_destroy(block);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_code_block_toggle_fold(void) {
  cmp_code_block_t *block = NULL;
  int is_expanded = 0;
  int res;

  res = cmp_code_block_create(&block);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_code_block_is_expanded(block, &is_expanded);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, is_expanded); /* Expanded by default */

  res = cmp_code_block_toggle_fold(block);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_code_block_is_expanded(block, &is_expanded);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, is_expanded);

  res = cmp_code_block_toggle_fold(block);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_code_block_is_expanded(block, &is_expanded);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, is_expanded);

  res = cmp_code_block_destroy(block);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_code_block_highlights(void) {
  cmp_code_block_t *block = NULL;
  int res;

  res = cmp_code_block_create(&block);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Add highlights and force reallocation */
  res = cmp_code_block_add_reference_highlight(block, 0, 10, "ref1");
  ASSERT_EQ(CMP_SUCCESS, res);
  res = cmp_code_block_add_reference_highlight(block, 15, 5, "ref2");
  ASSERT_EQ(CMP_SUCCESS, res);
  res = cmp_code_block_add_reference_highlight(block, 25, 8, "ref3");
  ASSERT_EQ(CMP_SUCCESS, res);
  res = cmp_code_block_add_reference_highlight(block, 40, 12, "ref4");
  ASSERT_EQ(CMP_SUCCESS, res);
  res = cmp_code_block_add_reference_highlight(block, 60, 2, "ref5");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_code_block_destroy(block);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(code_block_suite) {
  RUN_TEST(test_code_block_lifecycle);
  RUN_TEST(test_code_block_null_args);
  RUN_TEST(test_code_block_toggle_fold);
  RUN_TEST(test_code_block_highlights);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(code_block_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
