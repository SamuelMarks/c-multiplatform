/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_grid_ctx_create(cmp_grid_ctx_t **out_ctx) {
  int rc = CMP_SUCCESS;
  cmp_grid_ctx_t *ctx = NULL;

  if (!out_ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_grid_ctx_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_grid_ctx_t), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_grid_ctx_create: Out of memory\n");
    return rc;
  }

  memset(ctx, 0, sizeof(cmp_grid_ctx_t));
  *out_ctx = ctx;
  return rc;
}

int cmp_grid_ctx_destroy(cmp_grid_ctx_t *ctx) {
  int rc = CMP_SUCCESS;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_grid_ctx_destroy: Invalid argument\n");
    return rc;
  }

  if (ctx->template_rows)
    CMP_FREE(ctx->template_rows);
  if (ctx->template_columns)
    CMP_FREE(ctx->template_columns);
  if (ctx->auto_rows)
    CMP_FREE(ctx->auto_rows);
  if (ctx->auto_columns)
    CMP_FREE(ctx->auto_columns);
  if (ctx->template_areas)
    CMP_FREE(ctx->template_areas);
  if (ctx->items)
    CMP_FREE(ctx->items);
  if (ctx->computed_row_sizes)
    CMP_FREE(ctx->computed_row_sizes);
  if (ctx->computed_col_sizes)
    CMP_FREE(ctx->computed_col_sizes);
  CMP_FREE(ctx);
  return rc;
}

int cmp_grid_ctx_add_item(cmp_grid_ctx_t *ctx, cmp_layout_node_t *node,
                          cmp_grid_item_t **out_item) {
  int rc = CMP_SUCCESS;
  size_t new_cap;
  cmp_grid_item_t *new_items = NULL;

  if (!ctx || !node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_grid_ctx_add_item: Invalid argument\n");
    return rc;
  }

  if (ctx->item_count >= ctx->item_capacity) {
    new_cap = ctx->item_capacity == 0 ? 16 : ctx->item_capacity * 2;
    rc = CMP_MALLOC(new_cap * sizeof(cmp_grid_item_t), (void **)&new_items);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_grid_ctx_add_item: Out of memory\n");
      return rc;
    }
    if (ctx->items) {
      memcpy(new_items, ctx->items, ctx->item_count * sizeof(cmp_grid_item_t));
      free(ctx->items);
    }
    ctx->items = new_items;
    ctx->item_capacity = new_cap;
  }

  ctx->items[ctx->item_count].node = node;
  memset(&ctx->items[ctx->item_count], 0, sizeof(cmp_grid_item_t));
  ctx->items[ctx->item_count].row_start.is_auto = 1;
  ctx->items[ctx->item_count].row_end.is_auto = 1;
  ctx->items[ctx->item_count].col_start.is_auto = 1;
  ctx->items[ctx->item_count].col_end.is_auto = 1;

  if (out_item)
    *out_item = &ctx->items[ctx->item_count];

  ctx->item_count++;
  return rc;
}

int cmp_grid_track_evaluate(cmp_grid_track_size_t *track, float container_size,
                            float *out_size) {
  int rc = CMP_SUCCESS;

  if (!track || !out_size) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_grid_track_evaluate: Invalid argument\n");
    return rc;
  }

  switch (track->type) {
  case CMP_GRID_TRACK_FIXED:
    *out_size = track->value;
    break;
  case CMP_GRID_TRACK_PERCENTAGE:
    *out_size = track->value * container_size / 100.0f;
    break;
  case CMP_GRID_TRACK_FR:
  case CMP_GRID_TRACK_AUTO:
  case CMP_GRID_TRACK_MIN_CONTENT:
  case CMP_GRID_TRACK_MAX_CONTENT:
  case CMP_GRID_TRACK_FIT_CONTENT:
    *out_size = 0; /* Base size zero, flex factor handles the rest */
    break;
  case CMP_GRID_TRACK_MINMAX:
    *out_size = track->min_value;
    break;
  default:
    *out_size = 0;
  }

  return rc;
}

