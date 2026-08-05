/* clang-format off */
#include "ui_data_source.h"

#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

struct ui_data_source {
  ui_data_source_fetch_page_fn fetch_page;
  void *fetch_page_user_data;
  ui_data_source_apply_sort_fn apply_sort;
  void *apply_sort_user_data;
  ui_data_source_apply_filter_fn apply_filter;
  void *apply_filter_user_data;

  struct ui_signal *state_signal;
  struct ui_signal *data_signal;
};

ui_error_t ui_data_source_create(struct ui_data_source **out_ds) {
  struct ui_data_source *ds;

  if (!out_ds) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ds = (struct ui_data_source *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_data_source));
  if (!ds) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  ds->fetch_page = NULL;
  ds->fetch_page_user_data = NULL;
  ds->apply_sort = NULL;
  ds->apply_sort_user_data = NULL;
  ds->apply_filter = NULL;
  ds->apply_filter_user_data = NULL;
  ds->state_signal = NULL;
  ds->data_signal = NULL;

  *out_ds = ds;
  return UI_ERROR_NONE;
}

ui_error_t ui_data_source_destroy(struct ui_data_source *ds) {
  if (!ds) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(ds);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_data_source_set_fetch_page_callback(struct ui_data_source *ds,
                                       ui_data_source_fetch_page_fn fetch_page,
                                       void *user_data) {
  if (!ds) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ds->fetch_page = fetch_page;
  ds->fetch_page_user_data = user_data;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_data_source_set_apply_sort_callback(struct ui_data_source *ds,
                                       ui_data_source_apply_sort_fn apply_sort,
                                       void *user_data) {
  if (!ds) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ds->apply_sort = apply_sort;
  ds->apply_sort_user_data = user_data;
  return UI_ERROR_NONE;
}

/** \brief ui_data_source_set_apply_filter_callback */
ui_error_t ui_data_source_set_apply_filter_callback(
    struct ui_data_source *ds, ui_data_source_apply_filter_fn apply_filter,
    void *user_data) {
  if (!ds) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ds->apply_filter = apply_filter;
  ds->apply_filter_user_data = user_data;
  return UI_ERROR_NONE;
}

ui_error_t ui_data_source_fetch_page(struct ui_data_source *ds,
                                     ui_uint32 offset, ui_uint32 limit) {
  ui_error_t rc;
  if (!ds) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!ds->fetch_page) {
    return UI_ERROR_UNSUPPORTED;
  }
  rc = ds->fetch_page(ds, offset, limit, ds->fetch_page_user_data);
  if (rc != UI_ERROR_NONE)
    return rc;
  return UI_ERROR_NONE;
}

ui_error_t ui_data_source_apply_sort(struct ui_data_source *ds,
                                     const struct ui_sort_descriptor *sorts,
                                     ui_uint32 num_sorts) {
  ui_error_t rc;
  if (!ds) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!ds->apply_sort) {
    return UI_ERROR_UNSUPPORTED;
  }
  rc = ds->apply_sort(ds, sorts, num_sorts, ds->apply_sort_user_data);
  if (rc != UI_ERROR_NONE)
    return rc;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_data_source_apply_filter(struct ui_data_source *ds,
                            const struct ui_filter_descriptor *filters,
                            ui_uint32 num_filters) {
  ui_error_t rc;
  if (!ds) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!ds->apply_filter) {
    return UI_ERROR_UNSUPPORTED;
  }
  rc = ds->apply_filter(ds, filters, num_filters, ds->apply_filter_user_data);
  if (rc != UI_ERROR_NONE)
    return rc;
  return UI_ERROR_NONE;
}

ui_error_t ui_data_source_bind_state(struct ui_data_source *ds,
                                     struct ui_signal *state_signal) {
  if (!ds || !state_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ds->state_signal = state_signal;
  return UI_ERROR_NONE;
}

ui_error_t ui_data_source_bind_data(struct ui_data_source *ds,
                                    struct ui_signal *data_signal) {
  if (!ds || !data_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ds->data_signal = data_signal;
  return UI_ERROR_NONE;
}
