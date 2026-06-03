/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_power_awareness_lifecycle(void) {
  int res;

  /* Just in case, clean it up before we test */
  cmp_power_awareness_cleanup();
  res = cmp_power_awareness_init();
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Calling it twice should be safe */
  res = cmp_power_awareness_init();
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_power_awareness_cleanup();
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Cleaning up twice should be safe */
  res = cmp_power_awareness_cleanup();
  ASSERT_EQ(CMP_SUCCESS, res);

  PASS();
}

TEST test_power_awareness_poll(void) {
  int res;

  /* If we poll without init, it should return INVALID_STATE */
  cmp_power_awareness_cleanup();
  res = cmp_power_awareness_poll();
  ASSERT_EQ(CMP_ERROR_INVALID_STATE, res);

  res = cmp_power_awareness_init();
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Polling should now succeed */
  res = cmp_power_awareness_poll();
  /* The poll function interacts with OS but shouldn't fail logically */
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_power_awareness_cleanup();
  ASSERT_EQ(CMP_SUCCESS, res);

  PASS();
}

SUITE(power_awareness_suite) {
  RUN_TEST(test_power_awareness_lifecycle);
  RUN_TEST(test_power_awareness_poll);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(power_awareness_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
