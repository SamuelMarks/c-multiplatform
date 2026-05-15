/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#ifdef __ANDROID__
#include <dlfcn.h>
#endif
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
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_android_renderer_t *r = NULL;

  if (out_renderer == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_android_renderer_create: Invalid argument "
                  "(out_renderer=NULL): %s\n",
                  err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_android_renderer_t), (void **)&r);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_android_renderer_create: Out of memory: %s\n", err_str);

    return rc;
  }

  r->active_backend = 0; /* NONE */
  *out_renderer = r;

  cmp_log_debug("cmp_android_renderer_create: Successfully created android "
                "renderer context\n");

  return rc;
}

/**
 * @brief cmp_android_renderer_destroy
 *
 * @param renderer Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_android_renderer_destroy(cmp_android_renderer_t *renderer) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (renderer == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_android_renderer_destroy: Invalid argument (renderer=NULL): %s\n",
        err_str);

    return rc;
  }

  CMP_FREE(renderer);
  cmp_log_debug("cmp_android_renderer_destroy: Successfully destroyed android "
                "renderer context\n");

  return rc;
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
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (renderer == NULL || window == NULL || out_backend == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_android_renderer_initialize_fallback: Invalid argument: %s\n",
        err_str);

    return rc;
  }

#ifdef __ANDROID__
  {
    /* Dynamically check if Vulkan library is available on the device */
    void *vulkan_lib = dlopen("libvulkan.so", RTLD_NOW);
    if (vulkan_lib != NULL) {
      renderer->active_backend = 1; /* Vulkan */
      dlclose(vulkan_lib);
    } else {
      renderer->active_backend = 2; /* Fallback to GLES */
    }
  }
#else
  /* For non-Android targets, default to Vulkan/Primary backend */
  renderer->active_backend = 1;
#endif
  *out_backend = renderer->active_backend;

  cmp_log_debug("cmp_android_renderer_initialize_fallback: Initialized "
                "fallback to backend %d\n",
                renderer->active_backend);

  return rc;
}
