#ifndef UI_LINK_BASE_H
#define UI_LINK_BASE_H

struct ui_signal;

/* clang-format off */
#include "ui_component.h"
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents an unstyled base link/hyperlink component.
 */
struct ui_link_base {
  struct ui_component base;
  struct ui_signal *disabled_signal;
  struct ui_signal *text_signal;
};

/**
 * @brief Creates a new base link component.
 *
 * @param out_link Pointer to output the initialized link.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_link_base_create(struct ui_link_base **out_link);

/**
 * @brief Sets the href (URL destination) for the link.
 *
 * @param link The link component.
 * @param url The destination URL.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_link_base_set_href(struct ui_link_base *link, const char *url);

/**
 * @brief Sets the label/text content of the link.
 *
 * @param link The link component.
 * @param text The text to display.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_link_base_set_text(struct ui_link_base *link, const char *text);

/**
 * @brief Binds the disabled state to a boolean signal.
 *
 * @param widget The widget.
 * @param disabled_signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_link_base_bind_disabled(struct ui_link_base *widget,
                                      struct ui_signal *disabled_signal);

/**
 * @brief Binds the text content to a string signal for dynamic
 * internationalization.
 *
 * @param widget The widget.
 * @param text_signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_link_base_bind_text(struct ui_link_base *widget,
                                  struct ui_signal *text_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_LINK_BASE_H */
