/* clang-format off */
#if !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 700
#endif
#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif
#include "cmpc/cmp_fs.h"
/* clang-format on */
#include <string.h>

typedef struct CMPFSReadCtx {
  CMPEnv *env;
  CMPTasks *tasks;
  CMPEventLoop *loop;
  const CMPAllocator *allocator;
  char *utf8_path;
  CMPFSReadCallback callback;
  void *user;

  void *data;
  cmp_usize size;
  int status;
} CMPFSReadCtx;

static int CMP_CALL fs_read_wake_cb(void *cb_ctx) {
  CMPFSReadCtx *ctx = (CMPFSReadCtx *)cb_ctx;
  if (ctx != NULL) {
    if (ctx->callback != NULL) {
      ctx->callback(ctx, ctx->utf8_path, ctx->data, ctx->size, ctx->status,
                    ctx->user);
    }
    if (ctx->utf8_path != NULL) {
      ctx->allocator->free(ctx->allocator->ctx, ctx->utf8_path);
    }
    ctx->allocator->free(ctx->allocator->ctx, ctx);
  }
  return CMP_OK;
}

static int CMP_CALL fs_read_task_cb(void *user) {
  CMPFSReadCtx *ctx = (CMPFSReadCtx *)user;
  CMPIO io;
  int rc;

  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = ctx->env->vtable->get_io(ctx->env->ctx, &io);
  if (rc == CMP_OK && io.vtable != NULL && io.vtable->read_file_alloc != NULL) {
    ctx->status = io.vtable->read_file_alloc(
        io.ctx, ctx->utf8_path, ctx->allocator, &ctx->data, &ctx->size);
  } else {
    ctx->status = CMP_ERR_UNSUPPORTED;
  }

  /* Wake main thread to run the callback. */
  if (ctx->loop != NULL && ctx->loop->vtable != NULL &&
      ctx->loop->vtable->wake != NULL) {
    rc = ctx->loop->vtable->wake(ctx->loop->ctx, fs_read_wake_cb, ctx);
    if (rc != CMP_OK) {
      /* Waking failed, cleanup must happen here. */
      if (ctx->data != NULL) {
        ctx->allocator->free(ctx->allocator->ctx, ctx->data);
      }
      if (ctx->utf8_path != NULL) {
        ctx->allocator->free(ctx->allocator->ctx, ctx->utf8_path);
      }
      ctx->allocator->free(ctx->allocator->ctx, ctx);
    }
  } else {
    /* No loop to wake, cleanup. */
    if (ctx->data != NULL) {
      ctx->allocator->free(ctx->allocator->ctx, ctx->data);
    }
    if (ctx->utf8_path != NULL) {
      ctx->allocator->free(ctx->allocator->ctx, ctx->utf8_path);
    }
    ctx->allocator->free(ctx->allocator->ctx, ctx);
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cmp_fs_read_async(CMPEnv *env, CMPTasks *tasks,
                                       CMPEventLoop *loop,
                                       const CMPAllocator *allocator,
                                       const char *utf8_path,
                                       CMPFSReadCallback callback, void *user) {
  CMPFSReadCtx *ctx = NULL;
  cmp_usize path_len;
  int rc;

  if (env == NULL || env->vtable == NULL || env->vtable->get_io == NULL ||
      tasks == NULL || tasks->vtable == NULL ||
      tasks->vtable->task_post == NULL || loop == NULL ||
      loop->vtable == NULL || allocator == NULL || utf8_path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = allocator->alloc(allocator->ctx, sizeof(CMPFSReadCtx), (void **)&ctx);
  if (rc != CMP_OK || ctx == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  path_len = strlen(utf8_path);
  rc = allocator->alloc(allocator->ctx, path_len + 1, (void **)&ctx->utf8_path);
  if (rc != CMP_OK || ctx->utf8_path == NULL) {
    allocator->free(allocator->ctx, ctx);
    return CMP_ERR_OUT_OF_MEMORY;
  }
  memcpy(ctx->utf8_path, utf8_path, path_len);
  ctx->utf8_path[path_len] = '\0';

  ctx->env = env;
  ctx->tasks = tasks;
  ctx->loop = loop;
  ctx->allocator = allocator;
  ctx->callback = callback;
  ctx->user = user;
  ctx->data = NULL;
  ctx->size = 0;
  ctx->status = CMP_ERR_UNKNOWN;

  rc = tasks->vtable->task_post(tasks->ctx, fs_read_task_cb, ctx);
  if (rc != CMP_OK) {
    allocator->free(allocator->ctx, ctx->utf8_path);
    allocator->free(allocator->ctx, ctx);
    return rc;
  }

  return CMP_OK;
}