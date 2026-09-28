/**
 * @file main.c
 * @brief Window basic example application showcasing layout and rendering.
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
#include "ui_test_visual.h"

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
 * @brief CSS stylesheet for the window basic example.
 */
static const char *WINDOW_CSS = "#app {"
                                "  display: flex;"
                                "  flex-direction: row;"
                                "  width: 100%;"
                                "  height: 100%;"
                                "}"
                                "#sidebar {"
                                "  width: 250px;"
                                "  height: 100%;"
                                "  flex-shrink: 0;"
                                "}"
                                "#content {"
                                "  display: flex;"
                                "  flex-direction: column;"
                                "  flex-grow: 1;"
                                "  height: 100%;"
                                "  padding: 20px;"
                                "}"
                                ".box {"
                                "  flex-grow: 1;"
                                "  margin: 10px;"
                                "}";

/**
 * @struct app_context
 * @brief State structure for window basic example.
 */
struct app_context {
  struct ui_dom_node *root;             /**< Root DOM node */
  struct ui_css_stylesheet *stylesheet; /**< Parsed stylesheet */
  struct ui_layout_node *layout_tree;   /**< Computed layout tree */
  float window_width;                   /**< Window width */
  float window_height;                  /**< Window height */
  int needs_layout;                     /**< Needs layout recalculation flag */
};

/**
 * @brief Creates the initial DOM structure.
 * @param ctx Pointer to the application context.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t create_dom(struct app_context *ctx) {
  struct ui_dom_node *app = NULL, *sidebar = NULL, *content = NULL,
                     *box1 = NULL, *box2 = NULL;
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

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sidebar);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_set_attribute(sidebar, "id", "sidebar");
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_append_child(app, sidebar);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_set_attribute(content, "id", "content");
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_append_child(app, content);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &box1);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_set_attribute(box1, "class", "box");
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_set_attribute(box1, "id", "box1");
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_append_child(content, box1);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &box2);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_set_attribute(box2, "class", "box");
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_set_attribute(box2, "id", "box2");
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_append_child(content, box2);
  if (err != UI_ERROR_NONE) {
    return err;
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
  ui_error_t err;

  color.r = 0.9f;
  color.g = 0.9f;
  color.b = 0.9f;
  color.a = 1.0f;

  err = ui_cssom_view_get_bounding_client_rect(node, &rect);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  if (node->dom_node) {
    err = ui_dom_node_get_attribute(node->dom_node, "id", &id);
    if (err != UI_ERROR_NONE) {
      return err;
    }

    if (id && strcmp(id, "app") == 0) {
      color.r = 1.0f;
      color.g = 1.0f;
      color.b = 1.0f;
    } else if (id && strcmp(id, "sidebar") == 0) {
      color.r = 0.2f;
      color.g = 0.2f;
      color.b = 0.2f;
    } else if (id && strcmp(id, "content") == 0) {
      color.r = 0.95f;
      color.g = 0.95f;
      color.b = 0.95f;
    } else if (id && strcmp(id, "box1") == 0) {
      color.r = 0.8f;
      color.g = 0.3f;
      color.b = 0.3f;
    } else if (id && strcmp(id, "box2") == 0) {
      color.r = 0.3f;
      color.g = 0.8f;
      color.b = 0.3f;
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
 * @brief Conditionally saves a screenshot to disk if UI_SCREENSHOT_PATH is set.
 * @param rctx Pointer to render context.
 * @param width Window width.
 * @param height Window height.
 * @return UI_ERROR_NONE on success or if unconfigured, or an error code on
 * failure.
 */
