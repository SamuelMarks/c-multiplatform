/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_dt_update(void) {
  cmp_dt_t dt;
  int res;
  dt.last_time_ms = 0.0;
  dt.current_time_ms = 0.0;
  dt.delta_time_ms = 0.0;

  /* First update, should set delta to 0 */
  res = cmp_dt_update(&dt, 100.0);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(0.0, dt.delta_time_ms, 0.0001);
  ASSERT_IN_RANGE(100.0, dt.last_time_ms, 0.0001);
  ASSERT_IN_RANGE(100.0, dt.current_time_ms, 0.0001);

  /* Second update */
  res = cmp_dt_update(&dt, 116.6);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(16.6, dt.delta_time_ms, 0.0001);
  ASSERT_IN_RANGE(116.6, dt.last_time_ms, 0.0001);
  ASSERT_IN_RANGE(116.6, dt.current_time_ms, 0.0001);

  /* Negative update */
  res = cmp_dt_update(&dt, 50.0);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(0.0, dt.delta_time_ms, 0.0001);
  ASSERT_IN_RANGE(50.0, dt.last_time_ms, 0.0001);
  ASSERT_IN_RANGE(50.0, dt.current_time_ms, 0.0001);

  PASS();
}

TEST test_dt_null_args(void) {
  int res = cmp_dt_update(NULL, 100.0);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);
  PASS();
}

SUITE(dt_suite) {
  RUN_TEST(test_dt_update);
  RUN_TEST(test_dt_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(dt_suite);
  GREATEST_MAIN_END();
}