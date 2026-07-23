/* clang-format off */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#if defined(_WIN32) || defined(__CYGWIN__)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <winuser.h>
#endif
#include <stdlib.h>
#include "ui_engine.h"
#include "ui_window_backend.h"
#include "ui_renderer_gles2.h"
#include "ui_renderer.h"
#include "ui_dom_node.h"
#include "ui_component.h"
#include "ui_button_base.h"
#include "ui_event.h"
#include "ui_css_parser.h"
#include "ui_layout.h"
#include "ui_cssom_view.h"

#if defined(__EMSCRIPTEN__)
#include "ui_window_backend_web.h"
#elif defined(_WIN32) || defined(__CYGWIN__)
#include "ui_window_backend_win32.h"
#elif defined(__APPLE__)
#include "ui_window_backend_macos.h"
#elif defined(__linux__) || defined(__unix__)
#include "ui_window_backend_linux.h"
#endif
/* clang-format on */

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

static const char *TOOLBAR_CSS = "body {"
                                 "  display: flex;"
                                 "  flex-direction: column;"
                                 "  width: 100%;"
                                 "  height: 100%;"
                                 "}"
                                 "#toolbar {"
                                 "  display: flex;"
                                 "  flex-direction: row;"
                                 "  width: 100%;"
                                 "  height: 80px;"
                                 "  align-items: center;"
                                 "}"
                                 ".btn-host {"
                                 "  width: 120px;"
                                 "  height: 50px;"
                                 "  margin: 10px;"
                                 "}";

struct app_state {
  struct ui_dom_node *root;
  struct ui_dom_node *toolbar;
  struct ui_dom_node *status_text;
  struct ui_button_base *btn_theme;
  struct ui_button_base *btn_lang;
  int is_dark;
  int lang_idx;
};

struct app_context {
  struct app_state state;
  struct ui_css_stylesheet *stylesheet;
  struct ui_layout_node *layout_tree;
  float window_width;
  float window_height;
  int needs_layout;
};

static const char *languages[] = {"English", "Hebrew", "Arabic", "Japanese"};
static const char *themes[] = {"Light", "Dark"};

static enum ui_error update_ui(struct app_context *ctx) {
  char buffer[256];
  const char *lang = languages[ctx->state.lang_idx];
  const char *theme = themes[ctx->state.is_dark];

#if defined(_MSC_VER)
  sprintf_s(buffer, sizeof(buffer), "Language: %s | Theme: %s", lang, theme);
#else
  sprintf(buffer, "Language: %s | Theme: %s", lang, theme);
#endif

  ui_dom_node_set_text_content(ctx->state.status_text, buffer);

  if (ctx->state.lang_idx == 1 || ctx->state.lang_idx == 2) {
    ui_dom_node_set_attribute(ctx->state.toolbar, "dir", "rtl");
    ui_dom_node_set_attribute(ctx->state.toolbar, "style",
                              "flex-direction: row-reverse;");
  } else {
    ui_dom_node_set_attribute(ctx->state.toolbar, "dir", "ltr");
    ui_dom_node_set_attribute(ctx->state.toolbar, "style",
                              "flex-direction: row;");
  }

  if (ctx->state.is_dark) {
    ui_dom_node_set_attribute(ctx->state.toolbar, "data-theme", "dark");
  } else {
    ui_dom_node_set_attribute(ctx->state.toolbar, "data-theme", "light");
  }

  ctx->needs_layout = 1;
}

static enum ui_error on_theme_click(struct ui_button_base *button,
                                    void *user_data) {
  struct app_context *ctx = (struct app_context *)user_data;
  ctx->state.is_dark = !ctx->state.is_dark;
  printf("[Event] Theme toggled to %s\n", themes[ctx->state.is_dark]);
  update_ui(ctx);
}

static enum ui_error on_lang_click(struct ui_button_base *button,
                                   void *user_data) {
  struct app_context *ctx = (struct app_context *)user_data;
  ctx->state.lang_idx = (ctx->state.lang_idx + 1) % 4;
  printf("[Event] Language toggled to %s\n", languages[ctx->state.lang_idx]);
  update_ui(ctx);
}

