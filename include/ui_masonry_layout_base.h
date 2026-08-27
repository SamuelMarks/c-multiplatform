/**
 * @file ui_masonry_layout_base.h
 * @brief Masonry layout component for cascading grid structures.
 */

#ifndef UI_MASONRY_LAYOUT_BASE_H
#define UI_MASONRY_LAYOUT_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
/* clang-format on */

struct ui_masonry_layout_base;

/**
 * @brief Creates a new unstyled masonry layout base component.
 *
 * @param out_masonry Pointer to receive the allocated component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_masonry_layout_base_create(struct ui_masonry_layout_base **out_masonry);

/**
 * @brief Destroys a masonry layout base component.
 *
 * @param masonry The component to destroy.
 */
ui_error_t
ui_masonry_layout_base_destroy(struct ui_masonry_layout_base *masonry);

/**
 * @brief Recalculates and applies the masonry layout (reflow).
 *
 * Efficiently recalculates positions based on column counts and available
 * width.
 *
 * @param masonry The component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_masonry_layout_base_reflow(struct ui_masonry_layout_base *masonry);

/**
 * @brief Gets the underlying component instance for style injection and DOM
 * mounting.
 *
 * @param masonry The masonry layout component.
 * @param out_component The pointer to receive the underlying component.
 * @return ui_error_t `UI_ERROR_NONE` on success.
 */
ui_error_t
ui_masonry_layout_base_get_component(struct ui_masonry_layout_base *masonry,
                                     struct ui_component **out_component);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_masonry_layout_base_bind_data(struct ui_masonry_layout_base *widget,
                                 struct ui_computed *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_MASONRY_LAYOUT_BASE_H */
