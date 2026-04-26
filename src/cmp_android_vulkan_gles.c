/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_android_renderer {
  int active_backend;
};

/**
 * @brief cmp_android_renderer_create
 *
 * @param out_renderer Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_android_renderer_create(cmp_android_renderer_t **out_renderer) {
  cmp_android_renderer_t *r;
  int rc;

  if (!out_renderer) {
    LOG_DEBUG("cmp_android_renderer_create: out_renderer is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_android_renderer_t), (void **)&r);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_android_renderer_create: OOM\n");
    return CMP_ERROR_OOM;
  }

  r->active_backend = 0; /* NONE */
  *out_renderer = r;

  return CMP_SUCCESS;
}

/**
 * @brief cmp_android_renderer_destroy
 *
 * @param renderer Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_android_renderer_destroy(cmp_android_renderer_t *renderer) {
  int rc;
  if (!renderer) {
    LOG_DEBUG("cmp_android_renderer_destroy: renderer is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(renderer);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_android_renderer_destroy: CMP_FREE failed\n");
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_android_renderer_initialize_fallback
 *
 * @param renderer Parameter description.
 * @param window Parameter description.
 * @param out_backend Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_android_renderer_initialize_fallback(cmp_android_renderer_t *renderer,
                                             cmp_window_t *window,
                                             int *out_backend) {
  if (!renderer || !window || !out_backend)
    return CMP_ERROR_INVALID_ARG;

  /* Mock: Try Vulkan first */
  /* In a real implementation, we would query
   * vkEnumerateInstanceExtensionProperties */
  /* For this cross-platform mock, we just select a fallback explicitly */
  renderer->active_backend = 1; /* e.g., CMP_BACKEND_VULKAN */
  *out_backend = renderer->active_backend;

  return CMP_SUCCESS;
}
