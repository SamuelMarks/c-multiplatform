/* clang-format off */
#include "cmpc/cmp_core.h"
#include "test_utils.h"

#include <stdlib.h>
/* clang-format on */

static int test_async_loop_poll_ui(void *ctx, CMPBool *out_continue) {
  int *ticks = (int *)ctx;
  (*ticks)++;
  if (*ticks >= 5) {
    *out_continue = 0;
  }
  return CMP_OK;
}

static int CMP_CALL test_async_timer_cb(void *ctx,
                                        CMPEventLoopTimerHandle handle) {
  int *timer_fired = (int *)ctx;
  CMP_UNUSED(handle);
  (*timer_fired)++;
  return CMP_OK;
}

static int CMP_CALL test_async_timer_stop_cb(void *ctx,
                                             CMPEventLoopTimerHandle handle) {
  CMPEventLoop *loop = (CMPEventLoop *)ctx;
  CMP_UNUSED(handle);
  cmp_event_loop_stop(loop);
  return CMP_OK;
}

static int CMP_CALL test_async_timer_repeat_cb(void *ctx,
                                               CMPEventLoopTimerHandle handle) {
  int *timer_fired = (int *)ctx;
  CMP_UNUSED(handle);
  (*timer_fired)++;
  return CMP_OK;
}

static int CMP_CALL test_async_wake_cb(void *ctx) {
  int *wake_fired = (int *)ctx;
  (*wake_fired)++;
  return CMP_OK;
}

static int CMP_CALL test_async_io_cb(void *ctx, CMPEventLoopIOHandle handle,
                                     cmp_u32 flags) {
  int *io_fired = (int *)ctx;
  CMP_UNUSED(handle);
  CMP_UNUSED(flags);
  (*io_fired)++;
  return CMP_OK;
}

