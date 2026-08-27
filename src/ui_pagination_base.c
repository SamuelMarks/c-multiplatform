/*
 * \file ui_pagination_base.c
 * \brief Implementation of pagination logic state management.
 */
/* clang-format off */
#include "ui_pagination_base.h"
#include "ui_internal_mem.h"
/* clang-format on */

/**
 * @struct ui_pagination_base
 * \struct ui_pagination_base
 * \brief Core state for pagination, including current page and total items.
 */
struct ui_pagination_base {
  size_t total_items;                    /**< total_items */
  size_t page_size;                      /**< page_size */
  size_t current_page;                   /**< current_page */
  struct ui_signal *current_page_signal; /**< current_page_signal */
};

/*
 * \brief Creates a new pagination base context.
 * \param[out] out_pagination Pointer to store the created context.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_pagination_base_create(struct ui_pagination_base **out_pagination) {
  struct ui_pagination_base *p;

  if (!out_pagination) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  p = (struct ui_pagination_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_pagination_base));
  if (!p) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  p->total_items = 0;
  p->page_size = 10; /* Default safe value */
  p->current_page = 0;

  *out_pagination = p;
  return UI_ERROR_NONE;
}

/*
 * \brief Destroys a pagination base context.
 * \param[in,out] pagination The context to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_pagination_base_destroy(struct ui_pagination_base *pagination) {
  if (!pagination) {
    return UI_ERROR_NONE;
  }
  C_MULTIPLATFORM_FREE(pagination);
  return UI_ERROR_NONE;
}

/*
 * \brief Configures the total items and page size for pagination.
 * \param[in,out] pagination The pagination context.
 * \param[in] total_items Total number of items.
 * \param[in] page_size Number of items per page.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_pagination_base_set_config(struct ui_pagination_base *pagination,
                                         size_t total_items, size_t page_size) {
  size_t total_pages;

  if (!pagination || page_size == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  pagination->total_items = total_items;
  pagination->page_size = page_size;

  total_pages = (total_items + page_size - 1) / page_size;
  if (total_pages == 0) {
    pagination->current_page = 0;
  } else if (pagination->current_page >= total_pages) {
    pagination->current_page = total_pages - 1;
  }

  return UI_ERROR_NONE;
}

/*
 * \brief Computes the total number of pages based on config.
 * \param[in] pagination The pagination context.
 * \param[out] out_total_pages Pointer to store the calculated total pages.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_pagination_base_get_total_pages(const struct ui_pagination_base *pagination,
                                   size_t *out_total_pages) {
  if (!pagination || !out_total_pages) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (pagination->total_items == 0) {
    *out_total_pages = 0;
  } else {
    *out_total_pages = (pagination->total_items + pagination->page_size - 1) /
                       pagination->page_size;
  }

  return UI_ERROR_NONE;
}

/*
 * \brief Sets the current active page index (0-based).
 * \param[in,out] pagination The pagination context.
 * \param[in] page_index The requested page index.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_pagination_base_set_current_page(struct ui_pagination_base *pagination,
                                    size_t page_index) {
  size_t total_pages;

  if (!pagination) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (pagination->total_items == 0) {
    pagination->current_page = 0;
    return UI_ERROR_NONE;
  }

  total_pages = (pagination->total_items + pagination->page_size - 1) /
                pagination->page_size;

  if (page_index >= total_pages) {
    pagination->current_page = total_pages - 1;
  } else {
    pagination->current_page = page_index;
  }

  return UI_ERROR_NONE;
}

/*
 * \brief Retrieves the current active page index.
 * \param[in] pagination The pagination context.
 * \param[out] out_current_page Pointer to store the current page index.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_pagination_base_get_current_page(const struct ui_pagination_base *pagination,
                                    size_t *out_current_page) {
  if (!pagination || !out_current_page) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_current_page = pagination->current_page;
  return UI_ERROR_NONE;
}

/*
 * \brief Advances to the next page if possible.
 * \param[in,out] pagination The pagination context.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_pagination_base_next(struct ui_pagination_base *pagination) {
  size_t total_pages;

  if (!pagination) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (pagination->total_items == 0) {
    return UI_ERROR_NONE;
  }

  total_pages = (pagination->total_items + pagination->page_size - 1) /
                pagination->page_size;

  if (pagination->current_page + 1 < total_pages) {
    pagination->current_page++;
  }

  return UI_ERROR_NONE;
}

/*
 * \brief Retreats to the previous page if possible.
 * \param[in,out] pagination The pagination context.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_pagination_base_previous(struct ui_pagination_base *pagination) {
  if (!pagination) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (pagination->current_page > 0) {
    pagination->current_page--;
  }

  return UI_ERROR_NONE;
}

/*
 * \brief Jumps to the first page.
 * \param[in,out] pagination The pagination context.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_pagination_base_first(struct ui_pagination_base *pagination) {
  if (!pagination) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  pagination->current_page = 0;
  return UI_ERROR_NONE;
}

/*
 * \brief Jumps to the last page.
 * \param[in,out] pagination The pagination context.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_pagination_base_last(struct ui_pagination_base *pagination) {
  size_t total_pages;

  if (!pagination) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (pagination->total_items == 0) {
    pagination->current_page = 0;
    return UI_ERROR_NONE;
  }

  total_pages = (pagination->total_items + pagination->page_size - 1) /
                pagination->page_size;
  pagination->current_page = total_pages - 1;

  return UI_ERROR_NONE;
}

/*
 * \brief Calculates the item index bounds for the current page.
 * \param[in] pagination The pagination context.
 * \param[out] out_start_index Pointer to store the start item index.
 * \param[out] out_end_index Pointer to store the end item index (exclusive).
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_pagination_base_get_bounds(const struct ui_pagination_base *pagination,
                              size_t *out_start_index, size_t *out_end_index) {
  size_t start, end;

  if (!pagination || !out_start_index || !out_end_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (pagination->total_items == 0) {
    *out_start_index = 0;
    *out_end_index = 0;
    return UI_ERROR_NONE;
  }

  start = pagination->current_page * pagination->page_size;
  end = start + pagination->page_size;

  if (end > pagination->total_items) {
    end = pagination->total_items;
  }

  *out_start_index = start;
  *out_end_index = end;

  return UI_ERROR_NONE;
}

/*
 * \brief Binds the current page index to a reactive signal.
 * \param[in,out] widget The pagination context.
 * \param[in,out] signal The signal to bind.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_pagination_base_bind_current_page(struct ui_pagination_base *widget,
                                     struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->current_page_signal = signal;
  return UI_ERROR_NONE;
}
