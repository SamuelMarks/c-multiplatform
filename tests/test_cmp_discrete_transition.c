/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_discrete_transition_lifecycle(void) {
  cmp_discrete_transition_t *transition = NULL;
  int res = cmp_discrete_transition_create(&transition);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, transition);

  res = cmp_discrete_transition_destroy(transition);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_discrete_transition_null_args(void) {
  int res = cmp_discrete_transition_create(NULL);
  int v;
  cmp_discrete_transition_t *transition = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_discrete_transition_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_discrete_transition_evaluate(NULL, 0.5f, &v);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_discrete_transition_create(&transition);
  res = cmp_discrete_transition_evaluate(transition, 0.5f, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_discrete_transition_evaluate(transition, -0.1f, &v);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_discrete_transition_destroy(transition);
  PASS();
}

TEST test_discrete_transition_evaluate(void) {
  cmp_discrete_transition_t *transition = NULL;
  int v;
  int res;

  cmp_discrete_transition_create(&transition);

  /* 0% progress */
  res = cmp_discrete_transition_evaluate(transition, 0.0f, &v);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, v);

  /* 49% progress */
  res = cmp_discrete_transition_evaluate(transition, 0.49f, &v);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, v);

  /* 50% progress */
  res = cmp_discrete_transition_evaluate(transition, 0.5f, &v);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, v);

  /* 100% progress */
  res = cmp_discrete_transition_evaluate(transition, 1.0f, &v);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, v);

  cmp_discrete_transition_destroy(transition);
  PASS();
}

SUITE(discrete_transition_suite) {
  RUN_TEST(test_discrete_transition_lifecycle);
  RUN_TEST(test_discrete_transition_null_args);
  RUN_TEST(test_discrete_transition_evaluate);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(discrete_transition_suite);
  GREATEST_MAIN_END();
}
