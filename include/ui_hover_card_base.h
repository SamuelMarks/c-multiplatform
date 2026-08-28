/**
 * @file ui_hover_card_base.h
 * @brief Base hover card component for tooltip-like informational overlays.
 */

#ifndef UI_HOVER_CARD_BASE_H
#define UI_HOVER_CARD_BASE_H
struct ui_computed;
struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
/* clang-format on */

struct ui_hover_card_base;

/**
 * @brief Creates a new unstyled hover card base component.
 *
 * @param out_hover_card Pointer to receive the allocated component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_hover_card_base_create(struct ui_hover_card_base **out_hover_card);

/**
 * @brief Destroys a hover card base component.
 *
 * @param hover_card The component to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_hover_card_base_destroy(struct ui_hover_card_base *hover_card);

/**
 * @brief Gets the underlying component instance for style injection and DOM
 * mounting.
 *
 * @param hover_card The hover card component.
 * @param out_component Pointer to receive the underlying component.
 * @return ui_error_t `UI_ERROR_NONE` on success.
 */
ui_error_t
ui_hover_card_base_get_component(struct ui_hover_card_base *hover_card,
                                 struct ui_component **out_component);

/**
 * @brief Simulates mouse enter event on the trigger element, starting the open
 * delay timer.
 *
 * @param hover_card The hover card component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_hover_card_base_on_mouse_enter(struct ui_hover_card_base *hover_card);

/**
 * @brief Simulates mouse leave event, which might be intercepted if within a
 * safe polygon.
 *
 * @param hover_card The hover card component.
 * @param cursor_x The cursor X coordinate.
 * @param cursor_y The cursor Y coordinate.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_hover_card_base_on_mouse_leave(struct ui_hover_card_base *hover_card,
                                  float cursor_x, float cursor_y);

/**
 * @brief Binds the open state to a signal.
 *
 * @param widget The widget.
 * @param open_signal The boolean signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_hover_card_base_bind_open(struct ui_hover_card_base *widget,
                                        struct ui_signal *open_signal);

/**
 * @brief Retrieves the computed signal indicating if the widget is animating.
 *
 * @param widget The widget.
 * @param out_animating Pointer to receive the computed signal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_hover_card_base_get_animating_signal(struct ui_hover_card_base *widget,
                                        struct ui_computed **out_animating);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_HOVER_CARD_BASE_H */
