#ifndef OAUTH2_GUI_H
#define OAUTH2_GUI_H

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_event.h"
#include "cmpc/cmp_router.h"
#include "cmpc/cmp_router_view.h"
#include "cmpc/cmp_http_integration.h"
#include "cmpc/cmp_form.h"
#include "cmpc/cmp_ui_builder.h"
#include "cmpc/cmp_layout.h"
#include "cmpc/cmp_text.h"
#include "m3/m3_text_field.h"
#include "m3/m3_button.h"
#include "m3/m3_scaffold.h"
#include "m3/m3_app_bar.h"
#include "m3/m3_card.h"
#include "m3/m3_color.h"
#include "m3/m3_typography.h"
#include "m3/m3_badge.h"
#include "f2/f2_text_field.h"
#include "f2/f2_button.h"
#include "cupertino/cupertino_text_field.h"
#include "cupertino/cupertino_button.h"

#ifndef C_ABSTRACT_HTTP_NO_MULTIPLATFORM_INTEGRATION
#define C_ABSTRACT_HTTP_NO_MULTIPLATFORM_INTEGRATION
#endif
#include <c_abstract_http/c_abstract_http.h>

#include <c_orm_oauth2.h>

#include "m3/m3_navigation.h"

/* clang-format on */

typedef enum OAuth2GuiTheme {
  OAUTH2_GUI_THEME_M3 = 0,
  OAUTH2_GUI_THEME_F2,
  OAUTH2_GUI_THEME_CUPERTINO
} OAuth2GuiTheme;

typedef union {
  M3TextField m3;
  F2TextField f2;
  CupertinoTextField cup;
} AnyTextField;

typedef union {
  M3Button m3;
  F2Button f2;
  CupertinoButton cup;
} AnyButton;

typedef struct LayoutWidget {
  CMPWidget widget;
  CMPLayoutNode *root_layout;
  CMPScalar max_width;
  CMPColor background_color;
  CMPBool has_background;
  CMPWidget *bg_widget;
} LayoutWidget;

typedef struct AppbarWithButtons {
  CMPWidget widget;
  M3AppBar m3_app_bar;
  AnyButton leading_btn;
  AnyButton trailing_btn;
  CMPUIBuilder builder;
  CMPLayoutNode *root_layout;
} AppbarWithButtons;

typedef struct OAuth2Gui {
  OAuth2GuiTheme theme;

  CMPRouter router;
  CMPRoute routes[2];
  CMPHttpIntegration http;
  CMPEventDispatcher dispatcher;
  CMPAllocator alloc;
  CMPTextBackend backend;

  CMPWidget *root_widget;
  CMPRouterView router_view;
  M3Scaffold m3_scaffold;
  AppbarWithButtons custom_app_bar;
  M3Navigation m3_nav;
  M3NavigationItem m3_nav_items[3];
  LayoutWidget fab_widget;
  CMPUIBuilder fab_builder;
  CMPLayoutNode *fab_layout;
  M3Button fab_button;
  M3Badge fab_badge;
  M3Scheme m3_scheme;
  M3TypographyScale m3_typography;

  char *server_url;
  char *access_token;
  char *error_msg;

  CMPForm form;
  AnyTextField username_field;
  AnyTextField password_field;
  AnyButton password_toggle_btn;
  AnyButton login_button;
  AnyButton forgot_button;
  CMPTextWidget login_title;
  CMPTextWidget login_subtitle;
  M3Card login_card;
  CMPUIBuilder login_builder;
  CMPLayoutNode *login_layout;
  LayoutWidget login_widget;
  CMPWidget *login_inputs[3];

  AnyButton logout_button;
  CMPTextWidget token_label;
  CMPUIBuilder token_builder;
  CMPLayoutNode *token_layout;
  LayoutWidget token_widget;

  struct HttpClient *client;
  struct HttpFuture *future;
  struct HttpRequest *request;
} OAuth2Gui;

extern int oauth2_gui_init(OAuth2Gui *gui, CMPAllocator alloc,
                           CMPTextBackend backend, OAuth2GuiTheme theme);
extern int oauth2_gui_shutdown(OAuth2Gui *gui);
extern int oauth2_gui_tick(OAuth2Gui *gui);
extern CMPColor oauth2_gui_get_bg_color(const OAuth2Gui *gui);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OAUTH2_GUI_H */
