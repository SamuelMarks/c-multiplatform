/* clang-format off */
#if defined(_MSC_VER)
#pragma warning(push, 0)
#endif
#include "cmpc/cmp_db.h"
#include <c_orm_api.h>
#include <c_orm_sqlite.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
/* clang-format on */
#include <string.h>

typedef struct CMPDbExecuteCtx {
  CMPDb *db;
  CMPTasks *tasks;
  CMPEventLoop *loop;
  const CMPAllocator *allocator;
  char *sql;
  CMPDbExecuteCallback callback;
  void *user;
  int status;
} CMPDbExecuteCtx;

CMP_API int CMP_CALL cmp_db_init(CMPDb *db, const CMPAllocator *allocator,
                                 const char *url) {
  c_orm_db_t *orm_db = NULL;
  c_orm_error_t err;

  if (db == NULL || allocator == NULL || url == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  db->allocator = *allocator;
  db->handle = NULL;

  err = c_orm_sqlite_connect(url, &orm_db);
  if (err != C_ORM_OK) {
    return CMP_ERR_IO;
  }

  db->handle = orm_db;
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_db_shutdown(CMPDb *db) {
  c_orm_db_t *orm_db;
  const c_orm_driver_vtable_t *vtable;

  if (db == NULL || db->handle == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  orm_db = (c_orm_db_t *)db->handle;
  vtable = orm_db->vtable;

  if (vtable != NULL && vtable->disconnect != NULL) {
    vtable->disconnect(orm_db);
  }

  db->handle = NULL;
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_db_execute(CMPDb *db, const char *sql) {
  c_orm_db_t *orm_db;
  c_orm_error_t err;

  if (db == NULL || db->handle == NULL || sql == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  orm_db = (c_orm_db_t *)db->handle;
  err = c_orm_execute_raw(orm_db, sql);
  if (err != C_ORM_OK) {
    return CMP_ERR_UNKNOWN;
  }

  return CMP_OK;
}

static int CMP_CALL cmp_db_execute_wake_cb(void *cb_ctx) {
  CMPDbExecuteCtx *ctx = (CMPDbExecuteCtx *)cb_ctx;
  if (ctx != NULL) {
    if (ctx->callback != NULL) {
      ctx->callback(ctx, ctx->status, ctx->user);
    }
    if (ctx->sql != NULL) {
      ctx->allocator->free(ctx->allocator->ctx, ctx->sql);
    }
    ctx->allocator->free(ctx->allocator->ctx, ctx);
  }
  return CMP_OK;
}

static int CMP_CALL cmp_db_execute_task_cb(void *user) {
  CMPDbExecuteCtx *ctx = (CMPDbExecuteCtx *)user;
  int rc;

  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  ctx->status = cmp_db_execute(ctx->db, ctx->sql);

  /* Wake main thread to run the callback. */
  if (ctx->loop != NULL && ctx->loop->vtable != NULL &&
      ctx->loop->vtable->wake != NULL) {
    rc = ctx->loop->vtable->wake(ctx->loop->ctx, cmp_db_execute_wake_cb, ctx);
    if (rc != CMP_OK) {
      /* Waking failed, cleanup must happen here. */
      if (ctx->sql != NULL) {
        ctx->allocator->free(ctx->allocator->ctx, ctx->sql);
      }
      ctx->allocator->free(ctx->allocator->ctx, ctx);
    }
  } else {
    /* No loop to wake, cleanup. */
    if (ctx->sql != NULL) {
      ctx->allocator->free(ctx->allocator->ctx, ctx->sql);
    }
    ctx->allocator->free(ctx->allocator->ctx, ctx);
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cmp_db_execute_async(CMPDb *db, CMPTasks *tasks,
                                          CMPEventLoop *loop, const char *sql,
                                          CMPDbExecuteCallback callback,
                                          void *user) {
  CMPDbExecuteCtx *ctx;
  int alloc_rc;
  cmp_usize sql_len;
  int rc;

  if (db == NULL || tasks == NULL || tasks->vtable == NULL ||
      tasks->vtable->task_post == NULL || sql == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc_rc = db->allocator.alloc(db->allocator.ctx, sizeof(CMPDbExecuteCtx),
                                 (void **)&ctx);
  if (alloc_rc != CMP_OK || ctx == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  ctx->db = db;
  ctx->tasks = tasks;
  ctx->loop = loop;
  ctx->allocator = &db->allocator;
  ctx->callback = callback;
  ctx->user = user;
  ctx->status = CMP_ERR_UNKNOWN;

  sql_len = strlen(sql);
  alloc_rc = ctx->allocator->alloc(ctx->allocator->ctx, sql_len + 1,
                                   (void **)&ctx->sql);
  if (alloc_rc != CMP_OK || ctx->sql == NULL) {
    ctx->allocator->free(ctx->allocator->ctx, ctx);
    return CMP_ERR_OUT_OF_MEMORY;
  }

#if defined(_MSC_VER)
  strcpy_s(ctx->sql, sql_len + 1, sql);
#else
  strcpy(ctx->sql, sql);
#endif

  rc = tasks->vtable->task_post(tasks->ctx, cmp_db_execute_task_cb, ctx);
  if (rc != CMP_OK) {
    ctx->allocator->free(ctx->allocator->ctx, ctx->sql);
    ctx->allocator->free(ctx->allocator->ctx, ctx);
    return rc;
  }

  return CMP_OK;
}
