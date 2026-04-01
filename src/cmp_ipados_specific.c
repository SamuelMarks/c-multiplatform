/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_ipados_features {
  int center_stage_enabled;
};

int cmp_ipados_features_create(cmp_ipados_features_t **out_features) {
  struct cmp_ipados_features *ctx;
  if (!out_features)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_ipados_features), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->center_stage_enabled = 0;

  *out_features = (cmp_ipados_features_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_ipados_features_destroy(cmp_ipados_features_t *features_opaque) {
  if (features_opaque)
    CMP_FREE(features_opaque);
  return CMP_SUCCESS;
}

int cmp_ipados_resolve_size_classes(float window_width, float window_height,
                                    cmp_size_class_t *out_horizontal,
                                    cmp_size_class_t *out_vertical) {
  if (!out_horizontal || !out_vertical || window_width <= 0.0f ||
      window_height <= 0.0f)
    return CMP_ERROR_INVALID_ARG;

  /* Apple HIG Trait Collection defaults:
     iPhone Portrait:  hCompact, vRegular
     iPad Fullscreen:  hRegular, vRegular
     iPad Slide Over:  hCompact, vRegular (acts exactly like iPhone portrait)
  */

  *out_horizontal =
      (window_width < 400.0f) ? CMP_SIZE_CLASS_COMPACT : CMP_SIZE_CLASS_REGULAR;
  *out_vertical = (window_height < 400.0f) ? CMP_SIZE_CLASS_COMPACT
                                           : CMP_SIZE_CLASS_REGULAR;

  return CMP_SUCCESS;
}

int cmp_ipados_resolve_sidebar_state(cmp_size_class_t horizontal_class,
                                     int *out_is_collapsed) {
  if (!out_is_collapsed)
    return CMP_ERROR_INVALID_ARG;

  /* Sidebars (like split views) collapse entirely into standard Push/Pop views
   * on compact widths */
  *out_is_collapsed = (horizontal_class == CMP_SIZE_CLASS_COMPACT) ? 1 : 0;
  return CMP_SUCCESS;
}

int cmp_ipados_request_scene_activation(cmp_ipados_features_t *features_opaque,
                                        const char *activity_identifier) {
  struct cmp_ipados_features *ctx =
      (struct cmp_ipados_features *)features_opaque;
  if (!ctx || !activity_identifier)
    return CMP_ERROR_INVALID_ARG;

  /* Invokes UIApplication.shared.requestSceneSessionActivation with the
   * NSUserActivity */
  return CMP_SUCCESS;
}

int cmp_ipados_set_center_stage_enabled(cmp_ipados_features_t *features_opaque,
                                        int is_enabled) {
  struct cmp_ipados_features *ctx =
      (struct cmp_ipados_features *)features_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  ctx->center_stage_enabled = is_enabled;
  /* AVCaptureDevice.CenterStageControlMode */

  return CMP_SUCCESS;
}
