/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_search_lifecycle(void) {
  cmp_search_controller_t *ctx = NULL;
  const char *sugs[] = {"Apple", "Banana", "Cherry"};

  ASSERT_EQ(CMP_SUCCESS, cmp_search_controller_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_search_controller_set_text(ctx, "Hello"));
  ASSERT_EQ(CMP_SUCCESS, cmp_search_controller_add_scope(ctx, "All"));
  ASSERT_EQ(CMP_SUCCESS, cmp_search_controller_add_scope(ctx, "Unread"));
  ASSERT_EQ(CMP_SUCCESS, cmp_search_controller_set_active(ctx, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_search_controller_set_suggestions(ctx, sugs, 3));

  ASSERT_EQ(CMP_SUCCESS, cmp_search_controller_destroy(ctx));
  PASS();
}

TEST test_search_metrics(void) {
  cmp_search_controller_t *ctx = NULL;
  int clear, cancel;
  float offset;

  ASSERT_EQ(CMP_SUCCESS, cmp_search_controller_create(&ctx));

  /* Inactive */
  ASSERT_EQ(CMP_SUCCESS, cmp_search_controller_set_active(ctx, 0));
  ASSERT_EQ(CMP_SUCCESS, cmp_search_controller_resolve_metrics(
                             ctx, 375.0f, &clear, &cancel, &offset));
  ASSERT_EQ(0, clear);
  ASSERT_EQ(0, cancel);
  ASSERT_GT(offset, 16.0f); /* Should be centered */

  /* Active Empty */
  ASSERT_EQ(CMP_SUCCESS, cmp_search_controller_set_active(ctx, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_search_controller_resolve_metrics(
                             ctx, 375.0f, &clear, &cancel, &offset));
  ASSERT_EQ(0, clear); /* No text yet */
  ASSERT_EQ(1, cancel);
  ASSERT_EQ(16.0f, offset); /* Snapped to left */

  /* Active with text */
  ASSERT_EQ(CMP_SUCCESS, cmp_search_controller_set_text(ctx, "Query"));
  ASSERT_EQ(CMP_SUCCESS, cmp_search_controller_resolve_metrics(
                             ctx, 375.0f, &clear, &cancel, &offset));
  ASSERT_EQ(1, clear);
  ASSERT_EQ(1, cancel);
  ASSERT_EQ(16.0f, offset);

  ASSERT_EQ(CMP_SUCCESS, cmp_search_controller_destroy(ctx));
  PASS();
}

TEST test_null_args(void) {
  cmp_search_controller_t *ctx = NULL;
  int i;
  float f;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_search_controller_create(NULL));
  cmp_search_controller_create(&ctx);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_search_controller_set_text(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_search_controller_add_scope(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_search_controller_add_scope(ctx, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_search_controller_set_active(NULL, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_search_controller_resolve_metrics(NULL, 100.0f, &i, &i, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_search_controller_resolve_metrics(ctx, 100.0f, NULL, &i, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_search_controller_resolve_metrics(ctx, 100.0f, &i, NULL, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_search_controller_resolve_metrics(ctx, 100.0f, &i, &i, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_search_controller_set_suggestions(NULL, NULL, 0));

  cmp_search_controller_destroy(ctx);
  PASS();
}

SUITE(search_bar_suite) {
  RUN_TEST(test_search_lifecycle);
  RUN_TEST(test_search_metrics);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(search_bar_suite);
  GREATEST_MAIN_END();
}