static ui_error_t maybe_save_screenshot(struct render_context *rctx,
                                        float width, float height) {
  const char *shot_path = NULL;
  unsigned char *pixels = NULL;
  unsigned char *row_tmp = NULL;
  int w = (int)width;
  int h = (int)height;
  int row;
  ui_error_t err = UI_ERROR_NONE;
#if defined(_MSC_VER)
  char *env_val = NULL;
  size_t env_len = 0;
  if (_dupenv_s(&env_val, &env_len, "UI_SCREENSHOT_PATH") == 0 &&
      env_val != NULL) {
    shot_path = env_val;
  }
#else
  shot_path = getenv("UI_SCREENSHOT_PATH");
#endif

  if (!shot_path) {
#if defined(_MSC_VER)
    if (env_val) {
      free(env_val);
    }
#endif
    return UI_ERROR_NONE;
  }

  if (!rctx || !rctx->renderer || !rctx->renderer->read_pixels || w <= 0 ||
      h <= 0) {
#if defined(_MSC_VER)
    if (env_val) {
      free(env_val);
    }
#endif
    return UI_ERROR_INVALID_ARGUMENT;
  }

  pixels = (unsigned char *)malloc((size_t)w * (size_t)h * 4);
  if (!pixels) {
#if defined(_MSC_VER)
    if (env_val) {
      free(env_val);
    }
#endif
    return UI_ERROR_OUT_OF_MEMORY;
  }

  err = rctx->renderer->read_pixels(rctx->renderer, w, h, pixels);
  if (err != UI_ERROR_NONE) {
    free(pixels);
#if defined(_MSC_VER)
    if (env_val) {
      free(env_val);
    }
#endif
    return err;
  }

  row_tmp = (unsigned char *)malloc((size_t)w * 4);
  if (row_tmp) {
    for (row = 0; row < h / 2; ++row) {
      unsigned char *top = pixels + (size_t)row * (size_t)w * 4;
      unsigned char *bot = pixels + (size_t)(h - 1 - row) * (size_t)w * 4;
      memcpy(row_tmp, top, (size_t)w * 4);
      memcpy(top, bot, (size_t)w * 4);
      memcpy(bot, row_tmp, (size_t)w * 4);
    }
    free(row_tmp);
  }

  err = ui_visual_write_heatmap_to_disk(shot_path, pixels, w, h);

  free(pixels);
#if defined(_MSC_VER)
  if (env_val) {
    free(env_val);
  }
#endif
  return err;
}

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
  err = maybe_save_screenshot(rctx, app_ctx->window_width,
                              app_ctx->window_height);
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
 * @brief Resize event callback.
 * @param user_data User data pointer (render context).
 * @param width New window width.
 * @param height New window height.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t on_resize_callback(void *user_data, int width, int height) {
  struct render_context *rctx = (struct render_context *)user_data;
  if (!rctx || !rctx->app_ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  rctx->app_ctx->window_width = (float)width;
  rctx->app_ctx->window_height = (float)height;
  rctx->app_ctx->needs_layout = 1;
  if (rctx->renderer) {
    return do_render(rctx);
  }
  return UI_ERROR_NONE;
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
 * @brief Testable entry point for window basic example.
 * @return 0 on success, non-zero on failure.
 */
int example_window_main(void) {
#endif
  struct ui_engine_config config;
  struct ui_engine *engine = NULL;
  struct ui_window_backend *backend = NULL;
  struct ui_renderer_backend *renderer = NULL;
  struct ui_window *window = NULL;
  struct app_context app_ctx;
  struct render_context rctx;
  struct ui_event event;
#if !defined(__EMSCRIPTEN__)
  const char *ci_test = NULL;
  int running = 1;
  int frame_count = 0;
  int has_event = 0;
#if !defined(CI_TEST_RUN) && defined(_MSC_VER)
  char *ci_env_val = NULL;
  size_t ci_env_len = 0;
#endif
#endif
#if defined(_WIN32) || defined(__CYGWIN__)
  int is_wine = 0;
#if defined(_MSC_VER)
  char *wine_val = NULL;
  size_t wine_len = 0;
#endif
#endif
  int exit_code = 0;
  ui_error_t err;

  memset(&app_ctx, 0, sizeof(app_ctx));
  memset(&rctx, 0, sizeof(rctx));
  memset(&event, 0, sizeof(event));
  app_ctx.window_width = 800.0f;
  app_ctx.window_height = 600.0f;
  app_ctx.needs_layout = 1;

  printf("Starting UI Engine Headful Window Example...\n");

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

  err = ui_css_parse_stylesheet(WINDOW_CSS, &app_ctx.stylesheet);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

#if defined(__EMSCRIPTEN__)
  err = ui_window_backend_web_create(&backend);
#elif defined(_WIN32) || defined(__CYGWIN__)
  err = ui_window_backend_win32_create(&backend);
#elif defined(__APPLE__)
err = ui_window_backend_macos_create(&backend);
#elif defined(__linux__) || defined(__unix__)
err = ui_window_backend_linux_create(&backend);
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(_MSC_VER)
  if (_dupenv_s(&wine_val, &wine_len, "WINELOADER") == 0 && wine_val != NULL) {
    is_wine = 1;
    free(wine_val);
  }
#else
  if (getenv("WINELOADER") != NULL) {
    is_wine = 1;
  }
#endif
  if (is_wine) {
    if (backend) {
      err = ui_window_backend_win32_destroy(backend);
      if (err != UI_ERROR_NONE && exit_code == 0) {
        exit_code = 1;
      }
      backend = NULL;
    }
  }
#endif

  if (backend != NULL && err == UI_ERROR_NONE) {
    err = backend->create_window(backend, "C-Multiplatform Window Basic",
                                 (int)app_ctx.window_width,
                                 (int)app_ctx.window_height, &window);
    if (err == UI_ERROR_NONE && window != NULL) {
      err = ui_renderer_gles2_create(&renderer);
      if (err == UI_ERROR_NONE && renderer != NULL) {
        err = renderer->init(renderer, backend, window);
        if (err != UI_ERROR_NONE) {
          err = ui_renderer_gles2_destroy(renderer);
          if (err != UI_ERROR_NONE && exit_code == 0) {
            exit_code = 1;
          }
          renderer = NULL;
        }
      }
      if (renderer != NULL) {
        rctx.app_ctx = &app_ctx;
        rctx.renderer = renderer;
        rctx.window_backend = backend;
        rctx.window = window;
        rctx.engine = engine;

        if (backend->set_on_resize_callback) {
          backend->set_on_resize_callback(backend, window, on_resize_callback,
                                          &rctx);
        }

        err = backend->show_window(backend, window);
        if (err != UI_ERROR_NONE) {
          err = ui_renderer_gles2_destroy(renderer);
          if (err != UI_ERROR_NONE && exit_code == 0) {
            exit_code = 1;
          }
          renderer = NULL;
        }
      }
    }
  }

  if (backend != NULL && window != NULL && renderer != NULL) {
#if defined(__EMSCRIPTEN__)
    g_app_ctx = app_ctx;
    g_rctx = rctx;
    g_rctx.app_ctx = &g_app_ctx;
    if (backend->set_on_resize_callback) {
      backend->set_on_resize_callback(backend, window, on_resize_callback,
                                      &g_rctx);
    }
    emscripten_set_main_loop(main_loop_step, 0, 1);
#else
#if defined(CI_TEST_RUN)
    ci_test = "1";
#else
#if defined(_MSC_VER)
    if (_dupenv_s(&ci_env_val, &ci_env_len, "CI_TEST_RUN") == 0 &&
        ci_env_val != NULL) {
      ci_test = "1";
      free(ci_env_val);
    }
#else
    ci_test = getenv("CI_TEST_RUN");
#endif
#endif

    while (running) {
      if (ci_test && frame_count++ > 0) {
        break;
      }

      do {
        err = backend->poll_events(backend, window, &event, &has_event);
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
    err = ui_renderer_gles2_destroy(renderer);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }
  if (window && backend) {
    err = backend->destroy_window(backend, window);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }
  if (backend) {
#if defined(__EMSCRIPTEN__)
    err = ui_window_backend_web_destroy(backend);
#elif defined(_WIN32) || defined(__CYGWIN__)
    err = ui_window_backend_win32_destroy(backend);
#elif defined(__APPLE__)
  err = ui_window_backend_macos_destroy(backend);
#elif defined(__linux__) || defined(__unix__)
  err = ui_window_backend_linux_destroy(backend);
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
