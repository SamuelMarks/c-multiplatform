/* clang-format off */
#include "cmpc/cmp_core.h"
#include "test_utils.h"

#include <stdlib.h>
/* clang-format on */

static int test_sync_loop_tick(void *ctx, CMPBool *out_continue) {
  int *ticks = (int *)ctx;
  (*ticks)++;
  if (*ticks >= 5) {
    *out_continue = 0;
  }
  return CMP_OK;
}

static int test_sync_loop_tick_error(void *ctx, CMPBool *out_continue) {
  (void)ctx;
  (void)out_continue;
  return CMP_ERR_UNKNOWN;
}

static int test_sync_loop(void) {
  CMPAllocator alloc;
  CMPEventLoop loop;
  CMPSyncEventLoopConfig config;
  int ticks;

  CMP_TEST_OK(cmp_get_default_allocator(&alloc));

  ticks = 0;
  config.ctx = &ticks;
  config.on_tick = test_sync_loop_tick;

  CMP_TEST_EXPECT(cmp_event_loop_sync_init(NULL, &config, &loop),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_event_loop_sync_init(&alloc, NULL, &loop),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_event_loop_sync_init(&alloc, &config, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  config.on_tick = NULL;
  CMP_TEST_EXPECT(cmp_event_loop_sync_init(&alloc, &config, &loop),
                  CMP_ERR_INVALID_ARGUMENT);

  config.on_tick = test_sync_loop_tick;
  CMP_TEST_OK(cmp_event_loop_sync_init(&alloc, &config, &loop));

  CMP_TEST_OK(cmp_event_loop_run(&loop));
  CMP_TEST_ASSERT(ticks == 5);

  CMP_TEST_OK(cmp_event_loop_sync_destroy(&alloc, &loop));

  return 0;
}

static int test_sync_loop_error(void) {
  CMPAllocator alloc;
  CMPEventLoop loop;
  CMPSyncEventLoopConfig config;

  CMP_TEST_OK(cmp_get_default_allocator(&alloc));

  config.ctx = NULL;
  config.on_tick = test_sync_loop_tick_error;

  CMP_TEST_OK(cmp_event_loop_sync_init(&alloc, &config, &loop));

  CMP_TEST_EXPECT(cmp_event_loop_run(&loop), CMP_ERR_UNKNOWN);

  CMP_TEST_OK(cmp_event_loop_sync_destroy(&alloc, &loop));

  return 0;
}

static int test_sync_loop_stop(void) {
  CMPAllocator alloc;
  CMPEventLoop loop;
  CMPSyncEventLoopConfig config;
  int ticks;

  CMP_TEST_OK(cmp_get_default_allocator(&alloc));

  ticks = 0;
  config.ctx = &ticks;
  config.on_tick = test_sync_loop_tick;

  CMP_TEST_OK(cmp_event_loop_sync_init(&alloc, &config, &loop));

  /* Test stop while not running (should just set state to not running) */
  CMP_TEST_OK(cmp_event_loop_stop(&loop));

  CMP_TEST_OK(cmp_event_loop_sync_destroy(&alloc, &loop));
  CMP_TEST_EXPECT(cmp_event_loop_sync_destroy(NULL, &loop),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_event_loop_sync_destroy(&alloc, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  return 0;
}

static int test_sync_loop_unsupported(void) {
  CMPAllocator alloc;
  CMPEventLoop loop;
  CMPSyncEventLoopConfig config;

  CMP_TEST_OK(cmp_get_default_allocator(&alloc));

  config.ctx = NULL;
  config.on_tick = test_sync_loop_tick;

  CMP_TEST_OK(cmp_event_loop_sync_init(&alloc, &config, &loop));

  CMP_TEST_EXPECT(cmp_event_loop_add_io(&loop, 0, 0, NULL, NULL, NULL),
                  CMP_ERR_UNSUPPORTED);
  CMP_TEST_EXPECT(cmp_event_loop_remove_io(&loop, NULL), CMP_ERR_UNSUPPORTED);
  CMP_TEST_EXPECT(cmp_event_loop_set_timer(&loop, 0, 0, NULL, NULL, NULL),
                  CMP_ERR_UNSUPPORTED);
  CMP_TEST_EXPECT(cmp_event_loop_clear_timer(&loop, NULL), CMP_ERR_UNSUPPORTED);
  CMP_TEST_EXPECT(cmp_event_loop_wake(&loop, NULL, NULL), CMP_ERR_UNSUPPORTED);

  CMP_TEST_OK(cmp_event_loop_sync_destroy(&alloc, &loop));

  return 0;
}

#ifdef CMP_TESTING
static int test_alloc_fail(void *ctx, cmp_usize size, void **out_ptr) {
  (void)ctx;
  (void)size;
  (void)out_ptr;
  return CMP_ERR_OUT_OF_MEMORY;
}

static int test_sync_loop_alloc_fail(void) {
  CMPAllocator alloc;
  CMPEventLoop loop;
  CMPSyncEventLoopConfig config;

  CMP_TEST_OK(cmp_get_default_allocator(&alloc));
  alloc.alloc = test_alloc_fail;

  config.ctx = NULL;
  config.on_tick = test_sync_loop_tick;

  CMP_TEST_EXPECT(cmp_event_loop_sync_init(&alloc, &config, &loop),
                  CMP_ERR_OUT_OF_MEMORY);

  return 0;
}
#endif

int main(void) {
  if (test_sync_loop() != 0) {
    return 1;
  }
  if (test_sync_loop_error() != 0) {
    return 1;
  }
  if (test_sync_loop_stop() != 0) {
    return 1;
  }
  if (test_sync_loop_unsupported() != 0) {
    return 1;
  }
#ifdef CMP_TESTING
  if (test_sync_loop_alloc_fail() != 0) {
    return 1;
  }
#endif
  return 0;
}
