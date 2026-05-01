/* clang-format off */
#include "cmp.h"
#include "cmp_ui_skeleton.h"
#include "greatest.h"
/* clang-format on */

TEST test_ui_skeleton_lifecycle(void) {
  cmp_ui_skeleton_t *skeleton = NULL;
  int res;

  res = cmp_ui_skeleton_create(&skeleton, 100.0f, 20.0f, 0xFFE0E0E0);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, skeleton);

  res = cmp_ui_skeleton_destroy(skeleton);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_ui_skeleton_null_args(void) {
  cmp_ui_skeleton_t *skeleton = NULL;
  cmp_ui_node_t *node = NULL;
  int res;

  res = cmp_ui_skeleton_create(NULL, 100.0f, 20.0f, 0);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_skeleton_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_skeleton_create(&skeleton, 100.0f, 20.0f, 0);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_skeleton_get_node(NULL, &node);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_skeleton_get_node(skeleton, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_skeleton_update(NULL, 16.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_skeleton_destroy(skeleton);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_ui_skeleton_operations(void) {
  cmp_ui_skeleton_t *skeleton = NULL;
  cmp_ui_node_t *node = NULL;
  int res;

  res = cmp_ui_skeleton_create(&skeleton, 200.0f, 50.0f, 0xFF000000);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_skeleton_get_node(skeleton, &node);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, node);

  /* Test Shimmer Animation phase wrapping */
  /* By default it starts at 0.0f, dt_ms / 1000.0f adds to it. */
  /* If we add 1500.0f, it should wrap around (1.5 -> 0.5) */
  res = cmp_ui_skeleton_update(skeleton, 1500.0f);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_skeleton_destroy(skeleton);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(ui_skeleton_suite) {
  RUN_TEST(test_ui_skeleton_lifecycle);
  RUN_TEST(test_ui_skeleton_null_args);
  RUN_TEST(test_ui_skeleton_operations);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_skeleton_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
