/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <stdio.h>
/* clang-format on */

extern int transport_factory_init_client(struct HttpClient *client);

static int g_http_initialized = 0;

/**
 * @brief cmp_http_init
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_init(void) {
  int rc = CMP_SUCCESS;
  if (g_http_initialized) {
    return rc;
  }
  g_http_initialized = 1;
  return rc;
}

/**
 * @brief cmp_http_shutdown
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_shutdown(void) {
  int rc = CMP_SUCCESS;
  if (!g_http_initialized) {
    return rc;
  }
  g_http_initialized = 0;
  return rc;
}

/**
 * @brief cmp_http_client_create
 *
 * @param mod Parameter description.
 * @param out_client Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_client_create(cmp_modality_t *mod,
                           struct HttpClient **out_client) {
  int rc = CMP_SUCCESS;
  struct HttpClient *client = NULL;
  enum ExecutionModality http_mod;

  if (mod == NULL || out_client == NULL || !g_http_initialized) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_http_client_create: Invalid argument or not "
              "initialized\n");
    return rc;
  }

  /* Map cmp_modality_t to enum ExecutionModality */
  switch (mod->type) {
  case CMP_MODALITY_SYNC_SINGLE:
    http_mod = MODALITY_SYNC;
    break;
  case CMP_MODALITY_SYNC_MULTI:
    http_mod = MODALITY_THREAD_POOL;
    break;
  case CMP_MODALITY_ASYNC_SINGLE:
  case CMP_MODALITY_ASYNC_MULTI:
    http_mod = MODALITY_ASYNC;
    break;
  default:
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_http_client_create: Unknown modality type\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct HttpClient), (void **)&client);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_http_client_create: Out of memory\n");
    return rc;
  }

  if (http_client_init(client) != 0) {
    CMP_FREE(client);
    rc = CMP_ERROR_NOT_FOUND; /* No general error code available */
    LOG_DEBUG("Error in cmp_http_client_create: http_client_init failed\n");
    return rc;
  }

  if (transport_factory_init_client(client) != 0) {
    http_client_free(client);
    CMP_FREE(client);
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("Error in cmp_http_client_create: transport_factory_init_client "
              "failed\n");
    return rc;
  }

  client->config.modality = http_mod;

  *out_client = client;
  return rc;
}

/**
 * @brief cmp_http_client_destroy
 *
 * @param client Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_client_destroy(struct HttpClient *client) {
  int rc = CMP_SUCCESS;

  if (client == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_http_client_destroy: Invalid argument\n");
    return rc;
  }

  http_client_free(client);
  CMP_FREE(client);
  return rc;
}

/**
 * @brief cmp_http_request_init
 *
 * @param req Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_request_init(struct HttpRequest *req) {
  int rc = CMP_SUCCESS;

  if (req == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_http_request_init: Invalid argument\n");
    return rc;
  }

  if (http_request_init(req) != 0) {
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("Error in cmp_http_request_init: http_request_init failed\n");
    return rc;
  }

  return rc;
}

/**
 * @brief cmp_http_request_free
 *
 * @param req Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
void cmp_http_request_free(struct HttpRequest *req) {
  if (req != NULL) {
    http_request_free(req);
  }
}

/**
 * @brief cmp_http_send
 *
 * @param client Parameter description.
 * @param req Parameter description.
 * @param out_res Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_send(struct HttpClient *client, const struct HttpRequest *req,
                  struct HttpResponse **out_res) {
  int rc = CMP_SUCCESS;

  if (client == NULL || req == NULL || out_res == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_http_send: Invalid argument\n");
    return rc;
  }

  if (client->send == NULL) {
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("Error in cmp_http_send: client->send is NULL\n");
    return rc;
  }

  if (client->send(client->transport, req, out_res) != 0) {
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("Error in cmp_http_send: client->send failed\n");
    return rc;
  }

  return rc;
}

struct cmp_http_progress_ctx {
  int (*progress_cb)(float percentage, void *user_data);
  void *user_data;
};

/**
 * @brief internal_http_chunk_cb
 *
 * @param user_data Parameter description.
 * @param chunk Parameter description.
 * @param chunk_len Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
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

/**
 * @brief cmp_http_send_with_progress
 *
 * @param client Parameter description.
 * @param req Parameter description.
 * @param percentage Parameter description.
 * @param user_data) Parameter description.
 * @param user_data Parameter description.
 * @param out_res Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_send_with_progress(
    struct HttpClient *client, struct HttpRequest *req,
    int (*progress_cb)(float percentage, void *user_data), void *user_data,
    struct HttpResponse **out_res) {
  int rc = CMP_SUCCESS;
  struct cmp_http_progress_ctx ctx;

  if (client == NULL || req == NULL || out_res == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_http_send_with_progress: Invalid argument\n");
    return rc;
  }

  ctx.progress_cb = progress_cb;
  ctx.user_data = user_data;

  req->on_chunk = internal_http_chunk_cb;
  req->on_chunk_user_data = &ctx;

  if (client->send == NULL) {
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("Error in cmp_http_send_with_progress: client->send is NULL\n");
    return rc;
  }

  if (client->send(client->transport, req, out_res) != 0) {
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("Error in cmp_http_send_with_progress: client->send failed\n");
    return rc;
  }

  /* Final progress */
  if (progress_cb) {
    progress_cb(100.0f, user_data);
  }

  return rc;
}

