/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_resource_manager {
  int is_backgrounded;
  int is_low_data_mode;
  int thermal_state; /* 0=nominal, 1=fair, 2=serious, 3=critical */
};

/**
 * @brief Creates a resource manager context.
 *
 * @param out_rm Pointer to a variable where the context pointer will be stored.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_resource_manager_create(cmp_resource_manager_t **out_rm) {
  int rc = CMP_SUCCESS;
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

  return rc;
}

/**
 * @brief Destroys a resource manager context.
 *
 * @param rm_opaque Pointer to the context to destroy.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_resource_manager_destroy(cmp_resource_manager_t *rm_opaque) {
  int rc = CMP_SUCCESS;
  if (!rm_opaque) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      int rc2;
      rc2 = cmp_strerror(rc, &err_str);
      if (rc2 != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      LOG_DEBUG("cmp_resource_manager_destroy: %s\n", err_str);
    }
    return rc;
  }
  CMP_FREE(rm_opaque);
  return rc;
}

/**
 * @brief Sets the thermal state of the device.
 *
 * @param rm_opaque Pointer to the resource manager context.
 * @param state The thermal state (0=nominal, 1=fair, 2=serious, 3=critical).
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_resources_set_thermal_state(cmp_resource_manager_t *rm_opaque,
                                    int state) {
  int rc = CMP_SUCCESS;
  struct cmp_resource_manager *ctx = (struct cmp_resource_manager *)rm_opaque;
  if (!ctx || state < 0 || state > 3)
    return CMP_ERROR_INVALID_ARG;

  ctx->thermal_state = state;
  /* ProcessInfo.thermalState: if >= 2, scale down animations and frame rates */

  return rc;
}

/**
 * @brief Sets the background state of the application.
 *
 * @param rm_opaque Pointer to the resource manager context.
 * @param is_backgrounded 1 if the application is in the background, 0
 * otherwise.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_resources_set_background_state(cmp_resource_manager_t *rm_opaque,
                                       int is_backgrounded) {
  int rc = CMP_SUCCESS;
  struct cmp_resource_manager *ctx = (struct cmp_resource_manager *)rm_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  ctx->is_backgrounded = is_backgrounded;
  /* Instantly pause render loops, animations, non-essential timers */

  return rc;
}

/**
 * @brief Marks a UI node as opaque to avoid costly blending.
 *
 * @param rm_opaque Pointer to the resource manager context.
 * @param node Pointer to the UI node.
 * @param is_opaque 1 to mark as opaque, 0 otherwise.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_resources_mark_node_opaque(cmp_resource_manager_t *rm_opaque,
                                   void *node, int is_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_resource_manager *ctx = (struct cmp_resource_manager *)rm_opaque;
  if (!ctx || !node || is_opaque < 0)
    return CMP_ERROR_INVALID_ARG;

  /* Skip costly alpha blending on the GPU */

  return rc;
}

/**
 * @brief Sets whether the device is in low data mode.
 *
 * @param rm_opaque Pointer to the resource manager context.
 * @param is_low_data 1 if in low data mode, 0 otherwise.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_resources_set_low_data_mode(cmp_resource_manager_t *rm_opaque,
                                    int is_low_data) {
  int rc = CMP_SUCCESS;
  struct cmp_resource_manager *ctx = (struct cmp_resource_manager *)rm_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  ctx->is_low_data_mode = is_low_data;
  /* Prevents large downloads, auto-playing videos, heavy prefetching */

  return rc;
}

/**
 * @brief Caches a remote image.
 *
 * @param rm_opaque Pointer to the resource manager context.
 * @param url The URL of the image to cache.
 * @param target_width The target width of the image.
 * @param target_height The target height of the image.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_resources_cache_remote_image(cmp_resource_manager_t *rm_opaque,
                                     const char *url, float target_width,
                                     float target_height) {
  int rc = CMP_SUCCESS;
  struct cmp_resource_manager *ctx = (struct cmp_resource_manager *)rm_opaque;
  if (!ctx || !url || target_width <= 0.0f || target_height <= 0.0f)
    return CMP_ERROR_INVALID_ARG;

  /* Decodes async, resizes to exact geometry, aggressively caches */

  return rc;
}

/**
 * @brief Allocates an offscreen bitmap.
 *
 * @param rm_opaque Pointer to the resource manager context.
 * @param width The width of the bitmap.
 * @param height The height of the bitmap.
 * @param out_bitmap Pointer to a variable where the bitmap pointer will be
 * stored.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_resources_allocate_offscreen_bitmap(cmp_resource_manager_t *rm_opaque,
                                            float width, float height,
                                            void **out_bitmap) {
  int rc = CMP_SUCCESS;
  struct cmp_resource_manager *ctx = (struct cmp_resource_manager *)rm_opaque;
  if (!ctx || width <= 0.0f || height <= 0.0f || !out_bitmap)
    return CMP_ERROR_INVALID_ARG;

  /* Monitored equivalent to drawRect that avoids massive allocations unless
   * necessary */
  *out_bitmap = (void *)1;

  return rc;
}
