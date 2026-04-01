/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <stdio.h>
/* clang-format on */

extern int transport_factory_init_client(struct HttpClient *client);

static int g_http_initialized = 0;

int cmp_http_init(void) {
  if (g_http_initialized) {
    return CMP_SUCCESS;
  }
  g_http_initialized = 1;
  return CMP_SUCCESS;
}

int cmp_http_shutdown(void) {
  if (!g_http_initialized) {
    return CMP_SUCCESS;
  }
  g_http_initialized = 0;
  return CMP_SUCCESS;
}

int cmp_http_client_create(cmp_modality_t *mod,
                           struct HttpClient **out_client) {
  struct HttpClient *client;
  enum ExecutionModality http_mod;

  if (mod == NULL || out_client == NULL || !g_http_initialized) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* Map cmp_modality_t to enum ExecutionModality */
  switch (mod->type) {
  case CMP_MODALITY_SINGLE:
    http_mod = MODALITY_SYNC;
    break;
  case CMP_MODALITY_THREADED:
    http_mod = MODALITY_THREAD_POOL;
    break;
  case CMP_MODALITY_ASYNC:
  case CMP_MODALITY_EVENTLOOP:
    http_mod = MODALITY_ASYNC;
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(struct HttpClient), (void **)&client) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  if (http_client_init(client) != 0) {
    CMP_FREE(client);
    return CMP_ERROR_NOT_FOUND; /* No general error code available */
  }

  if (transport_factory_init_client(client) != 0) {
    http_client_free(client);
    CMP_FREE(client);
    return CMP_ERROR_NOT_FOUND;
  }

  client->config.modality = http_mod;

  *out_client = client;
  return CMP_SUCCESS;
}

