/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_mermaid_renderer {
  int is_initialized;
};

int cmp_mermaid_renderer_create(cmp_mermaid_renderer_t **out_renderer) {
  cmp_mermaid_renderer_t *renderer;
  if (!out_renderer) {
    return CMP_ERROR_INVALID_ARG;
  }

  renderer = (cmp_mermaid_renderer_t *)malloc(sizeof(cmp_mermaid_renderer_t));
  if (!renderer) {
    return CMP_ERROR_OOM;
  }

  renderer->is_initialized = 1;
  *out_renderer = renderer;
  return CMP_SUCCESS;
}

int cmp_mermaid_renderer_destroy(cmp_mermaid_renderer_t *renderer) {
  if (!renderer) {
    return CMP_ERROR_INVALID_ARG;
  }
  free(renderer);
  return CMP_SUCCESS;
}

int cmp_mermaid_renderer_generate_svg(cmp_mermaid_renderer_t *renderer,
                                      const char *mermaid_syntax,
                                      char **out_svg_xml) {
  const char *dummy_svg = "<svg><rect width=\"10\" height=\"10\"/></svg>";
  char *svg_out;
  size_t len;

  if (!renderer || !mermaid_syntax || !out_svg_xml) {
    return CMP_ERROR_INVALID_ARG;
  }

  len = strlen(dummy_svg);
  svg_out = (char *)malloc(len + 1);
  if (!svg_out) {
    return CMP_ERROR_OOM;
  }

  strncpy(svg_out, dummy_svg, len);
  svg_out[len] = '\0';
  *out_svg_xml = svg_out;

  return CMP_SUCCESS;
}

int cmp_mermaid_renderer_free_svg(char *svg_xml) {
  if (svg_xml) {
    free(svg_xml);
  }
  return CMP_SUCCESS;
}
