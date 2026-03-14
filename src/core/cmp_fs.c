/* clang-format off */
#if !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 700
#endif
#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif
#if defined(_MSC_VER)
#pragma warning(push, 0)
#endif
#include "cmpc/cmp_fs.h"
#include <cfs/cfs.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
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

static int cmp_cfs_path_init_utf8(cfs_path *p, const char *utf8_path) {
#if defined(CFS_OS_WINDOWS) || defined(_WIN32)
  cfs_size_t wlen = 0;
  wchar_t *wbuf = NULL;
  int rc;

  rc = cfs_utf8_to_utf16(utf8_path, NULL, 0, &wlen);
  if (rc != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  wbuf = (wchar_t *)malloc((wlen + 1) * sizeof(wchar_t));
  if (wbuf == NULL)
    return CMP_ERR_OUT_OF_MEMORY;

  rc = cfs_utf8_to_utf16(utf8_path, wbuf, wlen + 1, &wlen);
  if (rc != 0) {
    free(wbuf);
    return CMP_ERR_INVALID_ARGUMENT;
  }
  wbuf[wlen] = 0;
  cfs_path_init_str(p, wbuf);
  free(wbuf);
#else
  cfs_path_init_str(p, utf8_path);
#endif
  return CMP_OK;
}

static int CMP_CALL cmp_generic_io_read_file(void *io, const char *utf8_path,
                                             void *buffer,
                                             cmp_usize buffer_size,
                                             cmp_usize *out_read) {
  FILE *f;
  cmp_usize bytes_read;
  CMP_UNUSED(io);
  if (io == NULL || utf8_path == NULL || buffer == NULL || out_read == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(_MSC_VER)
  if (fopen_s(&f, utf8_path, "rb") != 0) {
    return CMP_ERR_NOT_FOUND;
  }
#else
  f = fopen(utf8_path, "rb");
#endif
  if (f == NULL) {
    return CMP_ERR_NOT_FOUND;
  }
  bytes_read = fread(buffer, 1, buffer_size, f);
  *out_read = bytes_read;
  fclose(f);
  return CMP_OK;
}

static int CMP_CALL cmp_generic_io_read_file_alloc(
    void *io, const char *utf8_path, const CMPAllocator *allocator,
    void **out_data, cmp_usize *out_size) {
  cfs_path p;
  cfs_uintmax_t file_size;
  int rc;
  FILE *f;
  void *data;
  cmp_usize len;
  cfs_error_code ec;

  CMP_UNUSED(io);
  if (io == NULL || utf8_path == NULL || allocator == NULL ||
      out_data == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  cmp_cfs_path_init_utf8(&p, utf8_path);
  cfs_clear_error(&ec);
  if (cfs_file_size(&p, &file_size, &ec) != 0 || ec.value != 0) {
    cfs_path_destroy(&p);
    return CMP_ERR_NOT_FOUND;
  }
  cfs_path_destroy(&p);

  len = (cmp_usize)file_size;
  rc = allocator->alloc(allocator->ctx, len, &data);
  if (rc != CMP_OK) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

#if defined(_MSC_VER)
  if (fopen_s(&f, utf8_path, "rb") != 0) {
    allocator->free(allocator->ctx, data);
    return CMP_ERR_NOT_FOUND;
  }
#else
  f = fopen(utf8_path, "rb");
#endif
  if (f == NULL) {
    allocator->free(allocator->ctx, data);
    return CMP_ERR_NOT_FOUND;
  }
  if (len > 0) {
    fread(data, 1, len, f);
  }
  fclose(f);

  *out_data = data;
  *out_size = len;
  return CMP_OK;
}

static int CMP_CALL cmp_generic_io_file_exists(void *io, const char *utf8_path,
                                               CMPBool *out_exists);

static int CMP_CALL cmp_generic_io_write_file(void *io, const char *utf8_path,
                                              const void *data, cmp_usize size,
                                              CMPBool overwrite) {
  FILE *f;
  cfs_bool exists;
  CMP_UNUSED(io);
  if (io == NULL || utf8_path == NULL || (data == NULL && size > 0)) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (!overwrite) {
    if (cmp_generic_io_file_exists(io, utf8_path, &exists) == CMP_OK &&
        exists) {
      return CMP_ERR_STATE;
    }
  }

#if defined(_MSC_VER)
  if (fopen_s(&f, utf8_path, "wb") != 0) {
    return CMP_ERR_IO;
  }
#else
  f = fopen(utf8_path, "wb");
#endif
  if (f == NULL) {
    return CMP_ERR_IO;
  }
  if (size > 0) {
    fwrite(data, 1, size, f);
  }
  fclose(f);
  return CMP_OK;
}

static int CMP_CALL cmp_generic_io_file_exists(void *io, const char *utf8_path,
                                               CMPBool *out_exists) {
  FILE *f;
  CMP_UNUSED(io);
  if (io == NULL || utf8_path == NULL || out_exists == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(_MSC_VER)
  if (fopen_s(&f, utf8_path, "rb") == 0 && f != NULL) {
    *out_exists = CMP_TRUE;
    fclose(f);
  } else {
    *out_exists = CMP_FALSE;
  }
#else
  f = fopen(utf8_path, "rb");
  if (f != NULL) {
    *out_exists = CMP_TRUE;
    fclose(f);
  } else {
    *out_exists = CMP_FALSE;
  }
#endif
  return CMP_OK;
}

static int CMP_CALL cmp_generic_io_delete_file(void *io,
                                               const char *utf8_path) {
  cfs_path p;
  int rc;
  cfs_error_code ec;
  CMP_UNUSED(io);
  if (io == NULL || utf8_path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  cmp_cfs_path_init_utf8(&p, utf8_path);
  cfs_clear_error(&ec);
  rc = cfs_remove(&p, &ec);
  cfs_path_destroy(&p);
  if (rc != 0 || ec.value != 0) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int CMP_CALL cmp_generic_io_stat_file(void *io, const char *utf8_path,
                                             CMPFileInfo *out_info) {
  cfs_path p;
  cfs_uintmax_t file_size;
  cfs_error_code ec;
  CMP_UNUSED(io);
  if (io == NULL || utf8_path == NULL || out_info == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  cmp_cfs_path_init_utf8(&p, utf8_path);
  cfs_clear_error(&ec);
  if (cfs_file_size(&p, &file_size, &ec) != 0 || ec.value != 0) {
    cfs_path_destroy(&p);
    return CMP_ERR_NOT_FOUND;
  }
  cfs_path_destroy(&p);
  out_info->size_bytes = (cmp_u32)file_size;
  out_info->flags = 0;
  return CMP_OK;
}

static const CMPIOVTable g_cmp_generic_io_vtable = {
    cmp_generic_io_read_file,   cmp_generic_io_read_file_alloc,
    cmp_generic_io_write_file,  cmp_generic_io_file_exists,
    cmp_generic_io_delete_file, cmp_generic_io_stat_file};

CMP_API int CMP_CALL cmp_get_generic_io_vtable(const CMPIOVTable **out_vtable) {
  if (out_vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_vtable = &g_cmp_generic_io_vtable;
  return CMP_OK;
}