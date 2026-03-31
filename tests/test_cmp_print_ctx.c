/* clang-format off */
#include <cmp.h>
#include <greatest.h>
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_print_ctx_suite);

TEST test_cmp_print_ctx_create_destroy(void) {
  cmp_print_ctx_t *ctx = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_print_ctx_create(&ctx));
  ASSERT_NEQ(NULL, ctx);

  ASSERT_EQ(CMP_SUCCESS, cmp_print_ctx_destroy(ctx));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_print_ctx_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_print_ctx_destroy(NULL));

  PASS();
}

TEST test_cmp_print_ctx_lifecycle(void) {
  cmp_print_ctx_t *ctx = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_print_ctx_create(&ctx));

  /* Can't save an empty document */
  ASSERT_EQ(CMP_ERROR_INVALID_STATE, cmp_print_ctx_save_pdf(ctx, "test.pdf"));

  /* Begin a page */
  ASSERT_EQ(CMP_SUCCESS, cmp_print_ctx_begin_page(ctx));

  /* Can't begin a page while already in one */
  ASSERT_EQ(CMP_ERROR_INVALID_STATE, cmp_print_ctx_begin_page(ctx));

  /* Can't save while in the middle of a page */
  ASSERT_EQ(CMP_ERROR_INVALID_STATE, cmp_print_ctx_save_pdf(ctx, "test.pdf"));

  /* End the page */
  ASSERT_EQ(CMP_SUCCESS, cmp_print_ctx_end_page(ctx));

  /* Can't end a page if not in one */
  ASSERT_EQ(CMP_ERROR_INVALID_STATE, cmp_print_ctx_end_page(ctx));

  /* Save the document */
  ASSERT_EQ(CMP_SUCCESS, cmp_print_ctx_save_pdf(ctx, "test.pdf"));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_print_ctx_begin_page(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_print_ctx_end_page(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_print_ctx_save_pdf(NULL, "test.pdf"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_print_ctx_save_pdf(ctx, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_print_ctx_destroy(ctx));
  PASS();
}

SUITE(cmp_print_ctx_suite) {
  RUN_TEST(test_cmp_print_ctx_create_destroy);
  RUN_TEST(test_cmp_print_ctx_lifecycle);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_print_ctx_suite);
  GREATEST_MAIN_END();
}
