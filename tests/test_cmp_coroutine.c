/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

typedef struct test_coro_ctx {
  int steps;
  cmp_coroutine_t *co;
} test_coro_ctx_t;

static void test_coroutine_func(cmp_coroutine_t *co, void *arg) {
  test_coro_ctx_t *ctx = (test_coro_ctx_t *)arg;

  ctx->steps++;
  cmp_coroutine_yield(co);

  ctx->steps++;
  cmp_coroutine_yield(co);

  ctx->steps++;
}

TEST test_coroutine_lifecycle(void) {
  cmp_coroutine_t *co = NULL;
  test_coro_ctx_t ctx;
  int res;

  ctx.steps = 0;
  ctx.co = NULL;

  res = cmp_coroutine_system_init();
  if (res == CMP_ERROR_NOT_FOUND) {
    SKIP(); /* Not supported on this platform */
  }
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_coroutine_create(&co, 0, test_coroutine_func, &ctx);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(co != NULL);
  ctx.co = co;

  /* Initial state is READY */
  ASSERT_EQ_FMT((int)CMP_CORO_READY, (int)co->state, "%d");
  ASSERT_EQ_FMT(0, ctx.steps, "%d");

  /* First resume */
  res = cmp_coroutine_resume(co);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT((int)CMP_CORO_SUSPENDED, (int)co->state, "%d");
  ASSERT_EQ_FMT(1, ctx.steps, "%d");

  /* Second resume */
  res = cmp_coroutine_resume(co);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT((int)CMP_CORO_SUSPENDED, (int)co->state, "%d");
  ASSERT_EQ_FMT(2, ctx.steps, "%d");

  /* Third resume (should finish) */
  res = cmp_coroutine_resume(co);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT((int)CMP_CORO_FINISHED, (int)co->state, "%d");
  ASSERT_EQ_FMT(3, ctx.steps, "%d");

  /* Resuming finished coroutine should fail */
  res = cmp_coroutine_resume(co);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  res = cmp_coroutine_destroy(co);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

SUITE(coroutine_suite) { RUN_TEST(test_coroutine_lifecycle); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(coroutine_suite);
  GREATEST_MAIN_END();
}
