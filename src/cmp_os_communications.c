/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_os_communications {
  int is_initialized;
  int is_focus_mode_suppressed; /* Stubbed internal state */
};

int cmp_os_communications_create(cmp_os_communications_t **out_ctx) {
  struct cmp_os_communications *ctx;
  if (!out_ctx)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_os_communications), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->is_initialized = 1;
  ctx->is_focus_mode_suppressed = 0; /* Default open */

  *out_ctx = (cmp_os_communications_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_os_communications_destroy(cmp_os_communications_t *ctx_opaque) {
  struct cmp_os_communications *ctx =
      (struct cmp_os_communications *)ctx_opaque;
  if (!ctx)
    return CMP_SUCCESS;

  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_os_communications_register_intent(cmp_os_communications_t *ctx_opaque,
                                          const char *intent_id,
                                          const char *title) {
  struct cmp_os_communications *ctx =
      (struct cmp_os_communications *)ctx_opaque;
  if (!ctx || !intent_id || !title)
    return CMP_ERROR_INVALID_ARG;
  /* Instructs OS to parse and map intent structures */
  return CMP_SUCCESS;
}

int cmp_os_communications_show_share_sheet(cmp_os_communications_t *ctx_opaque,
                                           cmp_window_t *window,
                                           const char *url_to_share,
                                           const char *text_to_share) {
  struct cmp_os_communications *ctx =
      (struct cmp_os_communications *)ctx_opaque;
  if (!ctx || !window || (!url_to_share && !text_to_share))
    return CMP_ERROR_INVALID_ARG;
  /* Triggers UIActivityViewController / NSSharingServicePicker */
  return CMP_SUCCESS;
}

int cmp_os_communications_broadcast_handoff(cmp_os_communications_t *ctx_opaque,
                                            const char *activity_type,
                                            const char *payload) {
  struct cmp_os_communications *ctx =
      (struct cmp_os_communications *)ctx_opaque;
  if (!ctx || !activity_type || !payload)
    return CMP_ERROR_INVALID_ARG;
  /* Sets current NSUserActivity */
  return CMP_SUCCESS;
}

int cmp_os_communications_index_for_spotlight(
    cmp_os_communications_t *ctx_opaque, const char *item_id, const char *title,
    const char *description) {
  struct cmp_os_communications *ctx =
      (struct cmp_os_communications *)ctx_opaque;
  if (!ctx || !item_id || !title || !description)
    return CMP_ERROR_INVALID_ARG;
  /* CSSearchableItemAttributeSet */
  return CMP_SUCCESS;
}

int cmp_os_communications_evaluate_focus_mode(
    cmp_os_communications_t *ctx_opaque, int *out_is_suppressed) {
  struct cmp_os_communications *ctx =
      (struct cmp_os_communications *)ctx_opaque;
  if (!ctx || !out_is_suppressed)
    return CMP_ERROR_INVALID_ARG;
  *out_is_suppressed = ctx->is_focus_mode_suppressed;
  return CMP_SUCCESS;
}

int cmp_os_communications_start_shareplay(cmp_os_communications_t *ctx_opaque,
                                          const char *activity_id) {
  struct cmp_os_communications *ctx =
      (struct cmp_os_communications *)ctx_opaque;
  if (!ctx || !activity_id)
    return CMP_ERROR_INVALID_ARG;
  /* GroupActivities framework binding */
  return CMP_SUCCESS;
}