struct cmp_http_multi_progress_ctx {
  int (*progress_cb)(float percentage, void *user_data);
  void *user_data;
};

/**
 * @brief internal_http_multi_progress_cb
 *
 * @param current_bytes Parameter description.
 * @param total_bytes Parameter description.
 * @param user_data Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
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

/**
 * @brief cmp_http_send_multi_with_progress
 *
 * @param client Parameter description.
 * @param requests Parameter description.
 * @param num_requests Parameter description.
 * @param percentage Parameter description.
 * @param user_data) Parameter description.
 * @param user_data Parameter description.
 * @param out_responses Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_send_multi_with_progress(
    struct HttpClient *client, struct HttpRequest *const *requests,
    size_t num_requests, int (*progress_cb)(float percentage, void *user_data),
    void *user_data, struct HttpResponse **out_responses) {
  int rc = CMP_SUCCESS;
  struct cmp_http_multi_progress_ctx ctx;
  struct HttpFuture **futures = NULL;
  size_t i;

  if (client == NULL || requests == NULL || num_requests == 0 ||
      out_responses == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_http_send_multi_with_progress: Invalid argument\n");
    return rc;
  }

  rc =
      CMP_MALLOC(sizeof(struct HttpFuture *) * num_requests, (void **)&futures);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_http_send_multi_with_progress: Out of memory\n");
    return rc;
  }

  for (i = 0; i < num_requests; i++) {
    rc = CMP_MALLOC(sizeof(struct HttpFuture), (void **)&futures[i]);
    if (rc != CMP_SUCCESS) {
      size_t j;
      for (j = 0; j < i; j++) {
        CMP_FREE(futures[j]);
      }
      CMP_FREE(futures);
      LOG_DEBUG("Error in cmp_http_send_multi_with_progress: Out of memory "
                "allocating future\n");
      return rc;
    }
    http_future_init(futures[i]);
  }

  ctx.progress_cb = progress_cb;
  ctx.user_data = user_data;

  if (http_client_send_multi(client, requests, num_requests, futures,
                             internal_http_multi_progress_cb, &ctx, 0) != 0) {
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("Error in cmp_http_send_multi_with_progress: "
              "http_client_send_multi failed\n");
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

  return rc;
}

struct cmp_http_download_ctx {
  int (*progress_cb)(float percentage, void *user_data);
  void *user_data;
  FILE *f;
};

/**
 * @brief internal_http_download_chunk_cb
 *
 * @param user_data Parameter description.
 * @param chunk Parameter description.
 * @param chunk_len Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
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

/**
 * @brief cmp_http_download
 *
 * @param client Parameter description.
 * @param url Parameter description.
 * @param save_virtual_path Parameter description.
 * @param percentage Parameter description.
 * @param user_data) Parameter description.
 * @param user_data Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_download(struct HttpClient *client, const char *url,
                      const char *save_virtual_path,
                      int (*progress_cb)(float percentage, void *user_data),
                      void *user_data) {
  int rc = CMP_SUCCESS;
  struct HttpRequest req;
  struct HttpResponse *res = NULL;
  struct cmp_http_download_ctx ctx;
  cmp_string_t resolved_path;
  const cfs_char_t *native_path;
  cfs_path p;

  if (client == NULL || url == NULL || save_virtual_path == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_http_download: Invalid argument\n");
    return rc;
  }

  rc = cmp_vfs_resolve_path(save_virtual_path, &resolved_path);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_http_download: cmp_vfs_resolve_path failed\n");
    return rc;
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
      rc = CMP_ERROR_INVALID_ARG;
      LOG_DEBUG("Error in cmp_http_download: UTF8 to UTF16 failed\n");
      return rc;
    }
    rc = CMP_MALLOC((size_t)req_len * sizeof(wchar_t), (void **)&wpath);
    if (rc != CMP_SUCCESS) {
      cfs_path_destroy(&p);
      cmp_string_destroy(&resolved_path);
      LOG_DEBUG("Error in cmp_http_download: Out of memory\n");
      return rc;
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
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("Error in cmp_http_download: Failed to open file for writing\n");
    return rc;
  }

  ctx.progress_cb = progress_cb;
  ctx.user_data = user_data;

  if (http_request_init(&req) != 0) {
    fclose(ctx.f);
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("Error in cmp_http_download: http_request_init failed\n");
    return rc;
  }

  req.method = HTTP_GET;
  req.url = (char *)url;
  req.on_chunk = internal_http_download_chunk_cb;
  req.on_chunk_user_data = &ctx;

  if (client->send == NULL ||
      client->send(client->transport, &req, &res) != 0) {
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("Error in cmp_http_download: HTTP send failed\n");
  }

  if (res != NULL) {
    http_response_free(res);
  }

  req.url = NULL; /* Not allocated by us */
  http_request_free(&req);
  fclose(ctx.f);

  if (rc == CMP_SUCCESS && progress_cb) {
    progress_cb(100.0f, user_data);
  }

  return rc;
}

