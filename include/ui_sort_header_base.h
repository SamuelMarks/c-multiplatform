#ifndef UI_SORT_HEADER_BASE_H
#define UI_SORT_HEADER_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @struct ui_sort_header_base
 * @brief Opaque handle for a sort header base manager.
 */
struct ui_sort_header_base;

/**
 * @enum ui_sort_direction
 * @brief Represents the tri-state sort direction.
 */
enum ui_sort_direction {
  /** @brief No active sorting. */
  UI_SORT_NONE,
  /** @brief Ascending sort order. */
  UI_SORT_ASCENDING,
  /** @brief Descending sort order. */
  UI_SORT_DESCENDING
};

/**
 * @struct ui_sort_state
 * @brief Represents a sort configuration for a specific column/id.
 */
struct ui_sort_state {
  /** @brief Opaque identifier for the column/header. */
  void *id;
  /** @brief The current sort direction. */
  enum ui_sort_direction direction;
};

/**
 * @brief Creates a new sort header base manager.
 *
 * @param out_sort_header Pointer to receive the allocated sort header base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_sort_header_base_create(struct ui_sort_header_base **out_sort_header);

/**
 * @brief Destroys a sort header base manager.
 *
 * @param sort_header The sort header manager.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_sort_header_base_destroy(struct ui_sort_header_base *sort_header);

/**
 * @brief Sets whether multi-column sorting is enabled.
 * If disabled (default), setting a new active sort clears the previous ones.
 *
 * @param sort_header The sort header manager.
 * @param is_multi 1 for multi-column sort, 0 for single-column sort.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_sort_header_base_set_multi_sort(struct ui_sort_header_base *sort_header,
                                   int is_multi);

/**
 * @brief Toggles the sort direction for a given ID (column).
 * Transitions: NONE -> ASCENDING -> DESCENDING -> NONE.
 * If multi-sort is disabled, other sorted columns will be reset to NONE.
 *
 * @param sort_header The sort header manager.
 * @param id The identifier for the column/header being sorted.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_sort_header_base_toggle(struct ui_sort_header_base *sort_header,
                                      void *id);

/**
 * @brief Explicitly sets the sort direction for a given ID.
 *
 * @param sort_header The sort header manager.
 * @param id The identifier for the column/header.
 * @param direction The desired sort direction.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_sort_header_base_set_direction(struct ui_sort_header_base *sort_header,
                                  void *id, enum ui_sort_direction direction);

/**
 * @brief Gets the current sort direction for a given ID.
 *
 * @param sort_header The sort header manager.
 * @param id The identifier for the column/header.
 * @param out_direction Pointer to receive the sort direction. Will be
 * UI_SORT_NONE if not found.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_sort_header_base_get_direction(const struct ui_sort_header_base *sort_header,
                                  void *id,
                                  enum ui_sort_direction *out_direction);

/**
 * @brief Retrieves the active sort states in order of prioritization.
 * For single sort, this will return at most 1 item.
 * For multi-sort, the array will be ordered by the most recently toggled/set.
 *
 * @param sort_header The sort header manager.
 * @param out_states Array to be filled with the sort states.
 * @param capacity Maximum number of states the out_states array can hold.
 * @param out_count Pointer to receive the actual number of populated states.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_sort_header_base_get_active_sorts(
    const struct ui_sort_header_base *sort_header,
    struct ui_sort_state *out_states, size_t capacity, size_t *out_count);

/**
 * @brief Clears all active sorts, resetting them to NONE.
 *
 * @param sort_header The sort header manager.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_sort_header_base_clear(struct ui_sort_header_base *sort_header);

/**
 * @page ui_sort_table_integration Integration Flow with ui_table_base
 *
 * To integrate `ui_sort_header_base` with a data presentation component like
 * `ui_table_base`:
 *
 * 1. **Instantiation:**
 *    Create a `struct ui_sort_header_base` instance during the
 * `ui_table_base_create` process.
 *
 * 2. **Handling Header Clicks:**
 *    When a user clicks on a table column header (handled via `ui_event` inside
 * `ui_table_base`), the table invokes `ui_sort_header_base_toggle(sort_manager,
 * (void*)column_index)`. The `sort_manager` will automatically handle the
 * Tri-state transition (`NONE` -> `ASC` -> `DESC` -> `NONE`) and manage
 * single-column vs multi-column prioritization (if `set_multi_sort(1)` was
 * called).
 *
 * 3. **Applying Sort Visuals:**
 *    During `ui_table_base_render`, the table queries the `sort_manager` using
 *    `ui_sort_header_base_get_direction(sort_manager, (void*)column_index,
 * &dir)`. Depending on the `dir` returned, the table appends appropriate
 * `aria-sort` attributes (e.g., `aria-sort="ascending"`) to the column header's
 * DOM node.
 *
 * 4. **Coordinating Data Reordering:**
 *    The `ui_table_base` itself is headless and should not directly sort the
 * user's data array. Instead, the application provides a
 * `ui_table_on_sort_change` callback (or polls `get_active_sorts()`). When the
 * application detects a change in `ui_sort_header_base_get_active_sorts()`, the
 * application re-orders the backing dataset and then calls a method like
 * `ui_table_base_refresh()` to redraw.
 */

/**
 * @brief Binds the direction property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_sort_header_base_bind_direction(struct ui_sort_header_base *widget,
                                   struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SORT_HEADER_BASE_H */
