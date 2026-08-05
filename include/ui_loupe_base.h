#ifndef UI_LOUPE_BASE_H
#define UI_LOUPE_BASE_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
#include "ui_signal.h"
#include "ui_component.h"
#include "ui_geometry.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle for the loupe/magnifier component.
 */
struct ui_loupe_base;

/**
 * @brief Configuration for a loupe/magnifier.
 */
struct ui_loupe_config {
  float magnification_level;
  float loupe_width;
  float loupe_height;
  float y_offset; /**< Vertical offset from the focal point to prevent the
                     finger from obscuring the loupe */
};

/**
 * @brief Creates a loupe base component.
 *
 * @param arena The memory arena to use for allocation.
 * @param config Configuration for the loupe.
 * @param out_loupe Pointer to receive the created component handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_loupe_base_create(struct ui_arena *arena,
                                const struct ui_loupe_config *config,
                                struct ui_loupe_base **out_loupe);

/**
 * @brief Destroys a loupe base component.
 *
 * @param loupe The component to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_loupe_base_destroy(struct ui_loupe_base *loupe);

/**
 * @brief Updates the focal point of the loupe (typically the user's touch
 * point). This will automatically update the calculated render overlay position
 * based on the y_offset.
 *
 * @param loupe The component.
 * @param focal_point The absolute focal point.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_loupe_base_set_focal_point(struct ui_loupe_base *loupe,
                              const struct ui_dom_point *focal_point);

/**
 * @brief Retrieves the signal for focal point changes.
 * The payload contains the calculated overlay origin (DOMPoint struct pointer),
 * not the raw touch point.
 *
 * @param loupe The component.
 * @param out_signal Pointer to receive the signal handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_loupe_base_get_overlay_origin_signal(struct ui_loupe_base *loupe,
                                                   ui_signal_t **out_signal);

/**
 * @brief Gets the current focal point.
 *
 * @param loupe The component.
 * @param out_focal_point Pointer to receive the focal point.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_loupe_base_get_focal_point(const struct ui_loupe_base *loupe,
                                         struct ui_dom_point *out_focal_point);

/**
 * @brief Toggles the visibility of the loupe (e.g. show on touch start, hide on
 * touch end).
 *
 * @param loupe The component.
 * @param visible True to show the loupe, false to hide.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_loupe_base_set_visible(struct ui_loupe_base *loupe,
                                     ui_bool_t visible);

/**
 * @brief Sets the magnification level.
 *
 * @param loupe The component.
 * @param magnification_level The new magnification level (e.g., 1.5f).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_loupe_base_set_magnification_level(struct ui_loupe_base *loupe,
                                                 float magnification_level);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_LOUPE_BASE_H */
