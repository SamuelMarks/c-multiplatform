#ifndef UI_LISTBOX_BASE_H
#define UI_LISTBOX_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_event.h"
#include "ui_selection_model.h"
#include "ui_control_value_accessor.h"
/* clang-format on */

struct ui_listbox_base;

/**
 * @brief Callback invoked to get the text of an item for typeahead search.
 *
 * @param listbox The listbox component.
 * @param index The item index.
 * @param user_data Opaque user data.
 * @return The text string of the item, or NULL. The string must remain valid
 * until the callback returns.
 */
typedef const char *(*ui_listbox_get_item_text_t)(
    struct ui_listbox_base *listbox, int index, void *user_data);

/**
 * @brief Creates a new unstyled listbox base component.
 *
 * @param out_listbox Pointer to receive the allocated listbox base.
 * @param out_cva Optional pointer to receive the CVA interface.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_listbox_base_create(struct ui_listbox_base **out_listbox,
                                  struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a listbox base component.
 *
 * @param listbox The listbox component to destroy.
 */
ui_error_t ui_listbox_base_destroy(struct ui_listbox_base *listbox);

/**
 * @brief Retrieves the underlying UI component.
 *
 * @param listbox The listbox.
 * @return The component, or NULL.
 */
ui_error_t ui_listbox_base_get_component(struct ui_listbox_base *listbox,
                                         struct ui_component **out_component);

/**
 * @brief Gets the selection model attached to this listbox.
 *
 * @param listbox The listbox.
 * @return The selection model, or NULL.
 */
ui_error_t
ui_listbox_base_get_selection_model(struct ui_listbox_base *listbox,
                                    struct ui_selection_model **out_model);

/**
 * @brief Sets whether the listbox allows multiple items to be selected.
 *
 * @param listbox The listbox.
 * @param is_multi 1 for multi-select, 0 for single-select.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_listbox_base_set_multi_select(struct ui_listbox_base *listbox,
                                            int is_multi);

/**
 * @brief Sets the number of items in the listbox.
 * Resets active index if it falls out of bounds.
 *
 * @param listbox The listbox.
 * @param num_items The total number of items.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_listbox_base_set_item_count(struct ui_listbox_base *listbox,
                                          int num_items);

/**
 * @brief Sets the text provider callback used for typeahead search.
 *
 * @param listbox The listbox.
 * @param provider The text provider callback.
 * @param user_data Opaque data passed to the callback.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_listbox_base_set_item_text_provider(struct ui_listbox_base *listbox,
                                       ui_listbox_get_item_text_t provider,
                                       void *user_data);

/**
 * @brief Sets the active (focused) item index.
 *
 * @param listbox The listbox.
 * @param index The index, or -1 for none.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_listbox_base_set_active_index(struct ui_listbox_base *listbox,
                                            int index);

/**
 * @brief Gets the active (focused) item index.
 *
 * @param listbox The listbox.
 * @return The index, or -1 if none.
 */
ui_error_t
ui_listbox_base_get_active_index(const struct ui_listbox_base *listbox,
                                 int *out_index);

/**
 * @brief Processes an input event for keyboard navigation and selection.
 *
 * @param listbox The listbox.
 * @param event The event.
 * @param timestamp_ms Current time in milliseconds (used for typeahead
 * timeout).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_listbox_base_process_event(struct ui_listbox_base *listbox,
                                         const struct ui_event *event,
                                         double timestamp_ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_LISTBOX_BASE_H */
