/**
 * @file main.c
 * @brief Entry point for OAuth2 sync flow example application.
 */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app_state.h"
#include "views.h"
#include "ui_engine.h"
#include "ui_router.h"
#include "ui_arena.h"
#include "ui_dom_node.h"
#include "ui_component.h"
#include "ui_css_parser.h"
#include "ui_cssom.h"
#include "ui_cssom_view.h"
#include "ui_layout.h"
#include "ui_renderer.h"
#include "ui_renderer_gles2.h"
#include "ui_window_backend.h"

#if defined(__EMSCRIPTEN__)
#include "ui_window_backend_web.h"
#elif defined(_WIN32) || defined(__CYGWIN__)
#include "ui_window_backend_win32.h"
__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);
#elif defined(__APPLE__)
#include "ui_window_backend_macos.h"
#elif defined(__linux__) || defined(__unix__)
#include "ui_window_backend_linux.h"
#endif

#if !defined(_MSC_VER)
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif
/* clang-format on */

static const char *APP_CSS_PARTS[] = {
    "body { width: 900px; height: 650px; display: flex; flex-direction: "
    "column; margin: 0; padding: 0; } ",
    "#router-outlet { display: flex; flex-direction: column; width: 100%; "
    "flex-grow: 1; } ",
    ".login-card { display: flex; flex-direction: column; width: 868px; "
    "height: 618px; padding: 24px; box-sizing: border-box; } ",
    ".profile-container { display: flex; flex-direction: column; width: 868px; "
    "height: 618px; padding: 24px; box-sizing: border-box; } ",
    ".admin-directory-container { display: flex; flex-direction: column; "
    "width: 868px; height: 618px; padding: 24px; box-sizing: border-box; } ",
    ".user-detail-container { display: flex; flex-direction: column; width: "
    "868px; height: 618px; padding: 24px; box-sizing: border-box; } ",
    ".login-title { width: 820px; height: 44px; margin-bottom: 16px; } ",
    ".profile-user { width: 820px; height: 44px; margin-bottom: 16px; } ",
    ".admin-dir-title { width: 600px; height: 44px; } ",
    ".user-detail-title { width: 820px; height: 44px; margin-bottom: 16px; } ",
    ".mode-control-container { display: flex; flex-direction: row; width: "
    "820px; height: 44px; margin-bottom: 12px; } ",
    ".backend-url-group { display: flex; flex-direction: row; width: 820px; "
    "height: 44px; margin-bottom: 12px; } ",
    ".credentials-group { display: flex; flex-direction: row; width: 820px; "
    "height: 44px; margin-bottom: 12px; } ",
    ".quick-accounts { display: flex; flex-direction: row; width: 820px; "
    "height: 44px; margin-bottom: 12px; } ",
    ".admin-toolbar { display: flex; flex-direction: row; width: 820px; "
    "height: 44px; margin-bottom: 12px; } ",
    ".mode-toggle-btn { width: 180px; height: 38px; margin-right: 12px; } ",
    ".quick-chip-admin { width: 160px; height: 38px; margin-right: 12px; } ",
    ".quick-chip-alice { width: 160px; height: 38px; margin-right: 12px; } ",
    ".login-submit-btn { width: 180px; height: 40px; margin-top: 12px; } ",
    ".profile-save-btn { width: 180px; height: 38px; margin-right: 12px; } ",
    ".admin-directory-btn { width: 180px; height: 38px; margin-right: 12px; } ",
    ".sync-changes-btn { width: 180px; height: 38px; } ",
    ".back-to-profile-btn { width: 180px; height: 38px; margin-top: 12px; } ",
    ".back-to-directory-btn { width: 180px; height: 38px; margin-top: 12px; } ",
    ".backend-url-input { width: 320px; height: 36px; margin-right: 12px; } ",
    ".login-input { width: 240px; height: 36px; margin-right: 12px; } ",
    ".profile-bio-input { width: 600px; height: 80px; margin-bottom: 12px; } ",
    ".mode-indicator { width: 140px; height: 32px; margin-right: 12px; } ",
    ".profile-role { width: 120px; height: 32px; margin-right: 12px; } ",
    ".profile-sync-status { width: 120px; height: 32px; margin-right: 12px; } ",
    ".ui-dl-container { display: flex; flex-direction: column; width: 820px; "
    "height: 360px; } ",
    ".ui-dl-row { display: flex; flex-direction: row; width: 820px; height: "
    "36px; margin-bottom: 6px; } ",
    ".ui-dl-term { width: 240px; height: 32px; margin-right: 12px; } ",
    ".ui-dl-desc { width: 550px; height: 32px; } ",
    NULL};

