/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_undo_redo_lifecycle(void) {
  cmp_undo_redo_t *stack = NULL;
  int res = cmp_undo_redo_create(&stack);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, stack);

  res = cmp_undo_redo_destroy(stack);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_undo_redo_null_args(void) {
  int res = cmp_undo_redo_create(NULL);
  cmp_undo_redo_t *stack = NULL;
  char buf[32];

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_undo_redo_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_undo_redo_push(NULL, "test");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_undo_redo_create(&stack);
  res = cmp_undo_redo_push(stack, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_undo_redo_undo(NULL, buf, 32);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_undo_redo_undo(stack, NULL, 32);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_undo_redo_undo(stack, buf, 0);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_undo_redo_destroy(stack);
  PASS();
}

TEST test_undo_redo_push_undo(void) {
  cmp_undo_redo_t *stack = NULL;
  int res;
  char buf[32];

  cmp_undo_redo_create(&stack);

  res = cmp_undo_redo_push(stack, "state1");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_undo_redo_push(stack, "state2");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_undo_redo_undo(stack, buf, 32);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_STR_EQ("state1", buf);

  res = cmp_undo_redo_undo(stack, buf, 32);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_STR_EQ("", buf);

  res = cmp_undo_redo_undo(stack, buf, 32);
  ASSERT_EQ(CMP_ERROR_INVALID_STATE, res);

  cmp_undo_redo_destroy(stack);
  PASS();
}

SUITE(undo_redo_suite) {
  RUN_TEST(test_undo_redo_lifecycle);
  RUN_TEST(test_undo_redo_null_args);
  RUN_TEST(test_undo_redo_push_undo);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(undo_redo_suite);
  GREATEST_MAIN_END();
}
