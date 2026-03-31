/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

TEST test_scroll_ctx_create_destroy(void) {
  cmp_scroll_ctx_t *ctx = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_ctx_create(&ctx));
  ASSERT_NEQ(NULL, ctx);
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_ctx_destroy(ctx));

  PASS();
}

TEST test_scroll_ctx_offsets(void) {
  cmp_scroll_ctx_t *ctx = NULL;
  float x = -1.0f, y = -1.0f;

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_ctx_create(&ctx));

  /* Should default to 0,0 */
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_ctx_get_offset(ctx, &x, &y));
  ASSERT_EQ(0.0f, x);
  ASSERT_EQ(0.0f, y);

  /* Set and Get */
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_ctx_set_offset(ctx, 150.5f, 300.25f));
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_ctx_get_offset(ctx, &x, &y));
  ASSERT_EQ(150.5f, x);
  ASSERT_EQ(300.25f, y);

  /* Partial Get is allowed */
  x = -1.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_ctx_get_offset(ctx, &x, NULL));
  ASSERT_EQ(150.5f, x);

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_ctx_destroy(ctx));
  PASS();
}

TEST test_scroll_ctx_gpu_injection(void) {
  cmp_scroll_ctx_t *ctx = NULL;
  cmp_ubo_t *ubo = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_ctx_create(&ctx));
  ASSERT_EQ(CMP_SUCCESS, cmp_ubo_create(sizeof(float) * 2, &ubo));

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_ctx_set_offset(ctx, 100.0f, 200.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_ctx_inject_to_gpu(ctx, ubo));

  ASSERT_EQ(CMP_SUCCESS, cmp_ubo_destroy(ubo));
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_ctx_destroy(ctx));

  PASS();
}

TEST test_scroll_ctx_edge_cases(void) {
  cmp_scroll_ctx_t *ctx = NULL;
  float x = 0.0f, y = 0.0f;
  cmp_ubo_t *ubo = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_scroll_ctx_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_scroll_ctx_destroy(NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_scroll_ctx_get_offset(NULL, &x, &y));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_scroll_ctx_set_offset(NULL, x, y));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_scroll_ctx_inject_to_gpu(NULL, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_ctx_create(&ctx));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_scroll_ctx_inject_to_gpu(ctx, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_ubo_create(sizeof(float) * 2, &ubo));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_scroll_ctx_inject_to_gpu(NULL, ubo));

  ASSERT_EQ(CMP_SUCCESS, cmp_ubo_destroy(ubo));
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_ctx_destroy(ctx));

  PASS();
}

SUITE(cmp_scroll_ctx_suite) {
  RUN_TEST(test_scroll_ctx_create_destroy);
  RUN_TEST(test_scroll_ctx_offsets);
  RUN_TEST(test_scroll_ctx_gpu_injection);
  RUN_TEST(test_scroll_ctx_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_scroll_ctx_suite);
  GREATEST_MAIN_END();
}
