/**
 * @file main.c
 * @brief Unstyled components example application.
 */

/* clang-format off */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#if defined(_WIN32) || defined(__CYGWIN__)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#endif
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
#if defined(_MSC_VER)
__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);
#endif
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
 * @brief CSS stylesheet for unstyled components example.
 */
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

/**
 * @struct app_state
 * @brief State elements of the unstyled component application.
 */
struct app_state {
  struct ui_dom_node *root;         /**< Root DOM node */
  struct ui_dom_node *toolbar;      /**< Toolbar container node */
  struct ui_dom_node *status_text;  /**< Status text node */
  struct ui_button_base *btn_theme; /**< Theme switch button */
  struct ui_button_base *btn_lang;  /**< Language switch button */
  int is_dark;                      /**< Theme dark mode flag */
  int lang_idx;                     /**< Selected language index */
};

/**
 * @struct app_context
 * @brief Application aggregate context.
 */
struct app_context {
  struct app_state state;               /**< Application state */
  struct ui_css_stylesheet *stylesheet; /**< Parsed stylesheet */
  struct ui_layout_node *layout_tree;   /**< Computed layout tree */
  float window_width;                   /**< Window width */
  float window_height;                  /**< Window height */
  int needs_layout;                     /**< Needs layout recalculation flag */
};

static const char *languages[] = {"English", "Hebrew", "Arabic", "Japanese"};
static const char *themes[] = {"Light", "Dark"};

/**
 * @brief Updates the DOM nodes and attributes to reflect state changes.
 * @param ctx Pointer to application context.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t update_ui(struct app_context *ctx) {
  char buffer[256];
  const char *lang = languages[ctx->state.lang_idx];
  const char *theme = themes[ctx->state.is_dark];
  ui_error_t err;

#if defined(_MSC_VER)
  sprintf_s(buffer, sizeof(buffer), "Language: %s | Theme: %s", lang, theme);
#else
  sprintf(buffer, "Language: %s | Theme: %s", lang, theme);
#endif

  err = ui_dom_node_set_text_content(ctx->state.status_text, buffer);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  if (ctx->state.lang_idx == 1 || ctx->state.lang_idx == 2) {
    err = ui_dom_node_set_attribute(ctx->state.toolbar, "dir", "rtl");
    if (err != UI_ERROR_NONE) {
      return err;
    }
    err = ui_dom_node_set_attribute(ctx->state.toolbar, "style",
                                    "flex-direction: row-reverse;");
    if (err != UI_ERROR_NONE) {
      return err;
    }
  } else {
    err = ui_dom_node_set_attribute(ctx->state.toolbar, "dir", "ltr");
    if (err != UI_ERROR_NONE) {
      return err;
    }
    err = ui_dom_node_set_attribute(ctx->state.toolbar, "style",
                                    "flex-direction: row;");
    if (err != UI_ERROR_NONE) {
      return err;
    }
  }

  if (ctx->state.is_dark) {
    err = ui_dom_node_set_attribute(ctx->state.toolbar, "data-theme", "dark");
    if (err != UI_ERROR_NONE) {
      return err;
    }
  } else {
    err = ui_dom_node_set_attribute(ctx->state.toolbar, "data-theme", "light");
    if (err != UI_ERROR_NONE) {
      return err;
    }
  }

  ctx->needs_layout = 1;
  return UI_ERROR_NONE;
}

/**
 * @brief Button click handler for toggling the color theme.
 * @param button Pointer to the clicked button.
 * @param user_data Pointer to application context.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t on_theme_click(struct ui_button_base *button,
                                 void *user_data) {
  struct app_context *ctx = (struct app_context *)user_data;
  ui_error_t err;
  if (!button || !user_data) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ctx->state.is_dark = !ctx->state.is_dark;
  printf("[Event] Theme toggled to %s\n", themes[ctx->state.is_dark]);
  err = update_ui(ctx);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Button click handler for toggling the active language.
 * @param button Pointer to the clicked button.
 * @param user_data Pointer to application context.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t on_lang_click(struct ui_button_base *button,
                                void *user_data) {
  struct app_context *ctx = (struct app_context *)user_data;
  ui_error_t err;
  if (!button || !user_data) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ctx->state.lang_idx = (ctx->state.lang_idx + 1) % 4;
  printf("[Event] Language toggled to %s\n", languages[ctx->state.lang_idx]);
  err = update_ui(ctx);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Recursively renders a layout node and its children.
 * @param ctx Pointer to application context.
 * @param renderer Pointer to renderer backend.
 * @param node Pointer to layout node.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t draw_layout_node(struct app_context *ctx,
                                   struct ui_renderer_backend *renderer,
                                   struct ui_layout_node *node) {
  struct ui_dom_rect rect;
  struct ui_color color;
  struct ui_layout_node *child;
  const char *id = NULL;
  const char *cls = NULL;
  ui_error_t err;

  color.r = 0.8f;
  color.g = 0.8f;
  color.b = 0.8f;
  color.a = 1.0f;

  err = ui_cssom_view_get_bounding_client_rect(node, &rect);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  if (node->dom_node && node->dom_node->type == UI_DOM_NODE_TYPE_ELEMENT) {
    err = ui_dom_node_get_attribute(node->dom_node, "id", &id);
    if (err != UI_ERROR_NONE && err != UI_ERROR_NOT_FOUND) {
      return err;
    }
    err = ui_dom_node_get_attribute(node->dom_node, "class", &cls);
    if (err != UI_ERROR_NONE && err != UI_ERROR_NOT_FOUND) {
      return err;
    }

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
    err = renderer->draw_rect(renderer, (float)rect.x, (float)rect.y,
                              (float)rect.width, (float)rect.height, color);
    if (err != UI_ERROR_NONE) {
      return err;
    }
  }

  child = node->first_child;
  while (child) {
    err = draw_layout_node(ctx, renderer, child);
    if (err != UI_ERROR_NONE) {
      return err;
    }
    child = child->next_sibling;
  }
  return UI_ERROR_NONE;
}

/**
 * @struct render_context
 * @brief Aggregated context for rendering pipeline.
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

  bg.r = 0.1f;
  bg.g = 0.1f;
  bg.b = 0.1f;
  bg.a = 1.0f;

  if (app_ctx->needs_layout) {
    if (app_ctx->layout_tree) {
      err = ui_layout_tree_destroy(app_ctx->layout_tree);
      if (err != UI_ERROR_NONE) {
        return err;
      }
      app_ctx->layout_tree = NULL;
    }
    err = ui_layout_tree_generate(app_ctx->state.root, app_ctx->stylesheet,
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
    err = draw_layout_node(app_ctx, renderer, app_ctx->layout_tree);
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
 * @param user_data Pointer to render context.
 * @param width New window width.
 * @param height New window height.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t on_resize_callback(void *user_data, int width, int height) {
  struct render_context *rctx = (struct render_context *)user_data;
  rctx->app_ctx->window_width = (float)width;
  rctx->app_ctx->window_height = (float)height;
  rctx->app_ctx->needs_layout = 1;
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
 * @brief Application entry point.
 * @return 0 on success, non-zero on failure.
 */
