/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <stdio.h>
/* clang-format on */

extern int transport_factory_init_client(struct HttpClient *client);

/**
 * @brief Initialize global networking subsystem.
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_init(void) {
  int rc;
  rc = 0; /* No longer needed by c-abstract-http */
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief Shut down global networking subsystem.
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_http_shutdown(void) {
  int rc;
  rc = 0; /* No longer needed by c-abstract-http */
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
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
  int rc;
  rc = 0; /* CMP_SUCCESS */
  struct HttpClient *client = NULL;
  (void)mod;

  if (out_client == NULL) {
    LOG_DEBUG("cmp_http_client_create: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_client = NULL;

  rc = CMP_MALLOC(sizeof(struct HttpClient), (void **)&client);
  if (rc != 0 || client == NULL) {
    LOG_DEBUG("cmp_http_client_create: CMP_MALLOC failed\n");
    return CMP_ERROR_OOM;
  }

  http_client_init(client);

  rc = transport_factory_init_client(client);
  if (rc != 0) {
    CMP_FREE(client);
    LOG_DEBUG("Error in cmp_http_client_create: transport_factory_init_client "
              "failed\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
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
  int rc;
  rc = 0;
  if (client == NULL) {
    LOG_DEBUG("cmp_http_client_destroy: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  http_client_free(client);
  rc = CMP_FREE(client);
  if (rc != 0) {
    LOG_DEBUG("cmp_http_client_destroy: CMP_FREE failed\n");
  }
  if (rc != 0) {
    return rc;
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
  int rc;
  rc = 0;
  if (req == NULL) {
    LOG_DEBUG("cmp_http_request_init: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  http_request_init(req);
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief Free resources associated with an HTTP request.
 *
 * @param req The request to free.
 */
int cmp_http_request_free(struct HttpRequest *req) {
  int rc;
  rc = 0;
  if (req != NULL) {
    http_request_free(req);
  }
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief Free resources associated with an HTTP response.
 *
 * @param res The response to free.
 */
int cmp_http_response_free(struct HttpResponse *res) {
  int rc;
  if (res != NULL) {
    http_response_free(res);
    rc = CMP_FREE(res);
    if (rc != 0) {
      LOG_DEBUG("cmp_http_response_free: CMP_FREE failed\n");
      if (rc != 0) {
        return rc;
      }
      return rc;
    }
  }
  return 0;
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
  int rc;
  rc = 0; /* CMP_SUCCESS */
  struct HttpFuture *future = NULL;
  struct HttpRequest *req_arr[1];
  struct HttpFuture *futures_arr[1];

  if (client == NULL || req == NULL || out_res == NULL) {
    LOG_DEBUG("cmp_http_send: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_res = NULL;

  rc = CMP_MALLOC(sizeof(struct HttpFuture), (void **)&future);
  if (rc != 0 || future == NULL) {
    LOG_DEBUG("cmp_http_send: CMP_MALLOC failed\n");
    return CMP_ERROR_OOM;
  }
  http_future_init(future);

  req_arr[0] = (struct HttpRequest *)req;
  futures_arr[0] = future;

  rc = http_client_send_multi(client, req_arr, 1, futures_arr, NULL, NULL, 0);
  if (rc != 0) {
    LOG_DEBUG("Error in cmp_http_send: http_client_send_multi failed\n");
    http_future_free(future);
    CMP_FREE(future);
    return CMP_ERROR_IO;
  }

  if (future->error_code != 0 || !future->response) {
    http_future_free(future);
    CMP_FREE(future);
    return CMP_ERROR_IO;
  }

  *out_res = future->response;
  future->response = NULL;
  http_future_free(future);
  CMP_FREE(future);

  return 0;
}

/**
 * @brief Internal callback for progress tracking
 */
struct progress_ctx {
  int (*progress_cb)(float percentage, void *user_data);
  void *user_data;
};

CMP_EXEMPT(static void on_progress_chunk(const unsigned char *chunk,
                                         size_t length, size_t total_received,
                                         size_t total_expected,
                                         void *user_data)) {
  struct progress_ctx *ctx = (struct progress_ctx *)user_data;
  float percentage;

  /* Keep compiler happy if variables unused in non-verbose builds */
  (void)chunk;
  (void)length;

  if (ctx != NULL && ctx->progress_cb != NULL && total_expected > 0) {
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
int cmp_http_send_with_progress(
    struct HttpClient *client, struct HttpRequest *req,
    int (*progress_cb)(float percentage, void *user_data), void *user_data,
    struct HttpResponse **out_res) {
  int rc;
  rc = 0; /* CMP_SUCCESS */
  struct HttpFuture *future = NULL;
  struct HttpRequest *req_arr[1];
  struct HttpFuture *futures_arr[1];
  struct progress_ctx p_ctx;

  if (client == NULL || req == NULL || out_res == NULL) {
    LOG_DEBUG("cmp_http_send_with_progress: invalid argument\n");
    return 2; /* CMP_ERROR_INVALID_ARG */
  }

  *out_res = NULL;
  p_ctx.progress_cb = progress_cb;
  p_ctx.user_data = user_data;

  req->on_chunk = (http_on_chunk_fn)on_progress_chunk;
  req->on_chunk_user_data = &p_ctx;

  rc = CMP_MALLOC(sizeof(struct HttpFuture), (void **)&future);
  if (rc != 0 || future == NULL) {
    LOG_DEBUG("cmp_http_send_with_progress: CMP_MALLOC failed\n");
    return 1; /* CMP_ERROR_OOM */
  }
  http_future_init(future);

  req_arr[0] = req;
  futures_arr[0] = future;

  rc = http_client_send_multi(client, req_arr, 1, futures_arr, NULL, NULL, 0);
  if (rc != 0) {
    LOG_DEBUG("Error in cmp_http_send_with_progress: "
              "http_client_send_multi failed\n");
    http_future_free(future);
    CMP_FREE(future);
    return 5; /* CMP_ERROR_IO */
  }

  if (future->error_code != 0 || !future->response) {
    http_future_free(future);
    CMP_FREE(future);
    return 5; /* CMP_ERROR_IO */
  }

  *out_res = future->response;
  future->response = NULL;
  http_future_free(future);
  CMP_FREE(future);

  return 0;
}

/**
 * @brief Initialize a WebSocket connection request.
 */
int cmp_http_ws_init(struct HttpRequest *req,
                     const struct c_abstract_http_ws_config *config) {
  int rc;
  rc = 0;
  (void)req;
  (void)config;
  LOG_DEBUG("cmp_http_ws_init: Websockets not supported by current "
            "c-abstract-http\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return CMP_ERROR_NOT_FOUND;
}

/**
 * @brief Send a WebSocket frame.
 */
int cmp_http_ws_send(struct HttpRequest *req,
                     enum c_abstract_http_ws_opcode opcode,
                     const unsigned char *payload, size_t len) {
  int rc;
  rc = 0;
  (void)req;
  (void)opcode;
  (void)payload;
  (void)len;
  LOG_DEBUG("cmp_http_ws_send: Websockets not supported\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return CMP_ERROR_NOT_FOUND;
}

/**
 * @brief Close a WebSocket connection.
 */
int cmp_http_ws_close(struct HttpRequest *req, int status_code) {
  int rc;
  rc = 0;
  (void)req;
  (void)status_code;
  LOG_DEBUG("cmp_http_ws_close: Websockets not supported\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return CMP_ERROR_NOT_FOUND;
}

/**
 * @brief Read WebSocket events synchronously or queue to modality.
 */
int cmp_http_ws_run(cmp_modality_t *mod, struct HttpClient *client,
                    struct HttpRequest *req,
                    c_abstract_http_ws_on_message on_msg,
                    c_abstract_http_ws_on_error on_err,
                    c_abstract_http_ws_on_close on_close, void *user_data,
                    volatile int *exit_flag) {
  int rc;
  rc = 0;
  (void)mod;
  (void)client;
  (void)req;
  (void)on_msg;
  (void)on_err;
  (void)on_close;
  (void)user_data;
  (void)exit_flag;
  LOG_DEBUG("cmp_http_ws_run: Websockets not supported\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return CMP_ERROR_NOT_FOUND;
}

/**
 * @brief Initialize an SSE stream request.
 */
int cmp_http_sse_init(struct HttpRequest *req,
                      const struct c_abstract_http_sse_config *config) {
  int rc;
  rc = 0;
  (void)req;
  (void)config;
  LOG_DEBUG("cmp_http_sse_init: SSE not supported\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return CMP_ERROR_NOT_FOUND;
}

/**
 * @brief Read SSE events synchronously or queue to modality.
 */
int cmp_http_sse_run(cmp_modality_t *mod, struct HttpClient *client,
                     struct HttpRequest *req,
                     c_abstract_http_sse_on_event on_evt,
                     c_abstract_http_sse_on_error on_err,
                     c_abstract_http_sse_on_close on_close, void *user_data,
                     volatile int *exit_flag) {
  int rc;
  rc = 0;
  (void)mod;
  (void)client;
  (void)req;
  (void)on_evt;
  (void)on_err;
  (void)on_close;
  (void)user_data;
  (void)exit_flag;
  LOG_DEBUG("cmp_http_sse_run: SSE not supported\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return CMP_ERROR_NOT_FOUND;
}
