/* clang-format off */
#include "cmpc/cmp_router_view.h"
#include "test_utils.h"
#include <string.h>

#ifndef CMP_TESTING
#define CMP_TESTING
#endif
/* clang-format on */

static int mock_measure(void *widget, CMPMeasureSpec width,
                        CMPMeasureSpec height, CMPSize *out_size) {
  (void)widget;
  (void)width;
  (void)height;
  out_size->width = 100;
  out_size->height = 100;
  return CMP_OK;
}

static int mock_layout(void *widget, CMPRect bounds) {
  (void)widget;
  (void)bounds;
  return CMP_OK;
}

static int mock_paint(void *widget, CMPPaintContext *ctx) {
  (void)widget;
  (void)ctx;
  return CMP_OK;
}

static int mock_event(void *widget, const CMPInputEvent *event,
                      CMPBool *out_handled) {
  (void)widget;
  (void)event;
  *out_handled = CMP_TRUE;
  return CMP_OK;
}

static int mock_get_semantics(void *widget, CMPSemantics *out_semantics) {
  (void)widget;
  out_semantics->role = CMP_SEMANTIC_TEXT;
  return CMP_OK;
}

static int mock_build(void *ctx, const char *path, void **out_component) {
  (void)ctx;
  (void)path;
  *out_component = ctx;
  return CMP_OK;
}

static int test_router_view_init(void) {
  CMPRouter router;
  CMPRouterView view;

  CMP_TEST_EXPECT(cmp_router_view_init(NULL, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_router_view_init(&view, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_router_view_init(&view, &router));

  return 0;
}

static int test_router_view_lifecycle(void) {
  CMPRouter router;
  CMPRouterConfig config;
  CMPRoute routes[1];
  CMPRouterView view;
  CMPWidget mock_widget;
  CMPWidgetVTable mock_vtable;
  CMPSize size;
  CMPRect bounds;
  CMPInputEvent event;
  CMPBool handled = CMP_FALSE;
  CMPSemantics semantics;

  memset(&mock_widget, 0, sizeof(mock_widget));
  memset(&mock_vtable, 0, sizeof(mock_vtable));
  mock_vtable.measure = mock_measure;
  mock_vtable.layout = mock_layout;
  mock_vtable.paint = mock_paint;
  mock_vtable.event = mock_event;
  mock_vtable.get_semantics = mock_get_semantics;
  mock_widget.vtable = &mock_vtable;

  memset(&config, 0, sizeof(config));
  memset(routes, 0, sizeof(routes));
  routes[0].pattern = "/";
  routes[0].build = mock_build;
  routes[0].ctx = &mock_widget;

  config.routes = routes;
  config.route_count = 1;
  config.stack_capacity = 10;

  cmp_router_init(&router, &config);
  cmp_router_view_init(&view, &router);

  /* Test without component */
  size.width = 0;
  size.height = 0;
  view.widget.vtable->measure(&view, (CMPMeasureSpec){0, 0},
                              (CMPMeasureSpec){0, 0}, &size);
  CMP_TEST_ASSERT(0 == size.width);
  CMP_TEST_ASSERT(0 == size.height);

  bounds.x = 0;
  bounds.y = 0;
  bounds.width = 10;
  bounds.height = 10;
  CMP_TEST_OK(view.widget.vtable->layout(&view, bounds));
  CMP_TEST_OK(view.widget.vtable->paint(&view, NULL));
  CMP_TEST_OK(view.widget.vtable->event(&view, &event, &handled));

  memset(&semantics, 0, sizeof(semantics));
  CMP_TEST_OK(view.widget.vtable->get_semantics(&view, &semantics));
  CMP_TEST_ASSERT(CMP_SEMANTIC_CONTAINER == semantics.role);

  /* Navigate */
  cmp_router_navigate(&router, "/", NULL);

  /* Test with component */
  size.width = 0;
  size.height = 0;
  view.widget.vtable->measure(&view, (CMPMeasureSpec){0, 0},
                              (CMPMeasureSpec){0, 0}, &size);
  CMP_TEST_ASSERT(100 == size.width);
  CMP_TEST_ASSERT(100 == size.height);

  CMP_TEST_OK(view.widget.vtable->layout(&view, bounds));
  CMP_TEST_OK(view.widget.vtable->paint(&view, NULL));
  CMP_TEST_OK(view.widget.vtable->event(&view, &event, &handled));
  CMP_TEST_ASSERT(handled);

  memset(&semantics, 0, sizeof(semantics));
  CMP_TEST_OK(view.widget.vtable->get_semantics(&view, &semantics));
  CMP_TEST_ASSERT(CMP_SEMANTIC_TEXT == semantics.role);

  /* Destroy does nothing */
  if (view.widget.vtable->destroy) {
    view.widget.vtable->destroy(&view);
  }

  cmp_router_shutdown(&router);
  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_router_view_init();
  fails += test_router_view_lifecycle();
  return fails;
}
