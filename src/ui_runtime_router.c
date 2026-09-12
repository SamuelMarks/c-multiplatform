/**
 * @file ui_runtime_router.c
 * @brief Dynamic runtime navigation router implementation for switching
 * screens.
 */

/* clang-format off */
#include "ui_runtime_router.h"
#include "ui_runtime_builder.h"
#include "ui_internal_mem.h"
#include <string.h>
#include <stdlib.h>
/* clang-format on */

/**
 * @struct ui_runtime_route_entry
 * @brief Route mapping entry linking a path string to a root AST node.
 */
struct ui_runtime_route_entry {
  char *path;                          /**< URL/Route path */
  struct ui_runtime_node *root_node;   /**< Screen widget tree AST root */
  struct ui_runtime_route_entry *next; /**< Next route */
};

/**
 * @struct ui_history_entry
 * @brief Stack entry for backward navigation history.
 */
struct ui_history_entry {
  char *path;                    /**< History path */
  struct ui_history_entry *next; /**< Next history node */
};

/**
 * @struct ui_runtime_router
 * @brief Navigation router holding registered screens and navigation history.
 */
struct ui_runtime_router {
  struct ui_arena *arena; /**< Memory arena used for allocations */
  struct ui_runtime_route_entry *routes; /**< Head of registered routes */
  struct ui_history_entry *history;      /**< Navigation history stack */
  char *current_path;                    /**< Currently active route path */
};

/**
 * @brief Creates a new runtime router.
 *
 * @param arena Memory arena to allocate from (or NULL for default heap).
 * @param out_router Pointer to receive the allocated router.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_runtime_router_create(struct ui_arena *arena,
                                    struct ui_runtime_router **out_router) {
  struct ui_runtime_router *router = NULL;
  ui_error_t rc;

  if (!out_router) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (arena) {
    rc = ui_arena_alloc(arena, sizeof(struct ui_runtime_router), 8,
                        (void **)&router);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    router = (struct ui_runtime_router *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_runtime_router));
    if (!router) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
  }

  router->arena = arena;
  router->routes = NULL;
  router->history = NULL;
  router->current_path = NULL;
  *out_router = router;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a runtime router and its navigation history.
 *
 * @param router The router to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_runtime_router_destroy(struct ui_runtime_router *router) {
  if (!router) {
    return UI_ERROR_NONE;
  }

  /* If arena-allocated, arena owns memory */
  if (router->arena) {
    return UI_ERROR_NONE;
  }

  while (router->routes) {
    struct ui_runtime_route_entry *next = router->routes->next;
    C_MULTIPLATFORM_FREE(router->routes->path);
    C_MULTIPLATFORM_FREE(router->routes);
    router->routes = next;
  }

  while (router->history) {
    struct ui_history_entry *next = router->history->next;
    C_MULTIPLATFORM_FREE(router->history->path);
    C_MULTIPLATFORM_FREE(router->history);
    router->history = next;
  }

  C_MULTIPLATFORM_FREE(router);
  return UI_ERROR_NONE;
}

/**
 * @brief Registers a route definition with the router.
 *
 * @param router The router.
 * @param path Route path (e.g. "/home", "/survey").
 * @param root_node Root AST node defining the screen UI tree.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_runtime_router_register_route(struct ui_runtime_router *router,
                                            const char *path,
                                            struct ui_runtime_node *root_node) {
  struct ui_runtime_route_entry *curr;
  struct ui_runtime_route_entry *entry = NULL;
  size_t path_len;
  ui_error_t rc;

  if (!router || !path || !root_node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = router->routes;
  while (curr) {
    if (strcmp(curr->path, path) == 0) {
      curr->root_node = root_node;
      return UI_ERROR_NONE;
    }
    curr = curr->next;
  }

  path_len = strlen(path);
  if (router->arena) {
    rc = ui_arena_alloc(router->arena, sizeof(struct ui_runtime_route_entry), 8,
                        (void **)&entry);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = ui_arena_alloc(router->arena, path_len + 1, 1, (void **)&entry->path);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    entry = (struct ui_runtime_route_entry *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_runtime_route_entry));
    if (!entry) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    entry->path = (char *)C_MULTIPLATFORM_MALLOC(path_len + 1);
    if (!entry->path) {
      C_MULTIPLATFORM_FREE(entry);
      return UI_ERROR_OUT_OF_MEMORY;
    }
  }

  if (UI_STRCPY(entry->path, path_len + 1, path) != 0) {
    if (!router->arena) {
      C_MULTIPLATFORM_FREE(entry->path);
      C_MULTIPLATFORM_FREE(entry);
    }
    return UI_ERROR_UNKNOWN;
  }

  entry->root_node = root_node;
  entry->next = router->routes;
  router->routes = entry;

  return UI_ERROR_NONE;
}

/**
 * @brief Registers all routes from an application manifest into the router.
 *
 * @param router The router.
 * @param manifest The parsed application manifest.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_runtime_router_load_manifest(
    struct ui_runtime_router *router,
    const struct ui_runtime_app_manifest *manifest) {
  const struct ui_runtime_route_def *r;
  ui_error_t rc;

  if (!router || !manifest) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  r = manifest->routes;
  while (r) {
    if (r->path && r->root_node) {
      rc = ui_runtime_router_register_route(router, r->path, r->root_node);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
    r = r->next;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Internal helper to navigate with or without recording history.
 */
