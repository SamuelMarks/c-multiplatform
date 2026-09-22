/**
 * @file main.c
 * @brief Responsive Mosaic layout example application.
 */

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
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif
/* clang-format on */

/**
 * @brief CSS stylesheet for the responsive mosaic example.
 */
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

/**
 * @struct app_context
 * @brief Application state for responsive mosaic example.
 */
struct app_context {
  struct ui_dom_node *root;             /**< Root DOM node */
  struct ui_css_stylesheet *stylesheet; /**< Parsed CSS stylesheet */
  struct ui_layout_node *layout_tree;   /**< Computed layout tree */
  float window_width;                   /**< Current window width */
  float window_height;                  /**< Current window height */
  int needs_layout; /**< Flag indicating layout recalculation needed */
};

/**
 * @brief Creates DOM hierarchy with toolbar and mosaic grid items.
 * @param ctx Pointer to application context.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t create_dom(struct app_context *ctx) {
  struct ui_dom_node *app = NULL;
  struct ui_dom_node *toolbar = NULL;
  struct ui_dom_node *grid = NULL;
  struct ui_dom_node *item = NULL;
  int i;
  char id_buf[32];
  ui_error_t err;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &app);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_set_tag_name(app, "body");
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_set_attribute(app, "id", "app");
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &toolbar);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_set_attribute(toolbar, "id", "toolbar");
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_append_child(app, toolbar);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &grid);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_set_attribute(grid, "id", "grid");
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_append_child(app, grid);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  for (i = 0; i < 20; i++) {
    err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &item);
    if (err != UI_ERROR_NONE) {
      return err;
    }
    if (i % 5 == 0) {
      err = ui_dom_node_set_attribute(item, "class", "item item-large");
      if (err != UI_ERROR_NONE) {
        return err;
      }
    } else if (i % 4 == 0) {
      err = ui_dom_node_set_attribute(item, "class", "item item-wide");
      if (err != UI_ERROR_NONE) {
        return err;
      }
    } else if (i % 3 == 0) {
      err = ui_dom_node_set_attribute(item, "class", "item item-tall");
      if (err != UI_ERROR_NONE) {
        return err;
      }
    } else {
      err = ui_dom_node_set_attribute(item, "class", "item item-small");
      if (err != UI_ERROR_NONE) {
        return err;
      }
    }

#if defined(_MSC_VER)
    sprintf_s(id_buf, sizeof(id_buf), "item-%d", i);
#else
    sprintf(id_buf, "item-%d", i);
#endif
    err = ui_dom_node_set_attribute(item, "id", id_buf);
    if (err != UI_ERROR_NONE) {
      return err;
    }
    err = ui_dom_node_append_child(grid, item);
    if (err != UI_ERROR_NONE) {
      return err;
    }
  }

  ctx->root = app;
  return UI_ERROR_NONE;
}

/**
 * @brief Recursively renders a layout node and its children.
 * @param renderer Pointer to renderer backend.
 * @param node Pointer to layout node.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t draw_layout_node(struct ui_renderer_backend *renderer,
                                   struct ui_layout_node *node) {
  struct ui_dom_rect rect;
  struct ui_color color;
  struct ui_layout_node *child;
  const char *id = NULL;
  const char *cls = NULL;
  float r_val;
  float g_val;
  float b_val;
  ui_error_t err;

  color.r = 0.8f;
  color.g = 0.8f;
  color.b = 0.8f;
  color.a = 1.0f;

  err = ui_cssom_view_get_bounding_client_rect(node, &rect);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  if (node->dom_node) {
    err = ui_dom_node_get_attribute(node->dom_node, "id", &id);
    if (err != UI_ERROR_NONE && err != UI_ERROR_NOT_FOUND) {
      return err;
    }
    err = ui_dom_node_get_attribute(node->dom_node, "class", &cls);
    if (err != UI_ERROR_NONE && err != UI_ERROR_NOT_FOUND) {
      return err;
    }

    if (id && strcmp(id, "toolbar") == 0) {
      color.r = 0.2f;
      color.g = 0.2f;
      color.b = 0.8f;
    } else if (cls && strstr(cls, "item") != NULL) {
      r_val = (float)(((size_t)node >> 4) % 255) / 255.0f;
      g_val = (float)(((size_t)node >> 8) % 255) / 255.0f;
      b_val = (float)(((size_t)node >> 12) % 255) / 255.0f;
      color.r = 0.2f + (r_val * 0.8f);
      color.g = 0.2f + (g_val * 0.8f);
      color.b = 0.2f + (b_val * 0.8f);
    } else if (id && strcmp(id, "app") == 0) {
      color.r = 0.95f;
      color.g = 0.95f;
      color.b = 0.95f;
    }
  }

  if (rect.width > 0 && rect.height > 0) {
    err = renderer->draw_rect(renderer, (float)rect.x, (float)rect.y,
                              (float)rect.width, (float)rect.height, color);
    if (err != UI_ERROR_NONE) {
      return err;
    }
  }

  child = node->first_child;
  while (child) {
    err = draw_layout_node(renderer, child);
    if (err != UI_ERROR_NONE) {
      return err;
    }
    child = child->next_sibling;
  }
  return UI_ERROR_NONE;
}

/**
 * @struct render_context
 * @brief Context aggregate for rendering operations.
 */
