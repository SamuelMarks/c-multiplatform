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

  res = cmp_modality_sync_single_init(&mod);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(mod.type == CMP_MODALITY_SYNC_SINGLE);

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

TEST test_modality_null_args(void) {
  cmp_modality_t mod;
  cmp_actor_t *actor = NULL;
  cmp_msg_t *msg = (cmp_msg_t *)0x1234;
  cmp_msg_bus_t *bus = (cmp_msg_bus_t *)0x1234;

  cmp_modality_sync_single_init(&mod);

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_modality_sync_single_init(NULL),
                "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_modality_sync_multi_init(NULL, 1),
                "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_modality_sync_multi_init(&mod, 0),
                "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_modality_async_single_init(NULL),
                "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_modality_async_multi_init(NULL),
                "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_modality_greenthreads_init(NULL),
                "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_modality_multiprocess_init(NULL),
                "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_modality_destroy(NULL), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_modality_run(NULL), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_modality_stop(NULL), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_modality_queue_task(NULL, test_task_increment, NULL), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_modality_queue_task(&mod, NULL, NULL), "%d");

  cmp_modality_destroy(&mod);

  /* Stubs */
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_app_init(NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_app_init((cmp_app_config_t *)1), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_msg_subscribe(NULL, "chan", (void *)1), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_msg_subscribe(bus, NULL, (void *)1),
                "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_msg_subscribe(bus, "chan", NULL),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_msg_subscribe(bus, "chan", (void *)1), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_msg_publish(NULL, "chan", msg),
                "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_msg_publish(bus, NULL, msg), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_msg_publish(bus, "chan", NULL),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_msg_publish(bus, "chan", msg), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_actor_spawn(NULL, "name", (void *)1, NULL, &actor), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_actor_spawn(bus, NULL, (void *)1, NULL, &actor), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_actor_spawn(bus, "name", NULL, NULL, &actor), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_actor_spawn(bus, "name", (void *)1, NULL, NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_actor_spawn(bus, "name", (void *)1, NULL, &actor), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_actor_supervise(NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_actor_supervise((cmp_actor_t *)1), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_modality_async_multi_init(&mod), "%d");
#if defined(_WIN32) || defined(__linux__) ||                                   \
    (defined(__APPLE__) && !defined(__arm64__) && !defined(__aarch64__))
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_modality_greenthreads_init(&mod), "%d");
#else
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, cmp_modality_greenthreads_init(&mod),
                "%d");
#endif
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_modality_multiprocess_init(&mod), "%d");

  {
    int run_loop_called = 0;
    cmp_run_loop(NULL, NULL);
    ASSERT_EQ_FMT(0, run_loop_called, "%d");
  }

  PASS();
}

SUITE(modality_suite) {
  RUN_TEST(test_modality_single_lifecycle);
  RUN_TEST(test_modality_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(modality_suite);
  GREATEST_MAIN_END();
}
