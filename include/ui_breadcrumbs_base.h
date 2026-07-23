#ifndef UI_BREADCRUMBS_BASE_H
#define UI_BREADCRUMBS_BASE_H

struct ui_signal;

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_router.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_breadcrumbs_base;

/**
 * @brief Creates a new breadcrumbs component.
 *
 * @param router The router to integrate with.
 * @param out_breadcrumbs Pointer to receive the allocated breadcrumbs instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_breadcrumbs_base_create(struct ui_router *router,
                           struct ui_breadcrumbs_base **out_breadcrumbs);

/**
 * @brief Destroys the breadcrumbs instance.
 *
 * @param breadcrumbs The breadcrumbs instance.
 */
void ui_breadcrumbs_base_destroy(struct ui_breadcrumbs_base *breadcrumbs);

/**
 * @brief Gets the underlying UI component for rendering.
 *
 * @param breadcrumbs The breadcrumbs instance.
 * @return Pointer to the underlying ui_component, or NULL if invalid.
 */
enum ui_error
ui_breadcrumbs_base_get_component(struct ui_breadcrumbs_base *breadcrumbs,
                                  struct ui_component **out_component);

/**
 * @brief Auto-generates the breadcrumb links based on a URL path.
 *
 * @param breadcrumbs The breadcrumbs instance.
 * @param path The URL path (e.g., "/settings/profile/security").
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_breadcrumbs_base_set_path(struct ui_breadcrumbs_base *breadcrumbs,
                             const char *path);

/**
 * @brief Simulates a click on a breadcrumb item at the given index, triggering
 * navigation.
 *
 * @param breadcrumbs The breadcrumbs instance.
 * @param index The index of the item to click.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_breadcrumbs_base_simulate_click(struct ui_breadcrumbs_base *breadcrumbs,
                                   size_t index);

/**
 * @brief Binds the active state/index to a signal.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_breadcrumbs_base_bind_active_index(struct ui_breadcrumbs_base *widget,
                                      struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_BREADCRUMBS_BASE_H */
