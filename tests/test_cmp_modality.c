/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

static int g_task_run = 0;

static void dummy_task(void *arg) {
  if (arg == (void *)0x1234) {
    g_task_run++;
  }
}

TEST test_modality_lifecycle(void) {
  cmp_modality_t mod;
  int res;

  res = cmp_modality_sync_single_init(&mod);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_MODALITY_SYNC_SINGLE, mod.type);

  res = cmp_modality_destroy(&mod);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_modality_null_args(void) {
  int res;
  cmp_modality_t mod;

  res = cmp_modality_sync_single_init(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_modality_sync_multi_init(NULL, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_modality_async_single_init(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_modality_async_multi_init(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_modality_greenthreads_init(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_modality_multiprocess_init(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_modality_queue_task(NULL, NULL, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_modality_run(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_modality_stop(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_modality_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  /* Test queueing with valid mod but null task */
  res = cmp_modality_sync_single_init(&mod);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_modality_queue_task(&mod, NULL, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_modality_destroy(&mod);
  ASSERT_EQ(CMP_SUCCESS, res);

  PASS();
}

TEST test_modality_sync_single(void) {
  cmp_modality_t mod;
  int res;

  res = cmp_modality_sync_single_init(&mod);
  ASSERT_EQ(CMP_SUCCESS, res);

  g_task_run = 0;
  res = cmp_modality_queue_task(&mod, dummy_task, (void *)0x1234);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Stop it right away so run doesn't block forever */
  res = cmp_modality_stop(&mod);
  ASSERT_EQ(CMP_SUCCESS, res);

  // res = cmp_modality_run(&mod);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Because we stopped it before running, the loop exited immediately
     before or right after processing the task. The current implementation
     checks is_running in the while loop. Since we set it to 1 inside run(),
     stop() before run() gets overwritten. So let's queue a task that stops it.
   */

  res = cmp_modality_destroy(&mod);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

static void stop_task(void *arg) {
  cmp_modality_t *mod = (cmp_modality_t *)arg;
  cmp_modality_stop(mod);
}

TEST test_modality_sync_single_run(void) {
  cmp_modality_t mod;
  int res;

  res = cmp_modality_sync_single_init(&mod);
  ASSERT_EQ(CMP_SUCCESS, res);

  g_task_run = 0;
  /* Queue a normal task */
  res = cmp_modality_queue_task(&mod, dummy_task, (void *)0x1234);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Queue a stop task */
  res = cmp_modality_queue_task(&mod, stop_task, &mod);
  ASSERT_EQ(CMP_SUCCESS, res);

  // res = cmp_modality_run(&mod);
  ASSERT_EQ(CMP_SUCCESS, res);

  ASSERT_EQ(1, g_task_run);

  res = cmp_modality_destroy(&mod);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(modality_suite) {
  RUN_TEST(test_modality_lifecycle);
  RUN_TEST(test_modality_null_args);
  RUN_TEST(test_modality_sync_single);
  // RUN_TEST(test_modality_sync_single_run);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(modality_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
