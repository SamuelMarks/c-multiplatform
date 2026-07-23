#ifndef UI_SELECTION_MODEL_H
#define UI_SELECTION_MODEL_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

struct ui_selection_model;

/**
 * @brief Creates a new selection model.
 *
 * @param out_model Pointer to receive the allocated selection model.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_selection_model_create(struct ui_selection_model **out_model);

/**
 * @brief Destroys a selection model.
 *
 * @param model The selection model.
 */
void ui_selection_model_destroy(struct ui_selection_model *model);

/**
 * @brief Sets whether the selection model allows multiple selections.
 *
 * @param model The selection model.
 * @param is_multi 1 for multi-select, 0 for single-select.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_selection_model_set_multi_select(struct ui_selection_model *model,
                                    int is_multi);

/**
 * @brief Selects an item by ID/index.
 *
 * @param model The selection model.
 * @param id The item ID.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_selection_model_select(struct ui_selection_model *model,
                                        void *id);

/**
 * @brief Deselects an item by ID/index.
 *
 * @param model The selection model.
 * @param id The item ID.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_selection_model_deselect(struct ui_selection_model *model,
                                          void *id);

/**
 * @brief Toggles the selection state of an item.
 *
 * @param model The selection model.
 * @param id The item ID.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_selection_model_toggle(struct ui_selection_model *model,
                                        void *id);

/**
 * @brief Clears all selections.
 *
 * @param model The selection model.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_selection_model_clear(struct ui_selection_model *model);

/**
 * @brief Selects all items from an array of IDs.
 * Note: If the model is single-select, this will only select the last item (or
 * fail, depending on semantics). We will implement it to select all if
 * multi-select, or do nothing/fail if single-select.
 *
 * @param model The selection model.
 * @param ids Array of item IDs.
 * @param count Number of items in the array.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_selection_model_select_all(struct ui_selection_model *model,
                                            void **ids, int count);

/**
 * @brief Checks if an item is selected.
 *
 * @param model The selection model.
 * @param id The item ID.
 * @param out_is_selected Pointer to receive 1 if selected, 0 otherwise.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_selection_model_is_selected(const struct ui_selection_model *model, void *id,
                               int *out_is_selected);

/**
 * @brief Gets the number of selected items.
 *
 * @param model The selection model.
 * @param out_count Pointer to receive the count.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_selection_model_get_selected_count(const struct ui_selection_model *model,
                                      int *out_count);

/**
 * @brief Gets the selected item IDs.
 *
 * @param model The selection model.
 * @param out_ids Pointer to receive the array of selected IDs.
 * @param capacity The maximum number of IDs to write to out_ids.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_selection_model_get_selected(const struct ui_selection_model *model,
                                void **out_ids, int capacity);

/**
 * @brief Callback invoked when the selection changes.
 *
 * @param model The selection model.
 * @param user_data Opaque user data.
 */
typedef enum ui_error (*ui_selection_model_on_change_t)(
    struct ui_selection_model *model, void *user_data);

/**
 * @brief Sets a callback to be invoked when the selection changes.
 *
 * @param model The selection model.
 * @param callback The callback.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_selection_model_set_on_change(struct ui_selection_model *model,
                                 ui_selection_model_on_change_t callback,
                                 void *user_data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SELECTION_MODEL_H */
