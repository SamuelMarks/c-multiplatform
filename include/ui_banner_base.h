/**
 * @file ui_banner_base.h
 * @brief Banner/Message bar base component definitions.
 */

#ifndef UI_BANNER_BASE_H
#define UI_BANNER_BASE_H

/** @brief Forward declaration of ui_computed. */
struct ui_computed;

/** @brief Forward declaration of ui_signal. */
struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_component.h"
#include "ui_error.h"
#include "ui_signal.h"
#include "ui_computed.h"
/* clang-format on */

/**
 * @brief Represents an unstyled banner or message bar component.
 */
struct ui_banner_base;

/**
 * @brief Creates a new base banner component.
 *
 * @param out_banner Pointer to output the initialized banner.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_banner_base_create(struct ui_banner_base **out_banner);

/**
 * @brief Sets the text message of the banner.
 *
 * @param banner The banner component.
 * @param text The message text.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_banner_base_set_text(struct ui_banner_base *banner,
                                   const char *text);

/**
 * @brief Toggles whether the banner is user-dismissible (adds/removes a close
 * button semantic).
 *
 * @param banner The banner component.
 * @param is_dismissible 1 if dismissible, 0 otherwise.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_banner_base_set_dismissible(struct ui_banner_base *banner,
                                          int is_dismissible);

/**
 * @brief Sets the open state of the banner.
 *
 * @param banner The banner.
 * @param is_open 1 to open, 0 to close (dismiss).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_banner_base_set_open(struct ui_banner_base *banner, int is_open);

/**
 * @brief Checks if the banner is currently open.
 *
 * @param banner The banner.
 * @param out_is_open Pointer to receive the open state.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_banner_base_is_open(const struct ui_banner_base *banner,
                                  int *out_is_open);

/**
 * @brief Binds the banner's open state to a signal.
 *
 * @param banner The banner.
 * @param open_signal The boolean signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_banner_base_bind_open(struct ui_banner_base *banner,
                                    struct ui_signal *open_signal);

/**
 * @brief Retrieves the computed signal indicating if the banner is animating.
 *
 * @param banner The banner.
 * @param out_animating Pointer to receive the computed signal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_banner_base_get_animating_signal(struct ui_banner_base *banner,
                                    struct ui_computed **out_animating);

/**
 * @brief Gets the underlying component for the banner.
 *
 * @param banner The banner.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_banner_base_get_component(struct ui_banner_base *banner,
                                        struct ui_component **out_component);

/**
 * @brief Destroys a banner base component.
 *
 * @param banner The banner to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_banner_base_destroy(struct ui_banner_base *banner);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_BANNER_BASE_H */
