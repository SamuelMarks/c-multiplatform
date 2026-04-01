/* clang-format off */
#include "cmp.h"
#include "greatest.h"

#if defined(_WIN32)
__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);
long _InterlockedIncrement(long volatile *Addend);
#pragma intrinsic(_InterlockedIncrement)
#else
#include <unistd.h>
#endif
/* clang-format on */

typedef struct threaded_test_counter {
#if defined(_WIN32)
  volatile long count;
#else
  volatile size_t count;
#endif
} threaded_test_counter_t;

static void test_task_increment_atomic(void *arg) {
  threaded_test_counter_t *ctx = (threaded_test_counter_t *)arg;
#if defined(_WIN32)
  _InterlockedIncrement(&ctx->count);
#else
  __atomic_add_fetch(&ctx->count, 1, __ATOMIC_SEQ_CST);
#endif
}

TEST test_modality_threaded_lifecycle(void) {
  cmp_modality_t mod;
  threaded_test_counter_t ctx;
  int res;
  int i;
  int wait_cycles;

  ctx.count = 0;

  res = cmp_modality_threaded_init(&mod, 4);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(mod.type == CMP_MODALITY_THREADED);

  /* Queue 1000 tasks */
  for (i = 0; i < 1000; i++) {
    res = cmp_modality_queue_task(&mod, test_task_increment_atomic, &ctx);
    ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  }

  res = cmp_modality_run(&mod); /* Returns immediately for threaded */
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Wait for tasks to complete (max ~5 seconds) */
  wait_cycles = 0;
  while (ctx.count < 1000 && wait_cycles < 500) {
#if defined(_WIN32)
    Sleep(10);
#else
    usleep(10000);
#endif
    wait_cycles++;
  }

  res = cmp_modality_stop(&mod);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

#if defined(_WIN32)
  Sleep(50);
#else
  usleep(50000);
#endif

  res = cmp_modality_destroy(&mod);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  ASSERT_EQ_FMT(1000, (int)ctx.count, "%d");

  PASS();
}

TEST test_modality_threaded_massive_queue(void) {
  cmp_modality_t mod;
  threaded_test_counter_t ctx;
  int res;
  int i;
  int wait_cycles;

  ctx.count = 0;

  res = cmp_modality_threaded_init(&mod, 8); /* 8 workers */
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_modality_run(&mod);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Queue 100,000 tasks */
  for (i = 0; i < 100000; i++) {
    /* If the ring buffer fills up, queue_task will fail with CMP_ERROR_BOUNDS.
     * In a real app we'd block or retry, but for the test we retry spinning
     * slightly */
    do {
      res = cmp_modality_queue_task(&mod, test_task_increment_atomic, &ctx);
      if (res == CMP_ERROR_BOUNDS) {
#if defined(_WIN32)
        Sleep(1);
#else
        usleep(1000);
#endif
      }
    } while (res == CMP_ERROR_BOUNDS);

    ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  }

  wait_cycles = 0;
  while (ctx.count < 100000 && wait_cycles < 2000) {
#if defined(_WIN32)
    Sleep(10);
#else
    usleep(10000);
#endif
    wait_cycles++;
  }

  res = cmp_modality_stop(&mod);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

#if defined(_WIN32)
  Sleep(100);
#else
  usleep(100000);
#endif

  res = cmp_modality_destroy(&mod);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  ASSERT_EQ_FMT(100000, (int)ctx.count, "%d");

  PASS();
}

SUITE(modality_threaded_suite) {
  RUN_TEST(test_modality_threaded_lifecycle);
  RUN_TEST(test_modality_threaded_massive_queue);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(modality_threaded_suite);
  GREATEST_MAIN_END();
}
