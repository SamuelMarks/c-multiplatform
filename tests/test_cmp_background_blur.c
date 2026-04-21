/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_cmp_background_blur_lifecycle(void) {
  cmp_background_blur_t *blur = NULL;

  /* NULL param should fail */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_background_blur_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_background_blur_destroy(NULL));

  /* Create success */
  ASSERT_EQ(CMP_SUCCESS, cmp_background_blur_create(&blur));
  ASSERT_NEQ(NULL, blur);

  /* Destroy success */
  ASSERT_EQ(CMP_SUCCESS, cmp_background_blur_destroy(blur));
  PASS();
}

TEST test_cmp_background_blur_invalid_args(void) {
  cmp_background_blur_t *blur = NULL;
  int is_enabled = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_background_blur_create(&blur));

  /* Set enabled with NULLs */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_background_blur_set_enabled(NULL, NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_background_blur_set_enabled(blur, NULL, 1));

  /* Is enabled with NULLs */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_background_blur_is_enabled(NULL, NULL, &is_enabled));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_background_blur_is_enabled(blur, NULL, &is_enabled));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_background_blur_is_enabled(blur, (cmp_window_t *)1, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_background_blur_destroy(blur));
  PASS();
}

SUITE(cmp_background_blur_suite) {
  RUN_TEST(test_cmp_background_blur_lifecycle);
  RUN_TEST(test_cmp_background_blur_invalid_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_background_blur_suite);
  GREATEST_MAIN_END();
}
