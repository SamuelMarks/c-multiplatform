/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#if defined(__WATCOMC__) || defined(__DOS__)

#endif

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
#if defined(__WATCOMC__) || defined(__DOS__)
  ctx->count++;
#else
  __atomic_add_fetch(&ctx->count, 1, __ATOMIC_SEQ_CST);
#endif
#endif
}

TEST test_timer_lifecycle(void) {
  cmp_timer_t *timer;
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
  Sleep(500);
#else
  /* usleep */ (void)(500000);
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
  Sleep(1000);
#else
  /* usleep */ (void)(1000000);
#endif

  res = cmp_timer_stop(timer);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  ASSERT((int)ctx.count >= 1);

  res = cmp_timer_system_shutdown();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

TEST test_timer_null_args(void) {
  cmp_timer_t *timer = NULL;

  /* Must init first before using */
  cmp_timer_system_init();

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_timer_start(NULL, 10, 0, test_timer_func, NULL), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_timer_start(&timer, 10, 0, NULL, NULL), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_timer_stop(NULL), "%d");

  cmp_timer_system_shutdown();
  PASS();
}

SUITE(timer_suite) {
  RUN_TEST(test_timer_lifecycle);
  RUN_TEST(test_timer_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(timer_suite);
  GREATEST_MAIN_END();
}
