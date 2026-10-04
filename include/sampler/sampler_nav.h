/**
 * @file sampler_nav.h
 * @brief Navigation stack, routing, and deep link management for Compose
 * Material Catalog.
 */

#ifndef SAMPLER_NAV_H
#define SAMPLER_NAV_H

/* clang-format off */
#include "sampler/sampler_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct sampler_nav;

#define SAMPLER_ROUTE_SPECIFICATION "specification"
#define SAMPLER_ROUTE_MATERIAL2 "material"
#define SAMPLER_ROUTE_MATERIAL3 "material3"
#define SAMPLER_ROUTE_COMPONENT_PREFIX "material3/component/"
#define SAMPLER_ROUTE_EXAMPLE_PREFIX "material3/example/"

/**
 * @brief Create a navigation router instance with an empty navigation stack.
 * @param out_nav Pointer receiving newly created sampler_nav instance.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_OUT_OF_MEMORY.
 */
sampler_error_t sampler_nav_create(struct sampler_nav **out_nav);

/**
 * @brief Destroy a navigation router and free all route stack entries.
 * @param nav Double pointer to navigation router to destroy and nullify.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_NULL_POINTER.
 */
sampler_error_t sampler_nav_destroy(struct sampler_nav **nav);

/**
 * @brief Navigate to a new route, pushing it onto the history stack.
 * @param nav Navigation router handle.
 * @param route Route path string.
 * @return SAMPLER_SUCCESS on success, or appropriate error code.
 */
sampler_error_t sampler_nav_navigate(struct sampler_nav *nav,
                                     const char *route);

/**
 * @brief Pop the top route from the history stack (back navigation).
 * @param nav Navigation router handle.
 * @param out_popped Pointer receiving 1 if a route was popped, 0 if already at
 * root.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_NULL_POINTER.
 */
sampler_error_t sampler_nav_pop(struct sampler_nav *nav, int *out_popped);

/**
 * @brief Retrieve the current active route path at the top of the stack.
 * @param nav Navigation router handle.
 * @param out_route Pointer receiving the const char* to the active route.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_ROUTE_NOT_FOUND if
 * empty.
 */
sampler_error_t sampler_nav_get_current_route(const struct sampler_nav *nav,
                                              const char **out_route);

/**
 * @brief Retrieve the current depth of the history stack.
 * @param nav Navigation router handle.
 * @param out_depth Pointer receiving total number of entries on stack.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_NULL_POINTER.
 */
sampler_error_t sampler_nav_get_stack_depth(const struct sampler_nav *nav,
                                            size_t *out_depth);

/**
 * @brief Clear all entries on the navigation stack.
 * @param nav Navigation router handle.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_NULL_POINTER.
 */
sampler_error_t sampler_nav_clear(struct sampler_nav *nav);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SAMPLER_NAV_H */
