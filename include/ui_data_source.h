/**
 * @file ui_data_source.h
 * @brief Abstract data source management and related structures.
 *
 * This header defines the data source API, handling paginated fetching,
 * sorting, filtering, and data binding for UI components like data grids.
 */

/* clang-format off */
#ifndef UI_DATA_SOURCE_H
#define UI_DATA_SOURCE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_error.h"
#include "ui_signal.h"
#include "ui_types.h"

/* clang-format on */

/**
 * @brief Forward declaration for a data source instance.
 */
struct ui_data_source;

/**
 * @brief Sort direction for data source columns.
 */
enum ui_sort_direction {
  UI_SORT_ASCENDING = 0,  /**< Ascending sort direction. */
  UI_SORT_DESCENDING = 1, /**< Descending sort direction. */
  UI_SORT_NONE = 2        /**< No sort direction. */
};

/**
 * @brief Represents a sort descriptor.
 */
struct ui_sort_descriptor {
  const char *column_id;            /**< The ID of the column to sort by. */
  enum ui_sort_direction direction; /**< The sort direction. */
};

/**
 * @brief Represents a filter descriptor.
 */
struct ui_filter_descriptor {
  const char *column_id; /**< The ID of the column to filter. */
  const char *query;     /**< The filter query string. */
};

/**
 * @brief State structure for the data source.
 */
struct ui_data_source_state {
  ui_uint32 total_items;  /**< Total number of items available. */
  ui_uint32 loaded_items; /**< Number of items currently loaded. */
  ui_bool_t is_loading;   /**< 1 if a fetch is in progress, 0 otherwise. */
  ui_error_t last_error;  /**< The last error encountered, if any. */
};

/**
 * @brief Callback for fetching a page of data.
 *
 * @param ds The data source.
 * @param offset The starting offset index.
 * @param limit The maximum number of items to fetch.
 * @param user_data Opaque user data.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_data_source_fetch_page_fn)(struct ui_data_source *ds,
                                                   ui_uint32 offset,
                                                   ui_uint32 limit,
                                                   void *user_data);

/**
 * @brief Callback for applying sorts.
 *
 * @param ds The data source.
 * @param sorts An array of sort descriptors.
 * @param num_sorts The number of sort descriptors in the array.
 * @param user_data Opaque user data.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_data_source_apply_sort_fn)(
    struct ui_data_source *ds, const struct ui_sort_descriptor *sorts,
    ui_uint32 num_sorts, void *user_data);

/**
 * @brief Callback for applying filters.
 *
 * @param ds The data source.
 * @param filters An array of filter descriptors.
 * @param num_filters The number of filter descriptors in the array.
 * @param user_data Opaque user data.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_data_source_apply_filter_fn)(
    struct ui_data_source *ds, const struct ui_filter_descriptor *filters,
    ui_uint32 num_filters, void *user_data);

/**
 * @brief Creates a new abstract data source.
 *
 * @param out_ds Pointer to store the created data source.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_data_source_create(struct ui_data_source **out_ds);

/**
 * @brief Destroys the data source.
 *
 * @param ds Pointer to the data source to destroy.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_data_source_destroy(struct ui_data_source *ds);

/**
 * @brief Sets the callback for fetching a page.
 *
 * @param ds Pointer to the data source.
 * @param fetch_page The callback function.
 * @param user_data Opaque user data for the callback.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t
ui_data_source_set_fetch_page_callback(struct ui_data_source *ds,
                                       ui_data_source_fetch_page_fn fetch_page,
                                       void *user_data);

/**
 * @brief Sets the callback for sorting.
 *
 * @param ds Pointer to the data source.
 * @param apply_sort The callback function.
 * @param user_data Opaque user data for the callback.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t
ui_data_source_set_apply_sort_callback(struct ui_data_source *ds,
                                       ui_data_source_apply_sort_fn apply_sort,
                                       void *user_data);

/**
 * @brief Sets the callback for filtering.
 *
 * @param ds Pointer to the data source.
 * @param apply_filter The callback function.
 * @param user_data Opaque user data for the callback.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_data_source_set_apply_filter_callback(
    struct ui_data_source *ds, ui_data_source_apply_filter_fn apply_filter,
    void *user_data);

/**
 * @brief Fetches a page of data via the registered callback.
 *
 * @param ds Pointer to the data source.
 * @param offset The starting offset index.
 * @param limit The maximum number of items to fetch.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_data_source_fetch_page(struct ui_data_source *ds,
                                     ui_uint32 offset, ui_uint32 limit);

/**
 * @brief Applies sorts to the data via the registered callback.
 *
 * @param ds Pointer to the data source.
 * @param sorts Array of sort descriptors.
 * @param num_sorts Number of sort descriptors in the array.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_data_source_apply_sort(struct ui_data_source *ds,
                                     const struct ui_sort_descriptor *sorts,
                                     ui_uint32 num_sorts);

/**
 * @brief Applies filters to the data via the registered callback.
 *
 * @param ds Pointer to the data source.
 * @param filters Array of filter descriptors.
 * @param num_filters Number of filter descriptors in the array.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t
ui_data_source_apply_filter(struct ui_data_source *ds,
                            const struct ui_filter_descriptor *filters,
                            ui_uint32 num_filters);

/**
 * @brief Binds the data source state to a signal.
 *
 * @param ds Pointer to the data source.
 * @param state_signal The signal to bind to. Payload should be treated as a
 * pointer to `ui_data_source_state`.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_data_source_bind_state(struct ui_data_source *ds,
                                     struct ui_signal *state_signal);

/**
 * @brief Binds the data change notification to a signal.
 *
 * @param ds Pointer to the data source.
 * @param data_signal The signal to bind to.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_data_source_bind_data(struct ui_data_source *ds,
                                    struct ui_signal *data_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_DATA_SOURCE_H */
