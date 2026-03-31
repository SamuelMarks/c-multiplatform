/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_flip_lifecycle(void) {
  cmp_flip_t *flip = NULL;
  int res = cmp_flip_create(&flip);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, flip);

  res = cmp_flip_destroy(flip);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_flip_null_args(void) {
  int res = cmp_flip_create(NULL);
  cmp_rect_t rect = {0, 0, 10, 10};
  cmp_flip_t *flip = NULL;
  float tx, ty, sx, sy;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_flip_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_flip_first(NULL, &rect);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_flip_create(&flip);
  res = cmp_flip_first(flip, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_flip_last_and_invert(NULL, &rect, &tx, &ty, &sx, &sy);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_flip_last_and_invert(flip, NULL, &tx, &ty, &sx, &sy);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_flip_destroy(flip);
  PASS();
}

TEST test_flip_uninitialized(void) {
  cmp_flip_t *flip = NULL;
  cmp_rect_t rect = {0, 0, 10, 10};
  float tx, ty, sx, sy;
  int res;

  cmp_flip_create(&flip);

  /* Missing first */
  res = cmp_flip_last_and_invert(flip, &rect, &tx, &ty, &sx, &sy);
  ASSERT_EQ(CMP_ERROR_INVALID_STATE, res);

  cmp_flip_destroy(flip);
  PASS();
}

TEST test_flip_calculate(void) {
  cmp_flip_t *flip = NULL;
  cmp_rect_t first_rect = {10.0f, 10.0f, 100.0f, 100.0f};
  cmp_rect_t last_rect = {50.0f, 60.0f, 200.0f, 50.0f};
  float tx, ty, sx, sy;
  int res;

  cmp_flip_create(&flip);

  res = cmp_flip_first(flip, &first_rect);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_flip_last_and_invert(flip, &last_rect, &tx, &ty, &sx, &sy);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Invert translation: initial - final */
  ASSERT_IN_RANGE(-40.0f, tx, 0.001f); /* 10 - 50 */
  ASSERT_IN_RANGE(-50.0f, ty, 0.001f); /* 10 - 60 */

  /* Invert scale: initial / final */
  ASSERT_IN_RANGE(0.5f, sx, 0.001f); /* 100 / 200 */
  ASSERT_IN_RANGE(2.0f, sy, 0.001f); /* 100 / 50 */

  cmp_flip_destroy(flip);
  PASS();
}

SUITE(flip_suite) {
  RUN_TEST(test_flip_lifecycle);
  RUN_TEST(test_flip_null_args);
  RUN_TEST(test_flip_uninitialized);
  RUN_TEST(test_flip_calculate);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(flip_suite);
  GREATEST_MAIN_END();
}
