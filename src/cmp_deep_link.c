/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_state_restoration_ctx {
  char *scene_id;
};

int cmp_state_restoration_ctx_create(cmp_state_restoration_ctx_t **out_ctx) {
  int rc = CMP_SUCCESS;
  struct cmp_state_restoration_ctx *ctx = NULL;

  if (!out_ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_state_restoration_ctx_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_state_restoration_ctx), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_state_restoration_ctx_create: Out of memory\n");
    return rc;
  }

  ctx->scene_id = NULL;
  *out_ctx = (cmp_state_restoration_ctx_t *)ctx;
  return rc;
}

int cmp_state_restoration_ctx_destroy(cmp_state_restoration_ctx_t *ctx_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_state_restoration_ctx *ctx =
      (struct cmp_state_restoration_ctx *)ctx_opaque;

  if (!ctx) {
    return rc;
  }

  if (ctx->scene_id) {
    CMP_FREE(ctx->scene_id);
  }
  CMP_FREE(ctx);
  return rc;
}

int cmp_deep_link_handle_universal_link(cmp_state_restoration_ctx_t *ctx,
                                        const char *url_string,
                                        cmp_router_t *router) {
  int rc = CMP_SUCCESS;

  if (!ctx || !url_string || !router) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_deep_link_handle_universal_link: Invalid argument\n");
    return rc;
  }

  /* In a real implementation, we'd parse the URL fragment/query and directly
   * push/replace routes on the router instance */
  /* Here we simply trigger a push to the destination URI mapped by the
   * universal link */
  rc = cmp_router_push(router, url_string);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_deep_link_handle_universal_link: cmp_router_push "
              "failed\n");
  }
  return rc;
}

int cmp_state_restoration_encode(cmp_state_restoration_ctx_t *ctx,
                                 cmp_router_t *router, void **out_buffer,
                                 size_t *out_size) {
  int rc = CMP_SUCCESS;
  cmp_string_t uri;
  void *blob = NULL;
  size_t len;

  if (!ctx || !router || !out_buffer || !out_size) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_state_restoration_encode: Invalid argument\n");
    return rc;
  }

  rc = cmp_router_get_current(router, &uri);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_state_restoration_encode: cmp_router_get_current "
              "failed\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* Encode state: We serialize the current URI directly as the baseline
     state. A full robust JSON/MsgPack object capturing scroll offsets
     will be added in Phase 25. */
  if (uri.data && uri.length > 0) {
    len = uri.length;
    rc = CMP_MALLOC(len + 1, &blob);
    if (rc != CMP_SUCCESS) {
      cmp_string_destroy(&uri);
      LOG_DEBUG("Error in cmp_state_restoration_encode: Out of memory\n");
      return CMP_ERROR_OOM;
    }
#if defined(_MSC_VER)
    strcpy_s(blob, len + 1, uri.data);
#else
    strcpy(blob, uri.data);
#endif
    *out_buffer = blob;
    *out_size = len + 1;
  } else {
    *out_buffer = NULL;
    *out_size = 0;
  }

  cmp_string_destroy(&uri);
  return rc;
}

int cmp_state_restoration_decode(cmp_state_restoration_ctx_t *ctx,
                                 cmp_router_t *router, const void *buffer,
                                 size_t size) {
  int rc = CMP_SUCCESS;

  if (!ctx || !router || !buffer || size == 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_state_restoration_decode: Invalid argument\n");
    return rc;
  }

  /* Decoding the simple URI blob we made in encode */
  rc = cmp_router_replace(router, (const char *)buffer);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG(
        "Error in cmp_state_restoration_decode: cmp_router_replace failed\n");
  }
  return rc;
}

int cmp_state_restoration_set_scene_id(cmp_state_restoration_ctx_t *ctx_opaque,
                                       const char *scene_identifier) {
  int rc = CMP_SUCCESS;
  struct cmp_state_restoration_ctx *ctx =
      (struct cmp_state_restoration_ctx *)ctx_opaque;
  size_t len;

  if (!ctx || !scene_identifier) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_state_restoration_set_scene_id: Invalid argument\n");
    return rc;
  }

  if (ctx->scene_id) {
    CMP_FREE(ctx->scene_id);
    ctx->scene_id = NULL;
  }

  len = strlen(scene_identifier);
  rc = CMP_MALLOC(len + 1, (void **)&ctx->scene_id);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_state_restoration_set_scene_id: Out of memory\n");
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strcpy_s(ctx->scene_id, len + 1, scene_identifier);
#else
  strcpy(ctx->scene_id, scene_identifier);
#endif

  return rc;
}
