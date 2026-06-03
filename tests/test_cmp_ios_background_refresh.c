/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_ios_background_refresh_lifecycle(void) {
  cmp_ios_background_refresh_t *refresh = NULL;
  int task_id = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_ios_background_refresh_create(&refresh));
  ASSERT_NEQ(NULL, refresh);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_ios_background_refresh_begin_task(refresh, &task_id));
  ASSERT_NEQ(0, task_id);

  ASSERT_EQ(CMP_SUCCESS, cmp_ios_background_refresh_end_task(refresh, task_id));

  ASSERT_EQ(CMP_SUCCESS, cmp_ios_background_refresh_destroy(refresh));
  PASS();
}

TEST test_ios_background_refresh_null_args(void) {
  cmp_ios_background_refresh_t *refresh = NULL;
  int task_id = 0;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ios_background_refresh_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ios_background_refresh_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_ios_background_refresh_create(&refresh));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ios_background_refresh_begin_task(NULL, &task_id));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ios_background_refresh_begin_task(refresh, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ios_background_refresh_end_task(NULL, task_id));
  cmp_ios_background_refresh_destroy(refresh);
  PASS();
}

SUITE(ios_background_refresh_suite) {
  RUN_TEST(test_ios_background_refresh_lifecycle);
  RUN_TEST(test_ios_background_refresh_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ios_background_refresh_suite);
  GREATEST_MAIN_END();
}
