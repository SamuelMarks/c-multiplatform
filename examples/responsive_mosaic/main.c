/* clang-format off */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ui_engine.h"
#include "ui_window_backend.h"
#include "ui_renderer_gles2.h"
#include "ui_renderer.h"
#include "ui_event.h"
#include "ui_dom_node.h"
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

static const char *MOSAIC_CSS = "#app {"
                                "  display: flex;"
                                "  flex-direction: column;"
                                "  width: 100%;"
                                "  height: 100%;"
                                "}"
                                "#toolbar {"
                                "  display: flex;"
                                "  flex-direction: row;"
                                "  width: 100%;"
                                "  height: 60px;"
                                "}"
                                "#grid {"
                                "  display: flex;"
                                "  flex-direction: row;"
                                "  flex-wrap: wrap;"
                                "  flex-grow: 1;"
                                "  width: 100%;"
                                "  align-content: flex-start;"
                                "}"
                                ".item {"
                                "  flex-grow: 1;"
                                "  margin: 10px;"
                                "}"
                                ".item-small {"
                                "  width: 150px;"
                                "  height: 150px;"
                                "}"
                                ".item-wide {"
                                "  width: 320px;"
                                "  height: 150px;"
                                "}"
                                ".item-tall {"
                                "  width: 150px;"
                                "  height: 320px;"
                                "}"
                                ".item-large {"
                                "  width: 320px;"
                                "  height: 320px;"
                                "}";

struct app_context {
  struct ui_dom_node *root;
  struct ui_css_stylesheet *stylesheet;
  struct ui_layout_node *layout_tree;
  float window_width;
  float window_height;
  int needs_layout;
};

static enum ui_error create_dom(struct app_context *ctx) {
  struct ui_dom_node *app, *toolbar, *grid, *item;
  int i;
  char id_buf[32];

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &app);
  ui_dom_node_set_tag_name(app, "body");
  ui_dom_node_set_attribute(app, "id", "app");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &toolbar);
  ui_dom_node_set_attribute(toolbar, "id", "toolbar");
  ui_dom_node_append_child(app, toolbar);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &grid);
  ui_dom_node_set_attribute(grid, "id", "grid");
  ui_dom_node_append_child(app, grid);

  for (i = 0; i < 20; i++) {
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &item);
    if (i % 5 == 0) {
      ui_dom_node_set_attribute(item, "class", "item item-large");
    } else if (i % 4 == 0) {
      ui_dom_node_set_attribute(item, "class", "item item-wide");
    } else if (i % 3 == 0) {
      ui_dom_node_set_attribute(item, "class", "item item-tall");
    } else {
      ui_dom_node_set_attribute(item, "class", "item item-small");
    }

#if defined(_MSC_VER)
    sprintf_s(id_buf, sizeof(id_buf), "item-%d", i);
#else
    sprintf(id_buf, "item-%d", i);
#endif
    ui_dom_node_set_attribute(item, "id", id_buf);
    ui_dom_node_append_child(grid, item);
  }

  ctx->root = app;
}

