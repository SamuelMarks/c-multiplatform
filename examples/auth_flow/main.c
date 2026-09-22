/**
 * @file main.c
 * @brief Core entry point for the auth flow standalone application example.
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
#include "ui_theme_manager.h"
#include "ui_i18n.h"
#include "ui_button_base.h"
#include "ui_input_base.h"
#include "ui_arena.h"
#include "ui_component.h"
#include "ui_router.h"
#include "views.h"
#include "i18n_dict.h"

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

static const char *APP_CSS = "body {"
                             "  width: 100%;"
                             "  height: 100%;"
                             "  display: flex;"
                             "  flex-direction: column;"
                             "}"
                             "#toolbar {"
                             "  display: flex;"
                             "  flex-direction: row;"
                             "  justify-content: space-between;"
                             "  align-items: center;"
                             "  width: 100%;"
                             "  height: 64px;"
                             "  padding: 0 16px;"
                             "  box-sizing: border-box;"
                             "}"
                             ".toolbar-title {"
                             "  font-size: 20px;"
                             "  font-weight: bold;"
                             "}"
                             ".toolbar-actions {"
                             "  display: flex;"
                             "  flex-direction: row;"
                             "}"
                             ".btn-host {"
                             "  width: 120px;"
                             "  height: 40px;"
                             "  margin-left: 8px;"
                             "}"
                             "#router-outlet {"
                             "  flex-grow: 1;"
                             "  display: flex;"
                             "  width: 100%;"
                             "}";

/**
 * @struct app_state
 * @brief Global application state maintaining active UI references and mock
 * data.
 */
struct app_state {
  struct ui_arena *arena;                 /**< Memory arena */
  struct ui_theme_manager *theme_manager; /**< Theme manager */
  struct ui_i18n *i18n;                   /**< I18n localization context */
  struct ui_router *router;               /**< Router handle */

  struct ui_dom_node *root;          /**< Root DOM node */
  struct ui_dom_node *toolbar;       /**< Toolbar node */
  struct ui_dom_node *toolbar_title; /**< Title label */
  struct ui_dom_node *router_outlet; /**< Router outlet container */

  struct ui_button_base *btn_theme; /**< Theme toggle button */
  struct ui_button_base *btn_lang;  /**< Language toggle button */

  struct ui_input_base *input_username; /**< Username input field */
  struct ui_input_base *input_password; /**< Password input field */
  struct ui_button_base *btn_submit;    /**< Submit action button */
  struct ui_button_base *btn_logout;    /**< Logout action button */

  struct ui_dom_node *host_theme; /**< Theme host element */
  struct ui_dom_node *host_lang;  /**< Language host element */

  int current_lang_idx; /**< Current language index */

  int is_authenticated;         /**< Authentication state flag */
  char current_user[64];        /**< Current user identifier */
  char auth_error_message[128]; /**< Auth error string */
};

/**
 * @struct app_context
 * @brief Context structure passed to the rendering and event loops.
 */
struct app_context {
  struct app_state state;               /**< Application state */
  struct ui_css_stylesheet *stylesheet; /**< Parsed stylesheet */
  struct ui_layout_node *layout_tree;   /**< Layout tree */
  float window_width;                   /**< Window width */
  float window_height;                  /**< Window height */
  int needs_layout;                     /**< Layout recalculation flag */
};

/**
 * @struct render_context
 * @brief Context structure for pushing a frame to the rendering backend.
 */
struct render_context {
  struct app_context *app_ctx;              /**< Application context */
  struct ui_renderer_backend *renderer;     /**< Renderer backend */
  struct ui_window_backend *window_backend; /**< Window backend */
  struct ui_window *window;                 /**< Window handle */
  struct ui_engine *engine;                 /**< Core UI engine */
};

static const char *LANGUAGES[] = {"en-US", "es-ES", "fr-FR"};

