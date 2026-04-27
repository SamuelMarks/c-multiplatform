/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <math.h>
/* clang-format on */

TEST test_scroll_momentum_calculate(void) {
  float initial_velocity = 1000.0f;
  float elapsed_time_ms = 16.666f;
  float platform_decay_rate = 0.99f;
  float out_offset;
  float out_current_velocity;
  int res;

  res = cmp_scroll_momentum_calculate(initial_velocity, elapsed_time_ms,
                                      platform_decay_rate, &out_offset,
                                      &out_current_velocity);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(out_current_velocity < initial_velocity);
  ASSERT(out_offset > 0.0f);

  PASS();
}

TEST test_scroll_momentum_null_args(void) {
  float offset;
  float current_velocity;
  int res;

  res = cmp_scroll_momentum_calculate(1000.0f, 16.666f, 0.99f, NULL,
                                      &current_velocity);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_scroll_momentum_calculate(1000.0f, 16.666f, 0.99f, &offset, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  PASS();
}

SUITE(scroll_momentum_suite) {
  RUN_TEST(test_scroll_momentum_calculate);
  RUN_TEST(test_scroll_momentum_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(scroll_momentum_suite);
  GREATEST_MAIN_END();
}