static ui_error_t ui_runtime_router_navigate_internal(
    struct ui_runtime_router *router, const char *path,
    struct ui_dom_node *mount_host, struct ui_component_registry *registry,
    struct ui_dynamic_context *ctx, struct ui_app_state_registry *app_state,
    int record_history) {
  const struct ui_runtime_route_entry *target_route = NULL;
  const struct ui_runtime_route_entry *curr;
  struct ui_dom_node *screen_dom = NULL;
  struct ui_history_entry *hist = NULL;
  size_t path_len;
  ui_error_t rc;

  if (!router || !path || !mount_host) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = router->routes;
  while (curr) {
    if (strcmp(curr->path, path) == 0) {
      target_route = curr;
      break;
    }
    curr = curr->next;
  }

  if (!target_route) {
    return UI_ERROR_NOT_FOUND;
  }

  /* Clear previous screen children from mount_host */
  while (mount_host->first_child) {
    struct ui_dom_node *child = mount_host->first_child;
    ui_error_t rc_cleanup = ui_dom_node_remove_child(mount_host, child);
    (void)rc_cleanup;
    rc_cleanup = ui_dom_node_destroy(child);
    (void)rc_cleanup;
  }

  /* Build new screen DOM */
  rc = ui_runtime_build_tree(target_route->root_node, registry, ctx, app_state,
                             router, mount_host, &screen_dom);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (screen_dom) {
    ui_error_t rc_append = ui_dom_node_append_child(mount_host, screen_dom);
    (void)rc_append;
  }

  if (record_history) {
    /* Record history */
    path_len = strlen(path);
    if (router->arena) {
      rc = ui_arena_alloc(router->arena, sizeof(struct ui_history_entry), 8,
                          (void **)&hist);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = ui_arena_alloc(router->arena, path_len + 1, 1, (void **)&hist->path);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    } else {
      hist = (struct ui_history_entry *)C_MULTIPLATFORM_MALLOC(
          sizeof(struct ui_history_entry));
      if (!hist) {
        return UI_ERROR_OUT_OF_MEMORY;
      }
      hist->path = (char *)C_MULTIPLATFORM_MALLOC(path_len + 1);
      if (!hist->path) {
        C_MULTIPLATFORM_FREE(hist);
        return UI_ERROR_OUT_OF_MEMORY;
      }
    }

    if (UI_STRCPY(hist->path, path_len + 1, path) != 0) {
      if (!router->arena) {
        C_MULTIPLATFORM_FREE(hist->path);
        C_MULTIPLATFORM_FREE(hist);
      }
      return UI_ERROR_UNKNOWN;
    }

    hist->next = router->history;
    router->history = hist;
  }

  router->current_path = router->history->path;

  return UI_ERROR_NONE;
}

/**
 * @brief Navigates to a target route path, constructing the screen into
 * mount_host.
 *
 * @param router The router.
 * @param path The path to navigate to.
 * @param mount_host The DOM container element where the screen is mounted.
 * @param registry The component registry.
 * @param ctx The scoped dynamic context.
 * @param app_state The global application state registry.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if route not found.
 */
ui_error_t ui_runtime_router_navigate(struct ui_runtime_router *router,
                                      const char *path,
                                      struct ui_dom_node *mount_host,
                                      struct ui_component_registry *registry,
                                      struct ui_dynamic_context *ctx,
                                      struct ui_app_state_registry *app_state) {
  return ui_runtime_router_navigate_internal(router, path, mount_host, registry,
                                             ctx, app_state, 1);
}

/**
 * @brief Pops the top screen from the navigation stack and transitions back to
 * the previous screen.
 *
 * @param router The router.
 * @param mount_host The DOM container element where the screen is mounted.
 * @param registry The component registry.
 * @param ctx The scoped dynamic context.
 * @param app_state The global application state registry.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if history is empty.
 */
ui_error_t ui_runtime_router_pop(struct ui_runtime_router *router,
                                 struct ui_dom_node *mount_host,
                                 struct ui_component_registry *registry,
                                 struct ui_dynamic_context *ctx,
                                 struct ui_app_state_registry *app_state) {
  struct ui_history_entry *popped;
  const char *prev_path;
  ui_error_t rc;

  if (!router || !mount_host) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!router->history || !router->history->next) {
    return UI_ERROR_NOT_FOUND; /* No previous screen to pop to */
  }

  popped = router->history;
  router->history = popped->next;
  prev_path = router->history->path;

  if (!router->arena) {
    C_MULTIPLATFORM_FREE(popped->path);
    C_MULTIPLATFORM_FREE(popped);
  }

  /* Navigate to previous path without recording new history entry */
  rc = ui_runtime_router_navigate_internal(router, prev_path, mount_host,
                                           registry, ctx, app_state, 0);
  return rc;
}

/**
 * @brief Gets the current active route path.
 *
 * @param router The router.
 * @param out_path Pointer to receive the path string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if no route is
 * active.
 */
ui_error_t
ui_runtime_router_get_current_route(const struct ui_runtime_router *router,
                                    const char **out_path) {
  if (!router || !out_path) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!router->current_path) {
    return UI_ERROR_NOT_FOUND;
  }

  *out_path = router->current_path;
  return UI_ERROR_NONE;
}
