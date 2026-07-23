#ifndef UI_DIVIDER_BASE_H
#define UI_DIVIDER_BASE_H

struct ui_computed;

struct ui_signal;

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Divider orientation options.
 */
enum ui_divider_orientation {
  UI_DIVIDER_ORIENTATION_HORIZONTAL = 0,
  UI_DIVIDER_ORIENTATION_VERTICAL = 1
};

/**
 * @brief Opaque structure representing the base divider component.
 */
struct ui_divider_base;

/**
 * @brief Creates a new base divider instance.
 *
 * @param out_divider Pointer to receive the allocated divider instance.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY if allocation fails,
 * or UI_ERROR_INVALID_ARGUMENT if out_divider is null.
 */
enum ui_error ui_divider_base_create(struct ui_divider_base **out_divider);

/**
 * @brief Destroys a divider instance and frees its resources.
 *
 * @param divider The divider instance to destroy. If null, does nothing.
 */
void ui_divider_base_destroy(struct ui_divider_base *divider);

/**
 * @brief Sets the orientation of the divider.
 *
 * @param divider The divider instance.
 * @param orientation Horizontal or Vertical.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT if null.
 */
enum ui_error
ui_divider_base_set_orientation(struct ui_divider_base *divider,
                                enum ui_divider_orientation orientation);

/**
 * @brief Sets whether the divider should have an inset margin.
 *
 * @param divider The divider instance.
 * @param inset 1 for inset, 0 for full-bleed.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT if null.
 */
enum ui_error ui_divider_base_set_inset(struct ui_divider_base *divider,
                                        int inset);

/**
 * @brief Retrieves the underlying component of the divider.
 *
 * @param divider The divider instance.
 * @return Pointer to the ui_component, or NULL if invalid.
 */
enum ui_error
ui_divider_base_get_component(struct ui_divider_base *divider,
                              struct ui_component **out_component);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_divider_base_bind_data(struct ui_divider_base *widget,
                                        struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_DIVIDER_BASE_H */
