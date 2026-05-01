/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_custom_chrome_lifecycle(void) {
  int res;

  /* Cleanup first just in case */
  cmp_custom_chrome_cleanup();

  res = cmp_custom_chrome_init();
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Calling it twice should return SUCCESS (already initialized) */
  res = cmp_custom_chrome_init();
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_custom_chrome_cleanup();
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Calling cleanup twice should be safe */
  res = cmp_custom_chrome_cleanup();
  ASSERT_EQ(CMP_SUCCESS, res);

  PASS();
}

SUITE(custom_chrome_suite) { RUN_TEST(test_custom_chrome_lifecycle); }
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(custom_chrome_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
