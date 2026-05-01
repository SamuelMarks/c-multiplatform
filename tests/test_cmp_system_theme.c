/* clang-format off */
#include "cmp.h"
#include "cmp_system_theme.h"
#include "greatest.h"
/* clang-format on */

TEST test_system_theme_lifecycle(void) {
  int res;

  /* The actual implementation under test might be an empty stub depending on
     the platform, but the API contract must hold regardless of platform. */

  res = cmp_system_theme_init();
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_system_theme_shutdown();
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_system_theme_queries(void) {
  int is_dark;
  int is_high_contrast;
  int res;

  res = cmp_system_theme_init();
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_system_theme_is_dark(&is_dark);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(is_dark == 0 || is_dark == 1);

  res = cmp_system_theme_is_high_contrast(&is_high_contrast);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(is_high_contrast == 0 || is_high_contrast == 1);

  res = cmp_system_theme_shutdown();
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_system_theme_null_args(void) {
  int res;

  res = cmp_system_theme_init();
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_system_theme_is_dark(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_system_theme_is_high_contrast(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_system_theme_shutdown();
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(system_theme_suite) {
  RUN_TEST(test_system_theme_lifecycle);
  RUN_TEST(test_system_theme_queries);
  RUN_TEST(test_system_theme_null_args);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(system_theme_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
