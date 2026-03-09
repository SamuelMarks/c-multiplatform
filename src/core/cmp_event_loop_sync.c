/* clang-format off */
#include "cmpc/cmp_core.h"

#include <stdlib.h>
/* clang-format on */

typedef struct CMPSyncLoopState {
  CMPSyncEventLoopConfig config;
  CMPBool running;
} CMPSyncLoopState;

static int CMP_CALL sync_loop_run(void *ctx) {
  CMPSyncLoopState *state = (CMPSyncLoopState *)ctx;
  int rc = CMP_OK;
  CMPBool cont = 1;

  if (state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state->running = 1;
  while (state->running) {
    cont = 1;
    rc = state->config.on_tick(state->config.ctx, &cont);
    if (rc != CMP_OK || !cont) {
      state->running = 0;
      break;
    }
  }

  return rc;
}

static int CMP_CALL sync_loop_stop(void *ctx) {
  CMPSyncLoopState *state = (CMPSyncLoopState *)ctx;

  if (state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state->running = 0;
  return CMP_OK;
}

static int CMP_CALL sync_loop_add_io(void *ctx, int fd, cmp_u32 flags,
                                     CMPEventLoopIOCallback cb, void *cb_ctx,
                                     CMPEventLoopIOHandle *out_handle) {
  CMP_UNUSED(ctx);
  CMP_UNUSED(fd);
  CMP_UNUSED(flags);
  CMP_UNUSED(cb);
  CMP_UNUSED(cb_ctx);
  CMP_UNUSED(out_handle);
  return CMP_ERR_UNSUPPORTED;
}

static int CMP_CALL sync_loop_remove_io(void *ctx,
                                        CMPEventLoopIOHandle handle) {
  CMP_UNUSED(ctx);
  CMP_UNUSED(handle);
  return CMP_ERR_UNSUPPORTED;
}

static int CMP_CALL sync_loop_set_timer(void *ctx, cmp_u32 timeout_ms,
                                        CMPBool repeat,
                                        CMPEventLoopTimerCallback cb,
                                        void *cb_ctx,
                                        CMPEventLoopTimerHandle *out_handle) {
  CMP_UNUSED(ctx);
  CMP_UNUSED(timeout_ms);
  CMP_UNUSED(repeat);
  CMP_UNUSED(cb);
  CMP_UNUSED(cb_ctx);
  CMP_UNUSED(out_handle);
  return CMP_ERR_UNSUPPORTED;
}

static int CMP_CALL sync_loop_clear_timer(void *ctx,
                                          CMPEventLoopTimerHandle handle) {
  CMP_UNUSED(ctx);
  CMP_UNUSED(handle);
  return CMP_ERR_UNSUPPORTED;
}

static int CMP_CALL sync_loop_wake(void *ctx, CMPEventLoopWakeCallback cb,
                                   void *cb_ctx) {
  CMP_UNUSED(ctx);
  CMP_UNUSED(cb);
  CMP_UNUSED(cb_ctx);
  return CMP_ERR_UNSUPPORTED;
}

static const CMPEventLoopVTable g_sync_loop_vtable = {
    sync_loop_run,       sync_loop_stop,      sync_loop_add_io,
    sync_loop_remove_io, sync_loop_set_timer, sync_loop_clear_timer,
    sync_loop_wake};

int CMP_CALL cmp_event_loop_sync_init(CMPAllocator *alloc,
                                      const CMPSyncEventLoopConfig *config,
                                      CMPEventLoop *out_loop) {
  CMPSyncLoopState *state;
  int rc;

  if (alloc == NULL || config == NULL || out_loop == NULL ||
      config->on_tick == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = alloc->alloc(alloc->ctx, sizeof(CMPSyncLoopState), (void **)&state);
  if (rc != CMP_OK) {
    return rc;
  }

  state->config = *config;
  state->running = 0;

  out_loop->ctx = state;
  out_loop->vtable = &g_sync_loop_vtable;

  return CMP_OK;
}

int CMP_CALL cmp_event_loop_sync_destroy(CMPAllocator *alloc,
                                         CMPEventLoop *loop) {
  if (alloc == NULL || loop == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (loop->ctx != NULL) {
    alloc->free(alloc->ctx, loop->ctx);
    loop->ctx = NULL;
  }
  loop->vtable = NULL;

  return CMP_OK;
}
