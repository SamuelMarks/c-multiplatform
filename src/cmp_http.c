/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <stdio.h>
/* clang-format on */

extern int transport_factory_init_client(struct HttpClient *client);

static int g_http_initialized = 0;

/**
 * @brief Initialize global networking subsystem.
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_init(void) {
  int rc = 0;
  if (g_http_initialized) {
    return 0;
  }

  rc = c_abstract_http_global_init();
  if (rc != 0) {
    LOG_DEBUG("Error in cmp_http_init: c_abstract_http_global_init failed\n");
    return rc;
  }
  g_http_initialized = 1;
  return 0;
}

/**
 * @brief Shut down global networking subsystem.
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_shutdown(void) {
  if (!g_http_initialized) {
    return 0;
  }
  c_abstract_http_global_cleanup();
  g_http_initialized = 0;
  return 0;
}

/**
 * @brief Create an HTTP client configured for the provided modality.
 *
 * @param mod The execution modality to bind the client to.
 * @param out_client Pointer to receive the allocated client handle.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_client_create(cmp_modality_t *mod,
                           struct HttpClient **out_client) {
  int rc = 0; /* CMP_SUCCESS */
  struct HttpClient *client = NULL;

  if (!out_client) {
    LOG_DEBUG("cmp_http_client_create: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_client = NULL;

  rc = CMP_MALLOC(sizeof(struct HttpClient), (void **)&client);
  if (rc != 0 || !client) {
    LOG_DEBUG("cmp_http_client_create: CMP_MALLOC failed\n");
    return CMP_ERROR_OOM;
  }

  c_abstract_http_client_init(client);

  /* Set up transport using factory */
  rc = transport_factory_init_client(client);
  if (rc != 0) {
    CMP_FREE(client);
    LOG_DEBUG("Error in cmp_http_client_create: transport_factory_init_client "
              "failed\n");
    return rc;
  }

  /* Optional: Store modality context in client's internal structures if needed
   * for async event loops. For now, c-abstract-http handles transport. */
  if (mod) {
    /* If the modality has an event loop (e.g. epoll), map the transport's
     * sockets. (Platform specific bridging goes here) */
  }

  *out_client = client;
  return 0;
}

/**
 * @brief Destroy an HTTP client.
 *
 * @param client The client to destroy.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_client_destroy(struct HttpClient *client) {
  int rc = 0;
  if (!client) {
    LOG_DEBUG("cmp_http_client_destroy: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  c_abstract_http_client_cleanup(client);
  rc = CMP_FREE(client);
  if (rc != 0) {
    LOG_DEBUG("cmp_http_client_destroy: CMP_FREE failed\n");
  }
  return rc;
}

/**
 * @brief Initialize an HTTP request.
 *
 * @param req The request to initialize.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_request_init(struct HttpRequest *req) {
  if (!req) {
    LOG_DEBUG("cmp_http_request_init: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  c_abstract_http_request_init(req);
  return 0;
}

/**
 * @brief Free resources associated with an HTTP request.
 *
 * @param req The request to free.
 */
void cmp_http_request_free(struct HttpRequest *req) {
  if (req) {
    c_abstract_http_request_cleanup(req);
  }
}

/**
 * @brief Free resources associated with an HTTP response.
 *
 * @param res The response to free.
 */
void cmp_http_response_free(struct HttpResponse *res) {
  int rc;
  if (res) {
    c_abstract_http_response_cleanup(res);
    rc = CMP_FREE(res);
    if (rc != 0) {
      LOG_DEBUG("cmp_http_response_free: CMP_FREE failed\n");
    }
  }
}

/**
 * @brief Execute a single HTTP request.
 *
 * @param client The HTTP client.
 * @param req The request to execute.
 * @param out_res Pointer to receive the allocated HTTP response.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_send(struct HttpClient *client, const struct HttpRequest *req,
                  struct HttpResponse **out_res) {
  int rc = 0; /* CMP_SUCCESS */
  struct HttpResponse *res = NULL;

  if (!client || !req || !out_res) {
    LOG_DEBUG("cmp_http_send: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  
  *out_res = NULL;

  rc = CMP_MALLOC(sizeof(struct HttpResponse), (void **)&res);
  if (rc != 0 || !res) {
    LOG_DEBUG("cmp_http_send: CMP_MALLOC failed\n");
    return CMP_ERROR_OOM;
  }
  c_abstract_http_response_init(res);

  rc = c_abstract_http_client_send(client, req, res);
  if (rc != 0) {
    LOG_DEBUG("Error in cmp_http_send: c_abstract_http_client_send failed\n");
    c_abstract_http_response_cleanup(res);
    CMP_FREE(res);
    return CMP_ERROR_IO;
  }

  *out_res = res;
  return 0;
}

/**
 * @brief Internal callback for progress tracking
 */
struct progress_ctx {
  int (*progress_cb)(float percentage, void *user_data);
  void *user_data;
};

static void on_progress_chunk(const unsigned char *chunk, size_t length,
                              size_t total_received, size_t total_expected,
                              void *user_data) {
  struct progress_ctx *ctx = (struct progress_ctx *)user_data;
  float percentage;

  /* Keep compiler happy if variables unused in non-verbose builds */
  (void)chunk;
  (void)length;

  if (ctx && ctx->progress_cb && total_expected > 0) {
    percentage = ((float)total_received / (float)total_expected) * 100.0f;
    ctx->progress_cb(percentage, ctx->user_data);
  }
}

/**
 * @brief Execute an HTTP request with progress callbacks.
 *
 * @param client The HTTP client.
 * @param req The request to execute.
 * @param progress_cb Callback to receive progress updates (0 to 100).
 * @param user_data Data passed to the progress callback.
 * @param out_res Pointer to receive the allocated HTTP response.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_send_with_progress(struct HttpClient *client,
                                struct HttpRequest *req,
                                int (*progress_cb)(float percentage,
                                                   void *user_data),
                                void *user_data,
                                struct HttpResponse **out_res) {
  int rc = 0; /* CMP_SUCCESS */
  struct HttpResponse *res = NULL;
  struct progress_ctx p_ctx;

  if (!client || !req || !out_res) {
    LOG_DEBUG("cmp_http_send_with_progress: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_res = NULL;
  p_ctx.progress_cb = progress_cb;
  p_ctx.user_data = user_data;

  /* Attach the streaming chunk callback to c-abstract-http */
  req->on_chunk = on_progress_chunk;
  req->chunk_user_data = &p_ctx;

  rc = CMP_MALLOC(sizeof(struct HttpResponse), (void **)&res);
  if (rc != 0 || !res) {
    LOG_DEBUG("cmp_http_send_with_progress: CMP_MALLOC failed\n");
    return CMP_ERROR_OOM;
  }
  c_abstract_http_response_init(res);

  rc = c_abstract_http_client_send(client, req, res);
  if (rc != 0) {
    LOG_DEBUG("Error in cmp_http_send_with_progress: "
              "c_abstract_http_client_send failed\n");
    c_abstract_http_response_cleanup(res);
    CMP_FREE(res);
    return CMP_ERROR_IO;
  }

  /* Trigger final 100% callback */
  if (progress_cb) {
    progress_cb(100.0f, user_data);
  }

  *out_res = res;
  return 0;
}

/**
 * @brief Initialize a WebSocket connection request.
 *
 * @param req The HTTP request.
 * @param config Optional WS config.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_ws_init(struct HttpRequest *req,
                     const struct c_abstract_http_ws_config *config) {
  int rc;
  if (!req) {
    LOG_DEBUG("cmp_http_ws_init: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  rc = c_abstract_http_ws_request_init(req, config);
  if (rc != 0) {
    LOG_DEBUG("cmp_http_ws_init: c_abstract_http_ws_request_init failed\n");
    return CMP_ERROR_GENERAL;
  }
  return 0;
}

/**
 * @brief Send a WebSocket frame.
 *
 * @param req The HTTP request connection handle.
 * @param opcode The WebSocket opcode.
 * @param payload The payload to send.
 * @param len The length of the payload.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_ws_send(struct HttpRequest *req,
                     enum c_abstract_http_ws_opcode opcode,
                     const unsigned char *payload, size_t len) {
  int rc;
  if (!req) {
    LOG_DEBUG("cmp_http_ws_send: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  rc = c_abstract_http_ws_send_frame(req, opcode, payload, len);
  if (rc != 0) {
    LOG_DEBUG("cmp_http_ws_send: c_abstract_http_ws_send_frame failed\n");
    return CMP_ERROR_IO;
  }
  return 0;
}

/**
 * @brief Close a WebSocket connection.
 *
 * @param req The HTTP request connection handle.
 * @param status_code The closure status code.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_ws_close(struct HttpRequest *req, int status_code) {
  int rc;
  if (!req) {
    LOG_DEBUG("cmp_http_ws_close: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  rc = c_abstract_http_ws_send_close(req, status_code);
  if (rc != 0) {
    LOG_DEBUG("cmp_http_ws_close: c_abstract_http_ws_send_close failed\n");
    return CMP_ERROR_IO;
  }
  return 0;
}

struct cmp_ws_task_ctx {
  struct HttpClient *client;
  struct HttpRequest *req;
  c_abstract_http_ws_on_message on_msg;
  c_abstract_http_ws_on_error on_err;
  c_abstract_http_ws_on_close on_close;
  void *user_data;
  volatile int *exit_flag;
};

static void cmp_http_ws_task(void *arg) {
  struct cmp_ws_task_ctx *ctx = (struct cmp_ws_task_ctx *)arg;
  int rc;
  if (!ctx) {
    return;
  }
  rc = c_abstract_http_ws_run_loop(ctx->client, ctx->req, ctx->on_msg,
                                   ctx->on_err, ctx->on_close, ctx->user_data,
                                   ctx->exit_flag);
  if (rc != 0) {
    LOG_DEBUG("cmp_http_ws_task: c_abstract_http_ws_run_loop returned error\n");
  }
  rc = CMP_FREE(ctx);
  if (rc != 0) {
    LOG_DEBUG("cmp_http_ws_task: CMP_FREE failed\n");
  }
}

/**
 * @brief Read WebSocket events synchronously or queue to modality.
 *
 * @param mod The modality to execute on (if CMP_MODALITY_ASYNC_SINGLE, it
 * registers it).
 * @param client The HTTP client.
 * @param req The HTTP request.
 * @param on_msg Message callback.
 * @param on_err Error callback.
 * @param on_close Close callback.
 * @param user_data Data for callbacks.
 * @param exit_flag Optional flag to exit the read loop.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_ws_run(cmp_modality_t *mod, struct HttpClient *client,
                    struct HttpRequest *req,
                    c_abstract_http_ws_on_message on_msg,
                    c_abstract_http_ws_on_error on_err,
                    c_abstract_http_ws_on_close on_close, void *user_data,
                    volatile int *exit_flag) {
  int rc = 0; /* CMP_SUCCESS */

  if (!client || !req) {
    LOG_DEBUG("cmp_http_ws_run: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (!mod || mod->type == 3 /* CMP_MODALITY_SYNC_SINGLE */) {
    /* Blocking execution on current thread */
    rc = c_abstract_http_ws_run_loop(client, req, on_msg, on_err, on_close,
                                     user_data, exit_flag);
    if (rc != 0) {
      LOG_DEBUG("Error in cmp_http_ws_run: c_abstract_http_ws_run_loop "
                "failed\n");
      return CMP_ERROR_IO;
    }
  } else {
    /* Offload to modality engine */
    struct cmp_ws_task_ctx *ctx = NULL;
    rc = CMP_MALLOC(sizeof(struct cmp_ws_task_ctx), (void **)&ctx);
    if (rc != 0 || !ctx) {
      LOG_DEBUG("cmp_http_ws_run: CMP_MALLOC failed\n");
      return CMP_ERROR_OOM;
    }
    ctx->client = client;
    ctx->req = req;
    ctx->on_msg = on_msg;
    ctx->on_err = on_err;
    ctx->on_close = on_close;
    ctx->user_data = user_data;
    ctx->exit_flag = exit_flag;

    rc = cmp_modality_queue_task(mod, cmp_http_ws_task, ctx);
    if (rc != 0) {
      CMP_FREE(ctx);
      LOG_DEBUG("Error in cmp_http_ws_run: cmp_modality_queue_task failed\n");
      return rc;
    }
  }

  return 0;
}

/**
 * @brief Initialize an SSE stream request.
 *
 * @param req The HTTP request.
 * @param config Optional SSE config.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_sse_init(struct HttpRequest *req,
                      const struct c_abstract_http_sse_config *config) {
  int rc;
  if (!req) {
    LOG_DEBUG("cmp_http_sse_init: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  rc = c_abstract_http_sse_request_init(req, config);
  if (rc != 0) {
    LOG_DEBUG("cmp_http_sse_init: c_abstract_http_sse_request_init failed\n");
    return CMP_ERROR_GENERAL;
  }
  return 0;
}

struct cmp_sse_task_ctx {
  struct HttpClient *client;
  struct HttpRequest *req;
  c_abstract_http_sse_on_event on_evt;
  c_abstract_http_sse_on_error on_err;
  c_abstract_http_sse_on_close on_close;
  void *user_data;
  volatile int *exit_flag;
};

static void cmp_http_sse_task(void *arg) {
  struct cmp_sse_task_ctx *ctx = (struct cmp_sse_task_ctx *)arg;
  int rc;
  if (!ctx) {
    return;
  }
  rc = c_abstract_http_sse_run_loop(ctx->client, ctx->req, ctx->on_evt,
                                    ctx->on_err, ctx->on_close, ctx->user_data,
                                    ctx->exit_flag);
  if (rc != 0) {
    LOG_DEBUG(
        "cmp_http_sse_task: c_abstract_http_sse_run_loop returned error\n");
  }
  rc = CMP_FREE(ctx);
  if (rc != 0) {
    LOG_DEBUG("cmp_http_sse_task: CMP_FREE failed\n");
  }
}

/**
 * @brief Read SSE events synchronously or queue to modality.
 *
 * @param mod The modality to execute on.
 * @param client The HTTP client.
 * @param req The HTTP request.
 * @param on_evt Event callback.
 * @param on_err Error callback.
 * @param on_close Close callback.
 * @param user_data Data for callbacks.
 * @param exit_flag Optional flag to exit the read loop.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_sse_run(cmp_modality_t *mod, struct HttpClient *client,
                     struct HttpRequest *req,
                     c_abstract_http_sse_on_event on_evt,
                     c_abstract_http_sse_on_error on_err,
                     c_abstract_http_sse_on_close on_close, void *user_data,
                     volatile int *exit_flag) {
  int rc = 0; /* CMP_SUCCESS */

  if (!client || !req) {
    LOG_DEBUG("cmp_http_sse_run: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (!mod || mod->type == 3 /* CMP_MODALITY_SYNC_SINGLE */) {
    /* Blocking execution */
    rc = c_abstract_http_sse_run_loop(client, req, on_evt, on_err, on_close,
                                      user_data, exit_flag);
    if (rc != 0) {
      LOG_DEBUG("Error in cmp_http_sse_run: c_abstract_http_sse_run_loop "
                "failed\n");
      return CMP_ERROR_IO;
    }
  } else {
    /* Async offload */
    struct cmp_sse_task_ctx *ctx = NULL;
    rc = CMP_MALLOC(sizeof(struct cmp_sse_task_ctx), (void **)&ctx);
    if (rc != 0 || !ctx) {
      LOG_DEBUG("cmp_http_sse_run: CMP_MALLOC failed\n");
      return CMP_ERROR_OOM;
    }
    ctx->client = client;
    ctx->req = req;
    ctx->on_evt = on_evt;
    ctx->on_err = on_err;
    ctx->on_close = on_close;
    ctx->user_data = user_data;
    ctx->exit_flag = exit_flag;

    rc = cmp_modality_queue_task(mod, cmp_http_sse_task, ctx);
    if (rc != 0) {
      CMP_FREE(ctx);
      LOG_DEBUG("Error in cmp_http_sse_run: cmp_modality_queue_task failed\n");
      return rc;
    }
  }

  return 0;
}
