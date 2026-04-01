/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_resilience_features(void) {
  cmp_resilience_t *ctx = NULL;
  int dummy_node_data = 1;
  void *dummy_node = &dummy_node_data;

  ASSERT_EQ(CMP_SUCCESS, cmp_resilience_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_resilience_show_empty_state(ctx, dummy_node));
  ASSERT_EQ(CMP_SUCCESS, cmp_resilience_show_loading_skeleton(ctx, dummy_node));
  ASSERT_EQ(CMP_SUCCESS, cmp_resilience_show_non_blocking_error(
                             ctx, dummy_node, "Network failure"));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_resilience_handle_discard_changes_prompt(ctx, dummy_node));
  ASSERT_EQ(CMP_SUCCESS, cmp_resilience_graceful_degradation(ctx, "ARKit"));

  ASSERT_EQ(CMP_SUCCESS, cmp_resilience_destroy(ctx));
  PASS();
}

TEST test_resilience_null_args(void) {
  cmp_resilience_t *feat = NULL;
  void *node = (void *)1;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_resilience_create(NULL));
  cmp_resilience_create(&feat);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_resilience_show_empty_state(NULL, node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_resilience_show_empty_state(feat, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_resilience_show_loading_skeleton(NULL, node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_resilience_show_loading_skeleton(feat, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_resilience_show_non_blocking_error(NULL, node, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_resilience_show_non_blocking_error(feat, NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_resilience_show_non_blocking_error(feat, node, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_resilience_handle_discard_changes_prompt(NULL, node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_resilience_handle_discard_changes_prompt(feat, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_resilience_graceful_degradation(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_resilience_graceful_degradation(feat, NULL));

  cmp_resilience_destroy(feat);
  PASS();
}

SUITE(resilience_handling_suite) {
  RUN_TEST(test_resilience_features);
  RUN_TEST(test_resilience_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(resilience_handling_suite);
  GREATEST_MAIN_END();
}
