/* clang-format off */
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_tasks.h"

#include <stdlib.h>
/* clang-format on */

#ifdef CMP_TESTING
static CMPBool g_cmp_default_allocator_fail = CMP_FALSE;
#endif

static cmp_usize cmp_usize_max_value(void) { return (cmp_usize) ~(cmp_usize)0; }

static void *cmp_default_malloc(cmp_usize size) {
  if (size == cmp_usize_max_value()) {
    return NULL;
  }
  return malloc((size_t)size);
}

static void *cmp_default_realloc_impl(void *ptr, cmp_usize size) {
  if (size == cmp_usize_max_value()) {
    return NULL;
  }
  return realloc(ptr, (size_t)size);
}

static int CMP_CALL cmp_default_alloc(void *ctx, cmp_usize size,
                                      void **out_ptr) {
  void *mem;

  CMP_UNUSED(ctx);

  if (out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  mem = cmp_default_malloc(size);
  if (mem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = mem;
  return CMP_OK;
}

static int CMP_CALL cmp_default_realloc(void *ctx, void *ptr, cmp_usize size,
                                        void **out_ptr) {
  void *mem;

  CMP_UNUSED(ctx);

  if (out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  mem = cmp_default_realloc_impl(ptr, size);
  if (mem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = mem;
  return CMP_OK;
}

static int CMP_CALL cmp_default_free(void *ctx, void *ptr) {
  CMP_UNUSED(ctx);

  if (ptr == NULL) {
    return CMP_OK;
  }

  free(ptr);
  return CMP_OK;
}

int CMP_CALL cmp_event_loop_run(CMPEventLoop *loop) {
  if (loop == NULL || loop->vtable == NULL || loop->vtable->run == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return loop->vtable->run(loop->ctx);
}

int CMP_CALL cmp_event_loop_stop(CMPEventLoop *loop) {
  if (loop == NULL || loop->vtable == NULL || loop->vtable->stop == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return loop->vtable->stop(loop->ctx);
}

int CMP_CALL cmp_event_loop_add_io(CMPEventLoop *loop, int fd, cmp_u32 flags,
                                   CMPEventLoopIOCallback cb, void *cb_ctx,
                                   CMPEventLoopIOHandle *out_handle) {
  if (loop == NULL || loop->vtable == NULL || loop->vtable->add_io == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return loop->vtable->add_io(loop->ctx, fd, flags, cb, cb_ctx, out_handle);
}

int CMP_CALL cmp_event_loop_remove_io(CMPEventLoop *loop,
                                      CMPEventLoopIOHandle handle) {
  if (loop == NULL || loop->vtable == NULL || loop->vtable->remove_io == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return loop->vtable->remove_io(loop->ctx, handle);
}

int CMP_CALL cmp_event_loop_set_timer(CMPEventLoop *loop, cmp_u32 timeout_ms,
                                      CMPBool repeat,
                                      CMPEventLoopTimerCallback cb,
                                      void *cb_ctx,
                                      CMPEventLoopTimerHandle *out_handle) {
  if (loop == NULL || loop->vtable == NULL || loop->vtable->set_timer == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return loop->vtable->set_timer(loop->ctx, timeout_ms, repeat, cb, cb_ctx,
                                 out_handle);
}

int CMP_CALL cmp_event_loop_clear_timer(CMPEventLoop *loop,
                                        CMPEventLoopTimerHandle handle) {
  if (loop == NULL || loop->vtable == NULL ||
      loop->vtable->clear_timer == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return loop->vtable->clear_timer(loop->ctx, handle);
}

int CMP_CALL cmp_event_loop_wake(CMPEventLoop *loop,
                                 CMPEventLoopWakeCallback cb, void *cb_ctx) {
  if (loop == NULL || loop->vtable == NULL || loop->vtable->wake == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return loop->vtable->wake(loop->ctx, cb, cb_ctx);
}

static CMPEventLoop g_cmp_app_loop = {NULL, NULL};
static CMPTasks g_cmp_app_tasks = {NULL, NULL};
static CMPAppConfig g_cmp_app_config = {CMP_MODALITY_SYNC_SINGLE};
static CMPBool g_cmp_app_initialized = 0;

int CMP_CALL cmp_set_timeout(cmp_u32 timeout_ms, CMPEventLoopTimerCallback cb,
                             void *user, CMPEventLoopTimerHandle *out_handle) {
  return cmp_event_loop_set_timer(&g_cmp_app_loop, timeout_ms, CMP_FALSE, cb,
                                  user, out_handle);
}

int CMP_CALL cmp_set_interval(cmp_u32 interval_ms, CMPEventLoopTimerCallback cb,
                              void *user, CMPEventLoopTimerHandle *out_handle) {
  return cmp_event_loop_set_timer(&g_cmp_app_loop, interval_ms, CMP_TRUE, cb,
                                  user, out_handle);
}

int CMP_CALL cmp_clear_timeout(CMPEventLoopTimerHandle handle) {
  return cmp_event_loop_clear_timer(&g_cmp_app_loop, handle);
}

int CMP_CALL cmp_app_config_init(CMPAppConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  config->modality = CMP_MODALITY_SYNC_SINGLE;
  return CMP_OK;
}

static int CMP_CALL default_sync_tick(void *ctx, CMPBool *out_continue) {
  CMP_UNUSED(ctx);
  *out_continue = 0; /* Fallback tick just exits immediately. */
  return CMP_OK;
}

int CMP_CALL cmp_app_init(const CMPAppConfig *config) {
  CMPAllocator alloc;
  int rc;

  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (g_cmp_app_initialized) {
    return CMP_ERR_NOT_READY; /* Already initialized */
  }

  rc = cmp_get_default_allocator(&alloc);
  if (rc != CMP_OK) {
    return rc;
  }

  g_cmp_app_config = *config;

  if (config->modality == CMP_MODALITY_SYNC_SINGLE ||
      config->modality == CMP_MODALITY_SYNC_MULTI) {
    CMPSyncEventLoopConfig loop_config;
    loop_config.ctx = NULL;
    loop_config.on_tick = default_sync_tick;
    rc = cmp_event_loop_sync_init(&alloc, &loop_config, &g_cmp_app_loop);
    if (rc != CMP_OK) {
      return rc;
    }
  } else if (config->modality == CMP_MODALITY_ASYNC_SINGLE ||
             config->modality == CMP_MODALITY_ASYNC_MULTI) {
    CMPAsyncEventLoopConfig loop_config;
    loop_config.ctx = NULL;
    loop_config.on_poll_ui = default_sync_tick; /* stub for UI polling */
    rc = cmp_event_loop_async_init(&alloc, &loop_config, &g_cmp_app_loop);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  if (config->modality == CMP_MODALITY_SYNC_MULTI ||
      config->modality == CMP_MODALITY_ASYNC_MULTI) {
    CMPTasksDefaultConfig tasks_config;
    rc = cmp_tasks_default_config_init(&tasks_config);
    if (rc != CMP_OK) {
      if (config->modality == CMP_MODALITY_SYNC_MULTI) {
        cmp_event_loop_sync_destroy(&alloc, &g_cmp_app_loop);
      } else {
        cmp_event_loop_async_destroy(&alloc, &g_cmp_app_loop);
      }
      return rc;
    }
    rc = cmp_tasks_default_create(&tasks_config, &g_cmp_app_tasks);
    if (rc != CMP_OK) {
      if (config->modality == CMP_MODALITY_SYNC_MULTI) {
        cmp_event_loop_sync_destroy(&alloc, &g_cmp_app_loop);
      } else {
        cmp_event_loop_async_destroy(&alloc, &g_cmp_app_loop);
      }
      return rc;
    }
  } else {
    g_cmp_app_tasks.ctx = NULL;
    g_cmp_app_tasks.vtable = NULL;
  }

  g_cmp_app_initialized = 1;
  return CMP_OK;
}

int CMP_CALL cmp_app_destroy(void) {
  CMPAllocator alloc;
  int rc;

  if (!g_cmp_app_initialized) {
    return CMP_OK;
  }

  rc = cmp_get_default_allocator(&alloc);
  if (rc != CMP_OK) {
    return rc;
  }

  if (g_cmp_app_tasks.vtable != NULL) {
    cmp_tasks_default_destroy(&g_cmp_app_tasks);
    g_cmp_app_tasks.ctx = NULL;
    g_cmp_app_tasks.vtable = NULL;
  }

  if (g_cmp_app_loop.vtable != NULL) {
    rc = cmp_event_loop_async_destroy(&alloc, &g_cmp_app_loop);
    if (rc != CMP_OK) {
      cmp_event_loop_sync_destroy(&alloc, &g_cmp_app_loop);
    }
    g_cmp_app_loop.ctx = NULL;
    g_cmp_app_loop.vtable = NULL;
  }

  g_cmp_app_initialized = 0;
  return CMP_OK;
}

int CMP_CALL cmp_get_default_allocator(CMPAllocator *out_alloc) {
  if (out_alloc == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_cmp_default_allocator_fail) {
    return CMP_ERR_UNKNOWN;
  }
#endif

  out_alloc->ctx = NULL;
  out_alloc->alloc = cmp_default_alloc;
  out_alloc->realloc = cmp_default_realloc;
  out_alloc->free = cmp_default_free;
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL cmp_core_test_set_default_allocator_fail(CMPBool fail) {
  g_cmp_default_allocator_fail = fail ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}
#endif
