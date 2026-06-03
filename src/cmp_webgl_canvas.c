/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_webgl_canvas {
  int is_bound;
};

/**
 * @brief cmp_webgl_canvas_create
 *
 * @param out_canvas Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_webgl_canvas_create(cmp_webgl_canvas_t **out_canvas) {
  cmp_webgl_canvas_t *c = NULL;
  int rc = CMP_SUCCESS;

  if (!out_canvas) {
    LOG_DEBUG("cmp_webgl_canvas_create: out_canvas is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_webgl_canvas_t), (void **)&c);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_webgl_canvas_create: OOM\n");

    return rc;
  }

  c->is_bound = 0;
  *out_canvas = c;

  return rc;
}

/**
 * @brief cmp_webgl_canvas_destroy
 *
 * @param canvas Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_webgl_canvas_destroy(cmp_webgl_canvas_t *canvas) {
  int rc = CMP_SUCCESS;

  if (!canvas) {
    LOG_DEBUG("cmp_webgl_canvas_destroy: canvas is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(canvas);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_webgl_canvas_destroy: CMP_FREE failed\n");

    return rc;
  }
  return rc;
}

/**
 * @brief Initializes the Emscripten rendering bindings to the target DOM
 * element.
 *
 * @param canvas The manager.
 * @param dom_selector The HTML ID selector (e.g., "#canvas").
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_webgl_canvas_bind(cmp_webgl_canvas_t *canvas,
                          const char *dom_selector) {
  int rc = CMP_SUCCESS;
  if (!canvas || !dom_selector) {
    LOG_DEBUG("cmp_webgl_canvas_bind: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(__EMSCRIPTEN__)
  /* Call the c-multiplatform wasm binding hook */
  int rc = cmp_window_wasm_init(dom_selector);
  if (rc != CMP_SUCCESS) {
    return CMP_ERROR_GENERAL;
  }
#endif

  canvas->is_bound = 1;

  return rc;
}
