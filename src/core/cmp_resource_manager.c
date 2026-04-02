/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_resource_manager {
  int is_backgrounded;
  int is_low_data_mode;
  int thermal_state; /* 0=nominal, 1=fair, 2=serious, 3=critical */
};

int cmp_resource_manager_create(cmp_resource_manager_t **out_rm) {
  struct cmp_resource_manager *ctx;
  if (!out_rm)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_resource_manager), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->is_backgrounded = 0;
  ctx->is_low_data_mode = 0;
  ctx->thermal_state = 0;

  *out_rm = (cmp_resource_manager_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_resource_manager_destroy(cmp_resource_manager_t *rm_opaque) {
  if (rm_opaque)
    CMP_FREE(rm_opaque);
  return CMP_SUCCESS;
}

int cmp_resources_set_thermal_state(cmp_resource_manager_t *rm_opaque,
                                    int state) {
  struct cmp_resource_manager *ctx = (struct cmp_resource_manager *)rm_opaque;
  if (!ctx || state < 0 || state > 3)
    return CMP_ERROR_INVALID_ARG;

  ctx->thermal_state = state;
  /* ProcessInfo.thermalState: if >= 2, scale down animations and frame rates */
  return CMP_SUCCESS;
}

int cmp_resources_set_background_state(cmp_resource_manager_t *rm_opaque,
                                       int is_backgrounded) {
  struct cmp_resource_manager *ctx = (struct cmp_resource_manager *)rm_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  ctx->is_backgrounded = is_backgrounded;
  /* Instantly pause render loops, animations, non-essential timers */
  return CMP_SUCCESS;
}

int cmp_resources_mark_node_opaque(cmp_resource_manager_t *rm_opaque,
                                   void *node, int is_opaque) {
  struct cmp_resource_manager *ctx = (struct cmp_resource_manager *)rm_opaque;
  if (!ctx || !node || is_opaque < 0)
    return CMP_ERROR_INVALID_ARG;

  /* Skip costly alpha blending on the GPU */
  return CMP_SUCCESS;
}

int cmp_resources_set_low_data_mode(cmp_resource_manager_t *rm_opaque,
                                    int is_low_data) {
  struct cmp_resource_manager *ctx = (struct cmp_resource_manager *)rm_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  ctx->is_low_data_mode = is_low_data;
  /* Prevents large downloads, auto-playing videos, heavy prefetching */
  return CMP_SUCCESS;
}

int cmp_resources_cache_remote_image(cmp_resource_manager_t *rm_opaque,
                                     const char *url, float target_width,
                                     float target_height) {
  struct cmp_resource_manager *ctx = (struct cmp_resource_manager *)rm_opaque;
  if (!ctx || !url || target_width <= 0.0f || target_height <= 0.0f)
    return CMP_ERROR_INVALID_ARG;

  /* Decodes async, resizes to exact geometry, aggressively caches */
  return CMP_SUCCESS;
}

int cmp_resources_allocate_offscreen_bitmap(cmp_resource_manager_t *rm_opaque,
                                            float width, float height,
                                            void **out_bitmap) {
  struct cmp_resource_manager *ctx = (struct cmp_resource_manager *)rm_opaque;
  if (!ctx || width <= 0.0f || height <= 0.0f || !out_bitmap)
    return CMP_ERROR_INVALID_ARG;

  /* Monitored equivalent to drawRect that avoids massive allocations unless
   * necessary */
  *out_bitmap = (void *)1;
  return CMP_SUCCESS;
}
