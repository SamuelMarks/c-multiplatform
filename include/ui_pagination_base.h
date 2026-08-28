/**
 * @file ui_pagination_base.h
 * @brief Pagination logic and control component base definitions.
 */

#ifndef UI_PAGINATION_BASE_H
#define UI_PAGINATION_BASE_H

/**
 * @brief Opaque structure representing a signal.
 */
struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Opaque structure representing a pagination base instance.
 */
struct ui_pagination_base;

/**
 * @brief Creates a new pagination base instance.
 *
 * @param out_pagination Pointer to receive the allocated pagination base.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_pagination_base_create(struct ui_pagination_base **out_pagination);

/**
 * @brief Destroys a pagination base instance.
 *
 * @param pagination The pagination instance to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_pagination_base_destroy(struct ui_pagination_base *pagination);

/**
 * @brief Sets the total number of items and the page size.
 *
 * @param pagination The pagination instance.
 * @param total_items The total number of items to paginate.
 * @param page_size The number of items per page. Must be greater than 0.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT if page_size is
 * 0.
 */
ui_error_t ui_pagination_base_set_config(struct ui_pagination_base *pagination,
                                         size_t total_items, size_t page_size);

/**
 * @brief Gets the total number of pages.
 *
 * @param pagination The pagination instance.
 * @param out_total_pages Pointer to receive the total number of pages.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_pagination_base_get_total_pages(const struct ui_pagination_base *pagination,
                                   size_t *out_total_pages);

/**
 * @brief Sets the current active page index (0-based).
 * Clamps the index if it is out of bounds.
 *
 * @param pagination The pagination instance.
 * @param page_index The 0-based page index.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_pagination_base_set_current_page(struct ui_pagination_base *pagination,
                                    size_t page_index);

/**
 * @brief Gets the current active page index (0-based).
 *
 * @param pagination The pagination instance.
 * @param out_current_page Pointer to receive the current page index.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_pagination_base_get_current_page(const struct ui_pagination_base *pagination,
                                    size_t *out_current_page);

/**
 * @brief Navigates to the next page.
 *
 * @param pagination The pagination instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_pagination_base_next(struct ui_pagination_base *pagination);

/**
 * @brief Navigates to the previous page.
 *
 * @param pagination The pagination instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_pagination_base_previous(struct ui_pagination_base *pagination);

/**
 * @brief Navigates to the first page.
 *
 * @param pagination The pagination instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_pagination_base_first(struct ui_pagination_base *pagination);

/**
 * @brief Navigates to the last page.
 *
 * @param pagination The pagination instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_pagination_base_last(struct ui_pagination_base *pagination);

/**
 * @brief Gets the start and end item indices (0-based, end is exclusive) for
 * the current page.
 *
 * @param pagination The pagination instance.
 * @param out_start_index Pointer to receive the start item index.
 * @param out_end_index Pointer to receive the end item index (exclusive).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_pagination_base_get_bounds(const struct ui_pagination_base *pagination,
                              size_t *out_start_index, size_t *out_end_index);

/**
 * @brief Binds the active state/index to a signal.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_pagination_base_bind_current_page(struct ui_pagination_base *widget,
                                     struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_PAGINATION_BASE_H */
