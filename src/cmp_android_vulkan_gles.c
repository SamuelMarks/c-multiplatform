/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_android_renderer {
  int active_backend;
};

int cmp_android_renderer_create(cmp_android_renderer_t **out_renderer) {
  cmp_android_renderer_t *r;

  if (!out_renderer)
    return CMP_ERROR_INVALID_ARG;

  r = (cmp_android_renderer_t *)malloc(sizeof(cmp_android_renderer_t));
  if (!r)
    return CMP_ERROR_OOM;

  r->active_backend = 0; /* NONE */
  *out_renderer = r;

  return CMP_SUCCESS;
}

int cmp_android_renderer_destroy(cmp_android_renderer_t *renderer) {
  if (!renderer)
    return CMP_ERROR_INVALID_ARG;

  free(renderer);
  return CMP_SUCCESS;
}

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
