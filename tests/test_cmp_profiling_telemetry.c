/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

static void test_task(void *data) { (void)data; }

TEST test_profiling_telemetry_features(void) {
  cmp_profiling_telemetry_t *ctx = NULL;
  int dummy_node_data = 1;
  void *dummy_node = &dummy_node_data;

  ASSERT_EQ(CMP_SUCCESS, cmp_profiling_telemetry_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_profiling_emit_os_signpost(ctx, "Layout", 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_profiling_enforce_main_thread(ctx));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_profiling_offload_heavy_task(ctx, test_task, (void *)1));
  ASSERT_EQ(CMP_SUCCESS, cmp_profiling_detect_retain_cycles(ctx, dummy_node));
  ASSERT_EQ(CMP_SUCCESS, cmp_profiling_safe_destroy_node(ctx, &dummy_node));
  ASSERT_EQ(NULL, dummy_node);

  ASSERT_EQ(CMP_SUCCESS, cmp_profiling_telemetry_destroy(ctx));
  PASS();
}

TEST test_profiling_telemetry_null_args(void) {
  cmp_profiling_telemetry_t *feat = NULL;
  void *node = (void *)1;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_profiling_telemetry_create(NULL));
  cmp_profiling_telemetry_create(&feat);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_profiling_emit_os_signpost(NULL, "a", 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_profiling_emit_os_signpost(feat, NULL, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_profiling_enforce_main_thread(NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_profiling_offload_heavy_task(NULL, test_task, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_profiling_offload_heavy_task(feat, NULL, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_profiling_detect_retain_cycles(NULL, node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_profiling_detect_retain_cycles(feat, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_profiling_safe_destroy_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_profiling_safe_destroy_node(feat, NULL));

  cmp_profiling_telemetry_destroy(feat);
  PASS();
}

SUITE(profiling_telemetry_suite) {
  RUN_TEST(test_profiling_telemetry_features);
  RUN_TEST(test_profiling_telemetry_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(profiling_telemetry_suite);
  GREATEST_MAIN_END();
}