/**
 * @brief Constructs CSS string from part array.
 * @param buffer Output character buffer.
 * @param buffer_size Size of output buffer.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t build_app_css(char *buffer, size_t buffer_size) {
  size_t i;
  buffer[0] = '\0';
  for (i = 0; APP_CSS_PARTS[i] != NULL; ++i) {
#if defined(_MSC_VER)
    strcat_s(buffer, buffer_size, APP_CSS_PARTS[i]);
#else
    strncat(buffer, APP_CSS_PARTS[i], buffer_size - strlen(buffer) - 1);
#endif
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Registers application navigation endpoints on the router.
 * @param router Pointer to router instance.
 * @param state Pointer to application state coordinator.
 * @return UI_ERROR_NONE on success, or an error code.
 */
static ui_error_t setup_routes(struct ui_router *router,
                               struct app_state *state) {
  ui_error_t err;

  err = ui_router_add_route(router, "/login", oauth2_view_login_factory, state);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_router_add_route(router, "/profile", oauth2_view_profile_factory,
                            state);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_router_add_route(router, "/admin",
                            oauth2_view_admin_directory_factory, state);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_router_add_route(router, "/users/:id",
                            oauth2_view_user_detail_factory, state);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Detaches all child DOM nodes from a parent container without freeing
 * them.
 * @param parent Container node to clear.
 * @return UI_ERROR_NONE on success, or an error code.
 */
static ui_error_t clear_dom_children(struct ui_dom_node *parent) {
  ui_error_t err;
  if (!parent) {
    return UI_ERROR_NONE;
  }
  while (parent->first_child != NULL) {
    err = ui_dom_node_remove_child(parent, parent->first_child);
    if (err != UI_ERROR_NONE) {
      return err;
    }
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Mounts the active view from the router into the outlet container.
 * @param router Active router instance.
 * @param outlet Target outlet DOM node.
 * @return UI_ERROR_NONE on success, or an error code.
 */
static ui_error_t mount_active_route_view(struct ui_router *router,
                                          struct ui_dom_node *outlet) {
  struct ui_component *curr_comp = NULL;
  ui_error_t err;

  err = clear_dom_children(outlet);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_router_get_current(router, &curr_comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  if (curr_comp != NULL) {
    err = ui_component_mount(curr_comp, outlet);
    if (err != UI_ERROR_NONE) {
      return err;
    }
    if (curr_comp->shadow_root != NULL &&
        curr_comp->shadow_root->parent != outlet) {
      err = ui_dom_node_append_child(outlet, curr_comp->shadow_root);
      if (err != UI_ERROR_NONE) {
        return err;
      }
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Traverses layout tree to render colored boxes and borders for UI
 * components.
 * @param renderer Rendering backend.
 * @param node Layout node to draw.
 * @return UI_ERROR_NONE on success, or an error code.
 */
static ui_error_t draw_layout_node(struct ui_renderer_backend *renderer,
                                   struct ui_layout_node *node) {
  struct ui_dom_rect rect;
  struct ui_color fill_color;
  struct ui_color border_color;
  struct ui_layout_node *child;
  const char *id = NULL;
  const char *cls = NULL;
  const char *tag = NULL;
  int has_border = 0;
  ui_error_t err;

  fill_color.r = 0.0f;
  fill_color.g = 0.0f;
  fill_color.b = 0.0f;
  fill_color.a = 0.0f;

  border_color.r = 0.8f;
  border_color.g = 0.8f;
  border_color.b = 0.8f;
  border_color.a = 1.0f;

  err = ui_cssom_view_get_bounding_client_rect(node, &rect);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  if (node->dom_node) {
    tag = node->dom_node->tag_name;
    err = ui_dom_node_get_attribute(node->dom_node, "id", &id);
    if (err != UI_ERROR_NONE) {
      id = NULL;
    }
    err = ui_dom_node_get_attribute(node->dom_node, "class", &cls);
    if (err != UI_ERROR_NONE) {
      cls = NULL;
    }

    if (tag && strcmp(tag, "body") == 0) {
      fill_color.r = 0.94f;
      fill_color.g = 0.95f;
      fill_color.b = 0.97f;
      fill_color.a = 1.0f;
    } else if (cls && (strcmp(cls, "login-card") == 0 ||
                       strcmp(cls, "profile-container") == 0 ||
                       strcmp(cls, "admin-directory-container") == 0 ||
                       strcmp(cls, "user-detail-container") == 0)) {
      fill_color.r = 1.0f;
      fill_color.g = 1.0f;
      fill_color.b = 1.0f;
      fill_color.a = 1.0f;
      border_color.r = 0.82f;
      border_color.g = 0.85f;
      border_color.b = 0.90f;
      has_border = 1;
    } else if (cls && (strcmp(cls, "login-title") == 0 ||
                       strcmp(cls, "profile-user") == 0 ||
                       strcmp(cls, "admin-dir-title") == 0 ||
                       strcmp(cls, "user-detail-title") == 0)) {
      fill_color.r = 0.12f;
      fill_color.g = 0.16f;
      fill_color.b = 0.23f;
      fill_color.a = 1.0f;
    } else if (cls && (strcmp(cls, "login-submit-btn") == 0 ||
                       strcmp(cls, "profile-save-btn") == 0 ||
                       strcmp(cls, "sync-changes-btn") == 0)) {
      fill_color.r = 0.15f;
      fill_color.g = 0.45f;
      fill_color.b = 0.92f;
      fill_color.a = 1.0f;
      has_border = 1;
      border_color.r = 0.10f;
      border_color.g = 0.35f;
      border_color.b = 0.80f;
    } else if (cls && (strcmp(cls, "quick-chip-admin") == 0 ||
                       strcmp(cls, "quick-chip-alice") == 0)) {
      fill_color.r = 0.88f;
      fill_color.g = 0.91f;
      fill_color.b = 1.0f;
      fill_color.a = 1.0f;
      has_border = 1;
      border_color.r = 0.65f;
      border_color.g = 0.75f;
      border_color.b = 0.95f;
    } else if (cls && strcmp(cls, "mode-toggle-btn") == 0) {
      fill_color.r = 0.10f;
      fill_color.g = 0.65f;
      fill_color.b = 0.40f;
      fill_color.a = 1.0f;
      has_border = 1;
      border_color.r = 0.08f;
      border_color.g = 0.50f;
      border_color.b = 0.30f;
    } else if (cls && (strcmp(cls, "mode-indicator") == 0 ||
                       strcmp(cls, "profile-role") == 0 ||
                       strcmp(cls, "profile-sync-status") == 0 ||
                       strcmp(cls, "dl-status-badge") == 0)) {
      fill_color.r = 0.90f;
      fill_color.g = 0.93f;
      fill_color.b = 0.96f;
      fill_color.a = 1.0f;
      has_border = 1;
      border_color.r = 0.75f;
      border_color.g = 0.80f;
      border_color.b = 0.85f;
    } else if (cls && (strcmp(cls, "backend-url-input") == 0 ||
                       strcmp(cls, "login-input") == 0 ||
                       strcmp(cls, "profile-bio-input") == 0)) {
      fill_color.r = 0.98f;
      fill_color.g = 0.98f;
      fill_color.b = 1.0f;
      fill_color.a = 1.0f;
      has_border = 1;
      border_color.r = 0.78f;
      border_color.g = 0.82f;
      border_color.b = 0.88f;
    } else if (cls && (strcmp(cls, "admin-directory-btn") == 0 ||
                       strcmp(cls, "back-to-profile-btn") == 0 ||
                       strcmp(cls, "back-to-directory-btn") == 0)) {
      fill_color.r = 0.35f;
      fill_color.g = 0.40f;
      fill_color.b = 0.50f;
      fill_color.a = 1.0f;
      has_border = 1;
      border_color.r = 0.25f;
      border_color.g = 0.30f;
      border_color.b = 0.40f;
    } else if (cls && strcmp(cls, "ui-dl-row") == 0) {
      fill_color.r = 0.96f;
      fill_color.g = 0.97f;
      fill_color.b = 0.99f;
      fill_color.a = 1.0f;
      has_border = 1;
      border_color.r = 0.88f;
      border_color.g = 0.90f;
      border_color.b = 0.94f;
    } else if (cls && strcmp(cls, "ui-dl-term") == 0) {
      fill_color.r = 0.90f;
      fill_color.g = 0.93f;
      fill_color.b = 0.98f;
      fill_color.a = 1.0f;
      has_border = 1;
      border_color.r = 0.82f;
      border_color.g = 0.85f;
      border_color.b = 0.90f;
    } else if (cls && strcmp(cls, "ui-dl-desc") == 0) {
      fill_color.r = 0.98f;
      fill_color.g = 0.98f;
      fill_color.b = 0.99f;
      fill_color.a = 1.0f;
      has_border = 1;
      border_color.r = 0.88f;
      border_color.g = 0.90f;
      border_color.b = 0.94f;
    } else if (tag && strcmp(tag, "button") == 0) {
      fill_color.r = 0.25f;
      fill_color.g = 0.45f;
      fill_color.b = 0.85f;
      fill_color.a = 1.0f;
      has_border = 1;
    }
  }

  if (fill_color.a > 0.0f && rect.width > 0 && rect.height > 0) {
    err =
        renderer->draw_rect(renderer, (float)rect.x, (float)rect.y,
                            (float)rect.width, (float)rect.height, fill_color);
    if (err != UI_ERROR_NONE) {
      return err;
    }
    if (has_border) {
      err = renderer->draw_border(renderer, (float)rect.x, (float)rect.y,
                                  (float)rect.width, (float)rect.height, 1.5f,
                                  border_color);
      if (err != UI_ERROR_NONE) {
        return err;
      }
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
 * @brief Recursively hit-tests the layout tree to identify which element was
 * clicked.
 * @param node Layout node to test.
 * @param x Mouse X coordinate.
 * @param y Mouse Y coordinate.
 * @param out_target Pointer to receive the hit layout node.
 * @return UI_ERROR_NONE on success, or an error code.
 */
static ui_error_t hit_test_layout_node(struct ui_layout_node *node, float x,
                                       float y,
                                       struct ui_layout_node **out_target) {
  struct ui_dom_rect rect;
  struct ui_layout_node *child;
  struct ui_layout_node *hit;
  ui_error_t err;

  *out_target = NULL;
  if (!node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_cssom_view_get_bounding_client_rect(node, &rect);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  if (x < (float)rect.x || x > (float)(rect.x + rect.width) ||
      y < (float)rect.y || y > (float)(rect.y + rect.height)) {
    return UI_ERROR_NONE;
  }

  child = node->first_child;
  while (child) {
    err = hit_test_layout_node(child, x, y, &hit);
    if (err != UI_ERROR_NONE) {
      return err;
    }
    if (hit) {
      *out_target = hit;
      return UI_ERROR_NONE;
    }
    child = child->next_sibling;
  }

  *out_target = node;
  return UI_ERROR_NONE;
}

/**
 * @brief Handles user click events on interactive UI controls.
 * @param state Pointer to application state.
 * @param router Router instance.
 * @param outlet Outlet container node.
 * @param layout_root Layout root node.
 * @param x Mouse X coordinate.
 * @param y Mouse Y coordinate.
 * @param out_needs_layout Pointer to receive flag if layout must be updated.
 * @return UI_ERROR_NONE on success, or an error code.
 */
static ui_error_t handle_click(struct app_state *state,
                               struct ui_router *router,
                               struct ui_dom_node *outlet,
                               struct ui_layout_node *layout_root, float x,
                               float y, int *out_needs_layout) {
  struct ui_layout_node *target = NULL;
  const char *id = NULL;
  const char *cls = NULL;
  const char *user_id = NULL;
  char route_buf[64];
  size_t synced = 0;
  ui_error_t err;

  *out_needs_layout = 0;
  if (!layout_root) {
    return UI_ERROR_NONE;
  }

  err = hit_test_layout_node(layout_root, x, y, &target);
  if (err != UI_ERROR_NONE || !target || !target->dom_node) {
    return UI_ERROR_NONE;
  }

  err = ui_dom_node_get_attribute(target->dom_node, "id", &id);
  if (err != UI_ERROR_NONE) {
    id = NULL;
  }
  err = ui_dom_node_get_attribute(target->dom_node, "class", &cls);
  if (err != UI_ERROR_NONE) {
    cls = NULL;
  }

  if ((id && strcmp(id, "mode-toggle-btn") == 0) ||
      (cls && strcmp(cls, "mode-toggle-btn") == 0)) {
    err = oauth2_view_toggle_mode(state);
    if (err == UI_ERROR_NONE) {
      err = ui_router_navigate(router, "/login");
      if (err == UI_ERROR_NONE) {
        err = mount_active_route_view(router, outlet);
        if (err == UI_ERROR_NONE) {
          *out_needs_layout = 1;
        }
      }
    }
  } else if (id && strcmp(id, "quick-admin-btn") == 0) {
    err = oauth2_view_submit_login(state, "admin", "admin123");
    if (err == UI_ERROR_NONE) {
      err = ui_router_navigate(router, "/profile");
      if (err == UI_ERROR_NONE) {
        err = mount_active_route_view(router, outlet);
        if (err == UI_ERROR_NONE) {
          *out_needs_layout = 1;
        }
      }
    }
  } else if (id && strcmp(id, "quick-alice-btn") == 0) {
    err = oauth2_view_submit_login(state, "offline_alice", "secret123");
    if (err == UI_ERROR_NONE) {
      err = ui_router_navigate(router, "/profile");
      if (err == UI_ERROR_NONE) {
        err = mount_active_route_view(router, outlet);
        if (err == UI_ERROR_NONE) {
          *out_needs_layout = 1;
        }
      }
    }
  } else if (id && strcmp(id, "login-submit-btn") == 0) {
    err = oauth2_view_submit_login(state, "offline_alice", "secret123");
    if (err == UI_ERROR_NONE) {
      err = ui_router_navigate(router, "/profile");
      if (err == UI_ERROR_NONE) {
        err = mount_active_route_view(router, outlet);
        if (err == UI_ERROR_NONE) {
          *out_needs_layout = 1;
        }
      }
    }
  } else if (id && strcmp(id, "save-profile-btn") == 0) {
    err = oauth2_view_save_profile(state, "Updated bio from GUI!");
    if (err == UI_ERROR_NONE) {
      err = mount_active_route_view(router, outlet);
      if (err == UI_ERROR_NONE) {
        *out_needs_layout = 1;
      }
    }
  } else if (id && strcmp(id, "view-admin-btn") == 0) {
    err = ui_router_navigate(router, "/admin");
    if (err == UI_ERROR_NONE) {
      err = mount_active_route_view(router, outlet);
      if (err == UI_ERROR_NONE) {
        *out_needs_layout = 1;
      }
    }
  } else if (id && (strcmp(id, "sync-changes-btn") == 0 ||
                    strcmp(id, "admin-sync-btn") == 0)) {
    err = oauth2_view_sync_changes(state, &synced);
    if (err == UI_ERROR_NONE) {
      err = mount_active_route_view(router, outlet);
      if (err == UI_ERROR_NONE) {
        *out_needs_layout = 1;
      }
    }
  } else if (id && (strcmp(id, "back-profile-btn") == 0 ||
                    strcmp(id, "back-to-profile-btn") == 0)) {
    err = ui_router_navigate(router, "/profile");
    if (err == UI_ERROR_NONE) {
      err = mount_active_route_view(router, outlet);
      if (err == UI_ERROR_NONE) {
        *out_needs_layout = 1;
      }
    }
  } else if (id && strcmp(id, "back-to-directory-btn") == 0) {
    err = ui_router_navigate(router, "/admin");
    if (err == UI_ERROR_NONE) {
      err = mount_active_route_view(router, outlet);
      if (err == UI_ERROR_NONE) {
        *out_needs_layout = 1;
      }
    }
  } else {
    err = ui_dom_node_get_attribute(target->dom_node, "data-user-id", &user_id);
    if (err != UI_ERROR_NONE) {
      user_id = NULL;
    }
    if (!user_id && target->parent && target->parent->dom_node) {
      err = ui_dom_node_get_attribute(target->parent->dom_node, "data-user-id",
                                      &user_id);
      if (err != UI_ERROR_NONE) {
        user_id = NULL;
      }
    }
    if (user_id != NULL) {
#if defined(_MSC_VER)
      sprintf_s(route_buf, sizeof(route_buf), "/users/%s", user_id);
#else
      sprintf(route_buf, "/users/%s", user_id);
#endif
      err = ui_router_navigate(router, route_buf);
      if (err == UI_ERROR_NONE) {
        err = mount_active_route_view(router, outlet);
        if (err == UI_ERROR_NONE) {
          *out_needs_layout = 1;
        }
      }
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Application entry point implementation accepting arguments.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return 0 on success, or 1 on failure.
 */
int example_oauth2_sync_flow_main_args(int argc, char **argv) {
  struct app_state *state = NULL;
  struct ui_engine *engine = NULL;
  struct ui_router *router = NULL;
  struct ui_arena *arena = NULL;
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *router_outlet = NULL;
  struct ui_css_stylesheet *stylesheet = NULL;
  struct ui_layout_node *layout_tree = NULL;
  struct ui_window_backend *backend = NULL;
  struct ui_window *window = NULL;
  struct ui_renderer_backend *renderer = NULL;
  struct ui_engine_config config;
  char app_css[4096];
  const char *backend_url = NULL;
  const char *ci_test = NULL;
#if defined(_MSC_VER)
  char *ci_env_val = NULL;
  size_t ci_env_len = 0;
#endif
#if defined(_WIN32) || defined(__CYGWIN__)
  int is_wine = 0;
#if defined(_MSC_VER)
  char *wine_val = NULL;
  size_t wine_len = 0;
#endif
#endif
  int running = 1;
  int frame_count = 0;
  int demo_frame = 0;
  int needs_layout = 1;
  int exit_code = 0;
  enum oauth2_app_error app_rc;
  ui_error_t err;
  int i;

  /* Initialize core application state */
  app_rc = app_state_init(&state);
  if (app_rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Failed to initialize app state: %d\n", (int)app_rc);
    return 1;
  }

  /* Parse optional CLI backend URL */
  if (argv != NULL) {
    for (i = 1; i < argc; ++i) {
      if ((strcmp(argv[i], "--url") == 0 ||
           strcmp(argv[i], "--backend-url") == 0) &&
          i + 1 < argc) {
        app_rc = app_state_set_backend_url(state, argv[i + 1]);
        if (app_rc != OAUTH2_APP_OK) {
          fprintf(stderr, "Failed to set backend URL: %d\n", (int)app_rc);
        }
        i++;
      }
    }
  }

  /* Initialize engine */
  config.num_threads = 0;
  err = ui_engine_create(&config, &engine);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "Failed to initialize UI engine: %d\n", (int)err);
    exit_code = 1;
    goto cleanup;
  }

  err = ui_arena_create(1024 * 1024, &arena);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  state->arena = arena;

  /* Initialize router */
  err = ui_router_create(&router);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  state->router = router;

  err = setup_routes(router, state);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  /* Create DOM root and router outlet */
  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_tag_name(root, "body");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_attribute(root, "id", "app-root");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &router_outlet);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_tag_name(router_outlet, "div");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_set_attribute(router_outlet, "id", "router-outlet");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_dom_node_append_child(root, router_outlet);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  /* Parse stylesheet */
  err = build_app_css(app_css, sizeof(app_css));
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  err = ui_css_parse_stylesheet(app_css, &stylesheet);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  /* Navigate to initial login page */
  err = ui_router_navigate(router, "/login");
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  err = mount_active_route_view(router, router_outlet);
  if (err != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  /* Initialize native window backend */
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

  if (backend != NULL) {
    err =
        backend->create_window(backend, "OAuth2 Sync Flow", 900, 650, &window);
    if (err == UI_ERROR_NONE && window != NULL) {
      err = ui_renderer_gles2_create(&renderer);
      if (err == UI_ERROR_NONE && renderer != NULL) {
        err = renderer->init(renderer, backend, window);
        if (err != UI_ERROR_NONE) {
          fprintf(stderr, "Failed to init renderer: %d\n", (int)err);
          err = ui_renderer_gles2_destroy(renderer);
          if (err != UI_ERROR_NONE && exit_code == 0) {
            exit_code = 1;
          }
          renderer = NULL;
        }
      }
      if (renderer != NULL) {
        err = backend->show_window(backend, window);
        if (err != UI_ERROR_NONE) {
          fprintf(stderr, "Failed to show window: %d\n", (int)err);
        }
      }
    }
  }

  app_rc = app_state_get_backend_url(state, &backend_url);
  if (app_rc != OAUTH2_APP_OK) {
    backend_url = NULL;
  }
  printf("=======================================================\n");
  printf("  c-multiplatform OAuth2 Sync Flow GUI\n");
  printf("  Window: %s (900x650)\n",
         window != NULL ? "Active (Native Window)" : "Headless / Test Mode");
  printf("  Mode: %s (Toggle with checkbox in GUI)\n",
         state->mode == OAUTH2_MODE_ONLINE ? "ONLINE" : "OFFLINE");
  printf("  Backend URL: %s\n",
         backend_url != NULL ? backend_url : "http://127.0.0.1:8080");
  printf("  Active Route: /login\n");
  printf("=======================================================\n");
  fflush(stdout);

#if defined(_MSC_VER)
  if (_dupenv_s(&ci_env_val, &ci_env_len, "CI_TEST_RUN") == 0 &&
      ci_env_val != NULL) {
    ci_test = "1";
    free(ci_env_val);
  }
#else
  ci_test = getenv("CI_TEST_RUN");
#endif
#if defined(CI_TEST_RUN)
  ci_test = "1";
#endif

  /* Run GUI event loop if window is active */
  if (backend != NULL && window != NULL) {
    while (running) {
      struct ui_event event;
      int has_event = 0;

      if (ci_test != NULL && frame_count++ > 2) {
        break;
      }

      do {
        err = backend->poll_events(backend, window, &event, &has_event);
        if (err != UI_ERROR_NONE) {
          running = 0;
          break;
        }
        if (has_event) {
          if (event.type == UI_EVENT_WINDOW_CLOSE) {
            running = 0;
            break;
          }
          if (event.type == UI_EVENT_MOUSE_UP) {
            int click_needs_layout = 0;
            err = handle_click(state, router, router_outlet, layout_tree,
                               (float)event.event_data.mouse.x,
                               (float)event.event_data.mouse.y,
                               &click_needs_layout);
            if (click_needs_layout) {
              needs_layout = 1;
            }
          }
          ui_router_process_event(router, &event);
        }
      } while (has_event && running);

      if (!running) {
        break;
      }

      /* In non-CI demo mode, simulate automated interaction for demonstration
       */
      if (ci_test == NULL) {
        demo_frame++;
        if (demo_frame == 90) {
          oauth2_view_submit_login(state, "admin", "admin123");
          ui_router_navigate(router, "/profile");
          mount_active_route_view(router, router_outlet);
          needs_layout = 1;
        } else if (demo_frame == 200) {
          oauth2_view_save_profile(state,
                                   "Platform Architect (Synced via GUI)");
          mount_active_route_view(router, router_outlet);
          needs_layout = 1;
        } else if (demo_frame == 320) {
          ui_router_navigate(router, "/admin");
          mount_active_route_view(router, router_outlet);
          needs_layout = 1;
        } else if (demo_frame == 450) {
          size_t synced = 0;
          oauth2_view_sync_changes(state, &synced);
          mount_active_route_view(router, router_outlet);
          needs_layout = 1;
        } else if (demo_frame == 580) {
          ui_router_navigate(router, "/users/1");
          mount_active_route_view(router, router_outlet);
          needs_layout = 1;
        }
      }

      if (needs_layout) {
        if (layout_tree != NULL) {
          ui_layout_tree_destroy(layout_tree);
          layout_tree = NULL;
        }
        err = ui_layout_tree_generate(root, stylesheet, &layout_tree);
        if (err == UI_ERROR_NONE && layout_tree != NULL) {
          ui_layout_solve_viewport(layout_tree, 900.0f, 650.0f);
        }
        needs_layout = 0;
      }

      if (renderer != NULL) {
        struct ui_color bg;
        bg.r = 0.94f;
        bg.g = 0.95f;
        bg.b = 0.97f;
        bg.a = 1.0f;
        renderer->set_viewport(renderer, 0, 0, 900, 650);
        renderer->clear(renderer, bg);
        if (layout_tree != NULL) {
          draw_layout_node(renderer, layout_tree);
        }
        err = renderer->flush(renderer);
        if (err != UI_ERROR_NONE) {
          exit_code = 1;
          break;
        }
      }

      err = ui_engine_tick(engine);
      if (err != UI_ERROR_NONE) {
        exit_code = 1;
        break;
      }

      err = backend->swap_buffers(backend, window);
      if (err != UI_ERROR_NONE) {
        exit_code = 1;
        break;
      }

#if defined(_WIN32) || defined(__CYGWIN__)
      if (ci_test == NULL) {
        Sleep(16);
      }
#elif defined(__APPLE__) || defined(__linux__) || defined(__unix__)
      if (ci_test == NULL) {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 16000;
        select(0, NULL, NULL, NULL, &tv);
      }
#endif
    }
  }

cleanup:
  if (layout_tree != NULL) {
    ui_layout_tree_destroy(layout_tree);
    layout_tree = NULL;
  }
  if (router_outlet != NULL) {
    clear_dom_children(router_outlet);
  }
  if (stylesheet != NULL) {
    ui_css_stylesheet_destroy(stylesheet);
    stylesheet = NULL;
  }
  if (root != NULL) {
    err = ui_dom_node_destroy(root);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
    root = NULL;
  }
  if (renderer != NULL) {
    err = ui_renderer_gles2_destroy(renderer);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
    renderer = NULL;
  }
  if (backend != NULL && window != NULL) {
    err = backend->destroy_window(backend, window);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
    window = NULL;
  }
  if (backend != NULL) {
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

  if (router != NULL) {
    err = ui_router_destroy(router);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }
  if (arena != NULL) {
    err = ui_arena_destroy(arena);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }
  if (engine != NULL) {
    err = ui_engine_destroy(engine);
    if (err != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }
  if (state != NULL) {
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK && exit_code == 0) {
      exit_code = 1;
    }
  }

  return exit_code;
}

/**
 * @brief Zero-argument entry point wrapper.
 * @return 0 on success, or 1 on failure.
 */
int example_oauth2_sync_flow_main(void) {
  return example_oauth2_sync_flow_main_args(0, NULL);
}

#ifndef OMIT_MAIN
/**
 * @brief Process entry point.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return 0 on success, or non-zero on failure.
 */
int main(int argc, char **argv) {
  return example_oauth2_sync_flow_main_args(argc, argv);
}
#endif