/**
 * @brief cmp_http_response_free
 *
 * @param res Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
void cmp_http_response_free(struct HttpResponse *res) {
  if (res != NULL) {
    http_response_free(res);
  }
}

/**
 * @brief cmp_http_ws_init
 *
 * @param req Parameter description.
 * @param config Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_ws_init(struct HttpRequest *req,
                     const struct c_abstract_http_ws_config *config) {
  int rc = CMP_SUCCESS;

  if (req == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_http_ws_init: Invalid argument\n");
    return rc;
  }
  if (c_abstract_http_ws_init(req, config) != 0) {
    rc = CMP_ERROR_INVALID_ARG; /* Map c-abstract-http errors to cmp errors */
    LOG_DEBUG("Error in cmp_http_ws_init: c_abstract_http_ws_init failed\n");
    return rc;
  }
  return rc;
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

/**
 * @brief cmp_http_ws_task
 *
 * @param arg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static void cmp_http_ws_task(void *arg) {
  cmp_http_ws_task_ctx_t *ctx = (cmp_http_ws_task_ctx_t *)arg;
  c_abstract_http_ws_sync_read_loop(ctx->client, ctx->req, ctx->on_msg,
                                    ctx->on_err, ctx->on_close, ctx->user_data,
                                    ctx->exit_flag);
  CMP_FREE(ctx);
}

/**
 * @brief cmp_http_ws_run
 *
 * @param mod Parameter description.
 * @param client Parameter description.
 * @param req Parameter description.
 * @param on_msg Parameter description.
 * @param on_err Parameter description.
 * @param on_close Parameter description.
 * @param user_data Parameter description.
 * @param exit_flag Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_ws_run(cmp_modality_t *mod, struct HttpClient *client,
                    struct HttpRequest *req,
                    c_abstract_http_ws_on_message on_msg,
                    c_abstract_http_ws_on_error on_err,
                    c_abstract_http_ws_on_close on_close, void *user_data,
                    volatile int *exit_flag) {
  int rc = CMP_SUCCESS;

  if (client == NULL || req == NULL || mod == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_http_ws_run: Invalid argument\n");
    return rc;
  }

  if (mod->type == CMP_MODALITY_ASYNC_SINGLE ||
      mod->type == CMP_MODALITY_ASYNC_MULTI) {
    if (c_abstract_http_ws_async_register(client, req, on_msg, on_err, on_close,
                                          user_data) != 0) {
      rc = CMP_ERROR_INVALID_ARG;
      LOG_DEBUG("Error in cmp_http_ws_run: c_abstract_http_ws_async_register "
                "failed\n");
      return rc;
    }
  } else {
    cmp_http_ws_task_ctx_t *ctx = NULL;
    rc = CMP_MALLOC(sizeof(cmp_http_ws_task_ctx_t), (void **)&ctx);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_http_ws_run: Out of memory\n");
      return rc;
    }
    ctx->client = client;
    ctx->req = req;
    ctx->on_msg = on_msg;
    ctx->on_err = on_err;
    ctx->on_close = on_close;
    ctx->user_data = user_data;
    ctx->exit_flag = exit_flag;

    rc = cmp_modality_queue_task(mod, cmp_http_ws_task, ctx);
    if (rc != CMP_SUCCESS) {
      CMP_FREE(ctx);
      LOG_DEBUG("Error in cmp_http_ws_run: cmp_modality_queue_task failed\n");
      return rc;
    }
  }

  return rc;
}

/**
 * @brief cmp_http_ws_send
 *
 * @param req Parameter description.
 * @param opcode Parameter description.
 * @param payload Parameter description.
 * @param len Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_ws_send(struct HttpRequest *req,
                     enum c_abstract_http_ws_opcode opcode,
                     const unsigned char *payload, size_t len) {
  int rc = CMP_SUCCESS;

  if (req == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_http_ws_send: Invalid argument\n");
    return rc;
  }
  /* For synchronous environments or thread pools, c_abstract_http_ws_send
     writes to socket. For async event loops, c_abstract_http_ws_async_send
     enqueues it. We default to async_send which abstracts this logic depending
     on transport if possible, otherwise fallback to standard send. */
  if (c_abstract_http_ws_async_send(req, opcode, payload, len) != 0) {
    if (c_abstract_http_ws_send(req, opcode, payload, len) != 0) {
      rc = CMP_ERROR_INVALID_ARG;
      LOG_DEBUG("Error in cmp_http_ws_send: Failed to send WS payload\n");
      return rc;
    }
  }
  return rc;
}