static enum ui_error draw_layout_node(struct app_context *ctx,
                                      struct ui_renderer_backend *renderer,
                                      struct ui_layout_node *node) {
  struct ui_dom_rect rect;
  struct ui_color color = {0.8f, 0.8f, 0.8f, 1.0f};
  struct ui_layout_node *child;
  const char *id = NULL;
  const char *cls = NULL;

  ui_cssom_view_get_bounding_client_rect(node, &rect);

  if (node->dom_node) {
    ui_dom_node_get_attribute(node->dom_node, "id", &id);
    ui_dom_node_get_attribute(node->dom_node, "class", &cls);

    if (id && strcmp(id, "toolbar") == 0) {
      if (ctx->state.is_dark) {
        color.r = 0.2f;
        color.g = 0.2f;
        color.b = 0.2f;
      } else {
        color.r = 0.9f;
        color.g = 0.9f;
        color.b = 0.9f;
      }
    } else if (id && strcmp(id, "host-theme") == 0) {
      color.r = 0.8f;
      color.g = 0.3f;
      color.b = 0.3f;
    } else if (id && strcmp(id, "host-lang") == 0) {
      color.r = 0.3f;
      color.g = 0.3f;
      color.b = 0.8f;
    } else if (id && strcmp(id, "app") == 0) {
      color.r = 0.5f;
      color.g = 0.5f;
      color.b = 0.5f;
    }
  }

  if (rect.width > 0 && rect.height > 0) {
    renderer->draw_rect(renderer, (float)rect.x, (float)rect.y,
                        (float)rect.width, (float)rect.height, color);
  }

  child = node->first_child;
  while (child) {
    enum ui_error err = draw_layout_node(ctx, renderer, child);
    if (err != UI_ERROR_NONE)
      return err;
    child = child->next_sibling;
  }
  return UI_ERROR_NONE;
}

struct render_context {
  struct app_context *app_ctx;
  struct ui_renderer_backend *renderer;
  struct ui_window_backend *window_backend;
  struct ui_window *window;
  struct ui_engine *engine;
};

static enum ui_error do_render(struct render_context *rctx) {
  struct app_context *app_ctx = rctx->app_ctx;
  struct ui_renderer_backend *renderer = rctx->renderer;

  if (app_ctx->needs_layout) {
    if (app_ctx->layout_tree) {
      ui_layout_tree_destroy(app_ctx->layout_tree);
      app_ctx->layout_tree = NULL;
    }
    ui_layout_tree_generate(app_ctx->state.root, app_ctx->stylesheet,
                            &app_ctx->layout_tree);
    ui_layout_solve_viewport(app_ctx->layout_tree, app_ctx->window_width,
                             app_ctx->window_height);
    app_ctx->needs_layout = 0;
  }

  renderer->set_viewport(renderer, 0, 0, (int)app_ctx->window_width,
                         (int)app_ctx->window_height);
  {
    struct ui_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    renderer->clear(renderer, bg);
  }

  if (app_ctx->layout_tree) {
    draw_layout_node(app_ctx, renderer, app_ctx->layout_tree);
  }

  renderer->flush(renderer);
  rctx->window_backend->swap_buffers(rctx->window_backend, rctx->window);

  return ui_engine_tick(rctx->engine);
}

static enum ui_error on_resize_callback(void *user_data, int width,
                                        int height) {
  struct render_context *rctx = (struct render_context *)user_data;
  rctx->app_ctx->window_width = (float)width;
  rctx->app_ctx->window_height = (float)height;
  rctx->app_ctx->needs_layout = 1;
  return do_render(rctx);
}

#if defined(__EMSCRIPTEN__)
static struct app_context g_app_ctx;
static struct render_context g_rctx;
static void main_loop_step(void) {
  struct ui_event event;
  int has_event = 0;
  enum ui_error err;

  do {
    err = g_rctx.window_backend->poll_events(g_rctx.window_backend,
                                             g_rctx.window, &event, &has_event);
    if (err != UI_ERROR_NONE) {
      emscripten_cancel_main_loop();
      return;
    }
    if (has_event) {
      if (event.type == UI_EVENT_WINDOW_CLOSE) {
        emscripten_cancel_main_loop();
        return;
      } else if (event.type == UI_EVENT_WINDOW_RESIZE) {
        g_app_ctx.window_width = (float)event.event_data.window.width;
        g_app_ctx.window_height = (float)event.event_data.window.height;
        g_app_ctx.needs_layout = 1;
      }
    }
  } while (has_event);

  return do_render(&g_rctx);
}
#endif

