/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

typedef struct test_counter {
  int count;
  cmp_modality_t *mod;
} test_counter_t;

static void test_task_increment(void *arg) {
  test_counter_t *ctx = (test_counter_t *)arg;
  ctx->count++;
  if (ctx->count >= 5) {
    cmp_modality_stop(ctx->mod);
  }
}

TEST test_modality_single_lifecycle(void) {
  cmp_modality_t mod;
  test_counter_t ctx;
  int res;
  int i;

  ctx.count = 0;
  ctx.mod = &mod;

  res = cmp_modality_single_init(&mod);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(mod.type == CMP_MODALITY_SINGLE);

  /* Queue 5 tasks */
  for (i = 0; i < 5; i++) {
    res = cmp_modality_queue_task(&mod, test_task_increment, &ctx);
    ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  }

  /* Run until stopped by the 5th task */
  res = cmp_modality_run(&mod);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT(5, ctx.count, "%d");

  res = cmp_modality_destroy(&mod);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

SUITE(modality_suite) { RUN_TEST(test_modality_single_lifecycle); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(modality_suite);
  GREATEST_MAIN_END();
}
