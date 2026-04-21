/* clang-format off */
#ifndef M3_ROUTER_H
#define M3_ROUTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cmp.h>
#include "material_catalog.h"

/**
 * \enum m3_route_transition_t
 * \brief Type of transition animation to play when changing routes.
 */
typedef enum m3_route_transition_t {
    M3_TRANSITION_NONE = 0,
    M3_TRANSITION_SHARED_AXIS_Z = 1,
    M3_TRANSITION_SHARED_AXIS_X = 2,
    M3_TRANSITION_SHARED_AXIS_Y = 3,
    M3_TRANSITION_FADE_THROUGH = 4,
    M3_TRANSITION_ELEVATION_SCALE = 5
} m3_route_transition_t;

/**
 * \struct m3_route_t
 * \brief Represents a single route configuration in the navigation stack.
 */
typedef struct m3_route_t {
    catalog_screen_id_t screen_id;
    int component_id;
    int example_index;
    
    /* Argument pointer (e.g. string deep link or detailed struct) */
    void *args;
    
    /* Transition applied when entering this route */
    m3_route_transition_t transition;
    
    /* Cached scroll state */
    float saved_scroll_y;
    float saved_scroll_x;
} m3_route_t;

/**
 * \struct m3_router_engine_t
 * \brief The router engine managing back-stack, transitions, and deep links.
 */
typedef struct m3_router_engine_t {
    m3_route_t stack[CATALOG_MAX_BACK_STACK];
    int stack_size;
    
    /* A hook to intercept OS back buttons/Escape keys */
    int os_back_intercepted;
} m3_router_engine_t;

/**
 * \brief Initializes the router engine.
 * \param engine The router engine context.
 * \return 0 on success, non-zero on error.
 */
int m3_router_init(m3_router_engine_t *engine);

/**
 * \brief Cleans up the router engine (frees any active args).
 * \param engine The router engine context.
 * \return 0 on success, non-zero on error.
 */
int m3_router_cleanup(m3_router_engine_t *engine);

/**
 * \brief Pushes a new route onto the stack.
 * \param engine The router engine context.
 * \param screen_id The ID of the destination screen.
 * \param component_id The component ID if navigating to a details page.
 * \param example_index The example index if viewing an isolated component example.
 * \param args Optional pointer to custom arguments.
 * \param transition The transition animation to use.
 * \return 0 on success, non-zero on error (e.g. stack full).
 */
int m3_router_push(m3_router_engine_t *engine, catalog_screen_id_t screen_id, int component_id, int example_index, void *args, m3_route_transition_t transition);

/**
 * \brief Pops the top route from the stack, restoring the previous view.
 * \param engine The router engine context.
 * \return 0 on success, non-zero on error (e.g. stack empty).
 */
int m3_router_pop(m3_router_engine_t *engine);

/**
 * \brief Replaces the current top route with a new one.
 * \param engine The router engine context.
 * \param screen_id The ID of the destination screen.
 * \param component_id The component ID if navigating to a details page.
 * \param example_index The example index if viewing an isolated component example.
 * \param args Optional pointer to custom arguments.
 * \param transition The transition animation to use.
 * \return 0 on success, non-zero on error.
 */
int m3_router_replace(m3_router_engine_t *engine, catalog_screen_id_t screen_id, int component_id, int example_index, void *args, m3_route_transition_t transition);

/**
 * \brief Clears the stack and sets a new root route.
 * \param engine The router engine context.
 * \param screen_id The root screen ID.
 * \return 0 on success, non-zero on error.
 */
int m3_router_reset_to(m3_router_engine_t *engine, catalog_screen_id_t screen_id);

/**
 * \brief Parses a deep link string (e.g. "cmp://catalog/button/filled") and routes to it.
 * \param engine The router engine context.
 * \param url The URI string to parse.
 * \return 0 on success, non-zero on error (e.g. malformed URL).
 */
int m3_router_handle_deep_link(m3_router_engine_t *engine, const char *url);

/**
 * \brief Checks if the router can pop (i.e. stack size > 1).
 * \param engine The router engine context.
 * \param out_can_pop Pointer to receive boolean flag (1 if can pop).
 * \return 0 on success, non-zero on error.
 */
int m3_router_can_pop(const m3_router_engine_t *engine, int *out_can_pop);

/**
 * \brief Saves the current scroll position for the top route.
 * \param engine The router engine context.
 * \param scroll_x The horizontal scroll offset.
 * \param scroll_y The vertical scroll offset.
 * \return 0 on success, non-zero on error.
 */
int m3_router_save_scroll_state(m3_router_engine_t *engine, float scroll_x, float scroll_y);

/**
 * \brief Retrieves the saved scroll position for the top route.
 * \param engine The router engine context.
 * \param out_scroll_x Pointer to receive the horizontal scroll offset.
 * \param out_scroll_y Pointer to receive the vertical scroll offset.
 * \return 0 on success, non-zero on error.
 */
int m3_router_get_scroll_state(const m3_router_engine_t *engine, float *out_scroll_x, float *out_scroll_y);

#ifdef __cplusplus
}
#endif

#endif /* M3_ROUTER_H */
/* clang-format on */