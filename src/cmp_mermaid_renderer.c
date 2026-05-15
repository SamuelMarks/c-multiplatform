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
  int rc = CMP_SUCCESS;
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
  return rc;
}

/**
 * @brief Destroy a mermaid renderer context.
 *
 * @param renderer The renderer to destroy.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_mermaid_renderer_destroy(cmp_mermaid_renderer_t *renderer) {
  int rc = CMP_SUCCESS;

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
  return rc;
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
  int rc = CMP_SUCCESS;
  const char *svg_prefix =
      "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"200\" "
      "height=\"100\"><text x=\"10\" y=\"20\">";
  const char *svg_suffix = "</text></svg>";
  char *svg_out;
  size_t len;
  size_t syn_len;

  rc = CMP_SUCCESS;
  svg_out = NULL;

  if (renderer == NULL || mermaid_syntax == NULL || out_svg_xml == NULL) {
    LOG_DEBUG("Error in cmp_mermaid_renderer_generate_svg: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (strstr(mermaid_syntax, "graph") == NULL &&
      strstr(mermaid_syntax, "sequenceDiagram") == NULL) {
    /* Simple mocked syntax validation for stubs */
    return CMP_ERROR_INVALID_ARG;
  }

  syn_len = strlen(mermaid_syntax);
  len = strlen(svg_prefix) + syn_len + strlen(svg_suffix);
  rc = CMP_MALLOC(len + 1, (void **)&svg_out);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_mermaid_renderer_generate_svg: CMP_MALLOC failed "
              "(OOM)\n");
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  if (strcpy_s(svg_out, len + 1, svg_prefix) != 0) {
    CMP_FREE(svg_out);
    return CMP_ERROR_GENERAL;
  }
  if (strcat_s(svg_out, len + 1, mermaid_syntax) != 0) {
    CMP_FREE(svg_out);
    return CMP_ERROR_GENERAL;
  }
  if (strcat_s(svg_out, len + 1, svg_suffix) != 0) {
    CMP_FREE(svg_out);
    return CMP_ERROR_GENERAL;
  }
#else
  strcpy(svg_out, svg_prefix);
  strcat(svg_out, mermaid_syntax);
  strcat(svg_out, svg_suffix);
#endif

  svg_out[len] = '\0';
  *out_svg_xml = svg_out;

  return rc;
}

/**
 * @brief Free an SVG XML string allocated by generate_svg.
 *
 * @param svg_xml The SVG XML string.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_mermaid_renderer_free_svg(char *svg_xml) {
  int rc = CMP_SUCCESS;

  rc = CMP_SUCCESS;

  if (svg_xml != NULL) {
    rc = CMP_FREE(svg_xml);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_mermaid_renderer_free_svg: CMP_FREE failed\n");
      return rc;
    }
  }
  return rc;
}
