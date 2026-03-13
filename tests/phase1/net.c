/* clang-format off */
#include "cmpc/cmp_net.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_tasks.h"
#include "test_utils.h"

#include <stdlib.h>
/* clang-format on */

typedef struct NetTestState {
  int called;
  int status;
  int status_code;
  cmp_usize size;
} NetTestState;

static void CMP_CALL net_callback(void *task_ctx, const char *url, void *data,
                                  cmp_usize size, int status, int status_code,
                                  void *user) {
  NetTestState *state = (NetTestState *)user;
  CMP_UNUSED(task_ctx);
  CMP_UNUSED(url);
  CMP_UNUSED(data);
  state->called = 1;
  state->status = status;
  state->status_code = status_code;
  state->size = size;
  if (data != NULL) {
    CMPAllocator alloc;
    cmp_get_default_allocator(&alloc);
    alloc.free(alloc.ctx, data);
  }
}

static int test_net_poll_ui(void *ctx, CMPBool *out_continue) {
  NetTestState *state = (NetTestState *)ctx;
  if (state->called) {
    *out_continue = 0;
  }
  return CMP_OK;
}

int test_net(void) {
  CMPAllocator alloc;
  CMPEventLoop loop;
  CMPAsyncEventLoopConfig loop_config;
  CMPTasks tasks;
  CMPTasksDefaultConfig tasks_config;
  NetTestState state;
  CMPEnv env;

  CMP_TEST_OK(cmp_get_default_allocator(&alloc));
  CMP_TEST_OK(cmp_tasks_default_config_init(&tasks_config));
  tasks_config.allocator = &alloc;
  tasks_config.worker_count = 2;
  CMP_TEST_OK(cmp_tasks_default_create(&tasks_config, &tasks));

  state.called = 0;
  state.status = -1;
  state.status_code = -1;
  state.size = 0;

  loop_config.ctx = &state;
  loop_config.on_poll_ui = test_net_poll_ui;
  CMP_TEST_OK(cmp_event_loop_async_init(&alloc, &loop_config, &loop));

  /* Fetch a dummy URL.
   * It will likely fail to resolve, but we'll see a callback with an error.
   */
  CMP_TEST_OK(cmp_net_fetch_async(&env, &tasks, &loop, &alloc,
                                  "http://localhost:99999/dummy", net_callback,
                                  &state));

  CMP_TEST_OK(cmp_event_loop_run(&loop));

  CMP_TEST_ASSERT(state.called == 1);
  /* The HTTP client will return an error because the port is invalid or
   * unreachable. */
  CMP_TEST_ASSERT(state.status != CMP_OK);

  CMP_TEST_OK(cmp_event_loop_async_destroy(&alloc, &loop));
  CMP_TEST_OK(cmp_tasks_default_destroy(&tasks));

  return 0;
}

int main(void) {
  if (test_net() != 0)
    return 1;
  return 0;
}