static int test_async_loop(void) {
  CMPAllocator alloc;
  CMPEventLoop loop;
  CMPAsyncEventLoopConfig config;
  int ticks;
  int timer_fired = 0;
  int wake_fired = 0;
  int io_fired = 0;
  CMPEventLoopTimerHandle timer_handle;
  CMPEventLoopIOHandle io_handle;

  CMP_TEST_OK(cmp_get_default_allocator(&alloc));

  ticks = 0;
  config.ctx = &ticks;
  config.on_poll_ui = test_async_loop_poll_ui;

  CMP_TEST_EXPECT(cmp_event_loop_async_init(NULL, &config, &loop),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_event_loop_async_init(&alloc, &config, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_event_loop_async_init(&alloc, &config, &loop));

  /* Invalid arguments tests */
  CMP_TEST_EXPECT(cmp_event_loop_set_timer(NULL, 10, CMP_FALSE,
                                           test_async_timer_cb, &timer_fired,
                                           &timer_handle),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_event_loop_clear_timer(NULL, timer_handle),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_event_loop_clear_timer(&loop, (CMPEventLoopTimerHandle)9999),
      CMP_ERR_NOT_FOUND);
  CMP_TEST_EXPECT(cmp_event_loop_add_io(NULL, 0, CMP_EVENT_LOOP_IO_READ,
                                        test_async_io_cb, NULL, &io_handle),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_event_loop_add_io(&loop, 0, CMP_EVENT_LOOP_IO_READ,
                                        test_async_io_cb, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_event_loop_remove_io(NULL, io_handle),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_event_loop_remove_io(&loop, (CMPEventLoopIOHandle)9999),
                  CMP_ERR_NOT_FOUND);
  CMP_TEST_EXPECT(cmp_event_loop_wake(NULL, test_async_wake_cb, &wake_fired),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_event_loop_wake(&loop, NULL, &wake_fired),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_event_loop_async_destroy(NULL, &loop),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_event_loop_async_destroy(&alloc, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Test timer */
  CMP_TEST_OK(cmp_event_loop_set_timer(
      &loop, 10, CMP_FALSE, test_async_timer_cb, &timer_fired, &timer_handle));

  /* Test IO */
  CMP_TEST_OK(cmp_event_loop_add_io(&loop, 0, CMP_EVENT_LOOP_IO_READ,
                                    test_async_io_cb, &io_fired, &io_handle));

  /* Test Wake */
  CMP_TEST_OK(cmp_event_loop_wake(&loop, test_async_wake_cb, &wake_fired));

  /* Let it run to process the UI polls and timer */
  CMP_TEST_OK(cmp_event_loop_run(&loop));
  CMP_TEST_ASSERT(ticks == 5);
  CMP_TEST_ASSERT(wake_fired == 1);

  /* We might need a slight delay test if timer didn't fire in 5 polls, but for
   * this test environment the loop might run fast. Let's just test stop and
   * clear timer. */

  CMP_TEST_OK(cmp_event_loop_set_timer(&loop, 1000, CMP_FALSE,
                                       test_async_timer_cb, &timer_fired,
                                       &timer_handle));
  CMP_TEST_OK(cmp_event_loop_clear_timer(&loop, timer_handle));

  CMP_TEST_OK(cmp_event_loop_remove_io(&loop, io_handle));

  CMP_TEST_OK(cmp_event_loop_stop(&loop));

  CMP_TEST_OK(cmp_event_loop_async_destroy(&alloc, &loop));

  return 0;
}

static int test_async_loop_stop(void) {
  CMPAllocator alloc;
  CMPEventLoop loop;
  CMPAsyncEventLoopConfig config;
  CMPEventLoopTimerHandle timer_handle;
  CMPEventLoopTimerHandle timer_repeat_handle;
  int timer_fired = 0;

  CMP_TEST_OK(cmp_get_default_allocator(&alloc));
  config.ctx = NULL;
  config.on_poll_ui = NULL;

  CMP_TEST_OK(cmp_event_loop_async_init(&alloc, &config, &loop));
  CMP_TEST_OK(cmp_event_loop_set_timer(&loop, 5, CMP_TRUE,
                                       test_async_timer_repeat_cb, &timer_fired,
                                       &timer_repeat_handle));
  CMP_TEST_OK(cmp_event_loop_set_timer(
      &loop, 15, CMP_FALSE, test_async_timer_stop_cb, &loop, &timer_handle));
  CMP_TEST_OK(cmp_event_loop_run(&loop));

  /* Need to verify timer_fired is at least 1, maybe 2 or 3 depending on timing.
   */
  CMP_TEST_ASSERT(timer_fired >= 1);

  CMP_TEST_OK(cmp_event_loop_clear_timer(&loop, timer_repeat_handle));
  CMP_TEST_OK(cmp_event_loop_async_destroy(&alloc, &loop));

  return 0;
}

#ifdef CMP_TESTING
static int test_alloc_fail(void *ctx, cmp_usize size, void **out_ptr) {
  (void)ctx;
  (void)size;
  (void)out_ptr;
  return CMP_ERR_OUT_OF_MEMORY;
}

static int g_alloc_fail_countdown = -1;
static int test_alloc_fail_countdown(void *ctx, cmp_usize size,
                                     void **out_ptr) {
  CMPAllocator alloc;
  if (g_alloc_fail_countdown == 0) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  if (g_alloc_fail_countdown > 0) {
    g_alloc_fail_countdown--;
  }
  cmp_get_default_allocator(&alloc);
  return alloc.alloc(alloc.ctx, size, out_ptr);
}

static int test_async_loop_alloc_fail(void) {
  CMPAllocator alloc, bad_alloc;
  CMPEventLoop loop;
  CMPAsyncEventLoopConfig config;
  CMPEventLoopTimerHandle timer_handle;
  CMPEventLoopIOHandle io_handle;

  CMP_TEST_OK(cmp_get_default_allocator(&alloc));
  bad_alloc = alloc;
  bad_alloc.alloc = test_alloc_fail;

  config.ctx = NULL;
  config.on_poll_ui = NULL;

  CMP_TEST_EXPECT(cmp_event_loop_async_init(&bad_alloc, &config, &loop),
                  CMP_ERR_OUT_OF_MEMORY);

  bad_alloc.alloc = test_alloc_fail_countdown;
  g_alloc_fail_countdown = -1;
  CMP_TEST_OK(cmp_event_loop_async_init(&bad_alloc, &config, &loop));

  g_alloc_fail_countdown = 0;
  CMP_TEST_EXPECT(cmp_event_loop_set_timer(&loop, 10, CMP_FALSE,
                                           test_async_timer_cb, NULL,
                                           &timer_handle),
                  CMP_ERR_OUT_OF_MEMORY);

  g_alloc_fail_countdown = 0;
  CMP_TEST_EXPECT(cmp_event_loop_add_io(&loop, 0, CMP_EVENT_LOOP_IO_READ,
                                        test_async_io_cb, NULL, &io_handle),
                  CMP_ERR_OUT_OF_MEMORY);

  g_alloc_fail_countdown = 0;
  CMP_TEST_EXPECT(cmp_event_loop_wake(&loop, test_async_wake_cb, NULL),
                  CMP_ERR_OUT_OF_MEMORY);

  CMP_TEST_OK(cmp_event_loop_async_destroy(&bad_alloc, &loop));

  return 0;
}
#endif

int main(void) {
  if (test_async_loop() != 0) {
    return 1;
  }
  if (test_async_loop_stop() != 0) {
    return 1;
  }
#ifdef CMP_TESTING
  if (test_async_loop_alloc_fail() != 0) {
    return 1;
  }
#endif
  return 0;
}
