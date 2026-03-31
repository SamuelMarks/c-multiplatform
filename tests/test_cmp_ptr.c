/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_ptr_lifecycle(void) {
  cmp_ptr_t *ptr = NULL;
  int res = cmp_ptr_create(&ptr);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, ptr);

  res = cmp_ptr_destroy(ptr);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_ptr_null_args(void) {
  int res = cmp_ptr_create(NULL);
  cmp_ptr_state_t state;
  float progress;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ptr_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ptr_update(NULL, 10.0f, 50.0f, &state, &progress);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ptr_set_refreshing(NULL, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  PASS();
}

TEST test_ptr_update_state_machine(void) {
  cmp_ptr_t *ptr = NULL;
  cmp_ptr_state_t state;
  float progress;
  int res = cmp_ptr_create(&ptr);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Negative overscroll means no pull */
  res = cmp_ptr_update(ptr, -10.0f, 50.0f, &state, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_PTR_STATE_IDLE, state);
  ASSERT_EQ_FMT(0.0f, progress, "%f");

  /* Zero overscroll */
  res = cmp_ptr_update(ptr, 0.0f, 50.0f, &state, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_PTR_STATE_IDLE, state);
  ASSERT_EQ_FMT(0.0f, progress, "%f");

  /* Pulling slightly */
  res = cmp_ptr_update(ptr, 25.0f, 50.0f, &state, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_PTR_STATE_PULLING, state);
  ASSERT_EQ_FMT(0.5f, progress, "%f");

  /* Pulling past threshold */
  res = cmp_ptr_update(ptr, 60.0f, 50.0f, &state, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_PTR_STATE_READY_TO_REFRESH, state);
  ASSERT_EQ_FMT(1.0f, progress, "%f");

  /* Trigger refresh */
  res = cmp_ptr_set_refreshing(ptr, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Updates while refreshing should be locked */
  res = cmp_ptr_update(ptr, 10.0f, 50.0f, &state, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_PTR_STATE_REFRESHING, state);
  ASSERT_EQ_FMT(1.0f, progress, "%f");

  /* Stop refresh */
  res = cmp_ptr_set_refreshing(ptr, 0);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Should be idle again */
  res = cmp_ptr_update(ptr, 0.0f, 50.0f, &state, &progress);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_PTR_STATE_IDLE, state);
  ASSERT_EQ_FMT(0.0f, progress, "%f");

  cmp_ptr_destroy(ptr);
  PASS();
}

SUITE(ptr_suite) {
  RUN_TEST(test_ptr_lifecycle);
  RUN_TEST(test_ptr_null_args);
  RUN_TEST(test_ptr_update_state_machine);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ptr_suite);
  GREATEST_MAIN_END();
}
