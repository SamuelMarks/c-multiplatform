/* clang-format off */
#include "../include/ui_renderer.h"
#include "../include/ui_error.h"
#include "ui_internal_mem.h"
/* clang-format on */

/* Forward declare the Native and GLES backends.
   These will be implemented in subsequent steps. */

/* Native renderer initialization. Returns non-zero (error) if
 * unsupported/fails. */
ui_error_t ui_renderer_native_init(struct ui_renderer *renderer);

#if !defined(_WIN32) && !defined(__APPLE__)
/* Stub for platforms without native backends. */
ui_error_t ui_renderer_native_init(struct ui_renderer *renderer) {
  if (!renderer)
    return UI_ERROR_INVALID_ARGUMENT;
  return UI_ERROR_UNSUPPORTED;
}
#endif

/* GLES fallback initialization. */
ui_error_t ui_renderer_gles_fallback_init(struct ui_renderer *renderer);

ui_error_t ui_renderer_create(struct ui_renderer **out_renderer) {
  struct ui_renderer *renderer;
  ui_error_t rc;
  extern int g_native_init_fail;
  extern int g_gles_init_fail;

  if (!out_renderer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  renderer =
      (struct ui_renderer *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_renderer));
  if (!renderer) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  renderer->vtable = NULL;
  renderer->ctx = NULL;

  /* 1. Runtime Probing: Attempt to initialize Native backend first */
  {
    ui_error_t init_rc = g_native_init_fail ? UI_ERROR_UNKNOWN
                                            : ui_renderer_native_init(renderer);
    if (init_rc != UI_ERROR_NONE) {
      /* 2. Seamless fallback to GLES 2.0 */
      {
        ui_error_t fb_rc = g_gles_init_fail
                               ? UI_ERROR_UNKNOWN
                               : ui_renderer_gles_fallback_init(renderer);
        if (fb_rc != UI_ERROR_NONE) {
          C_MULTIPLATFORM_FREE(renderer);
          return UI_ERROR_UNKNOWN;
        }
      }
    }
  }

  *out_renderer = renderer;
  return UI_ERROR_NONE;
}

ui_error_t ui_renderer_destroy(struct ui_renderer *renderer) {
  if (!renderer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (renderer->vtable && renderer->vtable->destroy) {
    ui_error_t destroy_rc = renderer->vtable->destroy(renderer->ctx);
    (void)destroy_rc;
  }

  C_MULTIPLATFORM_FREE(renderer);
  return UI_ERROR_NONE;
}
