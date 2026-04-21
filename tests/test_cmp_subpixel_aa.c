/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

SUITE(cmp_subpixel_aa_suite);

TEST test_cmp_subpixel_aa_create(void) {
  cmp_subpixel_aa_t *ctx = NULL;
  ASSERT_EQ(CMP_SUCCESS, cmp_subpixel_aa_create(&ctx));
  ASSERT(ctx != NULL);

  int enabled = -1;
  ASSERT_EQ(CMP_SUCCESS, cmp_subpixel_aa_is_enabled(ctx, &enabled));
  ASSERT_EQ(0, enabled);

  ASSERT_EQ(CMP_SUCCESS, cmp_subpixel_aa_destroy(ctx));
  PASS();
}

TEST test_cmp_subpixel_aa_enable(void) {
  cmp_subpixel_aa_t *ctx = NULL;
  ASSERT_EQ(CMP_SUCCESS, cmp_subpixel_aa_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_subpixel_aa_enable(ctx, 1));

  int enabled = -1;
  ASSERT_EQ(CMP_SUCCESS, cmp_subpixel_aa_is_enabled(ctx, &enabled));
  ASSERT_EQ(1, enabled);

  ASSERT_EQ(CMP_SUCCESS, cmp_subpixel_aa_destroy(ctx));
  PASS();
}

TEST test_cmp_subpixel_aa_invalid_args(void) {
  cmp_subpixel_aa_t *ctx = NULL;
  int enabled;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_subpixel_aa_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_subpixel_aa_enable(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_subpixel_aa_is_enabled(NULL, &enabled));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_subpixel_aa_is_enabled(ctx, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_subpixel_aa_destroy(NULL));

  PASS();
}

SUITE(cmp_subpixel_aa_suite) {
  RUN_TEST(test_cmp_subpixel_aa_create);
  RUN_TEST(test_cmp_subpixel_aa_enable);
  RUN_TEST(test_cmp_subpixel_aa_invalid_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_subpixel_aa_suite);
  GREATEST_MAIN_END();
}
