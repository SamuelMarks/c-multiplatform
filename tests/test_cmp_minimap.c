/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_minimap_lifecycle(void) {
  cmp_minimap_t *ctx = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_minimap_create(&ctx));
  ASSERT_EQ(CMP_SUCCESS, cmp_minimap_set_text(ctx, "line1\nline2\nline3"));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_minimap_update_viewport(ctx, 10.0f, 100.0f, 500.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_minimap_compute_layout(ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_minimap_destroy(ctx));
  PASS();
}

TEST test_minimap_null_args(void) {
  cmp_minimap_t *ctx = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_minimap_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_minimap_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_minimap_create(&ctx));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_minimap_set_text(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_minimap_set_text(ctx, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_minimap_update_viewport(NULL, 10.0f, 100.0f, 500.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_minimap_update_viewport(ctx, 10.0f, 0.0f, 500.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_minimap_update_viewport(ctx, 10.0f, 100.0f, -1.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_minimap_compute_layout(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_minimap_destroy(ctx));
  PASS();
}

SUITE(minimap_suite) {
  RUN_TEST(test_minimap_lifecycle);
  RUN_TEST(test_minimap_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(minimap_suite);
  GREATEST_MAIN_END();
}
