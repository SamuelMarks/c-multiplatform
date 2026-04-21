/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_os_communications {
  int is_initialized;
  int is_focus_mode_suppressed;
};

int cmp_os_communications_create(cmp_os_communications_t **out_ctx) {
  int rc = CMP_SUCCESS;
  struct cmp_os_communications *ctx = NULL;

  if (!out_ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_os_communications_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_os_communications), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_os_communications_create: Out of memory\n");
    return rc;
  }

  ctx->is_initialized = 1;
  ctx->is_focus_mode_suppressed = 0; /* Default open */

  *out_ctx = (cmp_os_communications_t *)ctx;
  return rc;
}

int cmp_os_communications_destroy(cmp_os_communications_t *ctx_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_os_communications *ctx =
      (struct cmp_os_communications *)ctx_opaque;

  if (!ctx) {
    return rc;
  }

  CMP_FREE(ctx);
  return rc;
}

int cmp_os_communications_register_intent(cmp_os_communications_t *ctx_opaque,
                                          const char *intent_id,
                                          const char *title) {
  int rc = CMP_SUCCESS;
  struct cmp_os_communications *ctx =
      (struct cmp_os_communications *)ctx_opaque;

  if (!ctx || !intent_id || !title) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_os_communications_register_intent: Invalid argument\n");
    return rc;
  }
  /* Instructs OS to parse and map intent structures */
  return rc;
}

int cmp_os_communications_show_share_sheet(cmp_os_communications_t *ctx_opaque,
                                           cmp_window_t *window,
                                           const char *url_to_share,
                                           const char *text_to_share) {
  int rc = CMP_SUCCESS;
  struct cmp_os_communications *ctx =
      (struct cmp_os_communications *)ctx_opaque;

  if (!ctx || !window || (!url_to_share && !text_to_share)) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_os_communications_show_share_sheet: Invalid argument\n");
    return rc;
  }
  /* Triggers UIActivityViewController / NSSharingServicePicker */
  return rc;
}

int cmp_os_communications_broadcast_handoff(cmp_os_communications_t *ctx_opaque,
                                            const char *activity_type,
                                            const char *payload) {
  int rc = CMP_SUCCESS;
  struct cmp_os_communications *ctx =
      (struct cmp_os_communications *)ctx_opaque;

  if (!ctx || !activity_type || !payload) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_os_communications_broadcast_handoff: Invalid argument\n");
    return rc;
  }
  /* Sets current NSUserActivity */
  return rc;
}

int cmp_os_communications_index_for_spotlight(
    cmp_os_communications_t *ctx_opaque, const char *item_id, const char *title,
    const char *description) {
  int rc = CMP_SUCCESS;
  struct cmp_os_communications *ctx =
      (struct cmp_os_communications *)ctx_opaque;

  if (!ctx || !item_id || !title || !description) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_os_communications_index_for_spotlight: Invalid "
              "argument\n");
    return rc;
  }
  /* CSSearchableItemAttributeSet */
  return rc;
}

int cmp_os_communications_evaluate_focus_mode(
    cmp_os_communications_t *ctx_opaque, int *out_is_suppressed) {
  int rc = CMP_SUCCESS;
  struct cmp_os_communications *ctx =
      (struct cmp_os_communications *)ctx_opaque;

  if (!ctx || !out_is_suppressed) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_os_communications_evaluate_focus_mode: Invalid "
              "argument\n");
    return rc;
  }
  *out_is_suppressed = ctx->is_focus_mode_suppressed;
  return rc;
}

int cmp_os_communications_start_shareplay(cmp_os_communications_t *ctx_opaque,
                                          const char *activity_id) {
  int rc = CMP_SUCCESS;
  struct cmp_os_communications *ctx =
      (struct cmp_os_communications *)ctx_opaque;

  if (!ctx || !activity_id) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_os_communications_start_shareplay: Invalid argument\n");
    return rc;
  }
  /* GroupActivities framework binding */
  return rc;
}
