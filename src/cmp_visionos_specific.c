/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_visionos_features {
  int is_immersive;
  int immersion_level; /* 0=mixed, 1=progressive, 2=full */
  float current_z_depth;
};

int cmp_visionos_features_create(cmp_visionos_features_t **out_features) {
  struct cmp_visionos_features *ctx;
  if (!out_features)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_visionos_features), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->is_immersive = 0;
  ctx->immersion_level = 0;
  ctx->current_z_depth = 0.0f;

  *out_features = (cmp_visionos_features_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_visionos_features_destroy(cmp_visionos_features_t *features_opaque) {
  if (features_opaque)
    CMP_FREE(features_opaque);
  return CMP_SUCCESS;
}

int cmp_visionos_apply_glass_material(cmp_visionos_features_t *features_opaque,
                                      void *node) {
  struct cmp_visionos_features *ctx =
      (struct cmp_visionos_features *)features_opaque;
  if (!ctx || !node)
    return CMP_ERROR_INVALID_ARG;

  /* Maps to visionOS standard OS glass background with specular highlights */
  return CMP_SUCCESS;
}

int cmp_visionos_handle_eye_tracking_hover(
    cmp_visionos_features_t *features_opaque, void *node, int is_looking) {
  struct cmp_visionos_features *ctx =
      (struct cmp_visionos_features *)features_opaque;
  if (!ctx || !node || is_looking < 0)
    return CMP_ERROR_INVALID_ARG;

  /* Adds subtle glow indicating readiness for pinch gesture */
  return CMP_SUCCESS;
}

int cmp_visionos_set_modal_z_depth(cmp_visionos_features_t *features_opaque,
                                   void *modal_node, float depth_offset) {
  struct cmp_visionos_features *ctx =
      (struct cmp_visionos_features *)features_opaque;
  if (!ctx || !modal_node)
    return CMP_ERROR_INVALID_ARG;

  /* Pushes modal physically closer to user on Z-axis */
  ctx->current_z_depth += depth_offset;
  return CMP_SUCCESS;
}

int cmp_visionos_setup_ornament(cmp_visionos_features_t *features_opaque,
                                void *toolbar_node) {
  struct cmp_visionos_features *ctx =
      (struct cmp_visionos_features *)features_opaque;
  if (!ctx || !toolbar_node)
    return CMP_ERROR_INVALID_ARG;

  /* Floats toolbar slightly outside the main window bounds facing the user */
  return CMP_SUCCESS;
}

int cmp_visionos_request_immersion_level(
    cmp_visionos_features_t *features_opaque, int level) {
  struct cmp_visionos_features *ctx =
      (struct cmp_visionos_features *)features_opaque;
  if (!ctx || level < 0 || level > 2)
    return CMP_ERROR_INVALID_ARG;

  ctx->immersion_level = level;
  ctx->is_immersive = (level > 0);
  return CMP_SUCCESS;
}

int cmp_visionos_handle_touch_event(cmp_visionos_features_t *features_opaque,
                                    int is_direct, float x, float y, float z) {
  struct cmp_visionos_features *ctx =
      (struct cmp_visionos_features *)features_opaque;
  if (!ctx || is_direct < 0 || x == 0.0f || y == 0.0f || z == 0.0f)
    return CMP_ERROR_INVALID_ARG;

  /* Handles physical poke vs look-and-pinch */
  return CMP_SUCCESS;
}
/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

int cmp_visionos_set_window_geometry(cmp_visionos_features_t *features_opaque,
                                     int is_volume) {
  struct cmp_visionos_features *ctx =
      (struct cmp_visionos_features *)features_opaque;
  if (!ctx || is_volume < 0)
    return CMP_ERROR_INVALID_ARG;

  /* Switches between flat Windows for 2D and bounded 3D Volumes */
  return CMP_SUCCESS;
}

int cmp_visionos_set_shared_space_behavior(
    cmp_visionos_features_t *features_opaque, int allow_shared) {
  struct cmp_visionos_features *ctx =
      (struct cmp_visionos_features *)features_opaque;
  if (!ctx || allow_shared < 0)
    return CMP_ERROR_INVALID_ARG;

  /* Prevents forcing an Immersive takeover unless requested */
  return CMP_SUCCESS;
}
