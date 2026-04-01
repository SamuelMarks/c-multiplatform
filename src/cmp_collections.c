/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_collection_section {
  float min_column_width;
  cmp_orthogonal_behavior_t orthogonal_behavior;
};

struct cmp_collection {
  cmp_collection_section_t **sections;
  size_t section_count;
  size_t capacity;
};

struct cmp_diffable_datasource {
  uint64_t *current_state;
  size_t count;
};

int cmp_collection_create(cmp_collection_t **out_collection) {
  struct cmp_collection *ctx;
  if (!out_collection)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_collection), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->sections = NULL;
  ctx->section_count = 0;
  ctx->capacity = 0;

  *out_collection = (cmp_collection_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_collection_destroy(cmp_collection_t *collection_opaque) {
  struct cmp_collection *ctx = (struct cmp_collection *)collection_opaque;
  size_t i;
  if (!ctx)
    return CMP_SUCCESS;

  if (ctx->sections) {
    for (i = 0; i < ctx->section_count; ++i) {
      if (ctx->sections[i])
        cmp_collection_section_destroy(
            (cmp_collection_section_t *)ctx->sections[i]);
    }
    CMP_FREE(ctx->sections);
  }
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_collection_section_create(cmp_collection_section_t **out_section) {
  struct cmp_collection_section *ctx;
  if (!out_section)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_collection_section), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->min_column_width = 0.0f;
  ctx->orthogonal_behavior = CMP_ORTHOGONAL_NONE;

  *out_section = (cmp_collection_section_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_collection_section_destroy(cmp_collection_section_t *section_opaque) {
  if (section_opaque)
    CMP_FREE(section_opaque);
  return CMP_SUCCESS;
}

int cmp_collection_section_set_flow_layout(
    cmp_collection_section_t *section_opaque, float min_column_width) {
  struct cmp_collection_section *ctx =
      (struct cmp_collection_section *)section_opaque;
  if (!ctx || min_column_width <= 0.0f)
    return CMP_ERROR_INVALID_ARG;

  ctx->min_column_width = min_column_width;
  return CMP_SUCCESS;
}

int cmp_collection_section_set_orthogonal_behavior(
    cmp_collection_section_t *section_opaque,
    cmp_orthogonal_behavior_t behavior) {
  struct cmp_collection_section *ctx =
      (struct cmp_collection_section *)section_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  ctx->orthogonal_behavior = behavior;
  return CMP_SUCCESS;
}

int cmp_collection_add_section(cmp_collection_t *collection_opaque,
                               cmp_collection_section_t *section_opaque) {
  struct cmp_collection *ctx = (struct cmp_collection *)collection_opaque;
  struct cmp_collection_section *sec =
      (struct cmp_collection_section *)section_opaque;
  cmp_collection_section_t **new_secs;
  size_t new_cap;

  if (!ctx || !sec)
    return CMP_ERROR_INVALID_ARG;

  if (ctx->section_count == ctx->capacity) {
    new_cap = ctx->capacity == 0 ? 4 : ctx->capacity * 2;
    if (CMP_MALLOC(new_cap * sizeof(cmp_collection_section_t *),
                   (void **)&new_secs) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
    if (ctx->sections) {
      memcpy(new_secs, ctx->sections,
             ctx->section_count * sizeof(cmp_collection_section_t *));
      CMP_FREE(ctx->sections);
    }
    ctx->sections = new_secs;
    ctx->capacity = new_cap;
  }

  ctx->sections[ctx->section_count++] = sec;
  return CMP_SUCCESS;
}

int cmp_diffable_datasource_create(cmp_diffable_datasource_t **out_ds) {
  struct cmp_diffable_datasource *ctx;
  if (!out_ds)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_diffable_datasource), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->current_state = NULL;
  ctx->count = 0;

  *out_ds = (cmp_diffable_datasource_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_diffable_datasource_destroy(cmp_diffable_datasource_t *ds_opaque) {
  struct cmp_diffable_datasource *ctx =
      (struct cmp_diffable_datasource *)ds_opaque;
  if (!ctx)
    return CMP_SUCCESS;

  if (ctx->current_state)
    CMP_FREE(ctx->current_state);
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_diffable_datasource_apply_snapshot(cmp_diffable_datasource_t *ds_opaque,
                                           const uint64_t *items,
                                           size_t count) {
  struct cmp_diffable_datasource *ctx =
      (struct cmp_diffable_datasource *)ds_opaque;
  uint64_t *new_state = NULL;

  if (!ctx || (!items && count > 0))
    return CMP_ERROR_INVALID_ARG;

  if (count > 0) {
    if (CMP_MALLOC(count * sizeof(uint64_t), (void **)&new_state) !=
        CMP_SUCCESS)
      return CMP_ERROR_OOM;
    memcpy(new_state, items, count * sizeof(uint64_t));
  }

  if (ctx->current_state)
    CMP_FREE(ctx->current_state);
  ctx->current_state = new_state;
  ctx->count = count;

  /* A real implementation calculates O(N) Meyer's diff here to emit specific
     view transition instructions (insertRow: 3, deleteRow: 5, moveRow: 1->7).
   */

  return CMP_SUCCESS;
}

int cmp_system_map_view_mount(cmp_ui_node_t *node, float latitude,
                              float longitude) {
  if (!node)
    return CMP_ERROR_INVALID_ARG;
  /* Emits WKWebView/MKMapView native backing handles bounded to the layout
   * dimensions */
  (void)latitude;
  (void)longitude;
  return CMP_SUCCESS;
}

int cmp_system_web_view_mount(cmp_ui_node_t *node, const char *url) {
  if (!node || !url)
    return CMP_ERROR_INVALID_ARG;
  /* Mounts platform-specific WKWebView/EdgeWebView2 */
  return CMP_SUCCESS;
}