struct render_context {
  struct app_context *app_ctx;              /**< Application context */
  struct ui_renderer_backend *renderer;     /**< Renderer backend */
  struct ui_window_backend *window_backend; /**< Window backend */
  struct ui_window *window;                 /**< Native window handle */
  struct ui_engine *engine;                 /**< Core UI engine */
};

/**
 * @brief Solves layout and renders the current frame.
 * @param rctx Pointer to render context.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t do_render(struct render_context *rctx) {
  struct app_context *app_ctx = rctx->app_ctx;
  struct ui_renderer_backend *renderer = rctx->renderer;
  struct ui_color bg;
  ui_error_t err;

  bg.r = 1.0f;
  bg.g = 1.0f;
  bg.b = 1.0f;
  bg.a = 1.0f;

  if (app_ctx->needs_layout) {
    if (app_ctx->layout_tree) {
      err = ui_layout_tree_destroy(app_ctx->layout_tree);
      if (err != UI_ERROR_NONE) {
        return err;
      }
      app_ctx->layout_tree = NULL;
    }
    err = ui_layout_tree_generate(app_ctx->root, app_ctx->stylesheet,
                                  &app_ctx->layout_tree);
    if (err != UI_ERROR_NONE) {
      return err;
    }
    err = ui_layout_solve_viewport(app_ctx->layout_tree, app_ctx->window_width,
                                   app_ctx->window_height);
    if (err != UI_ERROR_NONE) {
      return err;
    }
    app_ctx->needs_layout = 0;
  }

  err = ui_engine_tick(rctx->engine);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = renderer->set_viewport(renderer, 0, 0, (int)app_ctx->window_width,
                               (int)app_ctx->window_height);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = renderer->clear(renderer, bg);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  if (app_ctx->layout_tree) {
    err = draw_layout_node(renderer, app_ctx->layout_tree);
    if (err != UI_ERROR_NONE) {
      return err;
    }
  }

  err = renderer->flush(renderer);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = rctx->window_backend->swap_buffers(rctx->window_backend, rctx->window);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Window resize event callback.
 * @param user_data User data pointer (render context).
 * @param width New window width.
 * @param height New window height.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t on_resize_callback(void *user_data, int width, int height) {
  struct render_context *rctx = (struct render_context *)user_data;
  rctx->app_ctx->window_width = (float)width;
  rctx->app_ctx->window_height = (float)height;
  rctx->app_ctx->needs_layout = 1;
  return do_render(rctx);
}

#if defined(__EMSCRIPTEN__)
static struct app_context g_app_ctx;
static struct render_context g_rctx;

/**
 * @brief Emscripten main loop tick.
 */
static void main_loop_step(void) {
  struct ui_event event;
  int has_event = 0;
  ui_error_t err;

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

  {
    ui_error_t rc_render = do_render(&g_rctx);
    if (rc_render != UI_ERROR_NONE) {
      emscripten_cancel_main_loop();
      return;
    }
  }
}
#endif

#ifndef OMIT_MAIN
/**
 * @brief Program entry point.
 * @return 0 on success, non-zero on failure.
 */
