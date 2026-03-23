/* clang-format off */
#include "cmpc/cmp_router_view.h"
#include <stddef.h>
/* clang-format on */

static CMPWidget *get_current_widget(const CMPRouterView *view) {
  void *comp = NULL;
  const char *path = NULL;
  if (!view || !view->router) {
    return NULL;
  }
  if (cmp_router_get_current(view->router, &path, &comp) == CMP_OK) {
    return (CMPWidget *)comp;
  }
  return NULL;
}

static int cmp_router_view_measure(void *widget, CMPMeasureSpec width,
                                   CMPMeasureSpec height, CMPSize *out_size) {
  CMPRouterView *view = (CMPRouterView *)widget;
  CMPWidget *curr = get_current_widget(view);

  if (curr && curr->vtable && curr->vtable->measure) {
    return curr->vtable->measure(curr->ctx, width, height, out_size);
  }

  out_size->width = 0;
  out_size->height = 0;
  return CMP_OK;
}

static int cmp_router_view_layout(void *widget, CMPRect bounds) {
  CMPRouterView *view = (CMPRouterView *)widget;
  CMPWidget *curr = get_current_widget(view);

  view->bounds = bounds;

  if (curr && curr->vtable && curr->vtable->layout) {
    return curr->vtable->layout(curr->ctx, bounds);
  }
  return CMP_OK;
}

static int cmp_router_view_paint(void *widget, CMPPaintContext *ctx) {
  CMPRouterView *view = (CMPRouterView *)widget;
  CMPWidget *curr = get_current_widget(view);

  if (curr && curr->vtable && curr->vtable->paint) {
    return curr->vtable->paint(curr->ctx, ctx);
  }
  return CMP_OK;
}

static int cmp_router_view_event(void *widget, const CMPInputEvent *event,
                                 CMPBool *out_handled) {
  CMPRouterView *view = (CMPRouterView *)widget;
  CMPWidget *curr = get_current_widget(view);

  if (curr && curr->vtable && curr->vtable->event) {
    return curr->vtable->event(curr->ctx, event, out_handled);
  }
  return CMP_OK;
}

static int cmp_router_view_get_semantics(void *widget,
                                         CMPSemantics *out_semantics) {
  CMPRouterView *view = (CMPRouterView *)widget;
  CMPWidget *curr = get_current_widget(view);

  if (curr && curr->vtable && curr->vtable->get_semantics) {
    return curr->vtable->get_semantics(curr->ctx, out_semantics);
  }

  out_semantics->role = CMP_SEMANTIC_CONTAINER;
  return CMP_OK;
}

static int cmp_router_view_destroy(void *widget) {
  (void)widget;
  /* We don't own the router or the component in this context,
     as the router destroys components when clearing the stack */
  return CMP_OK;
}
static const CMPWidgetVTable g_cmp_router_view_vtable = {
    cmp_router_view_measure,       cmp_router_view_layout,
    cmp_router_view_paint,         cmp_router_view_event,
    cmp_router_view_get_semantics, cmp_router_view_destroy};

CMP_API int CMP_CALL cmp_router_view_init(CMPRouterView *view,
                                          const CMPRouter *router) {
  if (!view || !router) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  view->widget.ctx = view;
  view->widget.vtable = &g_cmp_router_view_vtable;
  view->widget.flags = 0;
  view->router = router;

  return CMP_OK;
}
