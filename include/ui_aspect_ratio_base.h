#ifndef UI_ASPECT_RATIO_BASE_H
#define UI_ASPECT_RATIO_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
/* clang-format on */

struct ui_aspect_ratio_base;

/**
 * @brief Creates a new unstyled aspect ratio bounding box base component.
 *
 * @param out_aspect_ratio Pointer to receive the allocated component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_aspect_ratio_base_create(struct ui_aspect_ratio_base **out_aspect_ratio);

/**
 * @brief Destroys an aspect ratio base component.
 *
 * @param aspect_ratio The component to destroy.
 */
void ui_aspect_ratio_base_destroy(struct ui_aspect_ratio_base *aspect_ratio);

/**
 * @brief Sets the aspect ratio (width / height).
 *
 * @param aspect_ratio The component.
 * @param ratio The aspect ratio float value (e.g., 16.0f / 9.0f).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_aspect_ratio_base_set_ratio(struct ui_aspect_ratio_base *aspect_ratio,
                               float ratio);

/**
 * @brief Gets the underlying component instance for style injection and DOM
 * mounting.
 *
 * @param aspect_ratio The aspect ratio component.
 * @return The underlying component.
 */
enum ui_error
ui_aspect_ratio_base_get_component(struct ui_aspect_ratio_base *aspect_ratio,
                                   struct ui_component **out_component);

/**
 * @brief Binds the ratio property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_aspect_ratio_base_bind_ratio(struct ui_aspect_ratio_base *widget,
                                struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_ASPECT_RATIO_BASE_H */