int main(void) {
#else
/**
 * @brief Testable entry point for responsive mosaic example.
 * @return 0 on success, non-zero on failure.
 */
int example_responsive_main(void) {
#endif
  struct ui_engine_config config;
  struct ui_engine *engine = NULL;
  struct ui_window_backend *window_backend = NULL;
  struct ui_renderer_backend *renderer = NULL;
  struct ui_window *window = NULL;
  struct app_context app_ctx;
  struct render_context rctx;
  struct ui_event event;
  const char *ci_test = NULL;
  int running = 1;
  int frame_count = 0;
  int has_event = 0;
  int exit_code = 0;
  ui_error_t err;

  memset(&app_ctx, 0, sizeof(app_ctx));
  memset(&rctx, 0, sizeof(rctx));
  memset(&event, 0, sizeof(event));
  app_ctx.window_width = 800.0f;
  app_ctx.window_height = 600.0f;
  app_ctx.needs_layout = 1;

  printf("Starting Responsive Mosaic Example...\n");

  config.num_threads = 2;
  err = ui_engine_create(&config, &engine);
  if (err != UI_ERROR_NONE) {
    printf("Failed to create engine.\n");
    return 1;
  }

  err = create_dom(&app_ctx);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  err = ui_css_parse_stylesheet(MOSAIC_CSS, &app_ctx.stylesheet);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

#if defined(__EMSCRIPTEN__)
  err = ui_window_backend_web_create(&window_backend);
#elif defined(_WIN32) || defined(__CYGWIN__)
  err = ui_window_backend_win32_create(&window_backend);
#elif defined(__APPLE__)
err = ui_window_backend_macos_create(&window_backend);
#elif defined(__linux__) || defined(__unix__)
err = ui_window_backend_linux_create(&window_backend);
#endif

  if (window_backend != NULL && err == UI_ERROR_NONE) {
    err = window_backend->create_window(
        window_backend, "Responsive Mosaic (Watch -> TV)",
        (int)app_ctx.window_width, (int)app_ctx.window_height, &window);
    if (err == UI_ERROR_NONE && window != NULL) {
      err = ui_renderer_gles2_create(&renderer);
      if (err == UI_ERROR_NONE && renderer != NULL) {
        err = renderer->init(renderer, window_backend, window);
        if (err != UI_ERROR_NONE) {
          renderer = NULL;
        }
      }
      if (renderer != NULL) {
        rctx.app_ctx = &app_ctx;
        rctx.renderer = renderer;
        rctx.window_backend = window_backend;
        rctx.window = window;
        rctx.engine = engine;

        if (window_backend->set_on_resize_callback) {
          window_backend->set_on_resize_callback(window_backend, window,
                                                 on_resize_callback, &rctx);
        }

        err = window_backend->show_window(window_backend, window);
        if (err != UI_ERROR_NONE) {
          renderer = NULL;
        }
      }
    }
  }

  if (window_backend != NULL && window != NULL && renderer != NULL) {
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
#if defined(CI_TEST_RUN)
    ci_test = "1";
#else
    ci_test = getenv("CI_TEST_RUN");
#endif

    while (running) {
      if (ci_test && frame_count++ > 2) {
        break;
      }

      do {
        err = window_backend->poll_events(window_backend, window, &event,
                                          &has_event);
        if (err != UI_ERROR_NONE) {
          running = 0;
          exit_code = 1;
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

      if (!running) {
        break;
      }

      err = do_render(&rctx);
      if (err != UI_ERROR_NONE) {
        exit_code = 1;
        break;
      }
    }
#endif
  } else {
    err = ui_layout_tree_generate(app_ctx.root, app_ctx.stylesheet,
                                  &app_ctx.layout_tree);
    if (err != UI_ERROR_NONE) {
      exit_code = 1;
      goto cleanup;
    }
    err = ui_layout_solve_viewport(app_ctx.layout_tree, app_ctx.window_width,
                                   app_ctx.window_height);
    if (err != UI_ERROR_NONE) {
      exit_code = 1;
      goto cleanup;
    }
    err = ui_engine_tick(engine);
    if (err != UI_ERROR_NONE) {
      exit_code = 1;
      goto cleanup;
    }
  }

cleanup:
  if (app_ctx.layout_tree) {
    err = ui_layout_tree_destroy(app_ctx.layout_tree);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }
  if (app_ctx.stylesheet) {
    err = ui_css_stylesheet_destroy(app_ctx.stylesheet);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }
  if (app_ctx.root) {
    err = ui_dom_node_destroy(app_ctx.root);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }
  if (renderer) {
    err = renderer->destroy(renderer);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }
  if (window && window_backend) {
    err = window_backend->destroy_window(window_backend, window);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }
  if (window_backend) {
#if defined(__EMSCRIPTEN__)
    err = ui_window_backend_web_destroy(window_backend);
#elif defined(_WIN32) || defined(__CYGWIN__)
    err = ui_window_backend_win32_destroy(window_backend);
#elif defined(__APPLE__)
  err = ui_window_backend_macos_destroy(window_backend);
#elif defined(__linux__) || defined(__unix__)
  err = ui_window_backend_linux_destroy(window_backend);
#endif
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }
  if (engine) {
    err = ui_engine_destroy(engine);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }

  return exit_code;
}
