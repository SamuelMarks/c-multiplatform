#include <app/demo_app.h>
#include <cmpc/cmp_backend_gtk4.h>
#include <gtk/gtk.h>

int main(int argc, char **argv) {
  gtk_init();

  CMPGTK4Backend *backend;
  CMPGTK4BackendConfig config;
  cmp_gtk4_backend_config_init(&config);
  config.enable_gdk_texture = CMP_TRUE;

  if (cmp_gtk4_backend_create(&config, &backend) != CMP_OK)
    return 1;

  CMPWS ws;
  CMPGfx gfx;
  CMPEnv env;
  cmp_gtk4_backend_get_ws(backend, &ws);
  cmp_gtk4_backend_get_gfx(backend, &gfx);
  cmp_gtk4_backend_get_env(backend, &env);

  CMPHandle window;
  CMPWSWindowConfig wcfg = {
      .width = 400, .height = 700, .utf8_title = "Demo GTK4"};
  ws.vtable->create_window(ws.ctx, &wcfg, &window);
  ws.vtable->show_window(ws.ctx, window);

  CMPAllocator alloc;
  cmp_get_default_allocator(&alloc);
  DemoApp *app;
  demo_app_create(&alloc, &app);
  demo_app_init_resources(app, &gfx, &env);

  /* Simple polling loop for demo purposes event though GTK is event-driven */
  while (g_list_model_get_n_items(gtk_window_get_toplevels()) > 0) {
    g_main_context_iteration(NULL, FALSE);

    CMPInputEvent evt;
    CMPBool has;
    while (1) {
      ws.vtable->poll_event(ws.ctx, &evt, &has);
      if (!has)
        break;
      if (evt.type == CMP_INPUT_WINDOW_CLOSE) {
        // Handle close logic
      }
      CMPBool h;
      demo_app_handle_event(app, &evt, &h);
    }

    demo_app_update(app, 0.016);

    int w, h;
    CMPScalar s;
    ws.vtable->get_window_size(ws.ctx, window, &w, &h);
    ws.vtable->get_window_dpi_scale(ws.ctx, window, &s);

    demo_app_render(app, &gfx, window, w, h, (float)s);
  }

  demo_app_destroy(app);
  cmp_gtk4_backend_destroy(backend);
  return 0;
}