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

typedef struct test_timer_ctx {
#if defined(_WIN32)
  volatile long count;
#else
  volatile size_t count;
#endif
} test_timer_ctx_t;

static void test_timer_func(void *arg) {
  test_timer_ctx_t *ctx = (test_timer_ctx_t *)arg;
#if defined(_WIN32)
  _InterlockedIncrement(&ctx->count);
#else
  __atomic_add_fetch(&ctx->count, 1, __ATOMIC_SEQ_CST);
#endif
}

TEST test_timer_lifecycle(void) {
  cmp_timer_t *timer = NULL;
  test_timer_ctx_t ctx;
  int res;

  ctx.count = 0;

  res = cmp_timer_system_init();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Single shot timer */
  res = cmp_timer_start(&timer, 10, 0, test_timer_func, &ctx);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(timer != NULL);

#if defined(_WIN32)
  Sleep(50);
#else
  usleep(50000);
#endif

  ASSERT_EQ_FMT(1, (int)ctx.count, "%d");

  res = cmp_timer_stop(timer);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Repeating timer */
  ctx.count = 0;
  res = cmp_timer_start(&timer, 10, 1, test_timer_func, &ctx);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(timer != NULL);

#if defined(_WIN32)
  Sleep(100);
#else
  usleep(100000);
#endif

  res = cmp_timer_stop(timer);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  ASSERT((int)ctx.count > 2);

  res = cmp_timer_system_shutdown();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

SUITE(timer_suite) { RUN_TEST(test_timer_lifecycle); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(timer_suite);
  GREATEST_MAIN_END();
}