static enum ui_error draw_layout_node(struct ui_renderer_backend *renderer,
                                      struct ui_layout_node *node, int depth) {
  struct ui_dom_rect rect;
  struct ui_color color = {0.8f, 0.8f, 0.8f, 1.0f}; /* Default gray */
  struct ui_layout_node *child;
  const char *id = NULL;
  const char *cls = NULL;

  ui_cssom_view_get_bounding_client_rect(node, &rect);

  if (node->dom_node) {
    ui_dom_node_get_attribute(node->dom_node, "id", &id);
    ui_dom_node_get_attribute(node->dom_node, "class", &cls);

    if (id && strcmp(id, "toolbar") == 0) {
      color.r = 0.2f;
      color.g = 0.2f;
      color.b = 0.8f; /* Blue */
    } else if (cls && strstr(cls, "item") != NULL) {
      /* Generate a pseudo-random color based on pointer hash or similar */
      float r = (float)(((size_t)node >> 4) % 255) / 255.0f;
      float g = (float)(((size_t)node >> 8) % 255) / 255.0f;
      float b = (float)(((size_t)node >> 12) % 255) / 255.0f;
      color.r = 0.2f + (r * 0.8f);
      color.g = 0.2f + (g * 0.8f);
      color.b = 0.2f + (b * 0.8f);
    } else if (id && strcmp(id, "app") == 0) {
      color.r = 0.95f;
      color.g = 0.95f;
      color.b = 0.95f; /* Light background */
    }
  }

  /* Draw the bounding box */
  if (rect.width > 0 && rect.height > 0) {
    renderer->draw_rect(renderer, (float)rect.x, (float)rect.y,
                        (float)rect.width, (float)rect.height, color);
  }

  /* Recurse */
  child = node->first_child;
  while (child) {
    enum ui_error err = draw_layout_node(renderer, child, depth + 1);
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

  /* Recompute Layout if needed */
  if (app_ctx->needs_layout) {
    if (app_ctx->layout_tree) {
      ui_layout_tree_destroy(app_ctx->layout_tree);
      app_ctx->layout_tree = NULL;
    }
    ui_layout_tree_generate(app_ctx->root, app_ctx->stylesheet,
                            &app_ctx->layout_tree);
    ui_layout_solve_viewport(app_ctx->layout_tree, app_ctx->window_width,
                             app_ctx->window_height);
    ui_layout_compute(app_ctx->layout_tree, app_ctx->window_width,
                      app_ctx->window_height);
    app_ctx->needs_layout = 0;
  }

  /* Render */
  renderer->set_viewport(renderer, 0, 0, (int)app_ctx->window_width,
                         (int)app_ctx->window_height);

  {
    struct ui_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    renderer->clear(renderer, bg);
  }

  if (app_ctx->layout_tree) {
    draw_layout_node(renderer, app_ctx->layout_tree, 0);
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
  struct ui_window_backend *window_backend = NULL;
  struct ui_renderer_backend *renderer = NULL;
  struct ui_window *window = NULL;
  struct app_context app_ctx;
  struct render_context rctx;
  enum ui_error err;
  int running = 1;

  memset(&app_ctx, 0, sizeof(app_ctx));
  app_ctx.window_width = 800.0f;
  app_ctx.window_height = 600.0f;
  app_ctx.needs_layout = 1;

  printf("Starting Responsive Mosaic Example...\n");

  /* 1. Setup Engine */
  config.num_threads = 2;
  err = ui_engine_create(&config, &engine);
  if (err != UI_ERROR_NONE) {
    printf("Failed to create engine.\n");
    return 1;
  }

  /* 2. Create Window Backend */
#if defined(__EMSCRIPTEN__)
  err = ui_window_backend_web_create(&window_backend);
#elif defined(_WIN32) || defined(__CYGWIN__)
  err = ui_window_backend_win32_create(&window_backend);
#elif defined(__APPLE__)
  err = ui_window_backend_macos_create(&window_backend);
#elif defined(__linux__) || defined(__unix__)
  err = ui_window_backend_linux_create(&window_backend);
#else
  printf("Unsupported platform.\n");
  return 1;
#endif

  /* 3. Create Window */
  err = window_backend->create_window(
      window_backend, "Responsive Mosaic (Watch -> TV)",
      (int)app_ctx.window_width, (int)app_ctx.window_height, &window);
  if (err != UI_ERROR_NONE)
    goto cleanup;

  /* 4. Create Renderer */
  err = ui_renderer_gles2_create(&renderer);
  if (err != UI_ERROR_NONE) {
    printf("Failed to create GLES2 renderer.\n");
    goto cleanup;
  }
  renderer->init(renderer, window_backend, window);

  /* 5. Initialize DOM and CSS */
  create_dom(&app_ctx);
  ui_css_parse_stylesheet(MOSAIC_CSS, &app_ctx.stylesheet);

  rctx.app_ctx = &app_ctx;
  rctx.renderer = renderer;
  rctx.window_backend = window_backend;
  rctx.window = window;
  rctx.engine = engine;

  if (window_backend->set_on_resize_callback) {
    window_backend->set_on_resize_callback(window_backend, window,
                                           on_resize_callback, &rctx);
  }

  window_backend->show_window(window_backend, window);
  printf("Resize the window to see the flex-wrap mosaic adapt!\n");

/* 6. Main Loop */
#if defined(__EMSCRIPTEN__)
  g_app_ctx = app_ctx;
  g_rctx = rctx;
  g_rctx.app_ctx = &g_app_ctx;
  if (window_backend->set_on_resize_callback) {
    window_backend->set_on_resize_callback(window_backend, window,
                                           on_resize_callback, &g_rctx);
  }
  emscripten_set_main_loop(main_loop_step, 0, 1);
#else
  while (running) {
    struct ui_event event;
    int has_event = 0;

    do {
      err = window_backend->poll_events(window_backend, window, &event,
                                        &has_event);
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

    do_render(&rctx);
  }
#endif

cleanup:
  if (app_ctx.layout_tree)
    ui_layout_tree_destroy(app_ctx.layout_tree);
  if (app_ctx.root)
    ui_dom_node_destroy(app_ctx.root);
  if (renderer)
    renderer->destroy(renderer);
  if (window)
    window_backend->destroy_window(window_backend, window);
  if (window_backend) {
#if defined(__EMSCRIPTEN__)
    ui_window_backend_web_destroy(window_backend);
#elif defined(_WIN32) || defined(__CYGWIN__)
    ui_window_backend_win32_destroy(window_backend);
#elif defined(__APPLE__)
    ui_window_backend_macos_destroy(window_backend);
#elif defined(__linux__) || defined(__unix__)
    ui_window_backend_linux_destroy(window_backend);
#endif
  }
  if (engine)
    ui_engine_destroy(engine);

  return 0;
}
