#ifndef UI_MODIFIER_H
#define UI_MODIFIER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
/* clang-format on */

struct ui_modifier;

/**
 * @brief Creates a new modifier payload.
 *
 * @param out_modifier Pointer to receive the allocated modifier.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_modifier_create(struct ui_modifier **out_modifier);

/**
 * @brief Destroys a modifier payload.
 *
 * @param modifier The modifier to destroy.
 */
void ui_modifier_destroy(struct ui_modifier *modifier);

/**
 * @brief Adds a custom CSS class to the modifier.
 *
 * @param modifier The modifier.
 * @param class_name The CSS class name to inject.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_modifier_add_class(struct ui_modifier *modifier,
                                    const char *class_name);

/**
 * @brief Adds a custom inline CSS style to the modifier.
 *
 * @param modifier The modifier.
 * @param property_name The CSS property name (e.g., "background-color").
 * @param property_value The CSS property value (e.g., "red").
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_modifier_add_style(struct ui_modifier *modifier,
                                    const char *property_name,
                                    const char *property_value);

/**
 * @brief Applies the modifier to a given component's isolated DOM root.
 *
 * @param modifier The modifier.
 * @param component The component to apply the modifier to.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_modifier_apply(const struct ui_modifier *modifier,
                                struct ui_component *component);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_MODIFIER_H */
