/* clang-format off */
#include "ui_pagination_base.h"
#include "ui_internal_mem.h"
/* clang-format on */

struct ui_pagination_base {
  size_t total_items;
  size_t page_size;
  size_t current_page;
  struct ui_signal *current_page_signal;
};

/** \brief ui_error */
enum ui_error
ui_pagination_base_create(struct ui_pagination_base **out_pagination) {
  struct ui_pagination_base *p;

  if (!out_pagination) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  p = (struct ui_pagination_base *)UI_MALLOC(sizeof(struct ui_pagination_base));
  if (!p) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  p->total_items = 0;
  p->page_size = 10; /* Default safe value */
  p->current_page = 0;

  *out_pagination = p;
  return UI_ERROR_NONE;
}

void ui_pagination_base_destroy(struct ui_pagination_base *pagination) {
  if (!pagination) {
    return;
  }
  UI_FREE(pagination);
}

/** \brief ui_error */
enum ui_error
ui_pagination_base_set_config(struct ui_pagination_base *pagination,
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

/** \brief ui_error */
enum ui_error
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

/** \brief ui_error */
enum ui_error
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

/** \brief ui_error */
enum ui_error
ui_pagination_base_get_current_page(const struct ui_pagination_base *pagination,
                                    size_t *out_current_page) {
  if (!pagination || !out_current_page) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_current_page = pagination->current_page;
  return UI_ERROR_NONE;
}

enum ui_error ui_pagination_base_next(struct ui_pagination_base *pagination) {
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

/** \brief ui_error */
enum ui_error
ui_pagination_base_previous(struct ui_pagination_base *pagination) {
  if (!pagination) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (pagination->current_page > 0) {
    pagination->current_page--;
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_pagination_base_first(struct ui_pagination_base *pagination) {
  if (!pagination) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  pagination->current_page = 0;
  return UI_ERROR_NONE;
}

enum ui_error ui_pagination_base_last(struct ui_pagination_base *pagination) {
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

/** \brief ui_error */
enum ui_error
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

/** \brief ui_error */
enum ui_error
ui_pagination_base_bind_current_page(struct ui_pagination_base *widget,
                                     struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->current_page_signal = signal;
  return UI_ERROR_NONE;
}
