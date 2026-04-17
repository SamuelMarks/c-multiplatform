/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_mermaid_renderer {
  int is_initialized;
};

int cmp_mermaid_renderer_create(cmp_mermaid_renderer_t **out_renderer) {
  int rc = CMP_SUCCESS;
  cmp_mermaid_renderer_t *renderer = NULL;

  if (!out_renderer) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_mermaid_renderer_create: Invalid argument\n");
    return rc;
  }

  renderer = (cmp_mermaid_renderer_t *)malloc(sizeof(cmp_mermaid_renderer_t));
  if (!renderer) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_mermaid_renderer_create: Out of memory\n");
    return rc;
  }

  renderer->is_initialized = 1;
  *out_renderer = renderer;
  return rc;
}

int cmp_mermaid_renderer_destroy(cmp_mermaid_renderer_t *renderer) {
  int rc = CMP_SUCCESS;

  if (!renderer) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_mermaid_renderer_destroy: Invalid argument\n");
    return rc;
  }
  free(renderer);
  return rc;
}

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
  svg_out = (char *)malloc(len + 1);
  if (!svg_out) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_mermaid_renderer_generate_svg: Out of memory\n");
    return rc;
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

int cmp_mermaid_renderer_free_svg(char *svg_xml) {
  int rc = CMP_SUCCESS;

  if (svg_xml) {
    free(svg_xml);
  }
  return rc;
}
