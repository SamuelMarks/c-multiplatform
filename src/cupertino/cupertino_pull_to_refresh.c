/* clang-format off */
#include "cupertino/cupertino_pull_to_refresh.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL
cupertino_refresh_style_init(CupertinoRefreshStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->is_dark_mode = CMP_FALSE;
  cupertino_color_get_system(CUPERTINO_COLOR_GRAY, CMP_FALSE,
                             &style->tint_color);
  style->trigger_offset = 60.0f; /* Standard iOS pull distance */

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_refresh_init(CupertinoRefreshControl *ptr,
                                            CMPWidget *child) {
  if (ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(ptr, 0, sizeof(*ptr));
  cupertino_refresh_style_init(&ptr->style);

  ptr->child = child;
  ptr->widget.ctx = ptr;
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_refresh_set_refreshing(
    CupertinoRefreshControl *ptr, CMPBool is_refreshing) {
  if (ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  ptr->is_refreshing = is_refreshing;
  if (!is_refreshing) {
    ptr->current_pull = 0.0f;
  }
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_refresh_set_callback(
    CupertinoRefreshControl *ptr, CupertinoRefreshCallback cb, void *ctx) {
  if (ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  ptr->on_refresh = cb;
  ptr->on_refresh_ctx = ctx;
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_refresh_layout(CupertinoRefreshControl *ptr,
                                              CMPRect bounds) {
  if (ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  ptr->bounds = bounds;

  /* In a real scenario, this would intercept scroll events from the child to
     update current_pull. For rendering purposes, we just layout the child in
     the remaining space. */
  if (ptr->child && ptr->child->vtable && ptr->child->vtable->layout) {
    CMPRect child_bounds = bounds;

    /* If refreshing, push child down slightly */
    if (ptr->is_refreshing) {
      child_bounds.y += ptr->style.trigger_offset;
      child_bounds.height -= ptr->style.trigger_offset;
    }

    ptr->child->vtable->layout(ptr->child->ctx, child_bounds);
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_refresh_paint(const CupertinoRefreshControl *ptr,
                                             CMPPaintContext *ctx) {
  CMPColor spinner_color;
  CMPRect spinner_rect;
  CMPScalar spinner_size = 28.0f;

  if (ptr == NULL || ctx == NULL || ctx->gfx == NULL ||
      ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  /* Paint the child first so the spinner appears "behind" or "above" depending
     on the list style, usually standard iOS has the spinner in the background
     if the list is opaque, but let's draw it here. */
  if (ptr->child && ptr->child->vtable && ptr->child->vtable->paint) {
    ptr->child->vtable->paint(ptr->child->ctx, ctx);
  }

  if (ptr->current_pull <= 0.0f && !ptr->is_refreshing) {
    return CMP_OK;
  }

  spinner_color = ptr->style.tint_color;

  /* Calculate alpha based on pull distance */
  if (!ptr->is_refreshing) {
    CMPScalar progress = ptr->current_pull / ptr->style.trigger_offset;
    if (progress > 1.0f)
      progress = 1.0f;
    spinner_color.a = progress;
  }

  /* Draw the spinner. iOS spinner is a circle of small lines.
     As a simplified fallback, we draw a colored circle or arc. */
  if (spinner_color.a > 0.0f && ctx->gfx->vtable->draw_rect) {
    CMPScalar y_pos =
        ptr->bounds.y + (ptr->is_refreshing ? (ptr->style.trigger_offset / 2.0f)
                                            : (ptr->current_pull / 2.0f));
    y_pos -= spinner_size / 2.0f;

    spinner_rect.x = ptr->bounds.x + (ptr->bounds.width - spinner_size) / 2.0f;
    spinner_rect.y = y_pos;
    spinner_rect.width = spinner_size;
    spinner_rect.height = spinner_size;

    /* Simple circle placeholder for UIActivityIndicatorView */
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &spinner_rect, spinner_color,
                                spinner_size / 2.0f);
  }

  return CMP_OK;
}
