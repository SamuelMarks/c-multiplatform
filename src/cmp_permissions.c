/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_permissions {
  cmp_permission_status_t location_status;
  cmp_permission_status_t camera_status;
  cmp_permission_status_t mic_status;
  cmp_permission_status_t photo_status;
  cmp_permission_status_t tracking_status;
};

int cmp_permissions_create(cmp_permissions_t **out_ctx) {
  int rc = CMP_SUCCESS;
  struct cmp_permissions *ctx = NULL;

  if (!out_ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_permissions_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_permissions), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_permissions_create: Out of memory\n");
    return rc;
  }

  ctx->location_status = CMP_PERMISSION_STATUS_NOT_DETERMINED;
  ctx->camera_status = CMP_PERMISSION_STATUS_NOT_DETERMINED;
  ctx->mic_status = CMP_PERMISSION_STATUS_NOT_DETERMINED;
  ctx->photo_status = CMP_PERMISSION_STATUS_NOT_DETERMINED;
  ctx->tracking_status = CMP_PERMISSION_STATUS_NOT_DETERMINED;

  *out_ctx = (cmp_permissions_t *)ctx;
  return rc;
}

int cmp_permissions_destroy(cmp_permissions_t *ctx) {
  int rc = CMP_SUCCESS;

  if (ctx) {
    CMP_FREE(ctx);
  }
  return rc;
}

int cmp_permissions_get_status(cmp_permissions_t *ctx,
                               cmp_permission_type_t type,
                               cmp_permission_status_t *out_status) {
  int rc = CMP_SUCCESS;
  struct cmp_permissions *p = (struct cmp_permissions *)ctx;

  if (!p || !out_status) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_permissions_get_status: Invalid argument\n");
    return rc;
  }

  switch (type) {
  case CMP_PERMISSION_LOCATION:
    *out_status = p->location_status;
    break;
  case CMP_PERMISSION_CAMERA:
    *out_status = p->camera_status;
    break;
  case CMP_PERMISSION_MICROPHONE:
    *out_status = p->mic_status;
    break;
  case CMP_PERMISSION_PHOTO_LIBRARY:
    *out_status = p->photo_status;
    break;
  case CMP_PERMISSION_APP_TRACKING:
    *out_status = p->tracking_status;
    break;
  default:
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_permissions_get_status: Unknown permission type\n");
    return rc;
  }
  return rc;
}

int cmp_permissions_request(cmp_permissions_t *ctx,
                            cmp_permission_type_t type) {
  int rc = CMP_SUCCESS;
  struct cmp_permissions *p = (struct cmp_permissions *)ctx;

  if (!p) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_permissions_request: Invalid argument\n");
    return rc;
  }

  /* Simulating OS-level prompt resolution */
  switch (type) {
  case CMP_PERMISSION_LOCATION:
    p->location_status =
        CMP_PERMISSION_STATUS_AUTHORIZED_APPROXIMATE; /* Default to approximate
                                                         for privacy */
    break;
  case CMP_PERMISSION_CAMERA:
    p->camera_status = CMP_PERMISSION_STATUS_AUTHORIZED;
    break;
  case CMP_PERMISSION_MICROPHONE:
    p->mic_status = CMP_PERMISSION_STATUS_AUTHORIZED;
    break;
  case CMP_PERMISSION_PHOTO_LIBRARY:
    p->photo_status =
        CMP_PERMISSION_STATUS_LIMITED; /* iOS 14 limited default */
    break;
  case CMP_PERMISSION_APP_TRACKING:
    p->tracking_status =
        CMP_PERMISSION_STATUS_DENIED; /* Usually denied by default now */
    break;
  default:
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_permissions_request: Unknown permission type\n");
    return rc;
  }
  return rc;
}

int cmp_tree_set_screen_recording_prevention(cmp_a11y_tree_t *tree, int node_id,
                                             int enabled) {
  int rc = CMP_SUCCESS;

  (void)node_id;
  (void)enabled;

  if (!tree) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_tree_set_screen_recording_prevention: Invalid "
              "argument\n");
    return rc;
  }
  /* In a real engine, this sets an internal flag on the specific vdom node
     that triggers a hardware layer black-box over the surface during
     compositing. */
  return rc;
}

struct cmp_privacy_indicators {
  cmp_rect_t system_indicator_bounds; /* Where OS draws the orange/green dots */
};

int cmp_privacy_indicators_create(cmp_privacy_indicators_t **out_indicators) {
  int rc = CMP_SUCCESS;
  struct cmp_privacy_indicators *inds = NULL;

  if (!out_indicators) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_privacy_indicators_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_privacy_indicators), (void **)&inds);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_privacy_indicators_create: Out of memory\n");
    return rc;
  }

  /* Simulate standard dynamic island / notch indicator bounds */
  inds->system_indicator_bounds.x = 300.0f;
  inds->system_indicator_bounds.y = 10.0f;
  inds->system_indicator_bounds.width = 15.0f;
  inds->system_indicator_bounds.height = 15.0f;

  *out_indicators = (cmp_privacy_indicators_t *)inds;
  return rc;
}

int cmp_privacy_indicators_destroy(cmp_privacy_indicators_t *indicators) {
  int rc = CMP_SUCCESS;

  if (indicators) {
    CMP_FREE(indicators);
  }
  return rc;
}

int cmp_privacy_indicators_verify_layout(cmp_privacy_indicators_t *indicators,
                                         const cmp_rect_t *ui_bounds,
                                         int *out_is_obscured) {
  int rc = CMP_SUCCESS;
  struct cmp_privacy_indicators *inds =
      (struct cmp_privacy_indicators *)indicators;

  if (!inds || !ui_bounds || !out_is_obscured) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_privacy_indicators_verify_layout: Invalid argument\n");
    return rc;
  }

  /* Simple AABB intersection test */
  if (ui_bounds->x < inds->system_indicator_bounds.x +
                         inds->system_indicator_bounds.width &&
      ui_bounds->x + ui_bounds->width > inds->system_indicator_bounds.x &&
      ui_bounds->y < inds->system_indicator_bounds.y +
                         inds->system_indicator_bounds.height &&
      ui_bounds->y + ui_bounds->height > inds->system_indicator_bounds.y) {
    *out_is_obscured = 1;
  } else {
    *out_is_obscured = 0;
  }

  return rc;
}
