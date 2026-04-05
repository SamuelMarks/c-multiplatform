/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_webgl_canvas {
  int is_bound;
};

int cmp_webgl_canvas_create(cmp_webgl_canvas_t **out_canvas) {
  cmp_webgl_canvas_t *c;

  if (!out_canvas)
    return CMP_ERROR_INVALID_ARG;

  c = (cmp_webgl_canvas_t *)malloc(sizeof(cmp_webgl_canvas_t));
  if (!c)
    return CMP_ERROR_OOM;

  c->is_bound = 0;
  *out_canvas = c;

  return CMP_SUCCESS;
}

int cmp_webgl_canvas_destroy(cmp_webgl_canvas_t *canvas) {
  if (!canvas)
    return CMP_ERROR_INVALID_ARG;
  free(canvas);
  return CMP_SUCCESS;
}

int cmp_webgl_canvas_bind(cmp_webgl_canvas_t *canvas,
                          const char *dom_selector) {
  if (!canvas || !dom_selector)
    return CMP_ERROR_INVALID_ARG;

#if defined(__EMSCRIPTEN__)
  /* Call the c-multiplatform wasm binding hook */
  if (cmp_window_wasm_init(dom_selector) != CMP_SUCCESS) {
    return CMP_ERROR_GENERAL;
  }
#endif

  canvas->is_bound = 1;
  return CMP_SUCCESS;
}
