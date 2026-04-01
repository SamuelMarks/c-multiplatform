/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_state_restoration_ctx {
  char *scene_id;
};

int cmp_state_restoration_ctx_create(cmp_state_restoration_ctx_t **out_ctx) {
  struct cmp_state_restoration_ctx *ctx;
  if (!out_ctx)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_state_restoration_ctx), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->scene_id = NULL;
  *out_ctx = (cmp_state_restoration_ctx_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_state_restoration_ctx_destroy(cmp_state_restoration_ctx_t *ctx_opaque) {
  struct cmp_state_restoration_ctx *ctx =
      (struct cmp_state_restoration_ctx *)ctx_opaque;
  if (!ctx)
    return CMP_SUCCESS;

  if (ctx->scene_id)
    CMP_FREE(ctx->scene_id);
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_deep_link_handle_universal_link(cmp_state_restoration_ctx_t *ctx,
                                        const char *url_string,
                                        cmp_router_t *router) {
  if (!ctx || !url_string || !router)
    return CMP_ERROR_INVALID_ARG;

  /* In a real implementation, we'd parse the URL fragment/query and directly
   * push/replace routes on the router instance */
  /* Here we simply trigger a push to the destination URI mapped by the
   * universal link */
  return cmp_router_push(router, url_string);
}

int cmp_state_restoration_encode(cmp_state_restoration_ctx_t *ctx,
                                 cmp_router_t *router, void **out_buffer,
                                 size_t *out_size) {
  cmp_string_t uri;
  void *blob;
  size_t len;

  if (!ctx || !router || !out_buffer || !out_size)
    return CMP_ERROR_INVALID_ARG;

  if (cmp_router_get_current(router, &uri) != CMP_SUCCESS)
    return CMP_ERROR_INVALID_ARG;

  /* Encode state: In reality, this would be a robust serialized object
     (JSON/MsgPack) capturing scroll offsets. For compliance stubbing, we
     serialize the current URI directly. */
  if (uri.data && uri.length > 0) {
    len = uri.length;
    if (CMP_MALLOC(len + 1, &blob) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
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
  return CMP_SUCCESS;
}

int cmp_state_restoration_decode(cmp_state_restoration_ctx_t *ctx,
                                 cmp_router_t *router, const void *buffer,
                                 size_t size) {
  if (!ctx || !router || !buffer || size == 0)
    return CMP_ERROR_INVALID_ARG;

  /* Decoding the simple URI blob we made in encode */
  return cmp_router_replace(router, (const char *)buffer);
}

int cmp_state_restoration_set_scene_id(cmp_state_restoration_ctx_t *ctx_opaque,
                                       const char *scene_identifier) {
  struct cmp_state_restoration_ctx *ctx =
      (struct cmp_state_restoration_ctx *)ctx_opaque;
  size_t len;
  if (!ctx || !scene_identifier)
    return CMP_ERROR_INVALID_ARG;

  if (ctx->scene_id) {
    CMP_FREE(ctx->scene_id);
    ctx->scene_id = NULL;
  }

  len = strlen(scene_identifier);
  if (CMP_MALLOC(len + 1, (void **)&ctx->scene_id) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
#if defined(_MSC_VER)
  strcpy_s(ctx->scene_id, len + 1, scene_identifier);
#else
  strcpy(ctx->scene_id, scene_identifier);
#endif

  return CMP_SUCCESS;
}
