/* clang-format off */
#include "../include/ui_renderer.h"
#include "../include/ui_error.h"
#include "ui_internal_mem.h"
/* clang-format on */

/* Forward declare the Native and GLES backends.
   These will be implemented in subsequent steps. */

/* Native renderer initialization. Returns non-zero (error) if
 * unsupported/fails. */
enum ui_error ui_renderer_native_init(struct ui_renderer *renderer);

#if !defined(_WIN32) && !defined(__APPLE__)
/* Stub for platforms without native backends. */
enum ui_error ui_renderer_native_init(struct ui_renderer *renderer) {
  if (!renderer)
    return UI_ERROR_INVALID_ARGUMENT;
  return UI_ERROR_UNSUPPORTED;
}
#endif

/* GLES fallback initialization. */
enum ui_error ui_renderer_gles_fallback_init(struct ui_renderer *renderer);

enum ui_error ui_renderer_create(struct ui_renderer **out_renderer) {
  struct ui_renderer *renderer;
  enum ui_error rc;

  if (!out_renderer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  renderer = (struct ui_renderer *)UI_MALLOC(sizeof(struct ui_renderer));
  if (!renderer) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  renderer->vtable = NULL;
  renderer->ctx = NULL;

  /* 1. Runtime Probing: Attempt to initialize Native backend first */
  rc = ui_renderer_native_init(renderer);
  if (rc != UI_ERROR_NONE) {
    /* 2. Seamless fallback to GLES 2.0 */
    rc = ui_renderer_gles_fallback_init(renderer);
    if (rc != UI_ERROR_NONE) {
      UI_FREE(renderer);
      return UI_ERROR_UNKNOWN;
    }
  }

  *out_renderer = renderer;
  return UI_ERROR_NONE;
}

enum ui_error ui_renderer_destroy(struct ui_renderer *renderer) {
  if (!renderer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (renderer->vtable && renderer->vtable->destroy) {
    renderer->vtable->destroy(renderer->ctx);
  }

  UI_FREE(renderer);
  return UI_ERROR_NONE;
}
