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

TEST test_undo_redo_discard_on_push(void) {
  cmp_undo_redo_t *stack = NULL;
  char buf[32];

  cmp_undo_redo_create(&stack);

  cmp_undo_redo_push(stack, "1");
  cmp_undo_redo_push(stack, "2");
  cmp_undo_redo_push(stack, "3");

  cmp_undo_redo_undo(stack, buf, 32); /* back to 2 */
  cmp_undo_redo_undo(stack, buf, 32); /* back to 1 */

  cmp_undo_redo_push(stack, "4"); /* Should drop 2 and 3 */

  cmp_undo_redo_undo(stack, buf, 32);
  ASSERT_STR_EQ("1", buf);

  cmp_undo_redo_destroy(stack);
  PASS();
}

TEST test_undo_redo_overflow(void) {
  cmp_undo_redo_t *stack = NULL;
  int res;
  char buf[32];
  int i;

  cmp_undo_redo_create(&stack);

  for (i = 1; i <= 12; i++) {
    char state[32];
#if defined(_MSC_VER)
    sprintf_s(state, sizeof(state), "state%d", i);
#else
    sprintf(state, "state%d", i);
#endif
    cmp_undo_redo_push(stack, state);
  }

  res = cmp_undo_redo_undo(stack, buf, 32);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_STR_EQ("state11", buf);

  /* Undo all the way back */
  for (i = 0; i < 9; i++) {
    cmp_undo_redo_undo(stack, buf, 32);
  }

  /* We only store 10 items, so item 1 and 2 were dropped. The oldest state now
   * is state3, but wait, position is 0 so undo gives empty string representing
   * initial state before state3 */
  ASSERT_STR_EQ("", buf);

  cmp_undo_redo_destroy(stack);
  PASS();
}

SUITE(undo_redo_suite) {
  RUN_TEST(test_undo_redo_lifecycle);
  RUN_TEST(test_undo_redo_null_args);
  RUN_TEST(test_undo_redo_push_undo);
  RUN_TEST(test_undo_redo_discard_on_push);
  RUN_TEST(test_undo_redo_overflow);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(undo_redo_suite);
  GREATEST_MAIN_END();
}
