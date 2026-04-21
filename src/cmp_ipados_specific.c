/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_ipados_features {
  int center_stage_enabled;
};

int cmp_ipados_features_create(cmp_ipados_features_t **out_features) {
  int rc = CMP_SUCCESS;
  struct cmp_ipados_features *ctx = NULL;

  if (!out_features) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ipados_features_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_ipados_features), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_ipados_features_create: Out of memory\n");
    return rc;
  }

  ctx->center_stage_enabled = 0;

  *out_features = (cmp_ipados_features_t *)ctx;
  return rc;
}

int cmp_ipados_features_destroy(cmp_ipados_features_t *features_opaque) {
  int rc = CMP_SUCCESS;

  if (features_opaque) {
    CMP_FREE(features_opaque);
  }
  return rc;
}

int cmp_ipados_resolve_size_classes(float window_width, float window_height,
                                    cmp_size_class_t *out_horizontal,
                                    cmp_size_class_t *out_vertical) {
  int rc = CMP_SUCCESS;

  if (!out_horizontal || !out_vertical || window_width <= 0.0f ||
      window_height <= 0.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ipados_resolve_size_classes: Invalid argument\n");
    return rc;
  }

  /* Apple HIG Trait Collection defaults:
     iPhone Portrait:  hCompact, vRegular
     iPad Fullscreen:  hRegular, vRegular
     iPad Slide Over:  hCompact, vRegular (acts exactly like iPhone portrait)
  */

  *out_horizontal =
      (window_width < 400.0f) ? CMP_SIZE_CLASS_COMPACT : CMP_SIZE_CLASS_REGULAR;
  *out_vertical = (window_height < 400.0f) ? CMP_SIZE_CLASS_COMPACT
                                           : CMP_SIZE_CLASS_REGULAR;

  return rc;
}

int cmp_ipados_resolve_sidebar_state(cmp_size_class_t horizontal_class,
                                     int *out_is_collapsed) {
  int rc = CMP_SUCCESS;

  if (!out_is_collapsed) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ipados_resolve_sidebar_state: Invalid argument\n");
    return rc;
  }

  /* Sidebars (like split views) collapse entirely into standard Push/Pop views
   * on compact widths */
  *out_is_collapsed = (horizontal_class == CMP_SIZE_CLASS_COMPACT) ? 1 : 0;
  return rc;
}

int cmp_ipados_request_scene_activation(cmp_ipados_features_t *features_opaque,
                                        const char *activity_identifier) {
  int rc = CMP_SUCCESS;
  struct cmp_ipados_features *ctx =
      (struct cmp_ipados_features *)features_opaque;

  if (!ctx || !activity_identifier) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_ipados_request_scene_activation: Invalid argument\n");
    return rc;
  }

  /* Invokes UIApplication.shared.requestSceneSessionActivation with the
   * NSUserActivity */
  return rc;
}

int cmp_ipados_set_center_stage_enabled(cmp_ipados_features_t *features_opaque,
                                        int is_enabled) {
  int rc = CMP_SUCCESS;
  struct cmp_ipados_features *ctx =
      (struct cmp_ipados_features *)features_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_ipados_set_center_stage_enabled: Invalid argument\n");
    return rc;
  }

  ctx->center_stage_enabled = is_enabled;
  /* AVCaptureDevice.CenterStageControlMode */

  return rc;
}
