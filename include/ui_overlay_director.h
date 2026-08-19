/**
 * @file ui_overlay_director.h
 * @brief Manages the global overlay container for modals, popovers, and
 * tooltips.
 */

#ifndef UI_OVERLAY_DIRECTOR_H
#define UI_OVERLAY_DIRECTOR_H

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_dom_node.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque overlay director context.
 */
struct ui_overlay_director;

/**
 * @brief Represents an individual overlay instance mounted via the director.
 */
struct ui_overlay;

/**
 * @brief Creates a new overlay director.
 * @param root_node The global root node where overlays will be physically
 * appended (e.g., a special <body> wrapper).
 * @param out_director Pointer to receive the allocated director.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_overlay_director_create(struct ui_dom_node *root_node,
                           struct ui_overlay_director **out_director);

/**
 * @brief Destroys an overlay director and unmounts all active overlays.
 * @param director The director to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_overlay_director_destroy(struct ui_overlay_director *director);

/**
 * @brief Mounts a component as a top-level overlay.
 * @param director The director.
 * @param component The component to mount.
 * @param z_index The requested z-index stacking order.
 * @param out_overlay Pointer to receive the overlay instance handle.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_overlay_director_mount_component(struct ui_overlay_director *director,
                                    struct ui_component *component, int z_index,
                                    struct ui_overlay **out_overlay);

/**
 * @brief Unmounts and destroys an active overlay.
 * @param director The director.
 * @param overlay The overlay to unmount.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_overlay_director_unmount(struct ui_overlay_director *director,
                                       struct ui_overlay *overlay);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_OVERLAY_DIRECTOR_H */
