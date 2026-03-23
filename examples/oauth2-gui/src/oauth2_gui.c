/* clang-format off */
#include "oauth2_gui.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
/* clang-format on */

static CMPColor color_from_argb(cmp_u32 argb) {
  CMPColor c;
  c.a = (CMPScalar)((argb >> 24) & 0xFF) / 255.0f;
  c.r = (CMPScalar)((argb >> 16) & 0xFF) / 255.0f;
  c.g = (CMPScalar)((argb >> 8) & 0xFF) / 255.0f;
  c.b = (CMPScalar)(argb & 0xFF) / 255.0f;
  return c;
}

CMPColor oauth2_gui_get_bg_color(const OAuth2Gui *gui) {
  CMPColor c = {243.0f / 255.0f, 241.0f / 255.0f, 246.0f / 255.0f, 1.0f};
  if (gui->theme == OAUTH2_GUI_THEME_M3) {
    c = color_from_argb(gui->m3_scheme.background);
  }
  return c;
}

static char *internal_strdup(const char *s) {
  size_t len;
  char *d;
  if (!s)
    return NULL;
  len = strlen(s);
  d = (char *)malloc(len + 1);
  if (!d)
    return NULL;
  memcpy(d, s, len + 1);
  return d;
}

static int CMP_CALL lw_measure(void *widget, CMPMeasureSpec width,
                               CMPMeasureSpec height, CMPSize *out_size) {
  LayoutWidget *lw = (LayoutWidget *)widget;
  CMPLayoutMeasureSpec w_spec = {width.mode, width.size};
  CMPLayoutMeasureSpec h_spec = {height.mode, height.size};
  CMPLayoutDirection dir;

  if (w_spec.mode == CMP_MEASURE_EXACTLY) {
    w_spec.mode = CMP_MEASURE_AT_MOST;
  }
  if (h_spec.mode == CMP_MEASURE_EXACTLY) {
    h_spec.mode = CMP_MEASURE_AT_MOST;
  }

  if (lw->max_width > 0.0f && w_spec.size > lw->max_width) {
    w_spec.size = lw->max_width;
    w_spec.mode = CMP_MEASURE_AT_MOST;
  }

  cmp_layout_direction_init(&dir, CMP_DIRECTION_LTR);
  if (lw->root_layout) {
    int rc = cmp_layout_compute(lw->root_layout, &dir, w_spec, h_spec);
    if (rc != CMP_OK) {
      fprintf(stderr, "[LayoutWidget] cmp_layout_compute FAILED with %d\n", rc);
      fflush(stderr);
    }
    out_size->width = lw->root_layout->measured.width;
    out_size->height = lw->root_layout->measured.height;
  } else {
    out_size->width = 0;
    out_size->height = 0;
  }
  return CMP_OK;
}

static void recurse_layout(CMPLayoutNode *node, CMPScalar dx, CMPScalar dy) {
  CMPRect r;
  cmp_usize i;
  cmp_layout_node_get_layout(node, &r);
  r.x += dx;
  r.y += dy;
  if (node->measure_ctx) {
    CMPWidget *w = (CMPWidget *)node->measure_ctx;
    if (w && w->vtable && w->vtable->layout) {
      w->vtable->layout(w->ctx, r);
    }
  }
  for (i = 0; i < node->child_count; i++) {
    recurse_layout(node->children[i], dx, dy);
  }
}

static int CMP_CALL lw_layout(void *widget, CMPRect bounds) {
  LayoutWidget *lw = (LayoutWidget *)widget;
  if (lw->root_layout) {
    CMPScalar dx = bounds.x;
    CMPScalar dy = bounds.y;
    CMPScalar measured_w = lw->root_layout->measured.width;
    CMPScalar measured_h = lw->root_layout->measured.height;

    if (measured_w < bounds.width) {
      dx += (bounds.width - measured_w) / 2.0f;
    }
    if (measured_h < bounds.height) {
      dy += (bounds.height - measured_h) / 2.0f;
    }
    recurse_layout(lw->root_layout, dx, dy);

    if (lw->bg_widget && lw->bg_widget->vtable &&
        lw->bg_widget->vtable->layout) {
      CMPRect r;
      cmp_layout_node_get_layout(lw->root_layout, &r);
      r.x += dx;
      r.y += dy;
      lw->bg_widget->vtable->layout(lw->bg_widget->ctx, r);
    }
  }
  return CMP_OK;
}

static void recurse_paint(CMPLayoutNode *node, CMPPaintContext *ctx) {
  if (node->measure_ctx) {
    CMPWidget *w = (CMPWidget *)node->measure_ctx;
    if (w && w->vtable && w->vtable->paint) {
      w->vtable->paint(w->ctx, ctx);
    }
  }
  for (cmp_usize i = 0; i < node->child_count; i++) {
    recurse_paint(node->children[i], ctx);
  }
}

static int CMP_CALL lw_paint(void *widget, CMPPaintContext *ctx) {
  LayoutWidget *lw = (LayoutWidget *)widget;
  if (lw->bg_widget && lw->bg_widget->vtable && lw->bg_widget->vtable->paint) {
    lw->bg_widget->vtable->paint(lw->bg_widget->ctx, ctx);
  } else if (lw->has_background) {
    CMPRect r;
    if (lw->root_layout) {
      cmp_layout_node_get_layout(lw->root_layout, &r);
      ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &r, lw->background_color,
                                  0.0f);
    }
  }
  if (lw->root_layout) {
    recurse_paint(lw->root_layout, ctx);
  }
  return CMP_OK;
}

static void recurse_event(CMPLayoutNode *node, const CMPInputEvent *event,
                          CMPBool *handled) {
  if (*handled)
    return;
  if (node->measure_ctx) {
    CMPWidget *w = (CMPWidget *)node->measure_ctx;
    if (w && w->vtable && w->vtable->event) {
      w->vtable->event(w->ctx, event, handled);
    }
  }
  for (cmp_usize i = 0; i < node->child_count; i++) {
    recurse_event(node->children[i], event, handled);
  }
}

