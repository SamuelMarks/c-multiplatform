/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include "cmp_ink_ripple.h"
#include <stdlib.h>
/* clang-format on */

TEST test_ink_ripple_lifecycle(void) {
  cmp_ink_ripple_t *ripple = NULL;
  float r = 0, o = 0, x = 0, y = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_ink_ripple_create(&ripple));
  ASSERT_NEQ(NULL, ripple);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_ink_ripple_trigger(ripple, 50.0f, 50.0f, 100.0f, 100.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_ink_ripple_update(ripple, 16.6f));

  ASSERT_EQ(CMP_SUCCESS, cmp_ink_ripple_get_state(ripple, &r, &o, &x, &y));

  ASSERT_EQ(CMP_SUCCESS, cmp_ink_ripple_release(ripple));

  /* push update fully to expire t_expand and t_fade */
  ASSERT_EQ(CMP_SUCCESS, cmp_ink_ripple_update(ripple, 500.0f));

  ASSERT_EQ(CMP_SUCCESS, cmp_ink_ripple_destroy(ripple));
  PASS();
}

TEST test_ink_ripple_null_args(void) {
  cmp_ink_ripple_t *ripple = NULL;
  float r, o, x, y;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ink_ripple_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ink_ripple_destroy(NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ink_ripple_update(NULL, 10.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ink_ripple_trigger(NULL, 0, 0, 10, 10));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ink_ripple_release(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ink_ripple_get_state(NULL, &r, &o, &x, &y));
  cmp_ink_ripple_create(&ripple);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ink_ripple_update(ripple, -10.0f));

  /* should allow null args for state outputs */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ink_ripple_get_state(ripple, NULL, NULL, NULL, NULL));
  cmp_ink_ripple_destroy(ripple);
  PASS();
}

SUITE(suite_ink_ripple) {
  RUN_TEST(test_ink_ripple_lifecycle);
  RUN_TEST(test_ink_ripple_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(suite_ink_ripple);
  GREATEST_MAIN_END();
}
