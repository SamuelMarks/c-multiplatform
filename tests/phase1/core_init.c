/* clang-format off */
#include "cmpc/cmp_core.h"
#include "test_utils.h"

#include <stdlib.h>
/* clang-format on */

static int test_run(void *ctx) {
  int *val = (int *)ctx;
  *val = 1;
  return CMP_OK;
}

static int test_stop(void *ctx) {
  int *val = (int *)ctx;
  *val = 2;
  return CMP_OK;
}

static int test_add_io(void *ctx, int fd, cmp_u32 flags,
                       CMPEventLoopIOCallback cb, void *cb_ctx,
                       CMPEventLoopIOHandle *out_handle) {
  (void)ctx;
  (void)fd;
  (void)flags;
  (void)cb;
  (void)cb_ctx;
  (void)out_handle;
  return CMP_OK;
}

static int test_remove_io(void *ctx, CMPEventLoopIOHandle handle) {
  (void)ctx;
  (void)handle;
  return CMP_OK;
}

static int test_set_timer(void *ctx, cmp_u32 timeout_ms, CMPBool repeat,
                          CMPEventLoopTimerCallback cb, void *cb_ctx,
                          CMPEventLoopTimerHandle *out_handle) {
  (void)ctx;
  (void)timeout_ms;
  (void)repeat;
  (void)cb;
  (void)cb_ctx;
  (void)out_handle;
  return CMP_OK;
}

static int test_clear_timer(void *ctx, CMPEventLoopTimerHandle handle) {
  (void)ctx;
  (void)handle;
  return CMP_OK;
}

static int test_wake(void *ctx, CMPEventLoopWakeCallback cb, void *cb_ctx) {
  (void)ctx;
  (void)cb;
  (void)cb_ctx;
  return CMP_OK;
}

static int test_event_loop(void) {
  int val;
  CMPEventLoop loop;
  CMPEventLoopVTable vtable;

  val = 0;
  vtable.run = test_run;
  vtable.stop = test_stop;
  vtable.add_io = test_add_io;
  vtable.remove_io = test_remove_io;
  vtable.set_timer = test_set_timer;
  vtable.clear_timer = test_clear_timer;
  vtable.wake = test_wake;

  loop.ctx = &val;
  loop.vtable = &vtable;

  CMP_TEST_EXPECT(cmp_event_loop_run(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_event_loop_stop(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_event_loop_run(&loop));
  CMP_TEST_ASSERT(val == 1);

  CMP_TEST_OK(cmp_event_loop_stop(&loop));
  CMP_TEST_ASSERT(val == 2);

  loop.vtable = NULL;
  CMP_TEST_EXPECT(cmp_event_loop_run(&loop), CMP_ERR_INVALID_ARGUMENT);

  loop.vtable = &vtable;
  CMP_TEST_OK(cmp_event_loop_add_io(&loop, 0, 0, NULL, NULL, NULL));
  CMP_TEST_OK(cmp_event_loop_remove_io(&loop, NULL));
  CMP_TEST_OK(cmp_event_loop_set_timer(&loop, 0, 0, NULL, NULL, NULL));
  CMP_TEST_OK(cmp_event_loop_clear_timer(&loop, NULL));
  CMP_TEST_OK(cmp_event_loop_wake(&loop, NULL, NULL));

  loop.vtable = NULL;
  CMP_TEST_EXPECT(cmp_event_loop_add_io(&loop, 0, 0, NULL, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_event_loop_remove_io(&loop, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_event_loop_set_timer(&loop, 0, 0, NULL, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_event_loop_clear_timer(&loop, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_event_loop_wake(&loop, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  return 0;
}

static int test_app_config_init(void) {
  CMPAppConfig config;

  CMP_TEST_EXPECT(cmp_app_config_init(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_app_config_init(&config));
  CMP_TEST_ASSERT(config.modality == CMP_MODALITY_SYNC_SINGLE);

  return 0;
}

static int test_app_init(void) {
  CMPAppConfig config;

  CMP_TEST_EXPECT(cmp_app_init(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_app_config_init(&config));
  CMP_TEST_OK(cmp_app_init(&config));
  CMP_TEST_OK(cmp_app_destroy());

  config.modality = CMP_MODALITY_SYNC_MULTI;
  CMP_TEST_OK(cmp_app_init(&config));
  CMP_TEST_OK(cmp_app_destroy());

  return 0;
}

int main(void) {
  if (test_event_loop() != 0) {
    return 1;
  }
  if (test_app_config_init() != 0) {
    return 1;
  }
  if (test_app_init() != 0) {
    return 1;
  }
  return 0;
}
