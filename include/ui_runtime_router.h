/**
 * @file ui_runtime_router.h
 * @brief Dynamic runtime navigation router for switching and transitioning
 * between runtime-defined screens.
 */

#ifndef UI_RUNTIME_ROUTER_H
#define UI_RUNTIME_ROUTER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_runtime_schema.h"
#include "ui_component_registry.h"
#include "ui_dynamic_context.h"
#include "ui_app_state_registry.h"
#include "ui_dom_node.h"
#include "ui_arena.h"
#include <stddef.h>
/* clang-format on */

struct ui_runtime_router;

/**
 * @brief Creates a new runtime router.
 *
 * @param arena Memory arena to allocate from (or NULL for default heap).
 * @param out_router Pointer to receive the allocated router.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_runtime_router_create(
    struct ui_arena *arena, struct ui_runtime_router **out_router);

/**
 * @brief Destroys a runtime router and its navigation history.
 *
 * @param router The router to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_runtime_router_destroy(struct ui_runtime_router *router);

/**
 * @brief Registers a route definition with the router.
 *
 * @param router The router.
 * @param path Route path (e.g. "/home", "/survey").
 * @param root_node Root AST node defining the screen UI tree.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_runtime_router_register_route(
    struct ui_runtime_router *router, const char *path,
    struct ui_runtime_node *root_node);

/**
 * @brief Registers all routes from an application manifest into the router.
 *
 * @param router The router.
 * @param manifest The parsed application manifest.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_runtime_router_load_manifest(struct ui_runtime_router *router,
                                const struct ui_runtime_app_manifest *manifest);

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
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_runtime_router_navigate(
    struct ui_runtime_router *router, const char *path,
    struct ui_dom_node *mount_host, struct ui_component_registry *registry,
    struct ui_dynamic_context *ctx, struct ui_app_state_registry *app_state);

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
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_runtime_router_pop(
    struct ui_runtime_router *router, struct ui_dom_node *mount_host,
    struct ui_component_registry *registry, struct ui_dynamic_context *ctx,
    struct ui_app_state_registry *app_state);

/**
 * @brief Gets the current active route path.
 *
 * @param router The router.
 * @param out_path Pointer to receive the path string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if no route is
 * active.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_runtime_router_get_current_route(
    const struct ui_runtime_router *router, const char **out_path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_RUNTIME_ROUTER_H */
