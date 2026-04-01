/* clang-format off */
#include "cmp.h"
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

int cmp_list_create(cmp_list_t **out_list, cmp_list_style_t style) {
  struct cmp_list *ctx;
  if (!out_list)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_list), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->style = style;
  ctx->is_editing = 0;
  ctx->rows = NULL;
  ctx->row_count = 0;
  ctx->row_capacity = 0;

  *out_list = (cmp_list_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_list_destroy(cmp_list_t *list_opaque) {
  struct cmp_list *ctx = (struct cmp_list *)list_opaque;
  size_t i;
  if (!ctx)
    return CMP_SUCCESS;

  if (ctx->rows) {
    for (i = 0; i < ctx->row_count; ++i) {
      if (ctx->rows[i])
        cmp_list_row_destroy((cmp_list_row_t *)ctx->rows[i]);
    }
    CMP_FREE(ctx->rows);
  }
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_list_add_row(cmp_list_t *list_opaque, cmp_list_row_t *row_opaque) {
  struct cmp_list *ctx = (struct cmp_list *)list_opaque;
  struct cmp_list_row *row = (struct cmp_list_row *)row_opaque;
  cmp_list_row_t **new_rows;
  size_t new_cap;

  if (!ctx || !row)
    return CMP_ERROR_INVALID_ARG;

  if (ctx->row_count == ctx->row_capacity) {
    new_cap = ctx->row_capacity == 0 ? 8 : ctx->row_capacity * 2;
    if (CMP_MALLOC(new_cap * sizeof(cmp_list_row_t *), (void **)&new_rows) !=
        CMP_SUCCESS)
      return CMP_ERROR_OOM;
    if (ctx->rows) {
      memcpy(new_rows, ctx->rows, ctx->row_count * sizeof(cmp_list_row_t *));
      CMP_FREE(ctx->rows);
    }
    ctx->rows = new_rows;
    ctx->row_capacity = new_cap;
  }

  ctx->rows[ctx->row_count++] = row;
  return CMP_SUCCESS;
}

int cmp_list_row_create(cmp_list_row_t **out_row, const char *title) {
  struct cmp_list_row *ctx;
  size_t len;
  if (!out_row || !title)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_list_row), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->separator_inset_left = 16.0f; /* Default HIG left margin */
  ctx->actions = NULL;
  ctx->action_count = 0;

  len = strlen(title);
  if (CMP_MALLOC(len + 1, (void **)&ctx->title) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
#if defined(_MSC_VER)
  strcpy_s(ctx->title, len + 1, title);
#else
  strcpy(ctx->title, title);
#endif

  *out_row = (cmp_list_row_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_list_row_destroy(cmp_list_row_t *row_opaque) {
  struct cmp_list_row *ctx = (struct cmp_list_row *)row_opaque;
  size_t i;
  if (!ctx)
    return CMP_SUCCESS;

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
  return CMP_SUCCESS;
}

int cmp_list_row_set_separator_inset(cmp_list_row_t *row_opaque,
                                     float inset_left) {
  struct cmp_list_row *ctx = (struct cmp_list_row *)row_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->separator_inset_left = inset_left;
  return CMP_SUCCESS;
}

int cmp_list_row_add_swipe_action(cmp_list_row_t *row_opaque, int is_leading,
                                  const char *title,
                                  cmp_swipe_action_style_t style,
                                  int allows_continuous) {
  struct cmp_list_row *ctx = (struct cmp_list_row *)row_opaque;
  cmp_swipe_action_t *new_actions;
  size_t len;
  if (!ctx || !title)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC((ctx->action_count + 1) * sizeof(cmp_swipe_action_t),
                 (void **)&new_actions) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  if (ctx->actions) {
    memcpy(new_actions, ctx->actions,
           ctx->action_count * sizeof(cmp_swipe_action_t));
    CMP_FREE(ctx->actions);
  }
  ctx->actions = new_actions;

  len = strlen(title);
  if (CMP_MALLOC(len + 1, (void **)&ctx->actions[ctx->action_count].title) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
#if defined(_MSC_VER)
  strcpy_s(ctx->actions[ctx->action_count].title, len + 1, title);
#else
  strcpy(ctx->actions[ctx->action_count].title, title);
#endif

  ctx->actions[ctx->action_count].style = style;
  ctx->actions[ctx->action_count].is_leading = is_leading;
  ctx->actions[ctx->action_count].allows_continuous = allows_continuous;

  ctx->action_count++;
  return CMP_SUCCESS;
}

int cmp_list_set_edit_mode(cmp_list_t *list_opaque, int is_editing) {
  struct cmp_list *ctx = (struct cmp_list *)list_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->is_editing = is_editing;
  return CMP_SUCCESS;
}

int cmp_list_resolve_metrics(cmp_list_t *list_opaque,
                             float *out_margin_horizontal,
                             float *out_corner_radius,
                             float *out_content_offset_x) {
  struct cmp_list *ctx = (struct cmp_list *)list_opaque;
  if (!ctx || !out_margin_horizontal || !out_corner_radius ||
      !out_content_offset_x)
    return CMP_ERROR_INVALID_ARG;

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

  return CMP_SUCCESS;
}