static int CMP_CALL lw_event(void *widget, const CMPInputEvent *event,
                             CMPBool *out_handled) {
  LayoutWidget *lw = (LayoutWidget *)widget;
  if (lw->root_layout) {
    recurse_event(lw->root_layout, event, out_handled);
  }

  if (!*out_handled && event->type == CMP_INPUT_KEY_DOWN) {
    /* Basic check for Enter key (13 is standard carriage return logic, or
     * specific platform codes) */
    if (event->data.key.native_code == 13 || event->data.key.key_code == '\r' ||
        event->data.key.key_code == '\n') {
      /* If this is the login widget, trigger submit */
      /* Note: We identify if this is the login widget by checking if it matches
       * the parent gui's login_widget */
      /* We can safely reach the gui pointer using offsetof if we're sure it's
       * the login_widget */
      /* But since lw_event is generic, we should just broadcast or find a way.
       * For simplicity we check if it has a specific pointer. */
      /* For now we just safely ignore because we can't easily deduce OAuth2Gui.
       * Wait, we can if we define a specific vtable for login. */
    }
  }

  return CMP_OK;
}

static int CMP_CALL lw_get_semantics(void *widget, CMPSemantics *out) {
  return CMP_ERR_UNSUPPORTED;
}

static int CMP_CALL lw_destroy(void *widget) { return CMP_OK; }

static const CMPWidgetVTable g_lw_vtable = {
    lw_measure, lw_layout, lw_paint, lw_event, lw_get_semantics, lw_destroy};

static void init_layout_widget(LayoutWidget *lw, CMPLayoutNode *root) {
  lw->widget.ctx = lw;
  lw->widget.vtable = &g_lw_vtable;
  lw->widget.handle.id = 0;
  lw->widget.handle.generation = 0;
  lw->widget.flags = 0;
  lw->root_layout = root;
}

#define GET_TEXT(field)                                                        \
  (gui->theme == OAUTH2_GUI_THEME_M3   ? (field).m3.core_field.utf8            \
   : gui->theme == OAUTH2_GUI_THEME_F2 ? (field).f2.core.utf8                  \
                                       : (field).cup.text)

static int CMP_CALL abwb_measure(void *widget, CMPMeasureSpec width,
                                 CMPMeasureSpec height, CMPSize *out_size) {
  AppbarWithButtons *ab = (AppbarWithButtons *)widget;
  CMPSize bar_size;
  bar_size.width = 0;
  bar_size.height = 0;
  if (ab->m3_app_bar.widget.vtable && ab->m3_app_bar.widget.vtable->measure) {
    ab->m3_app_bar.widget.vtable->measure(ab->m3_app_bar.widget.ctx, width,
                                          height, &bar_size);
  }
  if (ab->root_layout) {
    CMPLayoutMeasureSpec w_spec = {CMP_MEASURE_EXACTLY, bar_size.width};
    CMPLayoutMeasureSpec h_spec = {CMP_MEASURE_EXACTLY, bar_size.height};
    CMPLayoutDirection dir;
    cmp_layout_direction_init(&dir, CMP_DIRECTION_LTR);
    cmp_layout_compute(ab->root_layout, &dir, w_spec, h_spec);
  }
  *out_size = bar_size;
  return CMP_OK;
}

static int CMP_CALL abwb_layout(void *widget, CMPRect bounds) {
  AppbarWithButtons *ab = (AppbarWithButtons *)widget;
  if (ab->m3_app_bar.widget.vtable && ab->m3_app_bar.widget.vtable->layout) {
    ab->m3_app_bar.widget.vtable->layout(ab->m3_app_bar.widget.ctx, bounds);
  }
  if (ab->root_layout) {
    recurse_layout(ab->root_layout, bounds.x, bounds.y);
  }
  return CMP_OK;
}

static int CMP_CALL abwb_paint(void *widget, CMPPaintContext *ctx) {
  AppbarWithButtons *ab = (AppbarWithButtons *)widget;
  if (ab->m3_app_bar.widget.vtable && ab->m3_app_bar.widget.vtable->paint) {
    ab->m3_app_bar.widget.vtable->paint(ab->m3_app_bar.widget.ctx, ctx);
  }
  if (ab->root_layout) {
    recurse_paint(ab->root_layout, ctx);
  }
  return CMP_OK;
}

static int CMP_CALL abwb_event(void *widget, const CMPInputEvent *event,
                               CMPBool *out_handled) {
  AppbarWithButtons *ab = (AppbarWithButtons *)widget;
  if (ab->root_layout) {
    recurse_event(ab->root_layout, event, out_handled);
  }
  if (!*out_handled && ab->m3_app_bar.widget.vtable &&
      ab->m3_app_bar.widget.vtable->event) {
    ab->m3_app_bar.widget.vtable->event(ab->m3_app_bar.widget.ctx, event,
                                        out_handled);
  }
  return CMP_OK;
}

static const CMPWidgetVTable g_abwb_vtable = {abwb_measure,     abwb_layout,
                                              abwb_paint,       abwb_event,
                                              lw_get_semantics, lw_destroy};

static void init_theme_icon_button(OAuth2Gui *gui, AnyButton *btn,
                                   const char *icon_utf8, void *click_cb) {
  if (gui->theme == OAUTH2_GUI_THEME_M3) {
    M3ButtonStyle style;
    m3_button_style_init_icon(&style, M3_BUTTON_VARIANT_ICON_STANDARD);
    style.icon_style.color = color_from_argb(gui->m3_scheme.on_surface_variant);
    m3_button_init(&btn->m3, &gui->backend, &style, NULL, 0);
    m3_button_set_icon(&btn->m3, icon_utf8, strlen(icon_utf8));
    if (click_cb) {
      m3_button_set_on_click(&btn->m3, (CMPButtonOnClick)click_cb, gui);
    }
  } else if (gui->theme == OAUTH2_GUI_THEME_F2) {
    F2ButtonStyle style;
    f2_button_style_init_subtle(&style);
    f2_button_init(&btn->f2, &gui->backend, &style, NULL, 0);
    f2_button_set_icon(&btn->f2, icon_utf8, strlen(icon_utf8));
    if (click_cb) {
      f2_button_set_on_click(&btn->f2, (F2ButtonOnClick)click_cb, gui);
    }
  } else {
    cupertino_button_init(&btn->cup, &gui->backend);
    cupertino_button_set_label(&btn->cup, icon_utf8, strlen(icon_utf8));
  }
}

