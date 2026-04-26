/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_overscroll_evaluate(void) {
  int chains;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_overscroll_evaluate(CMP_OVERSCROLL_AUTO, 0, &chains));
  ASSERT_EQ(1, chains);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_overscroll_evaluate(CMP_OVERSCROLL_AUTO, 1, &chains));
  ASSERT_EQ(1, chains);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_overscroll_evaluate(CMP_OVERSCROLL_CONTAIN, 1, &chains));
  ASSERT_EQ(0, chains);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_overscroll_evaluate(CMP_OVERSCROLL_NONE, 1, &chains));
  ASSERT_EQ(0, chains);

  PASS();
}

TEST test_overscroll_null(void) {
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_overscroll_evaluate(CMP_OVERSCROLL_AUTO, 0, NULL));
  PASS();
}

SUITE(cmp_overscroll_suite) {
  RUN_TEST(test_overscroll_evaluate);
  RUN_TEST(test_overscroll_null);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_overscroll_suite);
  GREATEST_MAIN_END();
}
