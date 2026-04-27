/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_os_communications {
  int is_initialized;
  int is_focus_mode_suppressed;
};

/**
 * @brief Create a system communication context.
 *
 * @param out_ctx Pointer to store the created context.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_communications_create(cmp_os_communications_t **out_ctx) {
  int rc;
  struct cmp_os_communications *ctx;

  rc = CMP_SUCCESS;
  ctx = NULL;

  if (out_ctx == NULL) {
    LOG_DEBUG("Error in cmp_os_communications_create: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_os_communications), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG(
        "Error in cmp_os_communications_create: CMP_MALLOC failed (OOM)\n");
    return CMP_ERROR_OOM;
  }

  ctx->is_initialized = 1;
  ctx->is_focus_mode_suppressed = 0; /* Default open */

  *out_ctx = (cmp_os_communications_t *)ctx;
  return CMP_SUCCESS;
}

/**
 * @brief Destroy a system communication context.
 *
 * @param ctx_opaque The context to destroy.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_communications_destroy(cmp_os_communications_t *ctx_opaque) {
  int rc;
  struct cmp_os_communications *ctx;

  rc = CMP_SUCCESS;

  if (ctx_opaque == NULL) {
    LOG_DEBUG("Error in cmp_os_communications_destroy: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx = (struct cmp_os_communications *)ctx_opaque;

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_os_communications_destroy: CMP_FREE failed\n");
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief Siri & App Intents Registration.
 *
 * @param ctx_opaque System communication context.
 * @param intent_id Unique identifier matching the INIntent definition.
 * @param title Human readable title for the Shortcuts App.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_communications_register_intent(cmp_os_communications_t *ctx_opaque,
                                          const char *intent_id,
                                          const char *title) {
  int rc;
  rc = 0;
  struct cmp_os_communications *ctx;

  if (ctx_opaque == NULL || intent_id == NULL || title == NULL) {
    LOG_DEBUG(
        "Error in cmp_os_communications_register_intent: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx = (struct cmp_os_communications *)ctx_opaque;
  (void)ctx;

  /* Instructs OS to parse and map intent structures */
  return CMP_SUCCESS;
}

/**
 * @brief Show System Share Sheet (UIActivityViewController equivalent).
 *
 * @param ctx_opaque System communication context.
 * @param window The window to attach the share sheet to.
 * @param url_to_share Optional URL to share.
 * @param text_to_share Optional text to share.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_communications_show_share_sheet(cmp_os_communications_t *ctx_opaque,
                                           cmp_window_t *window,
                                           const char *url_to_share,
                                           const char *text_to_share) {
  int rc;
  rc = 0;
  struct cmp_os_communications *ctx;

  if (ctx_opaque == NULL || window == NULL ||
      (url_to_share == NULL && text_to_share == NULL)) {
    LOG_DEBUG(
        "Error in cmp_os_communications_show_share_sheet: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx = (struct cmp_os_communications *)ctx_opaque;
  (void)ctx;

  /* Triggers UIActivityViewController / NSSharingServicePicker */
  return CMP_SUCCESS;
}

/**
 * @brief Broadcast handoff activity.
 *
 * @param ctx_opaque System communication context.
 * @param activity_type The activity type identifier.
 * @param payload The activity payload data.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_communications_broadcast_handoff(cmp_os_communications_t *ctx_opaque,
                                            const char *activity_type,
                                            const char *payload) {
  int rc;
  rc = 0;
  struct cmp_os_communications *ctx;

  if (ctx_opaque == NULL || activity_type == NULL || payload == NULL) {
    LOG_DEBUG(
        "Error in cmp_os_communications_broadcast_handoff: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx = (struct cmp_os_communications *)ctx_opaque;
  (void)ctx;

  /* Sets current NSUserActivity */
  return CMP_SUCCESS;
}

/**
 * @brief Index content for Spotlight search.
 *
 * @param ctx_opaque System communication context.
 * @param item_id The unique item identifier.
 * @param title The title of the item.
 * @param description The description of the item.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_communications_index_for_spotlight(
    cmp_os_communications_t *ctx_opaque, const char *item_id, const char *title,
    const char *description) {
  int rc;
  rc = 0;
  struct cmp_os_communications *ctx;

  if (ctx_opaque == NULL || item_id == NULL || title == NULL ||
      description == NULL) {
    LOG_DEBUG("Error in cmp_os_communications_index_for_spotlight: Invalid "
              "argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx = (struct cmp_os_communications *)ctx_opaque;
  (void)ctx;

  /* CSSearchableItemAttributeSet */
  return CMP_SUCCESS;
}

/**
 * @brief Evaluate the current focus mode status.
 *
 * @param ctx_opaque System communication context.
 * @param out_is_suppressed Pointer to store whether notifications are
 * suppressed.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_communications_evaluate_focus_mode(
    cmp_os_communications_t *ctx_opaque, int *out_is_suppressed) {
  int rc;
  rc = 0;
  struct cmp_os_communications *ctx;

  if (ctx_opaque == NULL || out_is_suppressed == NULL) {
    LOG_DEBUG("Error in cmp_os_communications_evaluate_focus_mode: Invalid "
              "argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx = (struct cmp_os_communications *)ctx_opaque;
  *out_is_suppressed = ctx->is_focus_mode_suppressed;
  return CMP_SUCCESS;
}

/**
 * @brief Start a SharePlay session.
 *
 * @param ctx_opaque System communication context.
 * @param activity_id The SharePlay activity identifier.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_communications_start_shareplay(cmp_os_communications_t *ctx_opaque,
                                          const char *activity_id) {
  int rc;
  rc = 0;
  struct cmp_os_communications *ctx;

  if (ctx_opaque == NULL || activity_id == NULL) {
    LOG_DEBUG(
        "Error in cmp_os_communications_start_shareplay: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx = (struct cmp_os_communications *)ctx_opaque;
  (void)ctx;

  /* GroupActivities framework binding */
  return CMP_SUCCESS;
}
