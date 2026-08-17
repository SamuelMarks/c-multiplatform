#ifndef UI_PAGE_CONTROL_BASE_H
#define UI_PAGE_CONTROL_BASE_H

struct ui_signal;

/* clang-format off */
#include "ui_component.h"
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct ui_page_control_base
 * @brief Represents an unstyled page control (dot indicator).
 */
struct ui_page_control_base {
  /** @brief The base component. */
  struct ui_component base;
  /** @brief The currently active page (0-based index). */
  int current_page;
  /** @brief The total number of pages. */
  int number_of_pages;
  /** @brief The signal controlling the currently active page. */
  struct ui_signal *current_page_signal;
};

/**
 * @brief Creates a new base page control component.
 *
 * @param out_control Pointer to output the initialized page control.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_page_control_base_create(struct ui_page_control_base **out_control);

/**
 * @brief Sets the total number of pages.
 *
 * @param control The page control component.
 * @param count The number of pages.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_page_control_base_set_number_of_pages(struct ui_page_control_base *control,
                                         int count);

/**
 * @brief Sets the currently active page.
 *
 * @param control The page control component.
 * @param page The index of the active page (0-based).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_page_control_base_set_current_page(struct ui_page_control_base *control,
                                      int page);

/**
 * @brief Binds the active state/index to a signal.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_page_control_base_bind_current_page(struct ui_page_control_base *widget,
                                       struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_PAGE_CONTROL_BASE_H */
