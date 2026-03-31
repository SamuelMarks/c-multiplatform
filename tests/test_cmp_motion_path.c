/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_motion_path_lifecycle(void) {
  cmp_motion_path_t *path = NULL;
  int res = cmp_motion_path_create(&path);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, path);

  res = cmp_motion_path_destroy(path);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_motion_path_null_args(void) {
  int res = cmp_motion_path_create(NULL);
  float x, y, a;
  cmp_motion_path_t *path = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_motion_path_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_motion_path_evaluate(NULL, 0.5f, 0.0f, &x, &y, &a);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_motion_path_create(&path);
  res = cmp_motion_path_evaluate(path, 0.5f, 0.0f, NULL, &y, &a);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_motion_path_evaluate(path, -0.1f, 0.0f, &x, &y, &a);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_motion_path_destroy(path);
  PASS();
}

TEST test_motion_path_evaluate(void) {
  cmp_motion_path_t *path = NULL;
  float x, y, a;
  int res;

  cmp_motion_path_create(&path);

  res = cmp_motion_path_evaluate(path, 0.0f, 0.0f, &x, &y, &a);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(0.0f, x, 0.001f);
  ASSERT_IN_RANGE(0.0f, y, 0.001f);

  res = cmp_motion_path_evaluate(path, 1.0f, 90.0f, &x, &y, &a);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(100.0f, x, 0.001f);
  ASSERT_IN_RANGE(0.0f, y, 0.001f);

  cmp_motion_path_destroy(path);
  PASS();
}

SUITE(motion_path_suite) {
  RUN_TEST(test_motion_path_lifecycle);
  RUN_TEST(test_motion_path_null_args);
  RUN_TEST(test_motion_path_evaluate);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(motion_path_suite);
  GREATEST_MAIN_END();
}
