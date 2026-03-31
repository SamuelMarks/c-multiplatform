/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_vt_shared_lifecycle(void) {
  cmp_vt_shared_t *shared = NULL;
  int res = cmp_vt_shared_create(&shared);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, shared);

  res = cmp_vt_shared_destroy(shared);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_vt_shared_null_args(void) {
  int res = cmp_vt_shared_create(NULL);
  cmp_rect_t o = {0, 0, 10, 10};
  cmp_rect_t n = {10, 10, 20, 20};
  cmp_rect_t r;
  cmp_vt_shared_t *shared = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_vt_shared_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_vt_shared_calculate_morph(NULL, &o, &n, 0.5f, &r);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_vt_shared_create(&shared);
  res = cmp_vt_shared_calculate_morph(shared, NULL, &n, 0.5f, &r);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_vt_shared_calculate_morph(shared, &o, NULL, 0.5f, &r);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_vt_shared_calculate_morph(shared, &o, &n, 0.5f, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_vt_shared_calculate_morph(shared, &o, &n, -0.1f, &r);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_vt_shared_destroy(shared);
  PASS();
}

TEST test_vt_shared_calculate_morph(void) {
  cmp_vt_shared_t *shared = NULL;
  cmp_rect_t o = {0.0f, 0.0f, 100.0f, 100.0f};
  cmp_rect_t n = {50.0f, 50.0f, 200.0f, 200.0f};
  cmp_rect_t r;
  int res;

  cmp_vt_shared_create(&shared);

  /* 0% progress */
  res = cmp_vt_shared_calculate_morph(shared, &o, &n, 0.0f, &r);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(0.0f, r.x, 0.001f);
  ASSERT_IN_RANGE(0.0f, r.y, 0.001f);
  ASSERT_IN_RANGE(100.0f, r.width, 0.001f);
  ASSERT_IN_RANGE(100.0f, r.height, 0.001f);

  /* 50% progress */
  res = cmp_vt_shared_calculate_morph(shared, &o, &n, 0.5f, &r);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(25.0f, r.x, 0.001f);
  ASSERT_IN_RANGE(25.0f, r.y, 0.001f);
  ASSERT_IN_RANGE(150.0f, r.width, 0.001f);
  ASSERT_IN_RANGE(150.0f, r.height, 0.001f);

  /* 100% progress */
  res = cmp_vt_shared_calculate_morph(shared, &o, &n, 1.0f, &r);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_IN_RANGE(50.0f, r.x, 0.001f);
  ASSERT_IN_RANGE(50.0f, r.y, 0.001f);
  ASSERT_IN_RANGE(200.0f, r.width, 0.001f);
  ASSERT_IN_RANGE(200.0f, r.height, 0.001f);

  cmp_vt_shared_destroy(shared);
  PASS();
}

SUITE(vt_shared_suite) {
  RUN_TEST(test_vt_shared_lifecycle);
  RUN_TEST(test_vt_shared_null_args);
  RUN_TEST(test_vt_shared_calculate_morph);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(vt_shared_suite);
  GREATEST_MAIN_END();
}
