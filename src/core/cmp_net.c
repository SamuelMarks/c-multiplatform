#include "cmpc/cmp_net.h"
#include <stdlib.h>
#include <string.h>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#if defined(_MSC_VER)
#pragma warning(push, 0)
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <c_abstract_http/c_abstract_http.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

typedef struct CMPNetFetchCtx {
  CMPEnv *env;
  CMPTasks *tasks;
  CMPEventLoop *loop;
  const CMPAllocator *allocator;
  char *url;
  CMPNetFetchCallback callback;
  void *user;

  void *data;
  cmp_usize size;
  int status;
  int status_code;
} CMPNetFetchCtx;

static int CMP_CALL net_fetch_wake_cb(void *cb_ctx) {
  CMPNetFetchCtx *ctx = (CMPNetFetchCtx *)cb_ctx;
  if (ctx != NULL) {
    if (ctx->callback != NULL) {
      ctx->callback(ctx, ctx->url, ctx->data, ctx->size, ctx->status,
                    ctx->status_code, ctx->user);
    }
    if (ctx->url != NULL) {
      ctx->allocator->free(ctx->allocator->ctx, ctx->url);
    }
    ctx->allocator->free(ctx->allocator->ctx, ctx);
  }
  return CMP_OK;
}

static int CMP_CALL net_fetch_task_cb(void *user) {
  CMPNetFetchCtx *ctx = (CMPNetFetchCtx *)user;
  struct HttpClient client;
  struct HttpRequest req;
  struct HttpResponse *res = NULL;
  cmp_usize url_len;
  int rc;
  int alloc_rc;

  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = http_client_init(&client);
  if (rc == 0) {
    rc = http_request_init(&req);
    if (rc == 0) {
      url_len = strlen(ctx->url);
      req.url = (char *)malloc(url_len + 1);
      if (req.url != NULL) {
#if defined(_MSC_VER)
        strcpy_s(req.url, url_len + 1, ctx->url);
#else
        strcpy(req.url, ctx->url);
#endif
      }
      req.method = HTTP_GET;

      if (client.send != NULL && req.url != NULL) {
        rc = client.send(client.transport, &req, &res);
        if (rc == 0 && res != NULL) {
          ctx->status = CMP_OK;
          ctx->status_code = res->status_code;

          if (res->body != NULL && res->body_len > 0) {
            ctx->size = res->body_len;
            alloc_rc = ctx->allocator->alloc(ctx->allocator->ctx, ctx->size,
                                             &ctx->data);
            if (alloc_rc == CMP_OK && ctx->data != NULL) {
              memcpy(ctx->data, res->body, ctx->size);
            } else {
              ctx->status = CMP_ERR_OUT_OF_MEMORY;
              ctx->size = 0;
            }
          }
          http_response_free(res);
        } else {
          ctx->status = CMP_ERR_IO;
        }
      } else {
        ctx->status = CMP_ERR_UNSUPPORTED;
      }
      http_request_free(&req);
    } else {
      ctx->status = CMP_ERR_UNKNOWN;
    }
    http_client_free(&client);
  } else {
    ctx->status = CMP_ERR_UNKNOWN;
  }

  /* Wake main thread to run the callback. */
  if (ctx->loop != NULL && ctx->loop->vtable != NULL &&
      ctx->loop->vtable->wake != NULL) {
    rc = ctx->loop->vtable->wake(ctx->loop->ctx, net_fetch_wake_cb, ctx);
    if (rc != CMP_OK) {
      /* Waking failed, cleanup must happen here. */
      if (ctx->data != NULL) {
        ctx->allocator->free(ctx->allocator->ctx, ctx->data);
      }
      if (ctx->url != NULL) {
        ctx->allocator->free(ctx->allocator->ctx, ctx->url);
      }
      ctx->allocator->free(ctx->allocator->ctx, ctx);
    }
  } else {
    /* No loop to wake, cleanup. */
    if (ctx->data != NULL) {
      ctx->allocator->free(ctx->allocator->ctx, ctx->data);
    }
    if (ctx->url != NULL) {
      ctx->allocator->free(ctx->allocator->ctx, ctx->url);
    }
    ctx->allocator->free(ctx->allocator->ctx, ctx);
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cmp_net_fetch_async(CMPEnv *env, CMPTasks *tasks,
                                         CMPEventLoop *loop,
                                         const CMPAllocator *allocator,
                                         const char *url,
                                         CMPNetFetchCallback callback,
                                         void *user) {
  CMPNetFetchCtx *ctx = NULL;
  cmp_usize url_len;
  int rc;

  if (env == NULL || tasks == NULL || tasks->vtable == NULL ||
      tasks->vtable->task_post == NULL || loop == NULL ||
      loop->vtable == NULL || allocator == NULL || url == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = allocator->alloc(allocator->ctx, sizeof(CMPNetFetchCtx), (void **)&ctx);
  if (rc != CMP_OK || ctx == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  url_len = strlen(url);
  rc = allocator->alloc(allocator->ctx, url_len + 1, (void **)&ctx->url);
  if (rc != CMP_OK || ctx->url == NULL) {
    allocator->free(allocator->ctx, ctx);
    return CMP_ERR_OUT_OF_MEMORY;
  }
  memcpy(ctx->url, url, url_len);
  ctx->url[url_len] = '\0';

  ctx->env = env;
  ctx->tasks = tasks;
  ctx->loop = loop;
  ctx->allocator = allocator;
  ctx->callback = callback;
  ctx->user = user;
  ctx->data = NULL;
  ctx->size = 0;
  ctx->status = CMP_ERR_UNKNOWN;
  ctx->status_code = 0;

  rc = tasks->vtable->task_post(tasks->ctx, net_fetch_task_cb, ctx);
  if (rc != CMP_OK) {
    allocator->free(allocator->ctx, ctx->url);
    allocator->free(allocator->ctx, ctx);
    return rc;
  }

  return CMP_OK;
}