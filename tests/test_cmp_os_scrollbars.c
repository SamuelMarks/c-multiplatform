/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_os_scrollbars_lifecycle(void) {
  cmp_os_scrollbar_t *sb = NULL;
  float y;

  ASSERT_EQ(CMP_SUCCESS, cmp_os_scrollbar_create(&sb));
  ASSERT_NEQ(NULL, sb);

  ASSERT_EQ(CMP_SUCCESS, cmp_os_scrollbar_step(sb, 10.0f, 16, &y));

  ASSERT_EQ(CMP_SUCCESS, cmp_os_scrollbar_step(sb, 0.0f, 16, &y));

  ASSERT_EQ(CMP_SUCCESS, cmp_os_scrollbar_destroy(sb));
  PASS();
}

TEST test_os_scrollbars_null(void) {
  cmp_os_scrollbar_t *sb = NULL;
  float y;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_os_scrollbar_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_os_scrollbar_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_os_scrollbar_create(&sb));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_os_scrollbar_step(NULL, 10.0f, 16, &y));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_os_scrollbar_step(sb, 10.0f, 16, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_os_scrollbar_destroy(sb));
  PASS();
}

SUITE(cmp_os_scrollbars_suite) {
  RUN_TEST(test_os_scrollbars_lifecycle);
  RUN_TEST(test_os_scrollbars_null);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_os_scrollbars_suite);
  GREATEST_MAIN_END();
}