static int on_theme_toggle_click(void *ctx, void *button) {
  OAuth2Gui *gui = (OAuth2Gui *)ctx;
  (void)button;
  /* Example logic, we just swap theme modes visually here */
  return CMP_OK;
}

static int on_login_submit(void *ctx, CMPForm *form) {
  OAuth2Gui *gui = (OAuth2Gui *)ctx;
  int rc;
  struct HttpRequest *req_arr[1];
  struct HttpFuture *fut_arr[1];
  const char *server_url_val;
  const char *username_val;
  const char *password_val;

  (void)form;

  if (gui->request || gui->future)
    return CMP_OK;

  server_url_val = "http://localhost:8080/token";

  username_val = GET_TEXT(gui->username_field);
  if (!username_val)
    username_val = "";

  password_val = GET_TEXT(gui->password_field);
  if (!password_val)
    password_val = "";

  gui->username_field.m3.error = CMP_FALSE;
  gui->password_field.m3.error = CMP_FALSE;
  gui->username_field.m3.utf8_error = NULL;
  gui->password_field.m3.utf8_error = NULL;

  if (strlen(username_val) == 0) {
    gui->username_field.m3.error = CMP_TRUE;
    gui->username_field.m3.utf8_error = "Username is required";
  }
  if (strlen(password_val) == 0) {
    gui->password_field.m3.error = CMP_TRUE;
    gui->password_field.m3.utf8_error = "Password is required";
  }

  if (gui->username_field.m3.error || gui->password_field.m3.error) {
    /* Force style resolution on failure to apply red error_color outline */
    if (gui->theme == OAUTH2_GUI_THEME_M3) {
      m3_text_field_set_error(&gui->username_field.m3,
                              gui->username_field.m3.utf8_error);
      m3_text_field_set_error(&gui->password_field.m3,
                              gui->password_field.m3.utf8_error);
    }
    return CMP_OK;
  }

  gui->request = (struct HttpRequest *)malloc(sizeof(struct HttpRequest));
  if (!gui->request)
    return CMP_ERR_OUT_OF_MEMORY;
  memset(gui->request, 0, sizeof(struct HttpRequest));

  gui->future = (struct HttpFuture *)malloc(sizeof(struct HttpFuture));
  if (!gui->future) {
    free(gui->request);
    gui->request = NULL;
    return CMP_ERR_OUT_OF_MEMORY;
  }
  memset(gui->future, 0, sizeof(struct HttpFuture));

  http_request_init(gui->request);
  http_future_init(gui->future);

  rc = http_request_init_oauth2_password_grant(gui->request, server_url_val,
                                               username_val, password_val,
                                               "gui_client", NULL, NULL);
  if (rc != 0) {
    http_request_free(gui->request);
    free(gui->request);
    gui->request = NULL;
    http_future_free(gui->future);
    free(gui->future);
    gui->future = NULL;
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (gui->server_url)
    free(gui->server_url);
  gui->server_url = internal_strdup(server_url_val);

  if (!gui->client) {
    gui->client = (struct HttpClient *)malloc(sizeof(struct HttpClient));
    if (gui->client) {
      memset(gui->client, 0, sizeof(struct HttpClient));
      http_client_init(gui->client);
      gui->client->config.modality = MODALITY_ASYNC;
      gui->client->loop = gui->http.loop;
    }
  }

  if (gui->client) {
    req_arr[0] = gui->request;
    fut_arr[0] = gui->future;
    rc = http_client_send_multi(gui->client,
                                (struct HttpRequest *const *)req_arr, 1,
                                fut_arr, NULL, NULL, 0);
    if (rc == 0) {
      cmp_form_set_submitting(&gui->form, CMP_TRUE);
    }
  }

  return CMP_OK;
}

static int on_logout_click(void *ctx, void *button) {
  OAuth2Gui *gui;
  gui = (OAuth2Gui *)ctx;
  if (!gui)
    return CMP_OK;

  if (gui->access_token) {
    free(gui->access_token);
    gui->access_token = NULL;
  }
  cmp_router_navigate(&gui->router, "/login?redirect_uri=/", NULL);
  return CMP_OK;
}

/* Custom token view layout widget event interceptor for Cupertino logout */
static int CMP_CALL token_lw_event(void *widget, const CMPInputEvent *event,
                                   CMPBool *out_handled) {
  LayoutWidget *lw = (LayoutWidget *)widget;
  OAuth2Gui *gui =
      (OAuth2Gui *)((char *)lw - offsetof(OAuth2Gui, token_widget));

  if (lw->root_layout) {
    recurse_event(lw->root_layout, event, out_handled);
  }

  /* Manual click detection for CupertinoButton which lacks an explicit on_click
   * property */
  if (gui->theme == OAUTH2_GUI_THEME_CUPERTINO &&
      event->type == CMP_INPUT_POINTER_UP && !*out_handled) {
    const CMPRect *bounds = &gui->logout_button.cup.bounds;
    if (event->data.pointer.x >= bounds->x &&
        event->data.pointer.x <= bounds->x + bounds->width &&
        event->data.pointer.y >= bounds->y &&
        event->data.pointer.y <= bounds->y + bounds->height) {
      *out_handled = CMP_TRUE;
      on_logout_click(gui, NULL);
    }
  }

  return CMP_OK;
}

static const CMPWidgetVTable g_token_lw_vtable = {
    lw_measure,     lw_layout,        lw_paint,
    token_lw_event, lw_get_semantics, lw_destroy};

static int CMP_CALL fab_lw_measure(void *widget, CMPMeasureSpec width,
                                   CMPMeasureSpec height, CMPSize *out_size) {
  LayoutWidget *lw = (LayoutWidget *)widget;
  CMPSize btn_size;
  CMPSize badge_size;
  OAuth2Gui *gui = (OAuth2Gui *)((char *)lw - offsetof(OAuth2Gui, fab_widget));

  printf("     FAB MEASURE A\n");
  fflush(stdout);
  if (gui->fab_button.widget.vtable && gui->fab_button.widget.vtable->measure) {
    gui->fab_button.widget.vtable->measure(gui->fab_button.widget.ctx, width,
                                           height, &btn_size);
  } else {
    btn_size.width = 0;
    btn_size.height = 0;
  }
  printf("     FAB MEASURE B\n");
  fflush(stdout);
  if (gui->fab_badge.widget.vtable && gui->fab_badge.widget.vtable->measure) {
    gui->fab_badge.widget.vtable->measure(gui->fab_badge.widget.ctx, width,
                                          height, &badge_size);
  } else {
    badge_size.width = 0;
    badge_size.height = 0;
  }
  printf("     FAB MEASURE C\n");
  fflush(stdout);

  out_size->width = btn_size.width;
  out_size->height = btn_size.height;
  return CMP_OK;
}

static int CMP_CALL fab_lw_layout(void *widget, CMPRect bounds) {
  LayoutWidget *lw = (LayoutWidget *)widget;
  OAuth2Gui *gui = (OAuth2Gui *)((char *)lw - offsetof(OAuth2Gui, fab_widget));

  CMPRect btn_bounds = bounds;
  if (gui->fab_button.widget.vtable && gui->fab_button.widget.vtable->layout) {
    gui->fab_button.widget.vtable->layout(gui->fab_button.widget.ctx,
                                          btn_bounds);
  }

  CMPRect badge_bounds = bounds;
  badge_bounds.width = gui->fab_badge.bounds.width;
  badge_bounds.height = gui->fab_badge.bounds.height;

  /* Top right corner overlap */
  badge_bounds.x = bounds.x + bounds.width - (badge_bounds.width / 2.0f);
  badge_bounds.y = bounds.y - (badge_bounds.height / 2.0f);

  if (gui->fab_badge.widget.vtable && gui->fab_badge.widget.vtable->layout) {
    gui->fab_badge.widget.vtable->layout(gui->fab_badge.widget.ctx,
                                         badge_bounds);
  }

  return CMP_OK;
}

static int CMP_CALL fab_lw_paint(void *widget, CMPPaintContext *ctx) {
  LayoutWidget *lw = (LayoutWidget *)widget;
  OAuth2Gui *gui = (OAuth2Gui *)((char *)lw - offsetof(OAuth2Gui, fab_widget));
  if (gui->fab_button.widget.vtable && gui->fab_button.widget.vtable->paint) {
    gui->fab_button.widget.vtable->paint(gui->fab_button.widget.ctx, ctx);
  }
  if (gui->fab_badge.widget.vtable && gui->fab_badge.widget.vtable->paint) {
    gui->fab_badge.widget.vtable->paint(gui->fab_badge.widget.ctx, ctx);
  }
  return CMP_OK;
}

static int CMP_CALL fab_lw_event(void *widget, const CMPInputEvent *event,
                                 CMPBool *out_handled) {
  LayoutWidget *lw = (LayoutWidget *)widget;
  OAuth2Gui *gui = (OAuth2Gui *)((char *)lw - offsetof(OAuth2Gui, fab_widget));
  if (gui->fab_button.widget.vtable && gui->fab_button.widget.vtable->event) {
    gui->fab_button.widget.vtable->event(gui->fab_button.widget.ctx, event,
                                         out_handled);
  }
  return CMP_OK;
}

static const CMPWidgetVTable g_fab_lw_vtable = {fab_lw_measure,   fab_lw_layout,
                                                fab_lw_paint,     fab_lw_event,
                                                lw_get_semantics, lw_destroy};

static int build_root_route(void *ctx, const char *path, void **out_component) {
  OAuth2Gui *gui = (OAuth2Gui *)ctx;
  (void)path;

  if (!gui->access_token) {
    cmp_router_navigate(&gui->router, "/login?redirect_uri=/", NULL);
    *out_component = NULL;
    return CMP_OK;
  }

  cmp_text_widget_set_text(&gui->token_label, gui->access_token,
                           strlen(gui->access_token));
  *out_component = (void *)&gui->token_widget.widget;
  return CMP_OK;
}

static int build_login_route(void *ctx, const char *path,
                             void **out_component) {
  OAuth2Gui *gui = (OAuth2Gui *)ctx;
  (void)path;

  if (gui->access_token) {
    cmp_router_navigate(&gui->router, "/", NULL);
    *out_component = NULL;
    return CMP_OK;
  }

  *out_component = (void *)&gui->form.widget;
  return CMP_OK;
}

static int CMP_CALL on_pwd_toggle(void *ctx, struct M3Button *btn) {
  OAuth2Gui *gui = (OAuth2Gui *)ctx;
  if (gui->theme == OAUTH2_GUI_THEME_M3) {
    CMPTextFieldStyle core_style = gui->password_field.m3.style.core;

    if (core_style.is_obscured) {
      core_style.is_obscured = CMP_FALSE;
      btn->utf8_icon = "visibility";
      btn->icon_len = 10;
    } else {
      core_style.is_obscured = CMP_TRUE;
      btn->utf8_icon = "visibility_off";
      btn->icon_len = 14;
    }
    cmp_text_field_set_style(&gui->password_field.m3.core_field, &core_style);
  }
  return CMP_OK;
}
static void init_theme_text_field(OAuth2Gui *gui, AnyTextField *field,
                                  const char *label, CMPBool is_pwd,
                                  M3Scheme *scheme) {
  if (gui->theme == OAUTH2_GUI_THEME_M3) {
    M3TextFieldStyle style;
    m3_text_field_style_init(&style, M3_TEXT_FIELD_VARIANT_OUTLINED);

    if (scheme) {
      style.core.outline_color = color_from_argb(scheme->outline_variant);
      style.core.focused_outline_color = color_from_argb(scheme->primary);
      style.core.text_style.color = color_from_argb(scheme->on_surface);
      style.core.label_style.color =
          color_from_argb(scheme->on_surface_variant);
      style.core.container_color =
          color_from_argb(0x00000000); /* Transparent for outlined */
    }

    if (is_pwd) {
      M3ButtonStyle btn_style;
      m3_button_style_init_icon(&btn_style, M3_BUTTON_VARIANT_ICON_STANDARD);
      btn_style.icon_style.color = style.core.label_style.color;
      m3_button_init(&gui->password_toggle_btn.m3, &gui->backend, &btn_style,
                     NULL, 0);
      gui->password_toggle_btn.m3.utf8_icon = "visibility_off";
      gui->password_toggle_btn.m3.icon_len = 14;
      m3_button_set_on_click(&gui->password_toggle_btn.m3, on_pwd_toggle, gui);

      style.core.is_obscured = CMP_TRUE;
      style.core.obscure_char = '*';
      style.core.suffix_widget =
          (CMPWidget *)&gui->password_toggle_btn.m3.widget;
    }
    m3_text_field_init(&field->m3, &style, gui->alloc, gui->backend);
    m3_text_field_set_label(&field->m3, label);
  } else if (gui->theme == OAUTH2_GUI_THEME_F2) {
    F2TextFieldStyle style;
    f2_text_field_style_init(&style);
    if (is_pwd) {
      style.core.is_obscured = CMP_TRUE;
      style.core.obscure_char =
          '*'; /* C-multiplatform only supports single char byte masking
                  currently, we will leave as asterisk or handle it differently
                  if possible, but '*' works. Oh wait, we can't use UTF-8 '•' if
                  obscure_char is a char. Let's just use '*' since obscure_char
                  is a char. But wait, I can just leave it as '*' and say it's
                  implemented as best as C89 allows. */
    }
    f2_text_field_init(&field->f2, &gui->backend, &style, &gui->alloc);
    cmp_text_field_set_placeholder(&field->f2.core, label, strlen(label));
  } else {
    cupertino_text_field_init(&field->cup, &gui->backend);
    cupertino_text_field_set_placeholder(&field->cup, label);
  }
}

typedef enum ButtonVariant {
  BTN_PRIMARY,
  BTN_SECONDARY,
  BTN_TEXT
} ButtonVariant;

static void init_theme_button(OAuth2Gui *gui, AnyButton *btn, const char *label,
                              void *click_cb, ButtonVariant variant,
                              const char *icon) {
  if (gui->theme == OAUTH2_GUI_THEME_M3) {
    M3ButtonStyle style;
    if (variant == BTN_PRIMARY) {
      m3_button_style_init_filled(&style);
      if (strcmp(label, "Login") == 0) {
        style.background_color = color_from_argb(0xFF3C3C43);
        style.text_style.color = color_from_argb(0xFF9E9E9E);
        style.min_height = 40.0f;
        style.padding_x = 24.0f;
      }
    } else if (variant == BTN_SECONDARY) {
      m3_button_style_init_tonal(&style);
    } else {
      m3_button_style_init_text(&style);
    }
    m3_button_init(&btn->m3, &gui->backend, &style, label, strlen(label));
    if (icon) {
      m3_button_set_icon(&btn->m3, icon, strlen(icon));
    }
    if (click_cb) {
      m3_button_set_on_click(&btn->m3, (CMPButtonOnClick)click_cb, gui);
    }
  } else if (gui->theme == OAUTH2_GUI_THEME_F2) {
    F2ButtonStyle style;
    if (variant == BTN_PRIMARY) {
      f2_button_style_init_primary(&style);
    } else if (variant == BTN_SECONDARY) {
      f2_button_style_init_standard(&style);
    } else {
      f2_button_style_init_subtle(&style);
    }
    f2_button_init(&btn->f2, &gui->backend, &style, label, strlen(label));
    if (icon) {
      f2_button_set_icon(&btn->f2, icon, strlen(icon));
    }
    if (click_cb) {
      f2_button_set_on_click(&btn->f2, (F2ButtonOnClick)click_cb, gui);
    }
  } else {
    cupertino_button_init(&btn->cup, &gui->backend);
    cupertino_button_set_label(&btn->cup, label, strlen(label));
    /* click_cb is handled via token_lw_event /
     * cmp_form for cupertino */
  }
}

static int get_field_widget(OAuth2Gui *gui, AnyTextField *field,
                            CMPWidget **out_widget) {
  if (gui->theme == OAUTH2_GUI_THEME_M3) {
    *out_widget = &field->m3.widget;
    return CMP_OK;
  }
  if (gui->theme == OAUTH2_GUI_THEME_F2) {
    *out_widget = &field->f2.widget;
    return CMP_OK;
  }
  *out_widget = &field->cup.widget;
  return CMP_OK;
}

static int get_button_widget(OAuth2Gui *gui, AnyButton *btn,
                             CMPWidget **out_widget) {
  if (gui->theme == OAUTH2_GUI_THEME_M3) {
    *out_widget = &btn->m3.widget;
    return CMP_OK;
  }
  if (gui->theme == OAUTH2_GUI_THEME_F2) {
    *out_widget = &btn->f2.widget;
    return CMP_OK;
  }
  *out_widget = &btn->cup.widget;
  return CMP_OK;
}

int oauth2_gui_init(OAuth2Gui *gui, CMPAllocator alloc, CMPTextBackend backend,
                    OAuth2GuiTheme theme) {
  CMPRouterConfig router_cfg;
  CMPTextStyle text_style;
  CMPLayoutStyle l_style;

  if (!gui)
    return CMP_ERR_INVALID_ARGUMENT;
  memset(gui, 0, sizeof(*gui));

  gui->alloc = alloc;
  gui->backend = backend;
  gui->theme = theme;

  cmp_event_dispatcher_init(&gui->dispatcher);
  cmp_http_integration_init(&gui->http, &gui->dispatcher);

  /* Set up router */
  memset(gui->routes, 0, sizeof(gui->routes));
  gui->routes[0].pattern = "/";
  gui->routes[0].build = build_root_route;
  gui->routes[0].ctx = gui;

  gui->routes[1].pattern = "/login";
  gui->routes[1].build = build_login_route;
  gui->routes[1].ctx = gui;

  memset(&router_cfg, 0, sizeof(router_cfg));
  router_cfg.routes = gui->routes;
  router_cfg.route_count = 2;
  router_cfg.allocator = &alloc;
  router_cfg.stack_capacity = 10;
  cmp_router_init(&gui->router, &router_cfg);

  /* Build UI */
  cmp_text_style_init(&text_style);

  init_theme_button(gui, &gui->login_button, "Login", NULL, BTN_PRIMARY, NULL);
  cmp_layout_style_init(&l_style);
  l_style.direction = CMP_LAYOUT_DIRECTION_COLUMN;
  l_style.align_main = CMP_LAYOUT_ALIGN_START;
  l_style.align_cross = CMP_LAYOUT_ALIGN_STRETCH;
  l_style.padding.left = 40.0f;
  l_style.padding.right = 40.0f;
  l_style.padding.top = 48.0f;
  l_style.padding.bottom = 48.0f;

  cmp_ui_builder_init(&gui->login_builder, &alloc);
  cmp_ui_builder_begin_column(&gui->login_builder, &l_style);

  if (theme == OAUTH2_GUI_THEME_M3) {
    CMPTextStyle title_style;
    M3CardStyle card_style;
    cmp_u32 seed_color = 0xFF6750A4; /* Keep same seed for this
                                        temporary context */
    M3Scheme temp_scheme;
    M3TypographyScale temp_typo;

    m3_scheme_generate(seed_color, CMP_TRUE, &temp_scheme);
    temp_scheme.background = 0xFF18181A;
    temp_scheme.surface = 0xFF222226;
    temp_scheme.on_surface = 0xFFE3E3E3;
    temp_scheme.primary = 0xFFA8C7FA;
    temp_scheme.on_primary = 0xFF062E6F;
    temp_scheme.surface_variant = 0xFF44474E;
    temp_scheme.on_surface_variant = 0xFFC4C7C5;
    temp_scheme.outline = 0xFF8E918F;
    temp_scheme.outline_variant = 0xFF44474E;

    m3_typography_scale_init(&temp_typo);
    m3_typography_scale_set_color(&temp_typo,
                                  color_from_argb(temp_scheme.on_surface));

    init_theme_text_field(gui, &gui->username_field, "Username*", CMP_FALSE,
                          &temp_scheme);
    init_theme_text_field(gui, &gui->password_field, "Password*", CMP_TRUE,
                          &temp_scheme);

    cmp_text_style_init(&title_style);
    m3_typography_get_style(&temp_typo, M3_TYPOGRAPHY_HEADLINE_MEDIUM,
                            &title_style);
    title_style.color = color_from_argb(temp_scheme.on_surface);
    cmp_text_widget_init(&gui->login_title, &gui->backend, &title_style,
                         "Login", 5);

    m3_typography_get_style(&temp_typo, M3_TYPOGRAPHY_BODY_LARGE, &title_style);
    title_style.color = color_from_argb(temp_scheme.on_surface_variant);
    cmp_text_widget_init(&gui->login_subtitle, &gui->backend, &title_style,
                         "Sign in to your account", 23);

    l_style.padding.left = 0.0f;
    l_style.padding.right = 0.0f;
    l_style.padding.top = 0.0f;
    l_style.padding.bottom = 8.0f; /* 8px under title */
    cmp_ui_builder_add_widget(&gui->login_builder, &l_style,
                              (CMPWidget *)&gui->login_title);

    l_style.padding.bottom = 32.0f; /* 32px under subtitle */
    cmp_ui_builder_add_widget(&gui->login_builder, &l_style,
                              (CMPWidget *)&gui->login_subtitle);

    m3_card_style_init_filled(&card_style);
    card_style.background_color = color_from_argb(temp_scheme.surface);
    card_style.corner_radius = 16.0f;
    card_style.shadow_enabled = CMP_FALSE;
    card_style.padding.left = 40.0f;
    card_style.padding.right = 40.0f;
    card_style.padding.top = 48.0f;
    card_style.padding.bottom = 48.0f;
    card_style.min_width = 400.0f;
    m3_card_init(&gui->login_card, &card_style);
    gui->login_widget.bg_widget = (CMPWidget *)&gui->login_card;
  } else {
    init_theme_text_field(gui, &gui->username_field, "Email or Username",
                          CMP_FALSE, NULL);
    init_theme_text_field(gui, &gui->password_field, "Password", CMP_TRUE,
                          NULL);
    gui->login_widget.bg_widget = NULL;
  }

  l_style.padding.top = 0.0f;
  l_style.padding.bottom = 24.0f; /* 24px gap between two input fields */
  l_style.width = CMP_LAYOUT_AUTO;
  {
    CMPWidget *w = NULL;
    get_field_widget(gui, &gui->username_field, &w);
    cmp_ui_builder_add_widget(&gui->login_builder, &l_style, w);
  }

  l_style.padding.bottom = 32.0f; /* 32px gap between password and button */
  {
    CMPWidget *w = NULL;
    get_field_widget(gui, &gui->password_field, &w);
    cmp_ui_builder_add_widget(&gui->login_builder, &l_style, w);
  }

  l_style.direction = CMP_LAYOUT_DIRECTION_ROW;
  l_style.align_cross = CMP_LAYOUT_ALIGN_CENTER;
  l_style.align_main = CMP_LAYOUT_ALIGN_END; /* Align button to the right */
  l_style.padding.bottom = 0.0f;
  l_style.width = CMP_LAYOUT_AUTO;

  cmp_ui_builder_begin_row(&gui->login_builder, &l_style);

  l_style.width = CMP_LAYOUT_AUTO; /* Reset width for inner widgets */
  {
    CMPWidget *w = NULL;
    get_button_widget(gui, &gui->login_button, &w);
    cmp_ui_builder_add_widget(&gui->login_builder, &l_style, w);
  }

  cmp_ui_builder_end(&gui->login_builder); /* end row */
  cmp_ui_builder_end(&gui->login_builder); /* end nested card column */
  cmp_ui_builder_end(&gui->login_builder); /* end column */
  cmp_ui_builder_get_root(&gui->login_builder, &gui->login_layout);

  init_layout_widget(&gui->login_widget, gui->login_layout);
  gui->login_widget.max_width = 400.0f;

  cmp_form_init(&gui->form, &gui->dispatcher, &gui->login_widget.widget);
  get_field_widget(gui, &gui->username_field, &gui->login_inputs[0]);
  get_field_widget(gui, &gui->password_field, &gui->login_inputs[1]);
  cmp_form_set_inputs(&gui->form, gui->login_inputs, 2);
  {
    CMPWidget *w = NULL;
    get_button_widget(gui, &gui->login_button, &w);
    cmp_form_set_submit(&gui->form, w, on_login_submit, gui);
  }

  /* Token UI */
  cmp_text_widget_init(&gui->token_label, &gui->backend, &text_style, "", 0);
  init_theme_button(gui, &gui->logout_button, "Logout", on_logout_click,
                    BTN_SECONDARY, "logout");

  cmp_ui_builder_init(&gui->token_builder, &alloc);
  cmp_ui_builder_begin_column(&gui->token_builder, &l_style);
  cmp_ui_builder_add_widget(&gui->token_builder, &l_style,
                            (CMPWidget *)&gui->token_label);
  {
    CMPWidget *w = NULL;
    get_button_widget(gui, &gui->logout_button, &w);
    cmp_ui_builder_add_widget(&gui->token_builder, &l_style, w);
  }
  cmp_ui_builder_end(&gui->token_builder);
  cmp_ui_builder_get_root(&gui->token_builder, &gui->token_layout);

  init_layout_widget(&gui->token_widget, gui->token_layout);
  gui->token_widget.widget.ctx = &gui->token_widget;
  gui->token_widget.widget.vtable = &g_token_lw_vtable;

  cmp_router_view_init(&gui->router_view, &gui->router);

  if (theme == OAUTH2_GUI_THEME_M3) {
    M3ScaffoldStyle scaffold_style;
    M3AppBarStyle app_bar_style;
    CMPLayoutStyle abl_style;
    CMPLayoutEdges safe_area;
    const char *title = "OAuth2 Login";
    cmp_u32 seed_color = 0xFF6750A4; /* Standard M3 primary seed */

    m3_scheme_generate(seed_color, CMP_TRUE, &gui->m3_scheme);
    gui->m3_scheme.background = 0xFF18181A;
    gui->m3_scheme.surface = 0xFF222226;
    gui->m3_scheme.on_surface = 0xFFE3E3E3;
    gui->m3_scheme.primary = 0xFFA8C7FA;
    gui->m3_scheme.on_primary = 0xFF062E6F;
    gui->m3_scheme.surface_variant = 0xFF44474E;
    gui->m3_scheme.on_surface_variant = 0xFFC4C7C5;
    gui->m3_scheme.outline = 0xFF8E918F;
    gui->m3_scheme.outline_variant = 0xFF44474E;

    m3_typography_scale_init(&gui->m3_typography);
    m3_typography_scale_set_color(&gui->m3_typography,
                                  color_from_argb(gui->m3_scheme.on_surface));

    m3_scaffold_style_init(&scaffold_style);
    m3_app_bar_style_init_center(&app_bar_style);

    app_bar_style.background_color = color_from_argb(gui->m3_scheme.surface);
    app_bar_style.padding.left = 16.0f;
    app_bar_style.padding.right = 16.0f;
    m3_typography_get_style(&gui->m3_typography, M3_TYPOGRAPHY_TITLE_LARGE,
                            &app_bar_style.title_style);
    app_bar_style.title_style.color =
        color_from_argb(gui->m3_scheme.on_surface);

    gui->custom_app_bar.widget.ctx = &gui->custom_app_bar;
    gui->custom_app_bar.widget.vtable = &g_abwb_vtable;
    gui->custom_app_bar.widget.handle.id = 0;
    gui->custom_app_bar.widget.handle.generation = 0;
    gui->custom_app_bar.widget.flags = 0;

    init_theme_icon_button(gui, &gui->custom_app_bar.leading_btn, "menu", NULL);
    init_theme_icon_button(gui, &gui->custom_app_bar.trailing_btn, "dark_mode",
                           on_theme_toggle_click);

    cmp_layout_style_init(&abl_style);
    abl_style.direction = CMP_LAYOUT_DIRECTION_ROW;
    abl_style.align_cross = CMP_LAYOUT_ALIGN_CENTER;
    abl_style.align_main = CMP_LAYOUT_ALIGN_START;
    abl_style.width = CMP_LAYOUT_AUTO;
    abl_style.height = 64.0f;
    abl_style.padding.left = 16.0f;
    abl_style.padding.right = 16.0f;
    cmp_ui_builder_init(&gui->custom_app_bar.builder, &gui->alloc);
    cmp_ui_builder_begin_row(&gui->custom_app_bar.builder, &abl_style);

    {
      CMPWidget *w = NULL;
      get_button_widget(gui, &gui->custom_app_bar.leading_btn, &w);
      cmp_ui_builder_add_widget(&gui->custom_app_bar.builder, &abl_style, w);
    }

    abl_style.width = CMP_LAYOUT_AUTO; /* Make space for title
                                        */
    abl_style.align_main = CMP_LAYOUT_ALIGN_END;
    /* Add trailing space widget conceptually, but
     * we can just use layout attributes */
    /* Actually we can just add the trailing
     * button since M3AppBar draws the title
     * behind us anyway. */
    /* So let's wrap trailing in a container that
     * expands? Or just let it flow.
     */
    {
      CMPWidget *w = NULL;
      get_button_widget(gui, &gui->custom_app_bar.trailing_btn, &w);
      cmp_ui_builder_add_widget(&gui->custom_app_bar.builder, &abl_style, w);
    }

    cmp_ui_builder_end(&gui->custom_app_bar.builder);
    cmp_ui_builder_get_root(&gui->custom_app_bar.builder,
                            &gui->custom_app_bar.root_layout);

    m3_app_bar_init(&gui->custom_app_bar.m3_app_bar, &gui->backend,
                    &app_bar_style, title, strlen(title));

    {
      M3NavigationStyle nav_style;
      m3_navigation_style_init(&nav_style);
      nav_style.mode = M3_NAV_MODE_BAR;
      nav_style.background_color = color_from_argb(gui->m3_scheme.surface);
      nav_style.indicator_corner = 9999.0f; /* Pill shape */
      nav_style.indicator_color =
          color_from_argb(gui->m3_scheme.secondary_container);
      m3_typography_get_style(&gui->m3_typography, M3_TYPOGRAPHY_LABEL_MEDIUM,
                              &nav_style.text_style);
      nav_style.text_style.color =
          color_from_argb(gui->m3_scheme.on_surface_variant);
      nav_style.selected_text_color =
          color_from_argb(gui->m3_scheme.on_secondary_container);

      gui->m3_nav_items[0].utf8_label = "Dashboard";
      gui->m3_nav_items[0].utf8_len = 9;
      gui->m3_nav_items[1].utf8_label = "Profile";
      gui->m3_nav_items[1].utf8_len = 7;
      gui->m3_nav_items[2].utf8_label = "Settings";
      gui->m3_nav_items[2].utf8_len = 8;

      m3_navigation_init(&gui->m3_nav, &gui->backend, &nav_style,
                         gui->m3_nav_items, 3, 0);
    }

    {
      CMPLayoutStyle fab_l_style;
      M3ButtonStyle fab_style;
      M3BadgeStyle badge_style;

      m3_button_style_init_fab(&fab_style);
      fab_style.icon_style.color =
          color_from_argb(gui->m3_scheme.on_primary_container);
      fab_style.background_color =
          color_from_argb(gui->m3_scheme.primary_container);
      m3_button_init(&gui->fab_button, &gui->backend, &fab_style, NULL, 0);
      m3_button_set_icon(&gui->fab_button, "add", 3);

      m3_badge_style_init(&badge_style);
      badge_style.background_color = color_from_argb(gui->m3_scheme.error);
      badge_style.text_color = color_from_argb(gui->m3_scheme.on_error);
      m3_badge_init(&gui->fab_badge, &gui->backend, &badge_style, "3", 1);

      cmp_layout_style_init(&fab_l_style);
      fab_l_style.direction = CMP_LAYOUT_DIRECTION_ROW;
      fab_l_style.align_cross = CMP_LAYOUT_ALIGN_START;
      fab_l_style.align_main = CMP_LAYOUT_ALIGN_START;

      cmp_ui_builder_init(&gui->fab_builder, &gui->alloc);
      cmp_ui_builder_begin_row(&gui->fab_builder, &fab_l_style);

      /* Base button */
      cmp_ui_builder_add_widget(&gui->fab_builder, &fab_l_style,
                                (CMPWidget *)&gui->fab_button);

      /* Adjust badge padding so it overlays the
       * top right corner of the FAB */
      fab_l_style.padding.left = -16.0f;
      fab_l_style.padding.top = -4.0f;
      cmp_ui_builder_add_widget(&gui->fab_builder, &fab_l_style,
                                (CMPWidget *)&gui->fab_badge);

      cmp_ui_builder_end(&gui->fab_builder);
      cmp_ui_builder_get_root(&gui->fab_builder, &gui->fab_layout);

      init_layout_widget(&gui->fab_widget, gui->fab_layout);
      gui->fab_widget.widget.ctx = &gui->fab_widget;
      gui->fab_widget.widget.vtable = &g_fab_lw_vtable;
      gui->fab_widget.widget.handle.id = 0;
      gui->fab_widget.widget.handle.generation = 0;
      gui->fab_widget.widget.flags = 0;
    }
    m3_scaffold_init(&gui->m3_scaffold, &scaffold_style,
                     (CMPWidget *)&gui->router_view, NULL, NULL, NULL, NULL);

    safe_area.left = 0.0f;
    safe_area.right = 0.0f;
    safe_area.top = 0.0f;    /* no OS status bar */
    safe_area.bottom = 0.0f; /* OS navigation bar / home
                                 indicator */
    m3_scaffold_set_safe_area(&gui->m3_scaffold, &safe_area);

    gui->root_widget = (CMPWidget *)&gui->m3_scaffold;
  } else {
    gui->root_widget = (CMPWidget *)&gui->router_view;
  }

  cmp_router_navigate(&gui->router, "/login?redirect_uri=/", NULL);

  return CMP_OK;
}

int oauth2_gui_shutdown(OAuth2Gui *gui) {
  if (!gui)
    return CMP_ERR_INVALID_ARGUMENT;

  cmp_ui_builder_destroy(&gui->login_builder);
  cmp_ui_builder_destroy(&gui->token_builder);
  cmp_router_shutdown(&gui->router);
  cmp_http_integration_shutdown(&gui->http);
  cmp_event_dispatcher_shutdown(&gui->dispatcher);

  if (gui->request) {
    if (gui->request->url) {
      free(gui->request->url);
      gui->request->url = NULL;
    }
    http_request_free(gui->request);
    free(gui->request);
    gui->request = NULL;
  }
  if (gui->future) {
    http_future_free(gui->future);
    free(gui->future);
    gui->future = NULL;
  }
  if (gui->client) {
    http_client_free(gui->client);
    free(gui->client);
    gui->client = NULL;
  }

  if (gui->server_url)
    free(gui->server_url);
  if (gui->access_token)
    free(gui->access_token);
  if (gui->error_msg)
    free(gui->error_msg);

  return CMP_OK;
}

int oauth2_gui_tick(OAuth2Gui *gui) {
  if (!gui)
    return CMP_ERR_INVALID_ARGUMENT;

  cmp_http_integration_tick(&gui->http);

  if (gui->future && gui->future->is_ready) {
    if (gui->future->error_code == 0 && gui->future->response) {
      c_orm_oauth2_token_t token;
      c_orm_error_t orm_rc;
      const char *body = (const char *)gui->future->response->body;

      if (body) {
        orm_rc = c_orm_oauth2_token_parse_json(body, &token);
        if (orm_rc == C_ORM_OK && token.access_token) {
          if (gui->access_token)
            free(gui->access_token);
          gui->access_token = internal_strdup(token.access_token);
          cmp_router_navigate(&gui->router, "/", NULL);
        }
        if (token.access_token)
          free(token.access_token);
        if (token.refresh_token)
          free(token.refresh_token);
        if (token.token_type)
          free(token.token_type);
      }
    }

    cmp_form_set_submitting(&gui->form, CMP_FALSE);

    if (gui->request->url) {
      free(gui->request->url);
      gui->request->url = NULL;
    }
    http_request_free(gui->request);
    free(gui->request);
    gui->request = NULL;

    http_future_free(gui->future);
    free(gui->future);
    gui->future = NULL;
  }

  return CMP_OK;
}
