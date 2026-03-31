/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_tick_lifecycle(void) {
  cmp_tick_t *tick = NULL;
  int res = cmp_tick_create(&tick);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, tick);

  res = cmp_tick_destroy(tick);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_tick_null_args(void) {
  int res = cmp_tick_create(NULL);
  double dt;
  cmp_tick_t *tick = NULL;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tick_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tick_fire(NULL, 100.0, &dt);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_tick_create(&tick);
  res = cmp_tick_fire(tick, 100.0, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_tick_destroy(tick);
  PASS();
}

TEST test_tick_fire_delta(void) {
  cmp_tick_t *tick = NULL;
  double dt;
  int res = cmp_tick_create(&tick);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* First tick should have 0 delta */
  res = cmp_tick_fire(tick, 1000.0, &dt);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(0.0, dt, 0.0001);

  /* Second tick */
  res = cmp_tick_fire(tick, 1016.6, &dt);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(16.6, dt, 0.0001);

  /* Third tick */
  res = cmp_tick_fire(tick, 1033.2, &dt);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(16.6, dt, 0.0001);

  /* Negative wrap should bound to 0 */
  res = cmp_tick_fire(tick, 500.0, &dt);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(0.0, dt, 0.0001);

  cmp_tick_destroy(tick);
  PASS();
}

SUITE(tick_suite) {
  RUN_TEST(test_tick_lifecycle);
  RUN_TEST(test_tick_null_args);
  RUN_TEST(test_tick_fire_delta);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(tick_suite);
  GREATEST_MAIN_END();
}
