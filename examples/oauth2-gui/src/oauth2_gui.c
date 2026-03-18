/* clang-format off */
#include "oauth2_gui.h"
#include <m3/m3_text_field.h>
#include <m3/m3_button.h>
#include <cmpc/cmp_layout.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
/* clang-format on */

static int build_root_route(void *ctx, const char *path, void **out_component) {
  (void)ctx;
  (void)path;
  *out_component = NULL;
  return CMP_OK;
}

static int build_login_route(void *ctx, const char *path,
                             void **out_component) {
  (void)ctx;
  (void)path;
  *out_component = NULL;
  return CMP_OK;
}

int oauth2_gui_init(OAuth2Gui *gui) {
  CMPRouterConfig router_cfg;
  CMPRoute routes[2];

  if (!gui)
    return CMP_ERR_INVALID_ARGUMENT;
  memset(gui, 0, sizeof(*gui));

  memset(&routes, 0, sizeof(routes));
  routes[0].pattern = "/";
  routes[0].build = build_root_route;
  routes[0].ctx = gui;

  routes[1].pattern = "/login";
  routes[1].build = build_login_route;
  routes[1].ctx = gui;

  memset(&router_cfg, 0, sizeof(router_cfg));
  router_cfg.routes = routes;
  router_cfg.route_count = 2;
  router_cfg.stack_capacity = 10;

  if (cmp_router_init(&gui->router, &router_cfg) != CMP_OK) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  gui->username_field = NULL;
  gui->password_field = NULL;
  gui->server_url_field = NULL;

  return CMP_OK;
}

int oauth2_gui_shutdown(OAuth2Gui *gui) {
  if (!gui)
    return CMP_ERR_INVALID_ARGUMENT;
  cmp_router_shutdown(&gui->router);
  return CMP_OK;
}

int oauth2_gui_tick(OAuth2Gui *gui) {
  if (!gui)
    return CMP_ERR_INVALID_ARGUMENT;
  return CMP_OK;
}
