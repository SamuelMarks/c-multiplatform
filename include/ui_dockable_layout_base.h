/* clang-format off */
#ifndef UI_DOCKABLE_LAYOUT_BASE_H
#define UI_DOCKABLE_LAYOUT_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_error.h"
#include "ui_component.h"
#include "ui_drag_drop.h"
#include <stddef.h>

/* clang-format on */

struct ui_dockable_layout_base;

/**
 * @brief Represents the edges where a panel can be docked.
 */
enum ui_dock_edge {
  UI_DOCK_EDGE_LEFT = 0,
  UI_DOCK_EDGE_RIGHT,
  UI_DOCK_EDGE_TOP,
  UI_DOCK_EDGE_BOTTOM,
  UI_DOCK_EDGE_CENTER /**< Used for tabbed docking */
};

/**
 * @brief Creates a new dockable layout base component.
 *
 * @param out_layout Pointer to receive the allocated layout.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_dockable_layout_base_create(struct ui_dockable_layout_base **out_layout);

/**
 * @brief Destroys a dockable layout base component.
 *
 * @param layout The layout to destroy.
 */
void ui_dockable_layout_base_destroy(struct ui_dockable_layout_base *layout);

/**
 * @brief Gets the underlying component for DOM mounting.
 *
 * @param layout The dockable layout.
 * @return The underlying component.
 */
enum ui_error
ui_dockable_layout_base_get_component(struct ui_dockable_layout_base *layout,
                                      struct ui_component **out_component);

/**
 * @brief Docks a panel relative to another panel (or the root if target is 0).
 *
 * @param layout The dockable layout.
 * @param panel_id The unique ID of the panel being docked.
 * @param target_panel_id The unique ID of the existing panel to dock relative
 * to (0 for root).
 * @param edge The edge to dock onto.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_dockable_layout_base_dock_panel(struct ui_dockable_layout_base *layout,
                                   int panel_id, int target_panel_id,
                                   enum ui_dock_edge edge);

/**
 * @brief Removes a panel from the layout.
 *
 * @param layout The dockable layout.
 * @param panel_id The unique ID of the panel to remove.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_dockable_layout_base_remove_panel(struct ui_dockable_layout_base *layout,
                                     int panel_id);

/**
 * @brief Serializes the layout state into a string format.
 *
 * @param layout The dockable layout.
 * @param out_buffer Buffer to receive the serialized layout.
 * @param buffer_size Size of the buffer.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_dockable_layout_base_serialize(struct ui_dockable_layout_base *layout,
                                  char *out_buffer, size_t buffer_size);

/**
 * @brief Deserializes the layout state from a string format.
 *
 * @param layout The dockable layout.
 * @param buffer The serialized layout string.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_dockable_layout_base_deserialize(struct ui_dockable_layout_base *layout,
                                    const char *buffer);

/**
 * @brief Integrates the layout with a drag and drop context, allowing it
 * to define drop zones and process incoming panel drops.
 *
 * @param layout The dockable layout.
 * @param drag_ctx The drag and drop context to integrate with.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_dockable_layout_base_integrate_drag_drop(
    struct ui_dockable_layout_base *layout,
    struct ui_drag_drop_context *drag_ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_DOCKABLE_LAYOUT_BASE_H */
