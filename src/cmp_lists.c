/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

typedef struct cmp_swipe_action {
  char *title;
  cmp_swipe_action_style_t style;
  int is_leading;
  int allows_continuous;
} cmp_swipe_action_t;

struct cmp_list_row {
  char *title;
  float separator_inset_left;
  cmp_swipe_action_t *actions;
  size_t action_count;
};

struct cmp_list {
  cmp_list_style_t style;
  int is_editing;
  cmp_list_row_t **rows;
  size_t row_count;
  size_t row_capacity;
};

/**
 * @brief cmp_list_create
 *
 * @param out_list Parameter description.
 * @param style Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_list_create(cmp_list_t **out_list, cmp_list_style_t style) {
  int rc = CMP_SUCCESS;
  struct cmp_list *ctx = NULL;

  if (!out_list) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_list_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_list), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_list_create: Out of memory\n");
    return rc;
  }

  ctx->style = style;
  ctx->is_editing = 0;
  ctx->rows = NULL;
  ctx->row_count = 0;
  ctx->row_capacity = 0;

  *out_list = (cmp_list_t *)ctx;
  return rc;
}

/**
 * @brief cmp_list_destroy
 *
 * @param list_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_list_destroy(cmp_list_t *list_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_list *ctx = (struct cmp_list *)list_opaque;
  size_t i;

  if (!ctx) {
    return rc;
  }

  if (ctx->rows) {
    for (i = 0; i < ctx->row_count; ++i) {
      if (ctx->rows[i]) {
        rc = cmp_list_row_destroy((cmp_list_row_t *)ctx->rows[i]);
        if (rc != CMP_SUCCESS) {
          LOG_DEBUG("Error in cmp_list_destroy: cmp_list_row_destroy failed\n");
        }
      }
    }
    CMP_FREE(ctx->rows);
  }
  CMP_FREE(ctx);
  return rc;
}

/**
 * @brief cmp_list_add_row
 *
 * @param list_opaque Parameter description.
 * @param row_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_list_add_row(cmp_list_t *list_opaque, cmp_list_row_t *row_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_list *ctx = (struct cmp_list *)list_opaque;
  struct cmp_list_row *row = (struct cmp_list_row *)row_opaque;
  cmp_list_row_t **new_rows = NULL;
  size_t new_cap;

  if (!ctx || !row) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_list_add_row: Invalid argument\n");
    return rc;
  }

  if (ctx->row_count == ctx->row_capacity) {
    new_cap = ctx->row_capacity == 0 ? 8 : ctx->row_capacity * 2;
    rc = CMP_MALLOC(new_cap * sizeof(cmp_list_row_t *), (void **)&new_rows);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_list_add_row: Out of memory allocating rows\n");
      return rc;
    }
    if (ctx->rows) {
      memcpy(new_rows, ctx->rows, ctx->row_count * sizeof(cmp_list_row_t *));
      CMP_FREE(ctx->rows);
    }
    ctx->rows = new_rows;
    ctx->row_capacity = new_cap;
  }

  ctx->rows[ctx->row_count++] = row;
  return rc;
}

/**
 * @brief cmp_list_row_create
 *
 * @param out_row Parameter description.
 * @param title Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_list_row_create(cmp_list_row_t **out_row, const char *title) {
  int rc = CMP_SUCCESS;
  struct cmp_list_row *ctx = NULL;
  size_t len;

  if (!out_row || !title) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_list_row_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_list_row), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_list_row_create: Out of memory\n");
    return rc;
  }

  ctx->separator_inset_left = 16.0f; /* Default HIG left margin */
  ctx->actions = NULL;
  ctx->action_count = 0;

  len = strlen(title);
  rc = CMP_MALLOC(len + 1, (void **)&ctx->title);
  if (rc != CMP_SUCCESS) {
    CMP_FREE(ctx);
    LOG_DEBUG("Error in cmp_list_row_create: Out of memory allocating title\n");
    return rc;
  }
#if defined(_MSC_VER)
  strcpy_s(ctx->title, len + 1, title);
#else
  strcpy(ctx->title, title);
#endif

  *out_row = (cmp_list_row_t *)ctx;
  return rc;
}

