/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_mutex_lifecycle(void) {
  cmp_mutex_t mtx;
  int res;

  res = cmp_mutex_init(&mtx);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_mutex_lock(&mtx);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_mutex_unlock(&mtx);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_mutex_destroy(&mtx);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

TEST test_mutex_null(void) {
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_mutex_init(NULL), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_mutex_lock(NULL), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_mutex_unlock(NULL), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_mutex_destroy(NULL), "%d");
  PASS();
}

TEST test_semaphore_lifecycle(void) {
  cmp_semaphore_t sem;
  int res;

  res = cmp_semaphore_init(&sem, 1);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_semaphore_wait(&sem);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_semaphore_post(&sem);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_semaphore_destroy(&sem);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

TEST test_semaphore_null(void) {
  cmp_semaphore_t sem;

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_semaphore_init(NULL, 1), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_semaphore_init(&sem, -1), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_semaphore_wait(NULL), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_semaphore_post(NULL), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_semaphore_destroy(NULL), "%d");

  PASS();
}

TEST test_cond_lifecycle(void) {
  cmp_cond_t cond;
  int res;

  res = cmp_cond_init(&cond);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_cond_signal(&cond);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_cond_broadcast(&cond);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_cond_destroy(&cond);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

TEST test_cond_null(void) {
  cmp_cond_t cond;
  cmp_mutex_t mtx;

  cmp_mutex_init(&mtx);
  cmp_cond_init(&cond);

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_cond_init(NULL), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_cond_wait(NULL, &mtx), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_cond_wait(&cond, NULL), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_cond_signal(NULL), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_cond_broadcast(NULL), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_cond_destroy(NULL), "%d");

  cmp_cond_destroy(&cond);
  cmp_mutex_destroy(&mtx);

  PASS();
}

SUITE(sync_suite) {
  RUN_TEST(test_mutex_lifecycle);
  RUN_TEST(test_mutex_null);
  RUN_TEST(test_semaphore_lifecycle);
  RUN_TEST(test_semaphore_null);
  RUN_TEST(test_cond_lifecycle);
  RUN_TEST(test_cond_null);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(sync_suite);
  GREATEST_MAIN_END();
}