/**
 * @brief cmp_http_ws_close
 *
 * @param req Parameter description.
 * @param status_code Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_ws_close(struct HttpRequest *req, int status_code) {
  int rc = CMP_SUCCESS;

  if (req == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_http_ws_close: Invalid argument\n");
    return rc;
  }
  if (c_abstract_http_ws_close(req, status_code) != 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_http_ws_close: c_abstract_http_ws_close failed\n");
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_http_sse_init
 *
 * @param req Parameter description.
 * @param config Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_sse_init(struct HttpRequest *req,
                      const struct c_abstract_http_sse_config *config) {
  int rc = CMP_SUCCESS;

  if (req == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_http_sse_init: Invalid argument\n");
    return rc;
  }
  if (c_abstract_http_sse_init(req, config) != 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_http_sse_init: c_abstract_http_sse_init failed\n");
    return rc;
  }
  return rc;
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

/**
 * @brief cmp_http_sse_task
 *
 * @param arg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static void cmp_http_sse_task(void *arg) {
  cmp_http_sse_task_ctx_t *ctx = (cmp_http_sse_task_ctx_t *)arg;
  c_abstract_http_sse_sync_read_loop(ctx->client, ctx->req, ctx->on_evt,
                                     ctx->on_err, ctx->on_close, ctx->user_data,
                                     ctx->exit_flag);
  CMP_FREE(ctx);
}

/**
 * @brief cmp_http_sse_run
 *
 * @param mod Parameter description.
 * @param client Parameter description.
 * @param req Parameter description.
 * @param on_evt Parameter description.
 * @param on_err Parameter description.
 * @param on_close Parameter description.
 * @param user_data Parameter description.
 * @param exit_flag Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_sse_run(cmp_modality_t *mod, struct HttpClient *client,
                     struct HttpRequest *req,
                     c_abstract_http_sse_on_event on_evt,
                     c_abstract_http_sse_on_error on_err,
                     c_abstract_http_sse_on_close on_close, void *user_data,
                     volatile int *exit_flag) {
  int rc = CMP_SUCCESS;

  if (client == NULL || req == NULL || mod == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_http_sse_run: Invalid argument\n");
    return rc;
  }

  if (mod->type == CMP_MODALITY_ASYNC_SINGLE ||
      mod->type == CMP_MODALITY_ASYNC_MULTI) {
    if (c_abstract_http_sse_async_register(client, req, on_evt, on_err,
                                           on_close, user_data) != 0) {
      rc = CMP_ERROR_INVALID_ARG;
      LOG_DEBUG("Error in cmp_http_sse_run: c_abstract_http_sse_async_register "
                "failed\n");
      return rc;
    }
  } else {
    cmp_http_sse_task_ctx_t *ctx = NULL;
    rc = CMP_MALLOC(sizeof(cmp_http_sse_task_ctx_t), (void **)&ctx);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_http_sse_run: Out of memory\n");
      return rc;
    }
    ctx->client = client;
    ctx->req = req;
    ctx->on_evt = on_evt;
    ctx->on_err = on_err;
    ctx->on_close = on_close;
    ctx->user_data = user_data;
    ctx->exit_flag = exit_flag;

    rc = cmp_modality_queue_task(mod, cmp_http_sse_task, ctx);
    if (rc != CMP_SUCCESS) {
      CMP_FREE(ctx);
      LOG_DEBUG("Error in cmp_http_sse_run: cmp_modality_queue_task failed\n");
      return rc;
    }
  }

  return rc;
}
