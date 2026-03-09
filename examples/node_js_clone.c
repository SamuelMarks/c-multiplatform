/* node_js_clone.c */
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_net.h"
#include "cmpc/cmp_tasks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  CMPAllocator alloc;
  CMPEventLoop loop;
  CMPTasks tasks;
  CMPEnv env;
} AppState;

static void CMP_CALL fetch_callback(void *task_ctx, const char *url, void *data,
                                    cmp_usize size, int status, int status_code,
                                    void *user) {
  AppState *state = (AppState *)user;
  CMP_UNUSED(task_ctx);

  printf("Fetch complete: %s\n", url);
  printf("Status: %d, HTTP Code: %d, Size: %zu bytes\n", status, status_code,
         size);

  if (data != NULL) {
    /* safely print first few bytes if text */
    if (size > 0 && ((char *)data)[0] == '<') {
      char buf[64];
      cmp_usize copy_size = size < 63 ? size : 63;
      memcpy(buf, data, copy_size);
      buf[copy_size] = '\0';
      printf("Data preview: %s...\n", buf);
    }
    state->alloc.free(state->alloc.ctx, data);
  }

  /* After fetch, stop the event loop */
  cmp_event_loop_stop(&state->loop);
}

static int poll_ui(void *ctx, CMPBool *out_continue) {
  /* No UI in this simple CLI example, just return ok */
  CMP_UNUSED(ctx);
  *out_continue = 1;
  return CMP_OK;
}

int main(void) {
  AppState state;
  CMPAsyncEventLoopConfig loop_config;
  CMPTasksDefaultConfig tasks_config;

  printf("Initializing Node.js Clone example (Async Single Threaded Event "
         "Loop)...\n");

  if (cmp_get_default_allocator(&state.alloc) != CMP_OK) {
    return 1;
  }

  if (cmp_tasks_default_config_init(&tasks_config) != CMP_OK) {
    return 1;
  }
  tasks_config.worker_count = 2; /* background threads for HTTP IO */
  if (cmp_tasks_default_create(&tasks_config, &state.tasks) != CMP_OK) {
    return 1;
  }

  loop_config.ctx = &state;
  loop_config.on_poll_ui = poll_ui;
  if (cmp_event_loop_async_init(&state.alloc, &loop_config, &state.loop) !=
      CMP_OK) {
    return 1;
  }

  state.env.ctx = NULL;
  state.env.vtable = NULL; /* Not strictly used by net fetch currently */

  printf("Fetching http://example.com asynchronously...\n");
  if (cmp_net_fetch_async(&state.env, &state.tasks, &state.loop, &state.alloc,
                          "http://example.com", fetch_callback,
                          &state) != CMP_OK) {
    printf("Failed to start fetch\n");
    return 1;
  }

  /* Run the event loop. This blocks until cmp_event_loop_stop is called in the
   * callback. */
  printf("Starting event loop...\n");
  cmp_event_loop_run(&state.loop);
  printf("Event loop exited.\n");

  cmp_event_loop_async_destroy(&state.alloc, &state.loop);
  cmp_tasks_default_destroy(&state.tasks);

  return 0;
}