int cmp_http_client_destroy(struct HttpClient *client) {
  if (client == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  http_client_free(client);
  CMP_FREE(client);
  return CMP_SUCCESS;
}

int cmp_http_request_init(struct HttpRequest *req) {
  if (req == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (http_request_init(req) != 0) {
    return CMP_ERROR_NOT_FOUND;
  }
  return CMP_SUCCESS;
}

void cmp_http_request_free(struct HttpRequest *req) {
  if (req != NULL) {
    http_request_free(req);
  }
}

int cmp_http_send(struct HttpClient *client, const struct HttpRequest *req,
                  struct HttpResponse **out_res) {
  if (client == NULL || req == NULL || out_res == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (client->send == NULL) {
    return CMP_ERROR_NOT_FOUND;
  }

  if (client->send(client->transport, req, out_res) != 0) {
    return CMP_ERROR_NOT_FOUND;
  }

  return CMP_SUCCESS;
}

struct cmp_http_progress_ctx {
  int (*progress_cb)(float percentage, void *user_data);
  void *user_data;
};

static int internal_http_chunk_cb(void *user_data, const void *chunk,
                                  size_t chunk_len) {
  /* For now we just return 0 to continue reading. True progress
     would require c-abstract-http exposing total content length
     to the chunk cb, which we will handle in the future via headers. */
  struct cmp_http_progress_ctx *ctx = (struct cmp_http_progress_ctx *)user_data;
  (void)chunk;
  (void)chunk_len;
  if (ctx && ctx->progress_cb) {
    /* Dummy progress tick for now */
    if (ctx->progress_cb(50.0f, ctx->user_data) != 0) {
      return 1; /* abort */
    }
  }
  return 0;
}

int cmp_http_send_with_progress(
    struct HttpClient *client, struct HttpRequest *req,
    int (*progress_cb)(float percentage, void *user_data), void *user_data,
    struct HttpResponse **out_res) {
  struct cmp_http_progress_ctx ctx;

  if (client == NULL || req == NULL || out_res == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  ctx.progress_cb = progress_cb;
  ctx.user_data = user_data;

  req->on_chunk = internal_http_chunk_cb;
  req->on_chunk_user_data = &ctx;

  if (client->send == NULL) {
    return CMP_ERROR_NOT_FOUND;
  }

  if (client->send(client->transport, req, out_res) != 0) {
    return CMP_ERROR_NOT_FOUND;
  }

  /* Final progress */
  if (progress_cb) {
    progress_cb(100.0f, user_data);
  }

  return CMP_SUCCESS;
}

struct cmp_http_multi_progress_ctx {
  int (*progress_cb)(float percentage, void *user_data);
  void *user_data;
};

static int internal_http_multi_progress_cb(size_t current_bytes,
                                           size_t total_bytes,
                                           void *user_data) {
  struct cmp_http_multi_progress_ctx *ctx =
      (struct cmp_http_multi_progress_ctx *)user_data;
  if (ctx && ctx->progress_cb) {
    float percentage = 0.0f;
    if (total_bytes > 0) {
      percentage = ((float)current_bytes / (float)total_bytes) * 100.0f;
    }
    if (ctx->progress_cb(percentage, ctx->user_data) != 0) {
      return 1; /* Abort */
    }
  }
  return 0;
}

int cmp_http_send_multi_with_progress(
    struct HttpClient *client, struct HttpRequest *const *requests,
    size_t num_requests, int (*progress_cb)(float percentage, void *user_data),
    void *user_data, struct HttpResponse **out_responses) {
  struct cmp_http_multi_progress_ctx ctx;
  struct HttpFuture **futures;
  size_t i;
  int res = CMP_SUCCESS;

  if (client == NULL || requests == NULL || num_requests == 0 ||
      out_responses == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(struct HttpFuture *) * num_requests,
                 (void **)&futures) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  for (i = 0; i < num_requests; i++) {
    if (CMP_MALLOC(sizeof(struct HttpFuture), (void **)&futures[i]) !=
        CMP_SUCCESS) {
      size_t j;
      for (j = 0; j < i; j++) {
        CMP_FREE(futures[j]);
      }
      CMP_FREE(futures);
      return CMP_ERROR_OOM;
    }
    http_future_init(futures[i]);
  }

  ctx.progress_cb = progress_cb;
  ctx.user_data = user_data;

  if (http_client_send_multi(client, requests, num_requests, futures,
                             internal_http_multi_progress_cb, &ctx, 0) != 0) {
    res = CMP_ERROR_NOT_FOUND;
  } else {
    for (i = 0; i < num_requests; i++) {
      out_responses[i] = futures[i]->response;
    }
  }

  for (i = 0; i < num_requests; i++) {
    /* We extract the response pointer so we don't free it with the future */
    futures[i]->response = NULL;
    http_future_free(futures[i]);
    CMP_FREE(futures[i]);
  }
  CMP_FREE(futures);

  return res;
}

struct cmp_http_download_ctx {
  int (*progress_cb)(float percentage, void *user_data);
  void *user_data;
  FILE *f;
};

static int internal_http_download_chunk_cb(void *user_data, const void *chunk,
                                           size_t chunk_len) {
  struct cmp_http_download_ctx *ctx = (struct cmp_http_download_ctx *)user_data;

  if (ctx && ctx->f && chunk && chunk_len > 0) {
    fwrite(chunk, 1, chunk_len, ctx->f);
  }

  if (ctx && ctx->progress_cb) {
    /* Dummy progress tick for now */
    if (ctx->progress_cb(50.0f, ctx->user_data) != 0) {
      return 1; /* abort */
    }
  }
  return 0;
}

int cmp_http_download(struct HttpClient *client, const char *url,
                      const char *save_virtual_path,
                      int (*progress_cb)(float percentage, void *user_data),
                      void *user_data) {
  struct HttpRequest req;
  struct HttpResponse *res = NULL;
  struct cmp_http_download_ctx ctx;
  cmp_string_t resolved_path;
  const cfs_char_t *native_path;
  cfs_path p;
  int err;

  if (client == NULL || url == NULL || save_virtual_path == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (cmp_vfs_resolve_path(save_virtual_path, &resolved_path) != CMP_SUCCESS) {
    return CMP_ERROR_INVALID_ARG;
  }

  cfs_path_init(&p);
#if defined(_WIN32)
  {
    wchar_t *wpath = NULL;
    cfs_size_t req_len = 0;
    cfs_utf8_to_utf16(resolved_path.data, NULL, 0, &req_len);
    if (req_len <= 0) {
      cfs_path_destroy(&p);
      cmp_string_destroy(&resolved_path);
      return CMP_ERROR_INVALID_ARG;
    }
    if (CMP_MALLOC((size_t)req_len * sizeof(wchar_t), (void **)&wpath) !=
        CMP_SUCCESS) {
      cfs_path_destroy(&p);
      cmp_string_destroy(&resolved_path);
      return CMP_ERROR_OOM;
    }
    cfs_utf8_to_utf16(resolved_path.data, wpath, req_len, NULL);
    cfs_path_assign(&p, wpath);
    CMP_FREE(wpath);
  }
#else
  cfs_path_assign(&p, resolved_path.data);
#endif

  cmp_string_destroy(&resolved_path);
  cfs_path_c_str(&p, &native_path);

#if defined(_WIN32)
  ctx.f = _wfopen(native_path, L"wb");
#else
  ctx.f = fopen(native_path, "wb");
#endif
  cfs_path_destroy(&p);

  if (ctx.f == NULL) {
    return CMP_ERROR_NOT_FOUND;
  }

  ctx.progress_cb = progress_cb;
  ctx.user_data = user_data;

  if (http_request_init(&req) != 0) {
    fclose(ctx.f);
    return CMP_ERROR_NOT_FOUND;
  }

  req.method = HTTP_GET;
  req.url = (char *)url;
  req.on_chunk = internal_http_download_chunk_cb;
  req.on_chunk_user_data = &ctx;

  err = CMP_SUCCESS;
  if (client->send == NULL ||
      client->send(client->transport, &req, &res) != 0) {
    err = CMP_ERROR_NOT_FOUND;
  }

  if (res != NULL) {
    http_response_free(res);
  }

  req.url = NULL; /* Not allocated by us */
  http_request_free(&req);
  fclose(ctx.f);

  if (err == CMP_SUCCESS && progress_cb) {
    progress_cb(100.0f, user_data);
  }

  return err;
}

void cmp_http_response_free(struct HttpResponse *res) {
  if (res != NULL) {
    http_response_free(res);
  }
}

int cmp_http_ws_init(struct HttpRequest *req,
                     const struct c_abstract_http_ws_config *config) {
  if (req == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (c_abstract_http_ws_init(req, config) != 0) {
    return CMP_ERROR_INVALID_ARG; /* Map c-abstract-http errors to cmp errors */
  }
  return CMP_SUCCESS;
}

typedef struct {
  struct HttpClient *client;
  struct HttpRequest *req;
  c_abstract_http_ws_on_message on_msg;
  c_abstract_http_ws_on_error on_err;
  c_abstract_http_ws_on_close on_close;
  void *user_data;
  volatile int *exit_flag;
} cmp_http_ws_task_ctx_t;

static void cmp_http_ws_task(void *arg) {
  cmp_http_ws_task_ctx_t *ctx = (cmp_http_ws_task_ctx_t *)arg;
  c_abstract_http_ws_sync_read_loop(ctx->client, ctx->req, ctx->on_msg,
                                    ctx->on_err, ctx->on_close, ctx->user_data,
                                    ctx->exit_flag);
  CMP_FREE(ctx);
}

int cmp_http_ws_run(cmp_modality_t *mod, struct HttpClient *client,
                    struct HttpRequest *req,
                    c_abstract_http_ws_on_message on_msg,
                    c_abstract_http_ws_on_error on_err,
                    c_abstract_http_ws_on_close on_close, void *user_data,
                    volatile int *exit_flag) {
  if (client == NULL || req == NULL || mod == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (mod->type == CMP_MODALITY_ASYNC || mod->type == CMP_MODALITY_EVENTLOOP) {
    if (c_abstract_http_ws_async_register(client, req, on_msg, on_err, on_close,
                                          user_data) != 0) {
      return CMP_ERROR_INVALID_ARG;
    }
  } else {
    cmp_http_ws_task_ctx_t *ctx;
    if (CMP_MALLOC(sizeof(cmp_http_ws_task_ctx_t), (void **)&ctx) !=
        CMP_SUCCESS) {
      return CMP_ERROR_OOM;
    }
    ctx->client = client;
    ctx->req = req;
    ctx->on_msg = on_msg;
    ctx->on_err = on_err;
    ctx->on_close = on_close;
    ctx->user_data = user_data;
    ctx->exit_flag = exit_flag;

    if (cmp_modality_queue_task(mod, cmp_http_ws_task, ctx) != CMP_SUCCESS) {
      CMP_FREE(ctx);
      return CMP_ERROR_INVALID_ARG;
    }
  }

  return CMP_SUCCESS;
}

int cmp_http_ws_send(struct HttpRequest *req,
                     enum c_abstract_http_ws_opcode opcode,
                     const unsigned char *payload, size_t len) {
  if (req == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  /* For synchronous environments or thread pools, c_abstract_http_ws_send
     writes to socket. For async event loops, c_abstract_http_ws_async_send
     enqueues it. We default to async_send which abstracts this logic depending
     on transport if possible, otherwise fallback to standard send. */
  if (c_abstract_http_ws_async_send(req, opcode, payload, len) != 0) {
    if (c_abstract_http_ws_send(req, opcode, payload, len) != 0) {
      return CMP_ERROR_INVALID_ARG;
    }
  }
  return CMP_SUCCESS;
}

int cmp_http_ws_close(struct HttpRequest *req, int status_code) {
  if (req == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (c_abstract_http_ws_close(req, status_code) != 0) {
    return CMP_ERROR_INVALID_ARG;
  }
  return CMP_SUCCESS;
}

int cmp_http_sse_init(struct HttpRequest *req,
                      const struct c_abstract_http_sse_config *config) {
  if (req == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (c_abstract_http_sse_init(req, config) != 0) {
    return CMP_ERROR_INVALID_ARG;
  }
  return CMP_SUCCESS;
}

typedef struct {
  struct HttpClient *client;
  struct HttpRequest *req;
  c_abstract_http_sse_on_event on_evt;
  c_abstract_http_sse_on_error on_err;
  c_abstract_http_sse_on_close on_close;
  void *user_data;
  volatile int *exit_flag;
} cmp_http_sse_task_ctx_t;

static void cmp_http_sse_task(void *arg) {
  cmp_http_sse_task_ctx_t *ctx = (cmp_http_sse_task_ctx_t *)arg;
  c_abstract_http_sse_sync_read_loop(ctx->client, ctx->req, ctx->on_evt,
                                     ctx->on_err, ctx->on_close, ctx->user_data,
                                     ctx->exit_flag);
  CMP_FREE(ctx);
}

int cmp_http_sse_run(cmp_modality_t *mod, struct HttpClient *client,
                     struct HttpRequest *req,
                     c_abstract_http_sse_on_event on_evt,
                     c_abstract_http_sse_on_error on_err,
                     c_abstract_http_sse_on_close on_close, void *user_data,
                     volatile int *exit_flag) {
  if (client == NULL || req == NULL || mod == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (mod->type == CMP_MODALITY_ASYNC || mod->type == CMP_MODALITY_EVENTLOOP) {
    if (c_abstract_http_sse_async_register(client, req, on_evt, on_err,
                                           on_close, user_data) != 0) {
      return CMP_ERROR_INVALID_ARG;
    }
  } else {
    cmp_http_sse_task_ctx_t *ctx;
    if (CMP_MALLOC(sizeof(cmp_http_sse_task_ctx_t), (void **)&ctx) !=
        CMP_SUCCESS) {
      return CMP_ERROR_OOM;
    }
    ctx->client = client;
    ctx->req = req;
    ctx->on_evt = on_evt;
    ctx->on_err = on_err;
    ctx->on_close = on_close;
    ctx->user_data = user_data;
    ctx->exit_flag = exit_flag;

    if (cmp_modality_queue_task(mod, cmp_http_sse_task, ctx) != CMP_SUCCESS) {
      CMP_FREE(ctx);
      return CMP_ERROR_INVALID_ARG;
    }
  }

  return CMP_SUCCESS;
}
