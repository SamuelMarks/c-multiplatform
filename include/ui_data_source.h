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

struct ui_data_source;

/**
 * @brief Sort direction for data source columns.
 */
enum ui_sort_direction {
  UI_SORT_ASCENDING = 0,
  UI_SORT_DESCENDING = 1,
  UI_SORT_NONE = 2
};

/**
 * @brief Represents a sort descriptor.
 */
struct ui_sort_descriptor {
  const char *column_id;
  enum ui_sort_direction direction;
};

/**
 * @brief Represents a filter descriptor.
 */
struct ui_filter_descriptor {
  const char *column_id;
  const char *query;
};

/**
 * @brief State structure for the data source.
 */
struct ui_data_source_state {
  ui_uint32 total_items;
  ui_uint32 loaded_items;
  ui_bool_t is_loading;
  enum ui_error last_error;
};

/**
 * @brief Callback for fetching a page of data.
 */
typedef enum ui_error (*ui_data_source_fetch_page_fn)(struct ui_data_source *ds,
                                                      ui_uint32 offset,
                                                      ui_uint32 limit,
                                                      void *user_data);

/**
 * @brief Callback for applying sorts.
 */
typedef enum ui_error (*ui_data_source_apply_sort_fn)(
    struct ui_data_source *ds, const struct ui_sort_descriptor *sorts,
    ui_uint32 num_sorts, void *user_data);

/**
 * @brief Callback for applying filters.
 */
typedef enum ui_error (*ui_data_source_apply_filter_fn)(
    struct ui_data_source *ds, const struct ui_filter_descriptor *filters,
    ui_uint32 num_filters, void *user_data);

/**
 * @brief Creates a new abstract data source.
 *
 * @param out_ds Pointer to store the created data source.
 * @return enum ui_error
 */
enum ui_error ui_data_source_create(struct ui_data_source **out_ds);

/**
 * @brief Destroys the data source.
 *
 * @param ds The data source.
 * @return enum ui_error
 */
enum ui_error ui_data_source_destroy(struct ui_data_source *ds);

/**
 * @brief Sets the callback for fetching a page.
 *
 * @param ds The data source.
 * @param fetch_page The callback function.
 * @param user_data Opaque user data for the callback.
 * @return enum ui_error
 */
enum ui_error
ui_data_source_set_fetch_page_callback(struct ui_data_source *ds,
                                       ui_data_source_fetch_page_fn fetch_page,
                                       void *user_data);

/**
 * @brief Sets the callback for sorting.
 *
 * @param ds The data source.
 * @param apply_sort The callback function.
 * @param user_data Opaque user data for the callback.
 * @return enum ui_error
 */
enum ui_error
ui_data_source_set_apply_sort_callback(struct ui_data_source *ds,
                                       ui_data_source_apply_sort_fn apply_sort,
                                       void *user_data);

/**
 * @brief Sets the callback for filtering.
 *
 * @param ds The data source.
 * @param apply_filter The callback function.
 * @param user_data Opaque user data for the callback.
 * @return enum ui_error
 */
enum ui_error ui_data_source_set_apply_filter_callback(
    struct ui_data_source *ds, ui_data_source_apply_filter_fn apply_filter,
    void *user_data);

/**
 * @brief Fetches a page of data via the registered callback.
 *
 * @param ds The data source.
 * @param offset The starting offset.
 * @param limit The number of items to fetch.
 * @return enum ui_error
 */
enum ui_error ui_data_source_fetch_page(struct ui_data_source *ds,
                                        ui_uint32 offset, ui_uint32 limit);

/**
 * @brief Applies sorts to the data via the registered callback.
 *
 * @param ds The data source.
 * @param sorts Array of sort descriptors.
 * @param num_sorts Number of sort descriptors.
 * @return enum ui_error
 */
enum ui_error ui_data_source_apply_sort(struct ui_data_source *ds,
                                        const struct ui_sort_descriptor *sorts,
                                        ui_uint32 num_sorts);

/**
 * @brief Applies filters to the data via the registered callback.
 *
 * @param ds The data source.
 * @param filters Array of filter descriptors.
 * @param num_filters Number of filter descriptors.
 * @return enum ui_error
 */
enum ui_error
ui_data_source_apply_filter(struct ui_data_source *ds,
                            const struct ui_filter_descriptor *filters,
                            ui_uint32 num_filters);

/**
 * @brief Binds the data source state to a signal.
 *
 * @param ds The data source.
 * @param state_signal The signal to bind to. Payload should be treated as a
 * pointer to ui_data_source_state.
 * @return enum ui_error
 */
enum ui_error ui_data_source_bind_state(struct ui_data_source *ds,
                                        struct ui_signal *state_signal);

/**
 * @brief Binds the data change notification to a signal.
 *
 * @param ds The data source.
 * @param data_signal The signal to bind to.
 * @return enum ui_error
 */
enum ui_error ui_data_source_bind_data(struct ui_data_source *ds,
                                       struct ui_signal *data_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_DATA_SOURCE_H */
