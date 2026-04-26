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
 * @brief cmp_mermaid_renderer_create
 *
 * @param out_renderer Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mermaid_renderer_create(cmp_mermaid_renderer_t **out_renderer) {
  int rc = CMP_SUCCESS;
  cmp_mermaid_renderer_t *renderer = NULL;

  if (!out_renderer) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_mermaid_renderer_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_mermaid_renderer_t), (void **)&(renderer));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  renderer->is_initialized = 1;
  *out_renderer = renderer;
  return rc;
}

/**
 * @brief cmp_mermaid_renderer_destroy
 *
 * @param renderer Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mermaid_renderer_destroy(cmp_mermaid_renderer_t *renderer) {
  int rc = CMP_SUCCESS;

  if (!renderer) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_mermaid_renderer_destroy: Invalid argument\n");
    return rc;
  }
  rc = CMP_FREE(renderer);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  return rc;
}

/**
 * @brief cmp_mermaid_renderer_generate_svg
 *
 * @param renderer Parameter description.
 * @param mermaid_syntax Parameter description.
 * @param out_svg_xml Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mermaid_renderer_generate_svg(cmp_mermaid_renderer_t *renderer,
                                      const char *mermaid_syntax,
                                      char **out_svg_xml) {
  int rc = CMP_SUCCESS;
  const char *dummy_svg = "<svg><rect width=\"10\" height=\"10\"/></svg>";
  char *svg_out = NULL;
  size_t len;

  if (!renderer || !mermaid_syntax || !out_svg_xml) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_mermaid_renderer_generate_svg: Invalid argument\n");
    return rc;
  }

  len = strlen(dummy_svg);
  rc = CMP_MALLOC(len + 1, (void **)&(svg_out));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  strncpy_s(svg_out, len + 1, dummy_svg, _TRUNCATE);
#else
  strncpy(svg_out, dummy_svg, len);
#endif
  svg_out[len] = '\0';
  *out_svg_xml = svg_out;

  return rc;
}

/**
 * @brief cmp_mermaid_renderer_free_svg
 *
 * @param svg_xml Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mermaid_renderer_free_svg(char *svg_xml) {
  int rc = CMP_SUCCESS;

  if (svg_xml) {
    rc = CMP_FREE(svg_xml);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
  }
  return rc;
}