int main(void) {
#else
/**
 * @brief Testable entry point for unstyled components example.
 * @return 0 on success, non-zero on failure.
 */
int example_unstyled_main(void) {
#endif
  struct ui_engine_config config;
  struct ui_engine *engine = NULL;
  struct ui_window_backend *backend = NULL;
  struct ui_renderer_backend *renderer = NULL;
  struct ui_window *window = NULL;
  struct app_context app_ctx;
  struct render_context rctx;
  struct ui_dom_node *host_theme = NULL;
  struct ui_dom_node *host_lang = NULL;
  struct ui_component *tmp_comp = NULL;
  struct ui_event event;
  struct ui_event simulate_click;
#if !defined(__EMSCRIPTEN__)
  const char *ci_test = NULL;
  int running = 1;
  int frame = 0;
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

  printf("Initializing Unstyled Components Windowed Example...\n");

  memset(&app_ctx, 0, sizeof(app_ctx));
  memset(&rctx, 0, sizeof(rctx));
  memset(&event, 0, sizeof(event));
  memset(&simulate_click, 0, sizeof(simulate_click));

  app_ctx.window_width = 800.0f;
  app_ctx.window_height = 600.0f;
  app_ctx.needs_layout = 1;

  config.num_threads = 2;
  err = ui_engine_create(&config, &engine);
  if (err != UI_ERROR_NONE) {
    return 1;
  }

  /* Setup DOM tree */
  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &app_ctx.state.root);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_tag_name(app_ctx.state.root, "body");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_attribute(app_ctx.state.root, "id", "app");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &app_ctx.state.toolbar);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_tag_name(app_ctx.state.toolbar, "div");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_attribute(app_ctx.state.toolbar, "id", "toolbar");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_append_child(app_ctx.state.root, app_ctx.state.toolbar);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &app_ctx.state.status_text);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_append_child(app_ctx.state.toolbar,
                                 app_ctx.state.status_text);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  err = ui_button_base_create(&app_ctx.state.btn_theme);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_button_base_set_on_click(app_ctx.state.btn_theme, on_theme_click,
                                    &app_ctx);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &host_theme);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_attribute(host_theme, "class", "btn-host");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_attribute(host_theme, "id", "host-theme");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_append_child(app_ctx.state.toolbar, host_theme);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  err = ui_button_base_get_component(app_ctx.state.btn_theme, &tmp_comp);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_component_mount(tmp_comp, host_theme);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  err = ui_button_base_create(&app_ctx.state.btn_lang);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_button_base_set_on_click(app_ctx.state.btn_lang, on_lang_click,
                                    &app_ctx);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &host_lang);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_attribute(host_lang, "class", "btn-host");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_attribute(host_lang, "id", "host-lang");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_append_child(app_ctx.state.toolbar, host_lang);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  tmp_comp = NULL;
  err = ui_button_base_get_component(app_ctx.state.btn_lang, &tmp_comp);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_component_mount(tmp_comp, host_lang);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  err = ui_css_parse_stylesheet(TOOLBAR_CSS, &app_ctx.stylesheet);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  app_ctx.state.is_dark = 0;
  app_ctx.state.lang_idx = 0;
  err = update_ui(&app_ctx);
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
    err = backend->create_window(backend, "Unstyled Components (Simulated)",
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

      frame++;
      if (frame == 60) {
        simulate_click.type = UI_EVENT_MOUSE_DOWN;
        err = ui_button_base_process_event(app_ctx.state.btn_theme,
                                           &simulate_click, 0.0);
        if (err != UI_ERROR_NONE) {
          exit_code = 1;
          break;
        }
        simulate_click.type = UI_EVENT_MOUSE_UP;
        err = ui_button_base_process_event(app_ctx.state.btn_theme,
                                           &simulate_click, 10.0);
        if (err != UI_ERROR_NONE) {
          exit_code = 1;
          break;
        }
      } else if (frame == 120) {
        simulate_click.type = UI_EVENT_MOUSE_DOWN;
        err = ui_button_base_process_event(app_ctx.state.btn_lang,
                                           &simulate_click, 0.0);
        if (err != UI_ERROR_NONE) {
          exit_code = 1;
          break;
        }
        simulate_click.type = UI_EVENT_MOUSE_UP;
        err = ui_button_base_process_event(app_ctx.state.btn_lang,
                                           &simulate_click, 10.0);
        if (err != UI_ERROR_NONE) {
          exit_code = 1;
          break;
        }
        frame = 0;
      }

      err = do_render(&rctx);
      if (err != UI_ERROR_NONE) {
        exit_code = 1;
        break;
      }
    }
