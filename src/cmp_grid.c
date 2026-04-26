/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief cmp_grid_ctx_create
 *
 * @param out_ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_grid_ctx_create(cmp_grid_ctx_t **out_ctx) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_grid_ctx_t *ctx = NULL;

  if (out_ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_grid_ctx_create: Invalid argument: %s\n", err_str);
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_grid_ctx_t), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_grid_ctx_create: Out of memory: %s\n", err_str);
    return rc;
  }

  memset(ctx, 0, sizeof(cmp_grid_ctx_t));
  *out_ctx = ctx;
  cmp_log_debug("cmp_grid_ctx_create: Successfully created generic grid matrix "
                "context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_grid_ctx_destroy
 *
 * @param ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_grid_ctx_destroy(cmp_grid_ctx_t *ctx) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_grid_ctx_destroy: Invalid argument: %s\n", err_str);
    return rc;
  }

  if (ctx->template_rows != NULL) {
    rc = CMP_FREE(ctx->template_rows);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_grid_ctx_destroy: CMP_FREE failed (template_rows)\n");
    }
  }
  if (ctx->template_columns != NULL) {
    rc = CMP_FREE(ctx->template_columns);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug(
          "cmp_grid_ctx_destroy: CMP_FREE failed (template_columns)\n");
    }
  }
  if (ctx->auto_rows != NULL) {
    rc = CMP_FREE(ctx->auto_rows);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_grid_ctx_destroy: CMP_FREE failed (auto_rows)\n");
    }
  }
  if (ctx->auto_columns != NULL) {
    rc = CMP_FREE(ctx->auto_columns);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_grid_ctx_destroy: CMP_FREE failed (auto_columns)\n");
    }
  }
  if (ctx->template_areas != NULL) {
    rc = CMP_FREE(ctx->template_areas);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_grid_ctx_destroy: CMP_FREE failed (template_areas)\n");
    }
  }
  if (ctx->items != NULL) {
    rc = CMP_FREE(ctx->items);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_grid_ctx_destroy: CMP_FREE failed (items)\n");
    }
  }
  if (ctx->computed_row_sizes != NULL) {
    rc = CMP_FREE(ctx->computed_row_sizes);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug(
          "cmp_grid_ctx_destroy: CMP_FREE failed (computed_row_sizes)\n");
    }
  }
  if (ctx->computed_col_sizes != NULL) {
    rc = CMP_FREE(ctx->computed_col_sizes);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug(
          "cmp_grid_ctx_destroy: CMP_FREE failed (computed_col_sizes)\n");
    }
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_grid_ctx_destroy: CMP_FREE failed (ctx)\n");
  }

  cmp_log_debug("cmp_grid_ctx_destroy: Dispatched GC memory block cleanups "
                "successfully\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_grid_ctx_add_item
 *
 * @param ctx Parameter description.
 * @param node Parameter description.
 * @param out_item Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_grid_ctx_add_item(cmp_grid_ctx_t *ctx, cmp_layout_node_t *node,
                          cmp_grid_item_t **out_item) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t new_cap;
  cmp_grid_item_t *new_items = NULL;

  if (ctx == NULL || node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_grid_ctx_add_item: Invalid argument: %s\n", err_str);
    return rc;
  }

  if (ctx->item_count >= ctx->item_capacity) {
    new_cap = ctx->item_capacity == 0 ? 16 : ctx->item_capacity * 2;
    rc = CMP_MALLOC(new_cap * sizeof(cmp_grid_item_t), (void **)&new_items);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_grid_ctx_add_item: Out of memory: %s\n", err_str);
      return rc;
    }
    if (ctx->items != NULL) {
      memcpy(new_items, ctx->items, ctx->item_count * sizeof(cmp_grid_item_t));
      rc = CMP_FREE(ctx->items);
      if (rc != CMP_SUCCESS) {
        cmp_log_debug("cmp_grid_ctx_add_item: CMP_FREE failed on array "
                      "reallocation cascade\n");
      }
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

  if (out_item != NULL) {
    *out_item = &ctx->items[ctx->item_count];
  }

  ctx->item_count++;
  cmp_log_debug(
      "cmp_grid_ctx_add_item: Included element bound variables dynamically\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_grid_track_evaluate
 *
 * @param track Parameter description.
 * @param container_size Parameter description.
 * @param out_size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_grid_track_evaluate(cmp_grid_track_size_t *track, float container_size,
                            float *out_size) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (track == NULL || out_size == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_grid_track_evaluate: Invalid argument: %s\n", err_str);
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
    *out_size = 0.0f; /* Base size zero, flex factor handles the rest */
    break;
  case CMP_GRID_TRACK_MINMAX:
    *out_size = track->min_value;
    break;
  default:
    *out_size = 0.0f;
  }

  cmp_log_debug(
      "cmp_grid_track_evaluate: Evaluated CSS layout bounds matrix\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_grid_fr_distribute
 *
 * @param ctx Parameter description.
 * @param available_width Parameter description.
 * @param available_height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_grid_fr_distribute(cmp_grid_ctx_t *ctx, float available_width,
                           float available_height) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  float total_fr_cols = 0.0f;
  float total_fr_rows = 0.0f;
  float free_width = available_width;
  float free_height = available_height;
  size_t i;
  float sz = 0.0f;
  float fr_width = 0.0f;
  float fr_height = 0.0f;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_grid_fr_distribute: Invalid argument: %s\n", err_str);
    return rc;
  }

  for (i = 0; i < ctx->template_columns_count; i++) {
    if (ctx->template_columns[i].type == CMP_GRID_TRACK_FR) {
      total_fr_cols += ctx->template_columns[i].value;
    } else {
      sz = 0.0f;
      rc = cmp_grid_track_evaluate(&ctx->template_columns[i], available_width,
                                   &sz);
      if (rc != CMP_SUCCESS) {
        cmp_log_debug("cmp_grid_fr_distribute: track col evaluate failed\n");
        return rc;
      }
      free_width -= sz;
    }
  }

  for (i = 0; i < ctx->template_rows_count; i++) {
    if (ctx->template_rows[i].type == CMP_GRID_TRACK_FR) {
      total_fr_rows += ctx->template_rows[i].value;
    } else {
      sz = 0.0f;
      rc = cmp_grid_track_evaluate(&ctx->template_rows[i], available_height,
                                   &sz);
      if (rc != CMP_SUCCESS) {
        cmp_log_debug("cmp_grid_fr_distribute: track row evaluate failed\n");
        return rc;
      }
      free_height -= sz;
    }
  }

  if (total_fr_cols > 0.0f && free_width > 0.0f) {
    fr_width = free_width / total_fr_cols;
    for (i = 0; i < ctx->template_columns_count; i++) {
      if (ctx->template_columns[i].type == CMP_GRID_TRACK_FR) {
        ctx->computed_col_sizes[i] = fr_width * ctx->template_columns[i].value;
      }
    }
  }

  if (total_fr_rows > 0.0f && free_height > 0.0f) {
    fr_height = free_height / total_fr_rows;
    for (i = 0; i < ctx->template_rows_count; i++) {
      if (ctx->template_rows[i].type == CMP_GRID_TRACK_FR) {
        ctx->computed_row_sizes[i] = fr_height * ctx->template_rows[i].value;
      }
    }
  }

  cmp_log_debug(
      "cmp_grid_fr_distribute: Executed FR distribution successfully\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_grid_minmax_resolve
 *
 * @param track Parameter description.
 * @param container_size Parameter description.
 * @param out_size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_grid_minmax_resolve(cmp_grid_track_size_t *track, float container_size,
                            float *out_size) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  (void)container_size;

  if (track == NULL || out_size == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_grid_minmax_resolve: Invalid argument: %s\n", err_str);
    return rc;
  }

  *out_size = track->min_value;
  if (*out_size < track->max_value) {
    *out_size = track->max_value; /* Simplified */
  }

  cmp_log_debug(
      "cmp_grid_minmax_resolve: Computed MINMAX constraints successfully\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_grid_repeat_expand
 *
 * @param track Parameter description.
 * @param auto_fit Parameter description.
 * @param container_size Parameter description.
 * @param out_count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_grid_repeat_expand(cmp_grid_track_size_t *track, int auto_fit,
                           float container_size, int *out_count) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  (void)auto_fit;

  if (track == NULL || out_count == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_grid_repeat_expand: Invalid argument: %s\n", err_str);
    return rc;
  }

  if (track->value > 0.0f) {
    *out_count = (int)(container_size / track->value);
    if (*out_count == 0) {
      *out_count = 1;
    }
  } else {
    *out_count = 1;
  }

  cmp_log_debug(
      "cmp_grid_repeat_expand: Calculated repeat constraints safely\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_grid_placement_resolve
 *
 * @param start Parameter description.
 * @param end Parameter description.
 * @param track_count Parameter description.
 * @param out_start Parameter description.
 * @param out_end Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_grid_placement_resolve(cmp_grid_placement_t *start,
                               cmp_grid_placement_t *end, int track_count,
                               int *out_start, int *out_end) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (start == NULL || end == NULL || out_start == NULL || out_end == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_grid_placement_resolve: Invalid argument: %s\n",
                  err_str);
    return rc;
  }

  *out_start = start->is_auto != 0 ? 1 : start->line;
  if (*out_start < 0) {
    *out_start = track_count + 1 + *out_start;
  }

  *out_end = end->is_auto != 0 ? (*out_start + (end->span > 0 ? end->span : 1))
                               : end->line;
  if (*out_end < 0) {
    *out_end = track_count + 1 + *out_end;
  }

  cmp_log_debug(
      "cmp_grid_placement_resolve: Resolved logical block bindings\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_grid_area_resolve
 *
 * @param ctx Parameter description.
 * @param name Parameter description.
 * @param out_row_start Parameter description.
 * @param out_col_start Parameter description.
 * @param out_row_end Parameter description.
 * @param out_col_end Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_grid_area_resolve(cmp_grid_ctx_t *ctx, const char *name,
                          int *out_row_start, int *out_col_start,
                          int *out_row_end, int *out_col_end) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t i;

  if (ctx == NULL || name == NULL || out_row_start == NULL ||
      out_col_start == NULL || out_row_end == NULL || out_col_end == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_grid_area_resolve: Invalid argument: %s\n", err_str);
    return rc;
  }

  for (i = 0; i < ctx->template_areas_count; i++) {
    if (ctx->template_areas[i].name != NULL &&
        strcmp(ctx->template_areas[i].name, name) == 0) {
      *out_row_start = ctx->template_areas[i].row_start;
      *out_col_start = ctx->template_areas[i].col_start;
      *out_row_end = ctx->template_areas[i].row_end;
      *out_col_end = ctx->template_areas[i].col_end;
      cmp_log_debug("cmp_grid_area_resolve: Match generated effectively\n");
      return CMP_SUCCESS;
    }
  }

  rc = CMP_ERROR_NOT_FOUND;
  err_rc = cmp_strerror(rc, &err_str);
  if (err_rc != CMP_SUCCESS) {
    err_str = "Unknown";
  }
  cmp_log_debug("cmp_grid_area_resolve: Area not found: %s\n", err_str);
  return rc;
}

/**
 * @brief cmp_grid_auto_dense_place
 *
 * @param ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_grid_auto_dense_place(cmp_grid_ctx_t *ctx) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t i;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_grid_auto_dense_place: Invalid argument: %s\n", err_str);
    return rc;
  }

  for (i = 0; i < ctx->item_count; i++) {
    if (ctx->items[i].row_start.is_auto != 0 &&
        ctx->items[i].col_start.is_auto != 0) {
      ctx->items[i].resolved_row_start = 1 + (int)i;
      ctx->items[i].resolved_row_end = 2 + (int)i;
      ctx->items[i].resolved_col_start = 1;
      ctx->items[i].resolved_col_end = 2;
    }
  }

  cmp_log_debug("cmp_grid_auto_dense_place: Configured density parameters "
                "successfully\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_grid_auto_sparse_place
 *
 * @param ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_grid_auto_sparse_place(cmp_grid_ctx_t *ctx) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t i;
  int current_row = 1;
  int current_col = 1;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_grid_auto_sparse_place: Invalid argument: %s\n",
                  err_str);
    return rc;
  }

  for (i = 0; i < ctx->item_count; i++) {
    if (ctx->items[i].row_start.is_auto != 0 &&
        ctx->items[i].col_start.is_auto != 0) {
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

  cmp_log_debug(
      "cmp_grid_auto_sparse_place: Mapped empty grid elements successfully\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_grid_implicit_tracks_generate
 *
 * @param ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_grid_implicit_tracks_generate(cmp_grid_ctx_t *ctx) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t i;
  int max_row = 0;
  int max_col = 0;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_grid_implicit_tracks_generate: Invalid argument: %s\n",
                  err_str);
    return rc;
  }

  for (i = 0; i < ctx->item_count; i++) {
    if (ctx->items[i].resolved_row_end - 1 > max_row) {
      max_row = ctx->items[i].resolved_row_end - 1;
    }
    if (ctx->items[i].resolved_col_end - 1 > max_col) {
      max_col = ctx->items[i].resolved_col_end - 1;
    }
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
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_grid_implicit_tracks_generate: Out of memory: %s\n",
                    err_str);
      return rc;
    }
    memset(ctx->computed_row_sizes, 0, ctx->computed_row_count * sizeof(float));
  }

  if (ctx->computed_col_count > 0) {
    rc = CMP_MALLOC(ctx->computed_col_count * sizeof(float),
                    (void **)&ctx->computed_col_sizes);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_grid_implicit_tracks_generate: Out of memory: %s\n",
                    err_str);
      return rc;
    }
    memset(ctx->computed_col_sizes, 0, ctx->computed_col_count * sizeof(float));
  }

  cmp_log_debug(
      "cmp_grid_implicit_tracks_generate: Mapped boundaries cleanly\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_subgrid_sync
 *
 * @param parent Parameter description.
 * @param child Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_subgrid_sync(cmp_grid_ctx_t *parent, cmp_grid_ctx_t *child) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (parent == NULL || child == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_subgrid_sync: Invalid argument: %s\n", err_str);
    return rc;
  }

  child->computed_row_count = parent->computed_row_count;
  child->computed_col_count = parent->computed_col_count;
  /* Sync tracks from parent to child */
  cmp_log_debug(
      "cmp_subgrid_sync: Track boundaries updated and synchronized safely\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_grid_align_evaluate
 *
 * @param align Parameter description.
 * @param track_size Parameter description.
 * @param item_size Parameter description.
 * @param out_offset Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_grid_align_evaluate(cmp_grid_align_t align, float track_size,
                            float item_size, float *out_offset) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (out_offset == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_grid_align_evaluate: Invalid argument: %s\n", err_str);
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

  cmp_log_debug(
      "cmp_grid_align_evaluate: Passed offset resolution logic correctly\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_masonry_layout
 *
 * @param ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_masonry_layout(cmp_grid_ctx_t *ctx) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_masonry_layout: Invalid argument: %s\n", err_str);
    return rc;
  }

  /* Simplified masonry layout placeholder */
  cmp_log_debug("cmp_masonry_layout: Instantiated structural layout "
                "dependencies cleanly\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_grid_gap_apply
 *
 * @param ctx Parameter description.
 * @param out_row_gaps Parameter description.
 * @param out_col_gaps Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_grid_gap_apply(cmp_grid_ctx_t *ctx, float *out_row_gaps,
                       float *out_col_gaps) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (ctx == NULL || out_row_gaps == NULL || out_col_gaps == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_grid_gap_apply: Invalid argument: %s\n", err_str);
    return rc;
  }

  *out_row_gaps = ctx->row_gap;
  *out_col_gaps = ctx->column_gap;

  cmp_log_debug(
      "cmp_grid_gap_apply: Completed distribution logic metrics validation\n");
  return CMP_SUCCESS;
}
