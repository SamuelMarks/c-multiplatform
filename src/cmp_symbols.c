/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

typedef struct cmp_symbol_node {
  char *name;
  int weight;
  cmp_symbol_scale_t scale;

  cmp_symbol_rendering_mode_t rendering_mode;
  uint32_t *colors;
  size_t color_count;

  float variable_fill;

  cmp_symbol_animation_t active_animation;
  int is_looping;
} cmp_symbol_node_t;

struct cmp_symbols {
  int is_initialized;
};

int cmp_symbols_create(cmp_symbols_t **out_ctx) {
  struct cmp_symbols *ctx;
  if (!out_ctx)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_symbols), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  ctx->is_initialized = 1;
  *out_ctx = (cmp_symbols_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_symbols_destroy(cmp_symbols_t *ctx) {
  if (ctx)
    CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_symbols_request(cmp_symbols_t *ctx, const char *symbol_name, int weight,
                        cmp_symbol_scale_t scale, void **out_symbol_handle) {
  cmp_symbol_node_t *node;
  size_t len;

  if (!ctx || !symbol_name || !out_symbol_handle)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(cmp_symbol_node_t), (void **)&node) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  len = strlen(symbol_name);
  if (CMP_MALLOC(len + 1, (void **)&node->name) != CMP_SUCCESS) {
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strcpy_s(node->name, len + 1, symbol_name);
#else
  strcpy(node->name, symbol_name);
#endif

  node->weight = weight;
  node->scale = scale;
  node->rendering_mode = CMP_SYMBOL_RENDERING_MONOCHROME;
  node->colors = NULL;
  node->color_count = 0;
  node->variable_fill = 1.0f;
  node->active_animation = CMP_SYMBOL_ANIM_NONE;
  node->is_looping = 0;

  *out_symbol_handle = (void *)node;
  return CMP_SUCCESS;
}

int cmp_symbols_apply_style(cmp_symbols_t *ctx, void *symbol_handle,
                            cmp_symbol_rendering_mode_t mode,
                            const uint32_t *colors, size_t color_count) {
  cmp_symbol_node_t *node = (cmp_symbol_node_t *)symbol_handle;
  if (!ctx || !node || (color_count > 0 && !colors))
    return CMP_ERROR_INVALID_ARG;

  node->rendering_mode = mode;

  if (node->colors) {
    CMP_FREE(node->colors);
    node->colors = NULL;
  }

  if (color_count > 0) {
    if (CMP_MALLOC(color_count * sizeof(uint32_t), (void **)&node->colors) !=
        CMP_SUCCESS)
      return CMP_ERROR_OOM;
    memcpy(node->colors, colors, color_count * sizeof(uint32_t));
  }
  node->color_count = color_count;

  return CMP_SUCCESS;
}

int cmp_symbols_set_variable_value(cmp_symbols_t *ctx, void *symbol_handle,
                                   float percentage) {
  cmp_symbol_node_t *node = (cmp_symbol_node_t *)symbol_handle;
  if (!ctx || !node)
    return CMP_ERROR_INVALID_ARG;

  if (percentage < 0.0f)
    percentage = 0.0f;
  if (percentage > 1.0f)
    percentage = 1.0f;

  node->variable_fill = percentage;
  return CMP_SUCCESS;
}

int cmp_symbols_trigger_animation(cmp_symbols_t *ctx, void *symbol_handle,
                                  cmp_symbol_animation_t animation,
                                  int looping) {
  cmp_symbol_node_t *node = (cmp_symbol_node_t *)symbol_handle;
  if (!ctx || !node)
    return CMP_ERROR_INVALID_ARG;

  node->active_animation = animation;
  node->is_looping = looping;
  return CMP_SUCCESS;
}

int cmp_symbols_load_custom_template(cmp_symbols_t *ctx, const char *svg_path,
                                     void **out_symbol_handle) {
  if (!ctx || !svg_path || !out_symbol_handle)
    return CMP_ERROR_INVALID_ARG;
  /* Simulating loading an external SVG template matching weight boundaries.
     For test mock, just return a generic valid request */
  return cmp_symbols_request(ctx, svg_path, 400, CMP_SYMBOL_SCALE_MEDIUM,
                             out_symbol_handle);
}
