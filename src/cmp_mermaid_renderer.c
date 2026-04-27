/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_mermaid_renderer {
  int is_initialized;
};

/**
 * @brief Create a mermaid renderer context.
 *
 * @param out_renderer Pointer to store the created renderer.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_mermaid_renderer_create(cmp_mermaid_renderer_t **out_renderer) {
  int rc;
  cmp_mermaid_renderer_t *renderer;

  rc = CMP_SUCCESS;
  renderer = NULL;

  if (out_renderer == NULL) {
    LOG_DEBUG("Error in cmp_mermaid_renderer_create: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_mermaid_renderer_t), (void **)&renderer);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG(
        "Error in cmp_mermaid_renderer_create: CMP_MALLOC failed (OOM)\n");
    return CMP_ERROR_OOM;
  }

  renderer->is_initialized = 1;
  *out_renderer = renderer;
  return CMP_SUCCESS;
}

/**
 * @brief Destroy a mermaid renderer context.
 *
 * @param renderer The renderer to destroy.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_mermaid_renderer_destroy(cmp_mermaid_renderer_t *renderer) {
  int rc;

  rc = CMP_SUCCESS;

  if (renderer == NULL) {
    LOG_DEBUG("Error in cmp_mermaid_renderer_destroy: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(renderer);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_mermaid_renderer_destroy: CMP_FREE failed\n");
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief Generate SVG from mermaid syntax.
 *
 * @param renderer The mermaid renderer.
 * @param mermaid_syntax The mermaid markdown syntax.
 * @param out_svg_xml Pointer to store the generated SVG string.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_mermaid_renderer_generate_svg(cmp_mermaid_renderer_t *renderer,
                                      const char *mermaid_syntax,
                                      char **out_svg_xml) {
  int rc;
  const char *dummy_svg = "<svg><rect width=\"10\" height=\"10\"/></svg>";
  char *svg_out;
  size_t len;

  rc = CMP_SUCCESS;
  svg_out = NULL;

  if (renderer == NULL || mermaid_syntax == NULL || out_svg_xml == NULL) {
    LOG_DEBUG("Error in cmp_mermaid_renderer_generate_svg: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  len = strlen(dummy_svg);
  rc = CMP_MALLOC(len + 1, (void **)&svg_out);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_mermaid_renderer_generate_svg: CMP_MALLOC failed "
              "(OOM)\n");
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  rc = strncpy_s(svg_out, len + 1, dummy_svg, _TRUNCATE);
  if (rc != 0) {
    LOG_DEBUG("Error in cmp_mermaid_renderer_generate_svg: strncpy_s failed\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strncpy(svg_out, dummy_svg, len);
#endif
  svg_out[len] = '\0';
  *out_svg_xml = svg_out;

  return CMP_SUCCESS;
}

/**
 * @brief Free an SVG XML string allocated by generate_svg.
 *
 * @param svg_xml The SVG XML string.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_mermaid_renderer_free_svg(char *svg_xml) {
  int rc;

  rc = CMP_SUCCESS;

  if (svg_xml != NULL) {
    rc = CMP_FREE(svg_xml);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_mermaid_renderer_free_svg: CMP_FREE failed\n");
      return rc;
    }
  }
  return CMP_SUCCESS;
}
