/**
 * @file ui_card_base.h
 * @brief Base card component for grouping related content and actions.
 */

#ifndef UI_CARD_BASE_H
#define UI_CARD_BASE_H

/** @brief Forward declaration of ui_computed. */
struct ui_computed;

/** @brief Forward declaration of ui_signal. */
struct ui_signal;

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque structure representing the base card component.
 */
struct ui_card_base;

/**
 * @brief Creates a new base card instance.
 *
 * @param out_card Pointer to receive the allocated card instance.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY if allocation fails,
 * or UI_ERROR_INVALID_ARGUMENT if out_card is null.
 */
ui_error_t ui_card_base_create(struct ui_card_base **out_card);

/**
 * @brief Destroys a card instance and frees its resources.
 *
 * @param card The card instance to destroy. If null, does nothing.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_card_base_destroy(struct ui_card_base *card);

/**
 * @brief Sets the header content of the card.
 *
 * @param card The card instance.
 * @param header_content The component to mount as the header, or NULL to
 * remove.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT if card is
 * null.
 */
ui_error_t ui_card_base_set_header(struct ui_card_base *card,
                                   struct ui_component *header_content);

/**
 * @brief Sets the main content of the card.
 *
 * @param card The card instance.
 * @param content The component to mount as the main content, or NULL to remove.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT if card is
 * null.
 */
ui_error_t ui_card_base_set_content(struct ui_card_base *card,
                                    struct ui_component *content);

/**
 * @brief Sets the action area content of the card.
 *
 * @param card The card instance.
 * @param actions The component to mount as the actions area, or NULL to remove.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT if card is
 * null.
 */
ui_error_t ui_card_base_set_actions(struct ui_card_base *card,
                                    struct ui_component *actions);

/**
 * @brief Retrieves the underlying component of the card.
 *
 * @param card The card instance.
 * @param out_component Pointer to receive the underlying ui_component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_card_base_get_component(struct ui_card_base *card,
                                      struct ui_component **out_component);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_card_base_bind_data(struct ui_card_base *widget,
                                  struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CARD_BASE_H */
