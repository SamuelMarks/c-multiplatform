/* clang-format off */
#include "m3/m3_pull_to_refresh.h"
#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_icon.h"
#include "cmpc/cmp_visuals.h"
#include <string.h>
/* clang-format on */

static int m3_ptr_pre_scroll(void *ctx, const CMPScrollDelta *delta,
                             CMPScrollDelta *out_consumed) {
  M3PullToRefresh *ptr = (M3PullToRefresh *)ctx;
  if (ptr == NULL || delta == NULL || out_consumed == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  out_consumed->x = 0.0f;
  out_consumed->y = 0.0f;

  if (delta->y > 0.0f && ptr->drag_offset > 0.0f) {
    if (delta->y >= ptr->drag_offset) {
      out_consumed->y = ptr->drag_offset;
      ptr->drag_offset = 0.0f;
    } else {
      out_consumed->y = delta->y;
      ptr->drag_offset -= delta->y;
    }
  }

  return CMP_OK;
}

static int m3_ptr_post_scroll(void *ctx, const CMPScrollDelta *delta,
                              const CMPScrollDelta *child_consumed,
                              CMPScrollDelta *out_consumed) {
  M3PullToRefresh *ptr = (M3PullToRefresh *)ctx;
  CMPScalar remaining_y;

  if (ptr == NULL || delta == NULL || child_consumed == NULL ||
      out_consumed == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  out_consumed->x = 0.0f;
  out_consumed->y = 0.0f;

  if (!ptr->is_refreshing) {
    remaining_y = delta->y - child_consumed->y;
    if (remaining_y < 0.0f) {
      /* Pulled down while at top */
      ptr->drag_offset += -remaining_y;
      out_consumed->y = remaining_y;

      /* Trigger refresh if we cross the threshold and maybe simulate release */
      /* Note: proper spring physics usually trigger on release, but for now we
       * clamp */
      if (ptr->drag_offset > ptr->style.max_drag) {
        ptr->drag_offset = ptr->style.max_drag;
      }
    }
  }

  return CMP_OK;
}

static const CMPScrollParentVTable m3_ptr_scroll_vtable = {m3_ptr_pre_scroll,
                                                           m3_ptr_post_scroll};

static int m3_ptr_measure(void *widget, CMPMeasureSpec width,
                          CMPMeasureSpec height, CMPSize *out_size) {
  M3PullToRefresh *ptr = (M3PullToRefresh *)widget;
  CMPSize child_size = {0.0f, 0.0f};

  if (ptr == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (ptr->child != NULL && ptr->child->vtable != NULL &&
      ptr->child->vtable->measure != NULL) {
    ptr->child->vtable->measure(ptr->child, width, height, &child_size);
  }

  *out_size = child_size;
  if (width.mode == CMP_MEASURE_EXACTLY) {
    out_size->width = width.size;
  }
  if (height.mode == CMP_MEASURE_EXACTLY) {
    out_size->height = height.size;
  }

  return CMP_OK;
}

static int m3_ptr_layout(void *widget, CMPRect bounds) {
  M3PullToRefresh *ptr = (M3PullToRefresh *)widget;
  if (ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  ptr->bounds = bounds;

  if (ptr->child != NULL && ptr->child->vtable != NULL &&
      ptr->child->vtable->layout != NULL) {
    /* The child occupies the full bounds; the indicator is an overlay */
    ptr->child->vtable->layout(ptr->child, bounds);
  }

  return CMP_OK;
}

static int m3_ptr_paint(void *widget, CMPPaintContext *ctx) {
  M3PullToRefresh *ptr = (M3PullToRefresh *)widget;
  CMPRect ind_bounds;
  CMPShadow shadow;
  CMPIconStyle icon_style;

  if (ptr == NULL || ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (ptr->child != NULL && ptr->child->vtable != NULL &&
      ptr->child->vtable->paint != NULL) {
    ptr->child->vtable->paint(ptr->child, ctx);
  }

  if (ptr->drag_offset > 0.0f || ptr->is_refreshing) {
    if (ctx->gfx != NULL && ctx->gfx->vtable != NULL &&
        ctx->gfx->vtable->draw_rect != NULL) {
      CMPScalar y_pos = ptr->bounds.y;

      if (ptr->is_refreshing) {
        y_pos += ptr->style.max_drag * 0.5f; /* settle position */
      } else {
        y_pos += ptr->drag_offset;
      }

      ind_bounds.width = ptr->style.size;
      ind_bounds.height = ptr->style.size;
      ind_bounds.x =
          ptr->bounds.x + (ptr->bounds.width - ind_bounds.width) * 0.5f;
      ind_bounds.y = y_pos - ind_bounds.height; /* Start above top edge */

      if (ind_bounds.y > ptr->bounds.y + ptr->style.max_drag) {
        ind_bounds.y = ptr->bounds.y + ptr->style.max_drag;
      }

      if (ptr->style.elevation > 0.0f) {
        CMPColor shadow_color;
        shadow_color.r = 0.0f;
        shadow_color.g = 0.0f;
        shadow_color.b = 0.0f;
        shadow_color.a = 0.3f;
        cmp_shadow_init(&shadow);
        cmp_shadow_set(&shadow, 0.0f, ptr->style.elevation,
                       ptr->style.elevation * 2.0f, 0.0f,
                       ind_bounds.width * 0.5f, 3, shadow_color);
        cmp_shadow_paint(&shadow, ctx->gfx, &ind_bounds, &ctx->clip);
      }

      ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &ind_bounds,
                                  ptr->style.container_color,
                                  ind_bounds.width * 0.5f);

      cmp_icon_style_init(&icon_style);
      icon_style.color = ptr->style.icon_color;
      icon_style.size_px = 24;

      /* Draw a refresh icon */
      {
        CMPRect icon_bounds;
        icon_bounds.width = 24.0f;
        icon_bounds.height = 24.0f;
        icon_bounds.x =
            ind_bounds.x + (ind_bounds.width - icon_bounds.width) * 0.5f;
        icon_bounds.y =
            ind_bounds.y + (ind_bounds.height - icon_bounds.height) * 0.5f;

        cmp_icon_draw_utf8(ctx->gfx, &icon_bounds, &icon_style, "refresh", 7,
                           NULL, CMP_ICON_RENDER_AUTO);
      }
    }
  }

  return CMP_OK;
}

static int m3_ptr_event(void *widget, const CMPInputEvent *event,
                        CMPBool *out_handled) {
  M3PullToRefresh *ptr = (M3PullToRefresh *)widget;

  if (ptr == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;

  /* Let child handle events first */
  if (ptr->child != NULL && ptr->child->vtable != NULL &&
      ptr->child->vtable->event != NULL) {
    ptr->child->vtable->event(ptr->child, event, out_handled);
  }

  /* On pointer up, if drag offset is maxed, trigger refresh */
  if (event->type == CMP_INPUT_POINTER_UP) {
    if (ptr->drag_offset >= ptr->style.max_drag && !ptr->is_refreshing) {
      ptr->is_refreshing = CMP_TRUE;
      if (ptr->on_refresh != NULL) {
        ptr->on_refresh(ptr->on_refresh_ctx);
      }
    } else if (!ptr->is_refreshing) {
      /* Snap back */
      ptr->drag_offset = 0.0f;
    }
  }

  return CMP_OK;
}

static int m3_ptr_get_semantics(void *widget, CMPSemantics *out_semantics) {
  M3PullToRefresh *ptr = (M3PullToRefresh *)widget;
  if (ptr == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_semantics, 0, sizeof(*out_semantics));

  out_semantics->role = CMP_SEMANTIC_NONE;
  out_semantics->flags = 0;
  out_semantics->utf8_label = "Pull to refresh";
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;

  return CMP_OK;
}

static int m3_ptr_destroy(void *widget) {
  M3PullToRefresh *ptr = (M3PullToRefresh *)widget;
  if (ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ptr->child != NULL && ptr->child->vtable != NULL &&
      ptr->child->vtable->destroy != NULL) {
    ptr->child->vtable->destroy(ptr->child);
  }
  return CMP_OK;
}

static const CMPWidgetVTable m3_ptr_vtable = {
    m3_ptr_measure, m3_ptr_layout,        m3_ptr_paint,
    m3_ptr_event,   m3_ptr_get_semantics, m3_ptr_destroy};

CMP_API int CMP_CALL
m3_pull_to_refresh_style_init(M3PullToRefreshStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  style->container_color.r = 0.910f;
  style->container_color.g = 0.867f;
  style->container_color.b = 0.965f;
  style->container_color.a = 1.0f;

  style->icon_color.r = 0.400f;
  style->icon_color.g = 0.314f;
  style->icon_color.b = 0.643f;
  style->icon_color.a = 1.0f;

  style->elevation = 2.0f;
  style->size = 48.0f;
  style->max_drag = 80.0f;

  return CMP_OK;
}

CMP_API int CMP_CALL m3_pull_to_refresh_init(M3PullToRefresh *ptr,
                                             const M3PullToRefreshStyle *style,
                                             CMPWidget *child) {
  if (ptr == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->size <= 0.0f || style->max_drag <= 0.0f) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(ptr, 0, sizeof(*ptr));
  ptr->widget.vtable = &m3_ptr_vtable;
  ptr->style = *style;
  ptr->child = child;

  ptr->scroll_parent.ctx = ptr;
  ptr->scroll_parent.vtable = &m3_ptr_scroll_vtable;

  return CMP_OK;
}

CMP_API int CMP_CALL m3_pull_to_refresh_set_refreshing(M3PullToRefresh *ptr,
                                                       CMPBool is_refreshing) {
  if (ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  ptr->is_refreshing = is_refreshing;
  if (!is_refreshing) {
    ptr->drag_offset = 0.0f;
  }
  return CMP_OK;
}

CMP_API int CMP_CALL m3_pull_to_refresh_set_on_refresh(
    M3PullToRefresh *ptr, M3PullToRefreshOnRefresh on_refresh, void *ctx) {
  if (ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  ptr->on_refresh = on_refresh;
  ptr->on_refresh_ctx = ctx;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_pull_to_refresh_set_child(M3PullToRefresh *ptr,
                                                  CMPWidget *child) {
  if (ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  ptr->child = child;
  return CMP_OK;
}