/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_anim_compose_numerical(void) {
  float result;

  /* Replace */
  int res = cmp_anim_compose_numerical(10.0f, 5.0f, CMP_ANIM_COMPOSE_REPLACE,
                                       &result);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(5.0f, result, "%f");

  /* Add */
  res = cmp_anim_compose_numerical(10.0f, 5.0f, CMP_ANIM_COMPOSE_ADD, &result);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(15.0f, result, "%f");

  /* Accumulate */
  res = cmp_anim_compose_numerical(10.0f, 5.0f, CMP_ANIM_COMPOSE_ACCUMULATE,
                                   &result);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(15.0f, result, "%f");

  PASS();
}

TEST test_anim_compose_null_args(void) {
  int res =
      cmp_anim_compose_numerical(10.0f, 5.0f, CMP_ANIM_COMPOSE_REPLACE, NULL);
  float result;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_anim_compose_numerical(10.0f, 5.0f, 999, &result);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  PASS();
}

SUITE(anim_compose_suite) {
  RUN_TEST(test_anim_compose_numerical);
  RUN_TEST(test_anim_compose_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(anim_compose_suite);
  GREATEST_MAIN_END();
}