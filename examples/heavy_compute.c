/* heavy_compute.c */
/* clang-format off */
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_tasks.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

#define CHUNKS 8
#define ITERATIONS 10000000

typedef struct {
  CMPTasks tasks;
  CMPHandle mutex;
  int completed_chunks;
  double result_sum;
  CMPEventLoop loop;
} AppState;

typedef struct {
  AppState *app_state;
  int chunk_id;
  double chunk_result;
} ChunkTask;

static int CMP_CALL compute_task(void *user) {
  ChunkTask *ctx = (ChunkTask *)user;
  double local_sum = 0.0;
  int i;

  printf("Thread worker starting chunk %d...\n", ctx->chunk_id);

  /* Heavy computation simulation */
  for (i = 0; i < ITERATIONS; i++) {
    local_sum += 1.0 / (double)(i + 1 + (ctx->chunk_id * ITERATIONS));
  }

  ctx->chunk_result = local_sum;

  /* Lock mutex to update global state safely */
  ctx->app_state->tasks.vtable->mutex_lock(ctx->app_state->tasks.ctx,
                                           ctx->app_state->mutex);
  ctx->app_state->result_sum += ctx->chunk_result;
  ctx->app_state->completed_chunks++;
  printf("Chunk %d complete (Total %d/%d)\n", ctx->chunk_id,
         ctx->app_state->completed_chunks, CHUNKS);

  if (ctx->app_state->completed_chunks == CHUNKS) {
    /* Wake main loop so it can exit */
    /* Or we can just let a timer check it in the main loop */
  }
  ctx->app_state->tasks.vtable->mutex_unlock(ctx->app_state->tasks.ctx,
                                             ctx->app_state->mutex);

  return CMP_OK;
}

static int poll_ui(void *ctx, CMPBool *out_continue) {
  AppState *state = (AppState *)ctx;
  int done = 0;

  state->tasks.vtable->mutex_lock(state->tasks.ctx, state->mutex);
  if (state->completed_chunks == CHUNKS) {
    done = 1;
  }
  state->tasks.vtable->mutex_unlock(state->tasks.ctx, state->mutex);

  if (done) {
    printf("All compute chunks finished. Final sum: %f\n", state->result_sum);
    *out_continue = 0; /* stop loop */
  } else {
    /* sleep UI thread a little bit so it doesn't spin at 100% */
    state->tasks.vtable->sleep_ms(state->tasks.ctx, 16);
    *out_continue = 1;
  }

  return CMP_OK;
}

int main(void) {
  AppState state;
  CMPAllocator alloc;
  CMPAsyncEventLoopConfig loop_config;
  CMPTasksDefaultConfig tasks_config;
  ChunkTask chunks[CHUNKS];
  int i;

  printf("Initializing Heavy Compute example (Multithreading)...\n");

  cmp_get_default_allocator(&alloc);

  cmp_tasks_default_config_init(&tasks_config);
  tasks_config.worker_count = 4; /* Use 4 worker threads for CPU compute */
  cmp_tasks_default_create(&tasks_config, &state.tasks);

  state.tasks.vtable->mutex_create(state.tasks.ctx, &state.mutex);
  state.completed_chunks = 0;
  state.result_sum = 0.0;

  loop_config.ctx = &state;
  loop_config.on_poll_ui = poll_ui;
  cmp_event_loop_async_init(&alloc, &loop_config, &state.loop);

  printf("Dispatching %d compute chunks to thread pool...\n", CHUNKS);
  for (i = 0; i < CHUNKS; i++) {
    chunks[i].app_state = &state;
    chunks[i].chunk_id = i;
    chunks[i].chunk_result = 0.0;
    state.tasks.vtable->task_post(state.tasks.ctx, compute_task, &chunks[i]);
  }

  printf("Main thread remaining responsive, running event loop...\n");
  cmp_event_loop_run(&state.loop);

  state.tasks.vtable->mutex_destroy(state.tasks.ctx, state.mutex);
  cmp_event_loop_async_destroy(&alloc, &state.loop);
  cmp_tasks_default_destroy(&state.tasks);

  return 0;
}