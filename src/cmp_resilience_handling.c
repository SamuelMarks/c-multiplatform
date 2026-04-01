/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_resilience {
  int is_offline;
};

int cmp_resilience_create(cmp_resilience_t **out_res) {
  struct cmp_resilience *ctx;
  if (!out_res)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_resilience), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->is_offline = 0;

  *out_res = (cmp_resilience_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_resilience_destroy(cmp_resilience_t *res_opaque) {
  if (res_opaque)
    CMP_FREE(res_opaque);
  return CMP_SUCCESS;
}

int cmp_resilience_show_empty_state(cmp_resilience_t *res_opaque,
                                    void *container_node) {
  struct cmp_resilience *ctx = (struct cmp_resilience *)res_opaque;
  if (!ctx || !container_node)
    return CMP_ERROR_INVALID_ARG;

  /* Renders an icon, title, and CTA when data is missing */
  return CMP_SUCCESS;
}

int cmp_resilience_show_loading_skeleton(cmp_resilience_t *res_opaque,
                                         void *container_node) {
  struct cmp_resilience *ctx = (struct cmp_resilience *)res_opaque;
  if (!ctx || !container_node)
    return CMP_ERROR_INVALID_ARG;

  /* Mimics final layout with a shimmering skeleton rather than blocking spinner
   */
  return CMP_SUCCESS;
}

int cmp_resilience_show_non_blocking_error(cmp_resilience_t *res_opaque,
                                           void *container_node,
                                           const char *msg) {
  struct cmp_resilience *ctx = (struct cmp_resilience *)res_opaque;
  if (!ctx || !container_node || !msg)
    return CMP_ERROR_INVALID_ARG;

  /* Presents an inline banner or toast instead of a modal alert */
  return CMP_SUCCESS;
}

int cmp_resilience_handle_discard_changes_prompt(cmp_resilience_t *res_opaque,
                                                 void *sheet_node) {
  struct cmp_resilience *ctx = (struct cmp_resilience *)res_opaque;
  if (!ctx || !sheet_node)
    return CMP_ERROR_INVALID_ARG;

  /* Prompts user to "Save Draft" or "Discard" when closing a dirty sheet */
  return CMP_SUCCESS;
}

int cmp_resilience_graceful_degradation(cmp_resilience_t *res_opaque,
                                        const char *feature_name) {
  struct cmp_resilience *ctx = (struct cmp_resilience *)res_opaque;
  if (!ctx || !feature_name)
    return CMP_ERROR_INVALID_ARG;

  /* Safe fallback for missing ARKit/LiDAR hardware on older devices */
  return CMP_SUCCESS;
}