int cmp_grid_fr_distribute(cmp_grid_ctx_t *ctx, float available_width,
                           float available_height) {
  int rc = CMP_SUCCESS;
  float total_fr_cols = 0.0f;
  float total_fr_rows = 0.0f;
  float free_width = available_width;
  float free_height = available_height;
  size_t i;
  float sz = 0.0f;
  float fr_width = 0.0f;
  float fr_height = 0.0f;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_grid_fr_distribute: Invalid argument\n");
    return rc;
  }

  for (i = 0; i < ctx->template_columns_count; i++) {
    if (ctx->template_columns[i].type == CMP_GRID_TRACK_FR) {
      total_fr_cols += ctx->template_columns[i].value;
    } else {
      sz = 0;
      rc = cmp_grid_track_evaluate(&ctx->template_columns[i], available_width,
                                   &sz);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG(
            "Error in cmp_grid_fr_distribute: track col evaluate failed\n");
        return rc;
      }
      free_width -= sz;
    }
  }

  for (i = 0; i < ctx->template_rows_count; i++) {
    if (ctx->template_rows[i].type == CMP_GRID_TRACK_FR) {
      total_fr_rows += ctx->template_rows[i].value;
    } else {
      sz = 0;
      rc = cmp_grid_track_evaluate(&ctx->template_rows[i], available_height,
                                   &sz);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG(
            "Error in cmp_grid_fr_distribute: track row evaluate failed\n");
        return rc;
      }
      free_height -= sz;
    }
  }

  if (total_fr_cols > 0 && free_width > 0) {
    fr_width = free_width / total_fr_cols;
    for (i = 0; i < ctx->template_columns_count; i++) {
      if (ctx->template_columns[i].type == CMP_GRID_TRACK_FR) {
        ctx->computed_col_sizes[i] = fr_width * ctx->template_columns[i].value;
      }
    }
  }

  if (total_fr_rows > 0 && free_height > 0) {
    fr_height = free_height / total_fr_rows;
    for (i = 0; i < ctx->template_rows_count; i++) {
      if (ctx->template_rows[i].type == CMP_GRID_TRACK_FR) {
        ctx->computed_row_sizes[i] = fr_height * ctx->template_rows[i].value;
      }
    }
  }
  return rc;
}

int cmp_grid_minmax_resolve(cmp_grid_track_size_t *track, float container_size,
                            float *out_size) {
  int rc = CMP_SUCCESS;
  (void)container_size;

  if (!track || !out_size) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_grid_minmax_resolve: Invalid argument\n");
    return rc;
  }

  *out_size = track->min_value;
  if (*out_size < track->max_value) {
    *out_size = track->max_value; /* Simplified */
  }

  return rc;
}

int cmp_grid_repeat_expand(cmp_grid_track_size_t *track, int auto_fit,
                           float container_size, int *out_count) {
  int rc = CMP_SUCCESS;
  (void)auto_fit;

  if (!track || !out_count) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_grid_repeat_expand: Invalid argument\n");
    return rc;
  }

  if (track->value > 0) {
    *out_count = (int)(container_size / track->value);
    if (*out_count == 0)
      *out_count = 1;
  } else {
    *out_count = 1;
  }

  return rc;
}

int cmp_grid_placement_resolve(cmp_grid_placement_t *start,
                               cmp_grid_placement_t *end, int track_count,
                               int *out_start, int *out_end) {
  int rc = CMP_SUCCESS;

  if (!start || !end || !out_start || !out_end) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_grid_placement_resolve: Invalid argument\n");
    return rc;
  }

  *out_start = start->is_auto ? 1 : start->line;
  if (*out_start < 0)
    *out_start = track_count + 1 + *out_start;

  *out_end =
      end->is_auto ? (*out_start + (end->span > 0 ? end->span : 1)) : end->line;
  if (*out_end < 0)
    *out_end = track_count + 1 + *out_end;

  return rc;
}

int cmp_grid_area_resolve(cmp_grid_ctx_t *ctx, const char *name,
                          int *out_row_start, int *out_col_start,
                          int *out_row_end, int *out_col_end) {
  int rc = CMP_SUCCESS;
  size_t i;

  if (!ctx || !name || !out_row_start || !out_col_start || !out_row_end ||
      !out_col_end) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_grid_area_resolve: Invalid argument\n");
    return rc;
  }

  for (i = 0; i < ctx->template_areas_count; i++) {
    if (ctx->template_areas[i].name &&
        strcmp(ctx->template_areas[i].name, name) == 0) {
      *out_row_start = ctx->template_areas[i].row_start;
      *out_col_start = ctx->template_areas[i].col_start;
      *out_row_end = ctx->template_areas[i].row_end;
      *out_col_end = ctx->template_areas[i].col_end;
      return rc;
    }
  }

  rc = CMP_ERROR_NOT_FOUND;
  LOG_DEBUG("Error in cmp_grid_area_resolve: Area not found\n");
  return rc;
}

int cmp_grid_auto_dense_place(cmp_grid_ctx_t *ctx) {
  int rc = CMP_SUCCESS;
  size_t i;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_grid_auto_dense_place: Invalid argument\n");
    return rc;
  }

  for (i = 0; i < ctx->item_count; i++) {
    if (ctx->items[i].row_start.is_auto && ctx->items[i].col_start.is_auto) {
      ctx->items[i].resolved_row_start = 1 + (int)i;
      ctx->items[i].resolved_row_end = 2 + (int)i;
      ctx->items[i].resolved_col_start = 1;
      ctx->items[i].resolved_col_end = 2;
    }
  }

  return rc;
}

