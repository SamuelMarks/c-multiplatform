/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
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

/**
 * @brief cmp_math_renderer_create
 *
 * @param out_renderer Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_math_renderer_create(cmp_math_renderer_t **out_renderer) {
  int rc = CMP_SUCCESS;
  cmp_math_renderer_t *renderer = NULL;

  if (!out_renderer) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_math_renderer_create: Invalid argument\n");

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_math_renderer_t), (void **)&renderer);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_math_renderer_create: Out of memory\n");

    return rc;
  }

  renderer->is_initialized = 1;
  *out_renderer = renderer;

  return rc;
}

/**
 * @brief cmp_math_renderer_destroy
 *
 * @param renderer Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_math_renderer_destroy(cmp_math_renderer_t *renderer) {
  int rc = CMP_SUCCESS;

  if (!renderer) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_math_renderer_destroy: Invalid argument\n");

    return rc;
  }
  rc = CMP_FREE(renderer);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_math_renderer_destroy: CMP_FREE failed\n");
  }

  return rc;
}

/**
 * @brief cmp_math_renderer_parse
 *
 * @param renderer Parameter description.
 * @param latex_string Parameter description.
 * @param is_inline Parameter description.
 * @param out_visual_tree Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_math_renderer_parse(cmp_math_renderer_t *renderer,
                            const char *latex_string, int is_inline,
                            void **out_visual_tree) {
  int rc = CMP_SUCCESS;
  int free_rc;
  struct cmp_math_visual_tree *tree = NULL;
  size_t len;

  if (!renderer || !latex_string || !out_visual_tree) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_math_renderer_parse: Invalid argument\n");

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_math_visual_tree), (void **)&tree);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG(
        "Error in cmp_math_renderer_parse: Out of memory for visual tree\n");

    return rc;
  }

  len = strlen(latex_string);
  rc = CMP_MALLOC(len + 1, (void **)&tree->raw_latex);
  if (rc != CMP_SUCCESS) {
    free_rc = CMP_FREE(tree);
    if (free_rc != CMP_SUCCESS)
      rc = free_rc;
    LOG_DEBUG(
        "Error in cmp_math_renderer_parse: Out of memory for raw_latex\n");

    return rc;
  }
#if defined(_MSC_VER)
  strcpy_s(tree->raw_latex, len + 1, latex_string);
#else
  strcpy(tree->raw_latex, latex_string);
#endif

  tree->is_inline = is_inline;
  *out_visual_tree = tree;

  return rc;
}

/**
 * @brief cmp_math_renderer_free_tree
 *
 * @param visual_tree Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_math_renderer_free_tree(void *visual_tree) {
  int rc = CMP_SUCCESS;
  int free_rc;
  struct cmp_math_visual_tree *tree =
      (struct cmp_math_visual_tree *)visual_tree;

  if (!tree) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_math_renderer_free_tree: Invalid argument\n");

    return rc;
  }

  if (tree->raw_latex) {
    free_rc = CMP_FREE(tree->raw_latex);
    if (free_rc != CMP_SUCCESS)
      rc = free_rc;
  }
  free_rc = CMP_FREE(tree);
  if (free_rc != CMP_SUCCESS)
    rc = free_rc;

  return rc;
}