/**
 * @brief cmp_list_row_destroy
 *
 * @param row_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_list_row_destroy(cmp_list_row_t *row_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_list_row *ctx = (struct cmp_list_row *)row_opaque;
  size_t i;

  if (!ctx) {
    return rc;
  }

  if (ctx->title)
    CMP_FREE(ctx->title);
  if (ctx->actions) {
    for (i = 0; i < ctx->action_count; ++i) {
      if (ctx->actions[i].title)
        CMP_FREE(ctx->actions[i].title);
    }
    CMP_FREE(ctx->actions);
  }
  CMP_FREE(ctx);
  return rc;
}

/**
 * @brief cmp_list_row_set_separator_inset
 *
 * @param row_opaque Parameter description.
 * @param inset_left Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_list_row_set_separator_inset(cmp_list_row_t *row_opaque,
                                     float inset_left) {
  int rc = CMP_SUCCESS;
  struct cmp_list_row *ctx = (struct cmp_list_row *)row_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_list_row_set_separator_inset: Invalid argument\n");
    return rc;
  }
  ctx->separator_inset_left = inset_left;
  return rc;
}

/**
 * @brief cmp_list_row_add_swipe_action
 *
 * @param row_opaque Parameter description.
 * @param is_leading Parameter description.
 * @param title Parameter description.
 * @param style Parameter description.
 * @param allows_continuous Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_list_row_add_swipe_action(cmp_list_row_t *row_opaque, int is_leading,
                                  const char *title,
                                  cmp_swipe_action_style_t style,
                                  int allows_continuous) {
  int rc = CMP_SUCCESS;
  struct cmp_list_row *ctx = (struct cmp_list_row *)row_opaque;
  cmp_swipe_action_t *new_actions = NULL;
  size_t len;

  if (!ctx || !title) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_list_row_add_swipe_action: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC((ctx->action_count + 1) * sizeof(cmp_swipe_action_t),
                  (void **)&new_actions);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_list_row_add_swipe_action: Out of memory actions "
              "array\n");
    return rc;
  }

  if (ctx->actions) {
    memcpy(new_actions, ctx->actions,
           ctx->action_count * sizeof(cmp_swipe_action_t));
    CMP_FREE(ctx->actions);
  }
  ctx->actions = new_actions;

  len = strlen(title);
  rc = CMP_MALLOC(len + 1, (void **)&ctx->actions[ctx->action_count].title);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_list_row_add_swipe_action: Out of memory title\n");
    return rc;
  }
#if defined(_MSC_VER)
  strcpy_s(ctx->actions[ctx->action_count].title, len + 1, title);
#else
  strcpy(ctx->actions[ctx->action_count].title, title);
#endif

  ctx->actions[ctx->action_count].style = style;
  ctx->actions[ctx->action_count].is_leading = is_leading;
  ctx->actions[ctx->action_count].allows_continuous = allows_continuous;

  ctx->action_count++;
  return rc;
}

/**
 * @brief cmp_list_set_edit_mode
 *
 * @param list_opaque Parameter description.
 * @param is_editing Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_list_set_edit_mode(cmp_list_t *list_opaque, int is_editing) {
  int rc = CMP_SUCCESS;
  struct cmp_list *ctx = (struct cmp_list *)list_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_list_set_edit_mode: Invalid argument\n");
    return rc;
  }

  ctx->is_editing = is_editing;
  return rc;
}

/**
 * @brief cmp_list_resolve_metrics
 *
 * @param list_opaque Parameter description.
 * @param out_margin_horizontal Parameter description.
 * @param out_corner_radius Parameter description.
 * @param out_content_offset_x Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_list_resolve_metrics(cmp_list_t *list_opaque,
                             float *out_margin_horizontal,
                             float *out_corner_radius,
                             float *out_content_offset_x) {
  int rc = CMP_SUCCESS;
  struct cmp_list *ctx = (struct cmp_list *)list_opaque;

  if (!ctx || !out_margin_horizontal || !out_corner_radius ||
      !out_content_offset_x) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_list_resolve_metrics: Invalid argument\n");
    return rc;
  }

  if (ctx->style == CMP_LIST_STYLE_INSET_GROUPED) {
    *out_margin_horizontal =
        16.0f; /* Standard iPad / iOS 15+ Settings margin */
    *out_corner_radius =
        10.0f; /* Continuous squircle radius for top/bottom rows */
  } else {
    *out_margin_horizontal = 0.0f;
    *out_corner_radius = 0.0f;
  }

  if (ctx->is_editing) {
    /* Reveal delete (minus) buttons on the leading edge */
    *out_content_offset_x = 38.0f; /* Shift content right to make room */
  } else {
    *out_content_offset_x = 0.0f;
  }

  return rc;
}
