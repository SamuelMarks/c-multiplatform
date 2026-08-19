/**
 * @file ui_badge_base.h
 * @brief Badge base component definitions.
 */

#ifndef UI_BADGE_BASE_H
#define UI_BADGE_BASE_H

/** @brief Forward declaration of ui_computed. */
struct ui_computed;

/** @brief Forward declaration of ui_signal. */
struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
/* clang-format on */

/** @brief Opaque handle to a badge component. */
struct ui_badge_base;

/**
 * @brief Creates a new badge base component.
 *
 * @param out_badge Pointer to receive the allocated badge component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_badge_base_create(struct ui_badge_base **out_badge);

/**
 * @brief Destroys a badge base component.
 *
 * @param badge The badge component to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_badge_base_destroy(struct ui_badge_base *badge);

/**
 * @brief Sets the value to display in the badge.
 * If the value exceeds max_value, the badge displays "max_value+".
 *
 * @param badge The badge component.
 * @param value The value to display.
 * @param max_value The maximum value before adding a '+' suffix (e.g., 99).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_badge_base_set_value(struct ui_badge_base *badge, int value,
                                   int max_value);

/**
 * @brief Sets the content of the badge to a custom text string, overriding
 * numeric value formatting.
 *
 * @param badge The badge component.
 * @param text The custom text to display.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_badge_base_set_text(struct ui_badge_base *badge,
                                  const char *text);

/**
 * @brief Hides or shows the badge. Useful for hiding empty badges.
 *
 * @param badge The badge component.
 * @param is_hidden 1 to hide, 0 to show.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_badge_base_set_hidden(struct ui_badge_base *badge, int is_hidden);

/**
 * @brief Gets the underlying component for rendering.
 *
 * @param badge The badge component.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_badge_base_get_component(struct ui_badge_base *badge,
                                       struct ui_component **out_component);

/**
 * @brief Binds the text property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_badge_base_bind_text(struct ui_badge_base *widget,
                                   struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_BADGE_BASE_H */
