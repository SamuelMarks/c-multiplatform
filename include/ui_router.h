#ifndef UI_ROUTER_H
#define UI_ROUTER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include <stddef.h>
/* clang-format on */

struct ui_router;
struct ui_route_request;

/**
 * @brief Gets a path parameter (e.g. from "/settings/:id") by name.
 *
 * @param req The route request.
 * @param param_name The name of the parameter (e.g. "id").
 * @return The parameter value, or NULL if not found.
 */
ui_error_t ui_route_request_get_param(const struct ui_route_request *req,
                                      const char *param_name,
                                      const char **out_param);

/**
 * @brief Gets a query string parameter (e.g. from "?tab=2") by name.
 *
 * @param req The route request.
 * @param query_name The name of the query parameter (e.g. "tab").
 * @return The query value, or NULL if not found.
 */
ui_error_t ui_route_request_get_query(const struct ui_route_request *req,
                                      const char *query_name,
                                      const char **out_query);

/**
 * @brief Gets the exact path string that was requested (excluding query
 * string).
 *
 * @param req The route request.
 * @return The path string.
 */
ui_error_t ui_route_request_get_path(const struct ui_route_request *req,
                                     const char **out_path);

/**
 * @brief Callback function to instantiate a screen component for a matched
 * route.
 *
 * @param req The route request containing path, params, and queries.
 * @param user_data User data provided during route registration.
 * @param out_screen Output pointer for the created screen component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_route_factory_t)(const struct ui_route_request *req,
                                         void *user_data,
                                         struct ui_component **out_screen);

/**
 * @brief Creates a new screen manager (router) navigation stack.
 *
 * @param out_router Pointer to receive the allocated router.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_router_create(struct ui_router **out_router);

/**
 * @brief Destroys a router and all screens in its stack.
 *
 * @param router The router to destroy.
 */
ui_error_t ui_router_destroy(struct ui_router *router);

/**
 * @brief Registers a route pattern mapping to a screen factory.
 *
 * @param router The router instance.
 * @param pattern The route pattern (e.g., "/settings/profile/:id").
 * @param factory The factory function to build the screen.
 * @param user_data Opaque data passed to the factory.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_router_add_route(struct ui_router *router, const char *pattern,
                               ui_route_factory_t factory, void *user_data);

/**
 * @brief Navigates to a specific URL by matching it against registered routes
 * and pushing the resulting screen.
 *
 * @param router The router instance.
 * @param path The URL path to navigate to (e.g., "/settings/profile/123").
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if no matching route is
 * found, or an appropriate error code.
 */
ui_error_t ui_router_navigate(struct ui_router *router, const char *path);

/**
 * @brief Gets the custom state pointer passed during navigation.
 *
 * @param req The route request.
 * @return The state pointer, or NULL if none.
 */
ui_error_t ui_route_request_get_state(const struct ui_route_request *req,
                                      void **out_state);

/**
 * @brief Navigates to a specific URL with an optional state payload.
 *
 * @param router The router instance.
 * @param path The URL path to navigate to.
 * @param state Opaque user state (e.g. form group pointer).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_router_navigate_with_state(struct ui_router *router,
                                         const char *path, void *state);

/**
 * @brief Pushes a new screen component onto the navigation stack.
 *
 * @param router The router.
 * @param screen The screen component to push.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_router_push(struct ui_router *router,
                          struct ui_component *screen);

/**
 * @brief Pops the top screen from the navigation stack and destroys it.
 *
 * @param router The router.
 * @return UI_ERROR_NONE on success, UI_ERROR_QUEUE_EMPTY if the stack is empty.
 */
ui_error_t ui_router_pop(struct ui_router *router);

/**
 * @brief Replaces the current top screen with a new screen component.
 *
 * @param router The router.
 * @param screen The new screen component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_router_replace(struct ui_router *router,
                             struct ui_component *screen);

/**
 * @brief Gets the current top screen from the navigation stack.
 *
 * @param router The router.
 * @return The current screen component, or NULL if the stack is empty.
 */
ui_error_t ui_router_get_current(struct ui_router *router,
                                 struct ui_component **out_current);

struct ui_event;

/**
 * @brief Processes an OS event, looking for deep link events to automatically
 * navigate.
 *
 * @param router The router instance.
 * @param event The event to process.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_router_process_event(struct ui_router *router,
                                   const struct ui_event *event);

/**
 * @brief Installs OS-level integration for the router (e.g., HTML5 History API
 * for Emscripten).
 *
 * @param router The router instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_router_install_os_hooks(struct ui_router *router);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_ROUTER_H */
