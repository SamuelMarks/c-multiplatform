/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>

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

struct cmp_math_visual_tree {
  char *raw_latex;
  int is_inline;
};

int cmp_math_renderer_create(cmp_math_renderer_t **out_renderer) {
  cmp_math_renderer_t *renderer;
  if (!out_renderer) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_math_renderer_t), (void **)&renderer) !=
      CMP_SUCCESS) {
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
  CMP_FREE(renderer);
  return CMP_SUCCESS;
}

int cmp_math_renderer_parse(cmp_math_renderer_t *renderer,
                            const char *latex_string, int is_inline,
                            void **out_visual_tree) {
  struct cmp_math_visual_tree *tree;
  size_t len;

  if (!renderer || !latex_string || !out_visual_tree) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(struct cmp_math_visual_tree), (void **)&tree) !=
      CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  len = strlen(latex_string);
  if (CMP_MALLOC(len + 1, (void **)&tree->raw_latex) != CMP_SUCCESS) {
    CMP_FREE(tree);
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  strcpy_s(tree->raw_latex, len + 1, latex_string);
#else
  strcpy(tree->raw_latex, latex_string);
#endif

  tree->is_inline = is_inline;
  *out_visual_tree = tree;

  return CMP_SUCCESS;
}

int cmp_math_renderer_free_tree(void *visual_tree) {
  struct cmp_math_visual_tree *tree =
      (struct cmp_math_visual_tree *)visual_tree;
  if (!tree)
    return CMP_ERROR_INVALID_ARG;

  if (tree->raw_latex) {
    CMP_FREE(tree->raw_latex);
  }
  CMP_FREE(tree);

  return CMP_SUCCESS;
}
