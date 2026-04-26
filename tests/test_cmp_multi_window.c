/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_multi_window_lifecycle(void) {
  cmp_multi_window_t *win = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_multi_window_init());

  ASSERT_EQ(CMP_SUCCESS, cmp_multi_window_tear_off("tab1", &win));
  ASSERT_NEQ(NULL, win);

  ASSERT_EQ(CMP_SUCCESS, cmp_multi_window_update_all());

  ASSERT_EQ(CMP_SUCCESS, cmp_multi_window_merge_back(win));

  ASSERT_EQ(CMP_SUCCESS, cmp_multi_window_cleanup());
  PASS();
}

TEST test_multi_window_uninitialized(void) {
  cmp_multi_window_t *win = NULL;

  /* Cleanup should be safe even if not initialized */
  ASSERT_EQ(CMP_SUCCESS, cmp_multi_window_cleanup());

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_multi_window_tear_off("tab1", &win));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_multi_window_merge_back((cmp_multi_window_t *)0x1234));
  ASSERT_EQ(CMP_ERROR_INVALID_STATE, cmp_multi_window_update_all());

  PASS();
}

TEST test_multi_window_null_args(void) {
  cmp_multi_window_t *win = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_multi_window_init());

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_multi_window_tear_off(NULL, &win));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_multi_window_tear_off("tab1", NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_multi_window_merge_back(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_multi_window_cleanup());
  PASS();
}

TEST test_multi_window_max(void) {
  cmp_multi_window_t *windows[128];
  cmp_multi_window_t *extra = NULL;
  int i;

  ASSERT_EQ(CMP_SUCCESS, cmp_multi_window_init());

  for (i = 0; i < 128; i++) {
    ASSERT_EQ(CMP_SUCCESS, cmp_multi_window_tear_off("tab", &windows[i]));
  }

  ASSERT_EQ(CMP_ERROR_BOUNDS, cmp_multi_window_tear_off("tab_extra", &extra));

  for (i = 0; i < 128; i++) {
    ASSERT_EQ(CMP_SUCCESS, cmp_multi_window_merge_back(windows[i]));
  }

  ASSERT_EQ(CMP_ERROR_NOT_FOUND, cmp_multi_window_merge_back(windows[0]));

  ASSERT_EQ(CMP_SUCCESS, cmp_multi_window_cleanup());
  PASS();
}

SUITE(multi_window_suite) {
  /* Run uninitialized first */
  RUN_TEST(test_multi_window_uninitialized);
  RUN_TEST(test_multi_window_lifecycle);
  RUN_TEST(test_multi_window_null_args);
  RUN_TEST(test_multi_window_max);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(multi_window_suite);
  GREATEST_MAIN_END();
}
