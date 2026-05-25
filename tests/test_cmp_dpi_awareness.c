/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_dpi_awareness_lifecycle(void) {
  int res;

  /* Cleanup first just in case */
  ASSERT_EQ(CMP_SUCCESS, cmp_dpi_awareness_cleanup());

  /* Initialize */
  res = cmp_dpi_awareness_init();
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Calling it twice should return SUCCESS and be safe */
  res = cmp_dpi_awareness_init();
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Cleanup */
  res = cmp_dpi_awareness_cleanup();
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Calling cleanup twice should be safe */
  res = cmp_dpi_awareness_cleanup();
  ASSERT_EQ(CMP_SUCCESS, res);

  PASS();
}

SUITE(dpi_awareness_suite) { RUN_TEST(test_dpi_awareness_lifecycle); }
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(dpi_awareness_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
