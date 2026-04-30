/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_state_restoration_ctx {
  char *scene_id;
};

/**
 * @brief cmp_state_restoration_ctx_create
 *
 * @param out_ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_state_restoration_ctx_create(cmp_state_restoration_ctx_t **out_ctx) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_state_restoration_ctx *ctx = NULL;

  if (out_ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_state_restoration_ctx_create: Invalid argument "
                  "(out_ctx=NULL): %s\n",
                  err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_state_restoration_ctx), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_state_restoration_ctx_create: Out of memory: %s\n",
                  err_str);

    return rc;
  }

  ctx->scene_id = NULL;
  *out_ctx = (cmp_state_restoration_ctx_t *)ctx;
  cmp_log_debug("cmp_state_restoration_ctx_create: Successfully created "
                "restoration context\n");
  return rc;
}

/**
 * @brief cmp_state_restoration_ctx_destroy
 *
 * @param ctx_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_state_restoration_ctx_destroy(cmp_state_restoration_ctx_t *ctx_opaque) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_state_restoration_ctx *ctx =
      (struct cmp_state_restoration_ctx *)ctx_opaque;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_state_restoration_ctx_destroy: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  if (ctx->scene_id != NULL) {
    rc = CMP_FREE(ctx->scene_id);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug(
          "cmp_state_restoration_ctx_destroy: CMP_FREE scene_id failed\n");
    }
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_state_restoration_ctx_destroy: CMP_FREE ctx failed\n");
  }

  cmp_log_debug("cmp_state_restoration_ctx_destroy: Successfully destroyed "
                "restoration context\n");
  return rc;
}

/**
 * @brief cmp_deep_link_handle_universal_link
 *
 * @param ctx Parameter description.
 * @param url_string Parameter description.
 * @param router Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_deep_link_handle_universal_link(cmp_state_restoration_ctx_t *ctx,
                                        const char *url_string,
                                        cmp_router_t *router) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (ctx == NULL || url_string == NULL || router == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_deep_link_handle_universal_link: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  /* In a real implementation, we'd parse the URL fragment/query and directly
   * push/replace routes on the router instance */
  /* Here we simply trigger a push to the destination URI mapped by the
   * universal link */
  rc = cmp_router_push(router, url_string);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_deep_link_handle_universal_link: cmp_router_push "
                  "failed: %s\n",
                  err_str);
  }

  cmp_log_debug("cmp_deep_link_handle_universal_link: Handled deep link\n");

  return rc;
}

/**
 * @brief cmp_state_restoration_encode
 *
 * @param ctx Parameter description.
 * @param router Parameter description.
 * @param out_buffer Parameter description.
 * @param out_size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_state_restoration_encode(cmp_state_restoration_ctx_t *ctx,
                                 cmp_router_t *router, void **out_buffer,
                                 size_t *out_size) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_string_t uri;
  void *blob = NULL;
  size_t len;

  if (ctx == NULL || router == NULL || out_buffer == NULL || out_size == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_state_restoration_encode: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  rc = cmp_router_get_current(router, &uri);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_state_restoration_encode: cmp_router_get_current "
                  "failed: %s\n",
                  err_str);
    return CMP_ERROR_INVALID_ARG;
  }

  /* Encode state: We serialize the current URI directly as the baseline
     state. A full robust JSON/MsgPack object capturing scroll offsets
     will be added in Phase 25. */
  if (uri.data != NULL && uri.length > 0) {
    len = uri.length;
    rc = CMP_MALLOC(len + 1, &blob);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_string_destroy(&uri);
      cmp_log_debug("cmp_state_restoration_encode: Out of memory: %s\n",
                    err_str);
      return CMP_ERROR_OOM;
    }
#if defined(_MSC_VER)
    if (strcpy_s(blob, len + 1, uri.data) != 0) {
      cmp_log_debug("cmp_state_restoration_encode: strcpy_s failed\n");
      CMP_FREE(blob);
      cmp_string_destroy(&uri);
      return CMP_ERROR_GENERAL;
    }
#else
    strcpy(blob, uri.data);
#endif
    *out_buffer = blob;
    *out_size = len + 1;
    cmp_log_debug("cmp_state_restoration_encode: Encoded state size=%u\n",
                  (unsigned int)*out_size);
  } else {
    *out_buffer = NULL;
    *out_size = 0;
    cmp_log_debug("cmp_state_restoration_encode: Encoded empty state\n");
  }

  rc = cmp_string_destroy(&uri);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_state_restoration_encode: cmp_string_destroy failed\n");
  }
  return rc;
}

/**
 * @brief cmp_state_restoration_decode
 *
 * @param ctx Parameter description.
 * @param router Parameter description.
 * @param buffer Parameter description.
 * @param size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_state_restoration_decode(cmp_state_restoration_ctx_t *ctx,
                                 cmp_router_t *router, const void *buffer,
                                 size_t size) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (ctx == NULL || router == NULL || buffer == NULL || size == 0) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_state_restoration_decode: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  /* Decoding the simple URI blob we made in encode */
  rc = cmp_router_replace(router, (const char *)buffer);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_state_restoration_decode: cmp_router_replace failed: %s\n",
        err_str);
  }

  cmp_log_debug("cmp_state_restoration_decode: Decoded state completely\n");

  return rc;
}

/**
 * @brief cmp_state_restoration_set_scene_id
 *
 * @param ctx_opaque Parameter description.
 * @param scene_identifier Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_state_restoration_set_scene_id(cmp_state_restoration_ctx_t *ctx_opaque,
                                       const char *scene_identifier) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_state_restoration_ctx *ctx =
      (struct cmp_state_restoration_ctx *)ctx_opaque;
  size_t len;

  if (ctx == NULL || scene_identifier == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_state_restoration_set_scene_id: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  if (ctx->scene_id != NULL) {
    rc = CMP_FREE(ctx->scene_id);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_state_restoration_set_scene_id: CMP_FREE failed\n");
    }
    ctx->scene_id = NULL;
  }

  len = strlen(scene_identifier);
  rc = CMP_MALLOC(len + 1, (void **)&ctx->scene_id);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_state_restoration_set_scene_id: Out of memory: %s\n",
                  err_str);
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  if (strcpy_s(ctx->scene_id, len + 1, scene_identifier) != 0) {
    CMP_FREE(ctx->scene_id);
    ctx->scene_id = NULL;
    cmp_log_debug("cmp_state_restoration_set_scene_id: strcpy_s failed\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strcpy(ctx->scene_id, scene_identifier);
#endif

  cmp_log_debug("cmp_state_restoration_set_scene_id: Set scene identifier\n");
  return rc;
}