#endif
  } else {
    err = ui_layout_tree_generate(app_ctx.state.root, app_ctx.stylesheet,
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
    simulate_click.type = UI_EVENT_MOUSE_DOWN;
    err = ui_button_base_process_event(app_ctx.state.btn_theme, &simulate_click,
                                       0.0);
    if (err != UI_ERROR_NONE) {
      exit_code = 1;
      goto cleanup;
    }
    simulate_click.type = UI_EVENT_MOUSE_UP;
    err = ui_button_base_process_event(app_ctx.state.btn_theme, &simulate_click,
                                       10.0);
    if (err != UI_ERROR_NONE) {
      exit_code = 1;
      goto cleanup;
    }
    simulate_click.type = UI_EVENT_MOUSE_DOWN;
    err = ui_button_base_process_event(app_ctx.state.btn_lang, &simulate_click,
                                       0.0);
    if (err != UI_ERROR_NONE) {
      exit_code = 1;
      goto cleanup;
    }
    simulate_click.type = UI_EVENT_MOUSE_UP;
    err = ui_button_base_process_event(app_ctx.state.btn_lang, &simulate_click,
                                       10.0);
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
  if (app_ctx.state.btn_theme) {
    err = ui_button_base_destroy(app_ctx.state.btn_theme);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }
  if (app_ctx.state.btn_lang) {
    err = ui_button_base_destroy(app_ctx.state.btn_lang);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }
  if (app_ctx.state.root) {
    err = ui_dom_node_destroy(app_ctx.state.root);
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