int main(void) {
  struct ui_engine_config config;
  struct ui_engine *engine = NULL;
  struct ui_window_backend *backend = NULL;
  struct ui_renderer_backend *renderer = NULL;
  struct ui_window *window = NULL;
  struct app_context app_ctx;
  struct render_context rctx;
  struct ui_dom_node *host_theme;
  struct ui_dom_node *host_lang;
  enum ui_error err;
  int running = 1;
  int frame = 0;
  struct ui_event simulate_click;

  printf("Initializing Unstyled Components Windowed Example...\n");

  memset(&app_ctx, 0, sizeof(app_ctx));
  memset(&simulate_click, 0, sizeof(simulate_click));

  app_ctx.window_width = 800.0f;
  app_ctx.window_height = 600.0f;
  app_ctx.needs_layout = 1;

  config.num_threads = 2;
  err = ui_engine_create(&config, &engine);
  if (err != UI_ERROR_NONE)
    return 1;

#if defined(__EMSCRIPTEN__)
  err = ui_window_backend_web_create(&backend);
#elif defined(_WIN32) || defined(__CYGWIN__)
  err = ui_window_backend_win32_create(&backend);
#elif defined(__APPLE__)
  err = ui_window_backend_macos_create(&backend);
#elif defined(__linux__) || defined(__unix__)
  err = ui_window_backend_linux_create(&backend);
#else
  return 1;
#endif

  if (err != UI_ERROR_NONE)
    goto cleanup;

  err = backend->create_window(backend, "Unstyled Components (Simulated)",
                               (int)app_ctx.window_width,
                               (int)app_ctx.window_height, &window);
  if (err != UI_ERROR_NONE)
    goto cleanup;

  err = ui_renderer_gles2_create(&renderer);
  if (err != UI_ERROR_NONE)
    goto cleanup;
  renderer->init(renderer, backend, window);

  /* Setup DOM tree */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &app_ctx.state.root);
  ui_dom_node_set_tag_name(app_ctx.state.root, "body");
  ui_dom_node_set_attribute(app_ctx.state.root, "id", "app");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &app_ctx.state.toolbar);
  ui_dom_node_set_tag_name(app_ctx.state.toolbar, "div");
  ui_dom_node_set_attribute(app_ctx.state.toolbar, "id", "toolbar");
  ui_dom_node_append_child(app_ctx.state.root, app_ctx.state.toolbar);

  ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &app_ctx.state.status_text);
  ui_dom_node_append_child(app_ctx.state.toolbar, app_ctx.state.status_text);

  ui_button_base_create(&app_ctx.state.btn_theme);
  ui_button_base_set_on_click(app_ctx.state.btn_theme, on_theme_click,
                              &app_ctx);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &host_theme);
  ui_dom_node_set_attribute(host_theme, "class", "btn-host");
  ui_dom_node_set_attribute(host_theme, "id", "host-theme");
  ui_dom_node_append_child(app_ctx.state.toolbar, host_theme);
  {
    struct ui_component *tmp_comp;
    if (ui_button_base_get_component(app_ctx.state.btn_theme, &tmp_comp) ==
        UI_ERROR_NONE) {
      ui_component_mount(tmp_comp, host_theme);
    }
  }

  ui_button_base_create(&app_ctx.state.btn_lang);
  ui_button_base_set_on_click(app_ctx.state.btn_lang, on_lang_click, &app_ctx);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &host_lang);
  ui_dom_node_set_attribute(host_lang, "class", "btn-host");
  ui_dom_node_set_attribute(host_lang, "id", "host-lang");
  ui_dom_node_append_child(app_ctx.state.toolbar, host_lang);
  {
    struct ui_component *tmp_comp;
    if (ui_button_base_get_component(app_ctx.state.btn_lang, &tmp_comp) ==
        UI_ERROR_NONE) {
      ui_component_mount(tmp_comp, host_lang);
    }
  }

  ui_css_parse_stylesheet(TOOLBAR_CSS, &app_ctx.stylesheet);

  app_ctx.state.is_dark = 0;
  app_ctx.state.lang_idx = 0;
  update_ui(&app_ctx);

  rctx.app_ctx = &app_ctx;
  rctx.renderer = renderer;
  rctx.window_backend = backend;
  rctx.window = window;
  rctx.engine = engine;

  if (backend->set_on_resize_callback) {
    backend->set_on_resize_callback(backend, window, on_resize_callback, &rctx);
  }

  backend->show_window(backend, window);

  while (running) {
    struct ui_event event;
    int has_event = 0;

    do {
      err = backend->poll_events(backend, window, &event, &has_event);
      if (err != UI_ERROR_NONE) {
        running = 0;
        break;
      }
      if (has_event) {
        if (event.type == UI_EVENT_WINDOW_CLOSE) {
          running = 0;
        } else if (event.type == UI_EVENT_WINDOW_RESIZE) {
          app_ctx.window_width = (float)event.event_data.window.width;
          app_ctx.window_height = (float)event.event_data.window.height;
          app_ctx.needs_layout = 1;
        }
      }
    } while (has_event && running);

    if (!running)
      break;

    /* Simulate User Interactions periodically */
    frame++;
    if (frame == 60) {
      simulate_click.type = UI_EVENT_MOUSE_DOWN;
      ui_button_base_process_event(app_ctx.state.btn_theme, &simulate_click,
                                   0.0);
      simulate_click.type = UI_EVENT_MOUSE_UP;
      ui_button_base_process_event(app_ctx.state.btn_theme, &simulate_click,
                                   10.0);
    } else if (frame == 120) {
      simulate_click.type = UI_EVENT_MOUSE_DOWN;
      ui_button_base_process_event(app_ctx.state.btn_lang, &simulate_click,
                                   100.0);
      simulate_click.type = UI_EVENT_MOUSE_UP;
      ui_button_base_process_event(app_ctx.state.btn_lang, &simulate_click,
                                   110.0);
    } else if (frame == 180) {
      simulate_click.type = UI_EVENT_MOUSE_DOWN;
      ui_button_base_process_event(app_ctx.state.btn_lang, &simulate_click,
                                   200.0);
      simulate_click.type = UI_EVENT_MOUSE_UP;
      ui_button_base_process_event(app_ctx.state.btn_lang, &simulate_click,
                                   210.0);
    } else if (frame == 240) {
      simulate_click.type = UI_EVENT_MOUSE_DOWN;
      ui_button_base_process_event(app_ctx.state.btn_lang, &simulate_click,
                                   300.0);
      simulate_click.type = UI_EVENT_MOUSE_UP;
      ui_button_base_process_event(app_ctx.state.btn_lang, &simulate_click,
                                   310.0);
      frame = 0; /* Loop simulation */
    }

    do_render(&rctx);

    /* Slight delay to make simulation visible */
#if defined(_WIN32) || defined(__CYGWIN__)
    Sleep(16);
#endif
  }

cleanup:
  if (app_ctx.layout_tree)
    ui_layout_tree_destroy(app_ctx.layout_tree);
  if (app_ctx.state.root)
    ui_dom_node_destroy(app_ctx.state.root);
  ui_button_base_destroy(app_ctx.state.btn_theme);
  ui_button_base_destroy(app_ctx.state.btn_lang);
  if (renderer)
    renderer->destroy(renderer);
  if (window)
    backend->destroy_window(backend, window);
  if (backend) {
#if defined(__EMSCRIPTEN__)
    ui_window_backend_web_destroy(backend);
#elif defined(_WIN32) || defined(__CYGWIN__)
    ui_window_backend_win32_destroy(backend);
#elif defined(__APPLE__)
    ui_window_backend_macos_destroy(backend);
#elif defined(__linux__) || defined(__unix__)
    ui_window_backend_linux_destroy(backend);
#endif
  }
  ui_engine_destroy(engine);

  return 0;
}
