/**
 * @file ui_app_load.c
 * @brief Unified isomorphic application loader implementation.
 */

/* clang-format off */
#include "ui_app_load.h"
#include "ui_runtime_schema.h"
#include "ui_runtime_builder.h"
#include "ui_arena.h"
#include <string.h>
#include <stdlib.h>
/* clang-format on */

/**
 * @brief Mounts an entire application UI tree using the isomorphic execution
 * model.
 *
 * @param config Loader configuration specifying mode and source
 * payload/function.
 * @param mount_host Host DOM node to mount the screen or application into.
 * @param out_root Pointer to receive the constructed root DOM element.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_app_load(const struct ui_app_load_config *config,
                       struct ui_dom_node *mount_host,
                       struct ui_dom_node **out_root) {
  ui_error_t rc;

  if (!config || !out_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (config->mode == UI_APP_LOAD_MODE_AOT) {
    if (!config->aot_mount) {
      return UI_ERROR_INVALID_ARGUMENT;
    }
    return config->aot_mount(mount_host, config->ctx, config->app_state,
                             out_root);
  }

  if (config->mode == UI_APP_LOAD_MODE_RUNTIME) {
    struct ui_arena *arena = NULL;
    struct ui_runtime_app_manifest *manifest = NULL;
    const struct ui_runtime_route_def *target_route = NULL;
    struct ui_dom_node *dom_tree = NULL;

    if (!config->schema_payload) {
      return UI_ERROR_INVALID_ARGUMENT;
    }

    rc = ui_arena_create(65536, &arena);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    rc = ui_runtime_schema_parse(arena, config->schema_payload, &manifest);
    if (rc != UI_ERROR_NONE) {
      ui_arena_destroy(arena);
      return rc;
    }

    if (!manifest->routes) {
      ui_arena_destroy(arena);
      return UI_ERROR_NOT_FOUND;
    }

    /* Find target route matching initial_route, or default to first route */
    target_route = manifest->routes;
    if (manifest->initial_route) {
      const struct ui_runtime_route_def *curr = manifest->routes;
      while (curr) {
        if (strcmp(curr->path, manifest->initial_route) == 0) {
          target_route = curr;
          break;
        }
        curr = curr->next;
      }
    }

    rc = ui_runtime_build_tree(target_route->root_node, config->registry,
                               config->ctx, config->app_state, NULL, mount_host,
                               &dom_tree);
    if (rc != UI_ERROR_NONE) {
      ui_arena_destroy(arena);
      return rc;
    }

    if (mount_host) {
      ui_error_t rc_append = ui_dom_node_append_child(mount_host, dom_tree);
      (void)rc_append;
    }

    *out_root = dom_tree;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief Mounts a dynamic runtime schema snippet into an existing DOM tree.
 *
 * @param snippet_json Serialized JSON widget node snippet.
 * @param parent_node DOM container node to append the dynamic snippet into.
 * @param ctx Dynamic context for data binding.
 * @param app_state Global application state registry.
 * @param out_snippet_root Pointer to receive the root DOM node of the mounted
 * snippet.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_app_load_snippet(const char *snippet_json,
                               struct ui_dom_node *parent_node,
                               struct ui_dynamic_context *ctx,
                               struct ui_app_state_registry *app_state,
                               struct ui_dom_node **out_snippet_root) {
  struct ui_arena *arena = NULL;
  struct ui_runtime_node *node = NULL;
  struct ui_dom_node *dom_node = NULL;
  ui_error_t rc;

  if (!snippet_json || !parent_node || !out_snippet_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_arena_create(32768, &arena);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_runtime_schema_parse_node(arena, snippet_json, &node);
  if (rc != UI_ERROR_NONE) {
    ui_arena_destroy(arena);
    return rc;
  }

  rc = ui_runtime_build_tree(node, NULL, ctx, app_state, NULL, parent_node,
                             &dom_node);
  if (rc != UI_ERROR_NONE) {
    ui_arena_destroy(arena);
    return rc;
  }

  {
    ui_error_t rc_append = ui_dom_node_append_child(parent_node, dom_node);
    (void)rc_append;
  }

  *out_snippet_root = dom_node;
  return UI_ERROR_NONE;
}
