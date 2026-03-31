/* clang-format off */
#include <cmp.h>
#include <greatest.h>
/* clang-format on */

SUITE(cmp_window_blur_suite);

TEST test_cmp_window_blur_create_destroy(void) {
  cmp_window_blur_t *blur = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_window_blur_create(&blur));
  ASSERT_NEQ(NULL, blur);

  ASSERT_EQ(CMP_SUCCESS, cmp_window_blur_destroy(blur));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_window_blur_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_window_blur_destroy(NULL));

  PASS();
}

TEST test_cmp_window_blur_enable_disable(void) {
  cmp_window_blur_t *blur = NULL;
  int enabled = 0;
  cmp_window_t *window1 = (cmp_window_t *)0x1234; /* Dummy pointers */
  cmp_window_t *window2 = (cmp_window_t *)0x5678;

  ASSERT_EQ(CMP_SUCCESS, cmp_window_blur_create(&blur));

  /* Initially disabled */
  ASSERT_EQ(CMP_SUCCESS, cmp_window_blur_is_enabled(blur, window1, &enabled));
  ASSERT_EQ(0, enabled);

  /* Enable for window1 */
  ASSERT_EQ(CMP_SUCCESS, cmp_window_blur_set_enabled(blur, window1, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_window_blur_is_enabled(blur, window1, &enabled));
  ASSERT_EQ(1, enabled);
  ASSERT_EQ(CMP_SUCCESS, cmp_window_blur_is_enabled(blur, window2, &enabled));
  ASSERT_EQ(0, enabled);

  /* Enable for window2 */
  ASSERT_EQ(CMP_SUCCESS, cmp_window_blur_set_enabled(blur, window2, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_window_blur_is_enabled(blur, window2, &enabled));
  ASSERT_EQ(1, enabled);

  /* Disable for window1 */
  ASSERT_EQ(CMP_SUCCESS, cmp_window_blur_set_enabled(blur, window1, 0));
  ASSERT_EQ(CMP_SUCCESS, cmp_window_blur_is_enabled(blur, window1, &enabled));
  ASSERT_EQ(0, enabled);
  ASSERT_EQ(CMP_SUCCESS, cmp_window_blur_is_enabled(blur, window2, &enabled));
  ASSERT_EQ(1, enabled);

  /* Disable for window2 */
  ASSERT_EQ(CMP_SUCCESS, cmp_window_blur_set_enabled(blur, window2, 0));
  ASSERT_EQ(CMP_SUCCESS, cmp_window_blur_is_enabled(blur, window2, &enabled));
  ASSERT_EQ(0, enabled);

  /* Error checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_window_blur_set_enabled(NULL, window1, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_window_blur_set_enabled(blur, NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_window_blur_is_enabled(NULL, window1, &enabled));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_window_blur_is_enabled(blur, NULL, &enabled));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_window_blur_is_enabled(blur, window1, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_window_blur_destroy(blur));

  PASS();
}

SUITE(cmp_window_blur_suite) {
  RUN_TEST(test_cmp_window_blur_create_destroy);
  RUN_TEST(test_cmp_window_blur_enable_disable);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_window_blur_suite);
  GREATEST_MAIN_END();
}