int cmp_grid_auto_sparse_place(cmp_grid_ctx_t *ctx) {
  int rc = CMP_SUCCESS;
  size_t i;
  int current_row = 1;
  int current_col = 1;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_grid_auto_sparse_place: Invalid argument\n");
    return rc;
  }

  for (i = 0; i < ctx->item_count; i++) {
    if (ctx->items[i].row_start.is_auto && ctx->items[i].col_start.is_auto) {
      ctx->items[i].resolved_row_start = current_row;
      ctx->items[i].resolved_row_end = current_row + 1;
      ctx->items[i].resolved_col_start = current_col;
      ctx->items[i].resolved_col_end = current_col + 1;
      current_col++;
      if (ctx->template_columns_count > 0 &&
          (size_t)current_col > ctx->template_columns_count) {
        current_col = 1;
        current_row++;
      }
    }
  }

  return rc;
}

int cmp_grid_implicit_tracks_generate(cmp_grid_ctx_t *ctx) {
  int rc = CMP_SUCCESS;
  size_t i;
  int max_row = 0;
  int max_col = 0;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_grid_implicit_tracks_generate: Invalid argument\n");
    return rc;
  }

  for (i = 0; i < ctx->item_count; i++) {
    if (ctx->items[i].resolved_row_end - 1 > max_row)
      max_row = ctx->items[i].resolved_row_end - 1;
    if (ctx->items[i].resolved_col_end - 1 > max_col)
      max_col = ctx->items[i].resolved_col_end - 1;
  }

  ctx->computed_row_count = (size_t)max_row > ctx->template_rows_count
                                ? (size_t)max_row
                                : ctx->template_rows_count;
  ctx->computed_col_count = (size_t)max_col > ctx->template_columns_count
                                ? (size_t)max_col
                                : ctx->template_columns_count;

  if (ctx->computed_row_count > 0) {
    rc = CMP_MALLOC(ctx->computed_row_count * sizeof(float),
                    (void **)&ctx->computed_row_sizes);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_grid_implicit_tracks_generate: Out of memory\n");
      return rc;
    }
    memset(ctx->computed_row_sizes, 0, ctx->computed_row_count * sizeof(float));
  }

  if (ctx->computed_col_count > 0) {
    rc = CMP_MALLOC(ctx->computed_col_count * sizeof(float),
                    (void **)&ctx->computed_col_sizes);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_grid_implicit_tracks_generate: Out of memory\n");
      return rc;
    }
    memset(ctx->computed_col_sizes, 0, ctx->computed_col_count * sizeof(float));
  }

  return rc;
}

int cmp_subgrid_sync(cmp_grid_ctx_t *parent, cmp_grid_ctx_t *child) {
  int rc = CMP_SUCCESS;

  if (!parent || !child) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_subgrid_sync: Invalid argument\n");
    return rc;
  }

  child->computed_row_count = parent->computed_row_count;
  child->computed_col_count = parent->computed_col_count;
  /* Sync tracks from parent to child */
  return rc;
}

int cmp_grid_align_evaluate(cmp_grid_align_t align, float track_size,
                            float item_size, float *out_offset) {
  int rc = CMP_SUCCESS;

  if (!out_offset) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_grid_align_evaluate: Invalid argument\n");
    return rc;
  }

  switch (align) {
  case CMP_GRID_ALIGN_START:
    *out_offset = 0.0f;
    break;
  case CMP_GRID_ALIGN_END:
    *out_offset = track_size - item_size;
    break;
  case CMP_GRID_ALIGN_CENTER:
    *out_offset = (track_size - item_size) / 2.0f;
    break;
  case CMP_GRID_ALIGN_STRETCH:
    *out_offset = 0.0f;
    break;
  default:
    *out_offset = 0.0f;
  }
  return rc;
}

int cmp_masonry_layout(cmp_grid_ctx_t *ctx) {
  int rc = CMP_SUCCESS;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_masonry_layout: Invalid argument\n");
    return rc;
  }

  /* Simplified masonry layout placeholder */
  return rc;
}

int cmp_grid_gap_apply(cmp_grid_ctx_t *ctx, float *out_row_gaps,
                       float *out_col_gaps) {
  int rc = CMP_SUCCESS;

  if (!ctx || !out_row_gaps || !out_col_gaps) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_grid_gap_apply: Invalid argument\n");
    return rc;
  }

  *out_row_gaps = ctx->row_gap;
  *out_col_gaps = ctx->column_gap;
  return rc;
}