/**
 * @brief Sets or updates text content of an element via a text child node.
 * @param element The container element.
 * @param text The string content.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t set_element_text(struct ui_dom_node *element,
                                   const char *text) {
  struct ui_dom_node *text_node = NULL;
  ui_error_t err;

  if (!element || !text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (element->first_child &&
      element->first_child->type == UI_DOM_NODE_TYPE_TEXT) {
    return ui_dom_node_set_text_content(element->first_child, text);
  }
  err = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_set_text_content(text_node, text);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(text_node);
    return err;
  }
  err = ui_dom_node_append_child(element, text_node);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(text_node);
    return err;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Refreshes the DOM to reflect the active theme manager state.
 * @param ctx The application context.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t update_theme_ui(struct app_context *ctx) {
  enum ui_theme_mode mode;
  struct ui_dom_node *host_theme;
  const char *label;
  ui_error_t err;

  err = ui_theme_manager_get_mode(ctx->state.theme_manager, &mode);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  if (mode == UI_THEME_MODE_DARK) {
    err = ui_dom_node_set_attribute(ctx->state.root, "data-theme", "dark");
    if (err != UI_ERROR_NONE) {
      return err;
    }
    label = "Light Mode";
  } else {
    err = ui_dom_node_set_attribute(ctx->state.root, "data-theme", "light");
    if (err != UI_ERROR_NONE) {
      return err;
    }
    label = "Dark Mode";
  }

  if (ctx->state.btn_theme) {
    err = ui_button_base_set_text(ctx->state.btn_theme, label);
    if (err != UI_ERROR_NONE) {
      return err;
    }
  }

  host_theme = ctx->state.host_theme;
  if (host_theme) {
    err =
        ui_dom_node_set_attribute(host_theme, "aria-label", "Toggle dark mode");
    if (err != UI_ERROR_NONE) {
      return err;
    }
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Refreshes the DOM to reflect the active I18n engine locale.
 * @param ctx The application context.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t update_lang_ui(struct app_context *ctx) {
  const char *locale = NULL;
  const char *btn_lang_toggle_str = NULL;
  const char *app_title_str = NULL;
  struct ui_dom_node *host_lang;
  ui_error_t err;

  err = ui_i18n_get_locale(ctx->state.i18n, &locale);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  if (!locale) {
    locale = "en-US";
  }

  if (ctx->state.btn_lang) {
    err = ui_button_base_set_text(ctx->state.btn_lang, locale);
    if (err != UI_ERROR_NONE) {
      return err;
    }
  }

  host_lang = ctx->state.host_lang;
  if (host_lang) {
    err =
        get_translated_string(locale, "btn_lang_toggle", &btn_lang_toggle_str);
    if (err != UI_ERROR_NONE) {
      return err;
    }
    err =
        ui_dom_node_set_attribute(host_lang, "aria-label", btn_lang_toggle_str);
    if (err != UI_ERROR_NONE) {
      return err;
    }
  }

  err = get_translated_string(locale, "app_title", &app_title_str);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  if (ctx->state.toolbar_title) {
    err = set_element_text(ctx->state.toolbar_title, app_title_str);
    if (err != UI_ERROR_NONE) {
      return err;
    }
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Callback bound to the theme toggle button.
 * @param button The button instance.
 * @param user_data Application context.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t on_theme_toggle(struct ui_button_base *button,
                                  void *user_data) {
  struct app_context *ctx = (struct app_context *)user_data;
  enum ui_theme_mode mode;
  ui_error_t err;

  if (button == NULL || user_data == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_theme_manager_get_mode(ctx->state.theme_manager, &mode);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  if (mode == UI_THEME_MODE_DARK) {
    err = ui_theme_manager_set_mode(ctx->state.theme_manager,
                                    UI_THEME_MODE_LIGHT);
    if (err != UI_ERROR_NONE) {
      return err;
    }
  } else {
    err =
        ui_theme_manager_set_mode(ctx->state.theme_manager, UI_THEME_MODE_DARK);
    if (err != UI_ERROR_NONE) {
      return err;
    }
  }

  err = update_theme_ui(ctx);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  ctx->needs_layout = 1;
  return UI_ERROR_NONE;
}

/**
 * @brief Callback bound to the language toggle button.
 * @param button The button instance.
 * @param user_data Application context.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t on_lang_toggle(struct ui_button_base *button,
                                 void *user_data) {
  struct app_context *ctx = (struct app_context *)user_data;
  ui_error_t err;

  if (button == NULL || user_data == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ctx->state.current_lang_idx = (ctx->state.current_lang_idx + 1) % 3;
  err = ui_i18n_set_locale(ctx->state.i18n,
                           LANGUAGES[ctx->state.current_lang_idx]);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = update_lang_ui(ctx);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  ctx->needs_layout = 1;
  return UI_ERROR_NONE;
}

/**
 * @brief Recursively iterates through the layout tree to emit renderer draw
 * commands.
 * @param ctx Application context.
 * @param renderer Window renderer backend.
 * @param node Current layout node in the tree traversal.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t draw_layout_node(struct app_context *ctx,
                                   struct ui_renderer_backend *renderer,
                                   struct ui_layout_node *node) {
  struct ui_dom_rect rect;
  struct ui_color color;
  struct ui_layout_node *child;
  const char *id = NULL;
  const char *cls = NULL;
  enum ui_theme_mode mode;
  ui_error_t err;

  color.r = 0.8f;
  color.g = 0.8f;
  color.b = 0.8f;
  color.a = 1.0f;

  err = ui_theme_manager_get_mode(ctx->state.theme_manager, &mode);
  if (err != UI_ERROR_NONE) {
    return err;
  }
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
      if (mode == UI_THEME_MODE_DARK) {
        color.r = 0.15f;
        color.g = 0.15f;
        color.b = 0.15f;
      } else {
        color.r = 0.95f;
        color.g = 0.95f;
        color.b = 0.95f;
      }
    } else if (cls && strcmp(cls, "btn-host") == 0) {
      if (mode == UI_THEME_MODE_DARK) {
        color.r = 0.3f;
        color.g = 0.3f;
        color.b = 0.3f;
      } else {
        color.r = 0.85f;
        color.g = 0.85f;
        color.b = 0.85f;
      }
    } else if (node->dom_node == ctx->state.root) {
      if (mode == UI_THEME_MODE_DARK) {
        color.r = 0.1f;
        color.g = 0.1f;
        color.b = 0.1f;
      } else {
        color.r = 1.0f;
        color.g = 1.0f;
        color.b = 1.0f;
      }
    } else {
      color.a = 0.0f;
    }
  } else {
    color.a = 0.0f;
  }

  if (color.a > 0.0f && rect.width > 0 && rect.height > 0) {
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
 * @brief Primary rendering pipeline function called per frame.
 * @param rctx The renderer context enclosing state and handles.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t do_render(struct render_context *rctx) {
  struct app_context *app_ctx;
  struct ui_renderer_backend *renderer;
  struct ui_color bg;
  enum ui_theme_mode mode;
  ui_error_t err;

  app_ctx = rctx->app_ctx;
  renderer = rctx->renderer;
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

  err = ui_theme_manager_get_mode(app_ctx->state.theme_manager, &mode);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  if (mode == UI_THEME_MODE_DARK) {
    bg.r = 0.1f;
    bg.g = 0.1f;
    bg.b = 0.1f;
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
 * @brief OS Window callback notifying the engine of a viewport dimension
 * change.
 * @param user_data Renderer context.
 * @param width New pixel width.
 * @param height New pixel height.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t on_resize_callback(void *user_data, int width, int height) {
  struct render_context *rctx = (struct render_context *)user_data;
  rctx->app_ctx->window_width = (float)width;
  rctx->app_ctx->window_height = (float)height;
  rctx->app_ctx->needs_layout = 1;
  return UI_ERROR_NONE;
}

/**
 * @brief Iterates the application tree passing events to active UI components.
 * @param ctx The application context.
 * @param event The generic input event from the OS window.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t route_event(struct app_context *ctx, struct ui_event *event) {
  ui_error_t err;

  if (ctx->state.input_username) {
    err = ui_input_base_process_event(ctx->state.input_username, event, 0.0);
    if (err != UI_ERROR_NONE) {
      return err;
    }
  }
  if (ctx->state.input_password) {
    err = ui_input_base_process_event(ctx->state.input_password, event, 0.0);
    if (err != UI_ERROR_NONE) {
      return err;
    }
  }
  if (ctx->state.btn_submit) {
    err = ui_button_base_process_event(ctx->state.btn_submit, event, 0.0);
    if (err != UI_ERROR_NONE) {
      return err;
    }
  }
  if (ctx->state.btn_logout) {
    err = ui_button_base_process_event(ctx->state.btn_logout, event, 0.0);
    if (err != UI_ERROR_NONE) {
      return err;
    }
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
      err = route_event(&g_app_ctx, &event);
      if (err != UI_ERROR_NONE) {
        emscripten_cancel_main_loop();
        return;
      }
      err = ui_router_process_event(g_app_ctx.state.router, &event);
      if (err != UI_ERROR_NONE) {
        emscripten_cancel_main_loop();
        return;
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
 * @brief Testable entry point for auth flow example.
 * @return 0 on success, non-zero on failure.
 */
