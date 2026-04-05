/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#if defined(_MSC_VER) && _MSC_VER <= 1400
#ifndef _UINTPTR_T_DEFINED
#define _UINTPTR_T_DEFINED
typedef unsigned long uintptr_t;
#endif
#else
#include <stdint.h>
#endif
/* clang-format on */

struct cmp_math_renderer {
  int is_initialized;
};

int cmp_math_renderer_create(cmp_math_renderer_t **out_renderer) {
  cmp_math_renderer_t *renderer;
  if (!out_renderer) {
    return CMP_ERROR_INVALID_ARG;
  }

  renderer = (cmp_math_renderer_t *)malloc(sizeof(cmp_math_renderer_t));
  if (!renderer) {
    return CMP_ERROR_OOM;
  }

  renderer->is_initialized = 1;
  *out_renderer = renderer;
  return CMP_SUCCESS;
}

int cmp_math_renderer_destroy(cmp_math_renderer_t *renderer) {
  if (!renderer) {
    return CMP_ERROR_INVALID_ARG;
  }
  free(renderer);
  return CMP_SUCCESS;
}

int cmp_math_renderer_parse(cmp_math_renderer_t *renderer,
                            const char *latex_string, int is_inline,
                            void **out_visual_tree) {
  if (!renderer || !latex_string || !out_visual_tree) {
    return CMP_ERROR_INVALID_ARG;
  }
  (void)is_inline;

  /* Stub implementation of KaTeX equivalent parser tree.
     Returns a dummy pointer for test. */
  *out_visual_tree = (void *)(uintptr_t)0xDEADBEEF;

  return CMP_SUCCESS;
}