int example_auth_flow_main(void) {
#endif
  struct ui_engine_config config;
  struct ui_engine *engine = NULL;
  struct ui_window_backend *backend = NULL;
  struct ui_renderer_backend *renderer = NULL;
  struct ui_window *window = NULL;
  struct app_context app_ctx;
  struct render_context rctx;
  struct ui_dom_node *actions_container = NULL;
  struct ui_component *tmp_comp = NULL;
  struct ui_event event;
  struct ui_event simulate_click;
  const char *ci_test = NULL;
  int running = 1;
  int frame = 0;
  int frame_count = 0;
  int has_event = 0;
  int exit_code = 0;
  ui_error_t err;

  printf("Starting Auth Flow Example...\n");

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

  /* Initialize Core Services */
  err = ui_arena_create(1024 * 1024, &app_ctx.state.arena);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_theme_manager_create(app_ctx.state.arena,
                                &app_ctx.state.theme_manager);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_i18n_create(&app_ctx.state.i18n);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_router_create(&app_ctx.state.router);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_router_install_os_hooks(app_ctx.state.router);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_router_add_route(app_ctx.state.router, "/secrets",
                            secrets_view_factory, &app_ctx.state);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_router_add_route(app_ctx.state.router, "/login",
                            auth_container_factory, &app_ctx.state);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_router_add_route(app_ctx.state.router, "/signup",
                            auth_container_factory, &app_ctx.state);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_router_add_route(app_ctx.state.router, "/", auth_container_factory,
                            &app_ctx.state);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  app_ctx.state.current_lang_idx = 0;
  err = ui_i18n_set_locale(app_ctx.state.i18n,
                           LANGUAGES[app_ctx.state.current_lang_idx]);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  /* Build DOM */
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
  err = ui_dom_node_set_attribute(app_ctx.state.toolbar, "role", "banner");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_append_child(app_ctx.state.root, app_ctx.state.toolbar);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT,
                           &app_ctx.state.toolbar_title);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_tag_name(app_ctx.state.toolbar_title, "div");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_attribute(app_ctx.state.toolbar_title, "class",
                                  "toolbar-title");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_append_child(app_ctx.state.toolbar,
                                 app_ctx.state.toolbar_title);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &actions_container);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_tag_name(actions_container, "div");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err =
      ui_dom_node_set_attribute(actions_container, "class", "toolbar-actions");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_append_child(app_ctx.state.toolbar, actions_container);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  err = ui_button_base_create(&app_ctx.state.btn_theme);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_button_base_set_on_click(app_ctx.state.btn_theme, on_theme_toggle,
                                    &app_ctx);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &app_ctx.state.host_theme);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err =
      ui_dom_node_set_attribute(app_ctx.state.host_theme, "class", "btn-host");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_attribute(app_ctx.state.host_theme, "tabindex", "0");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_append_child(actions_container, app_ctx.state.host_theme);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_button_base_get_component(app_ctx.state.btn_theme, &tmp_comp);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_component_mount(tmp_comp, app_ctx.state.host_theme);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  err = ui_button_base_create(&app_ctx.state.btn_lang);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_button_base_set_on_click(app_ctx.state.btn_lang, on_lang_toggle,
                                    &app_ctx);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &app_ctx.state.host_lang);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_attribute(app_ctx.state.host_lang, "class", "btn-host");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_attribute(app_ctx.state.host_lang, "tabindex", "0");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_append_child(actions_container, app_ctx.state.host_lang);
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
  err = ui_component_mount(tmp_comp, app_ctx.state.host_lang);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT,
                           &app_ctx.state.router_outlet);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_tag_name(app_ctx.state.router_outlet, "div");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_attribute(app_ctx.state.router_outlet, "id",
                                  "router-outlet");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err =
      ui_dom_node_append_child(app_ctx.state.root, app_ctx.state.router_outlet);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  err = ui_css_parse_stylesheet(APP_CSS, &app_ctx.stylesheet);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  err = update_theme_ui(&app_ctx);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = update_lang_ui(&app_ctx);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  err = ui_router_navigate(app_ctx.state.router, "/login");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  tmp_comp = NULL;
  err = ui_router_get_current(app_ctx.state.router, &tmp_comp);
  if (err == UI_ERROR_NONE && tmp_comp) {
    err = ui_component_mount(tmp_comp, app_ctx.state.router_outlet);
    if (err != UI_ERROR_NONE) {
      exit_code = 1;
      goto cleanup;
    }
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

  if (backend != NULL && err == UI_ERROR_NONE) {
    err =
        backend->create_window(backend, "Auth Flow", (int)app_ctx.window_width,
                               (int)app_ctx.window_height, &window);
    if (err == UI_ERROR_NONE && window != NULL) {
      err = ui_renderer_gles2_create(&renderer);
      if (err == UI_ERROR_NONE && renderer != NULL) {
        err = renderer->init(renderer, backend, window);
        if (err != UI_ERROR_NONE) {
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
    ci_test = getenv("CI_TEST_RUN");
#endif

    while (running) {
      if (ci_test && frame_count++ > 2) {
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
          err = route_event(&app_ctx, &event);
          if (err != UI_ERROR_NONE) {
            exit_code = 1;
            goto cleanup;
          }
          err = ui_router_process_event(app_ctx.state.router, &event);
          if (err != UI_ERROR_NONE) {
            exit_code = 1;
            goto cleanup;
          }
        }
      } while (has_event && running);

      if (!running) {
        break;
      }

      frame++;
      if (frame == 120) {
        simulate_click.type = UI_EVENT_MOUSE_DOWN;
        err = ui_button_base_process_event(app_ctx.state.btn_theme,
                                           &simulate_click, 0.0);
        if (err != UI_ERROR_NONE) {
          exit_code = 1;
          goto cleanup;
        }
        simulate_click.type = UI_EVENT_MOUSE_UP;
        err = ui_button_base_process_event(app_ctx.state.btn_theme,
                                           &simulate_click, 10.0);
        if (err != UI_ERROR_NONE) {
          exit_code = 1;
          goto cleanup;
        }
      } else if (frame == 240) {
        simulate_click.type = UI_EVENT_MOUSE_DOWN;
        err = ui_button_base_process_event(app_ctx.state.btn_lang,
                                           &simulate_click, 0.0);
        if (err != UI_ERROR_NONE) {
          exit_code = 1;
          goto cleanup;
        }
        simulate_click.type = UI_EVENT_MOUSE_UP;
        err = ui_button_base_process_event(app_ctx.state.btn_lang,
                                           &simulate_click, 10.0);
        if (err != UI_ERROR_NONE) {
          exit_code = 1;
          goto cleanup;
        }
      } else if (frame == 360) {
        frame = 0;
      }

      err = do_render(&rctx);
      if (err != UI_ERROR_NONE) {
        exit_code = 1;
        goto cleanup;
      }

#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(_MSC_VER)
      if (!ci_test) {
        Sleep(16);
      }
#endif
#endif
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
  if (app_ctx.state.root) {
    err = ui_dom_node_destroy(app_ctx.state.root);
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
  if (app_ctx.state.theme_manager) {
    err = ui_theme_manager_destroy(app_ctx.state.theme_manager);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }
  if (app_ctx.state.i18n) {
    err = ui_i18n_destroy(app_ctx.state.i18n);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }
  if (app_ctx.state.router) {
    err = ui_router_destroy(app_ctx.state.router);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }
  if (app_ctx.state.arena) {
    err = ui_arena_destroy(app_ctx.state.arena);
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
