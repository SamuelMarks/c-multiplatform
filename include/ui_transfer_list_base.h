#ifndef UI_TRANSFER_LIST_BASE_H
#define UI_TRANSFER_LIST_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_control_value_accessor.h"
#include <stddef.h>
/* clang-format on */

struct ui_component;

/**
 * @brief Node for a transfer list item.
 */
struct ui_transfer_list_item {
  int id;
  void *data;
  int selected;
  struct ui_transfer_list_item *next;
};

/**
 * @brief Structure representing the CVA payload for transfer lists.
 */
struct ui_transfer_list_payload {
  struct ui_transfer_list_item *left_list;
  struct ui_transfer_list_item *right_list;
};

/**
 * @brief Represents a Transfer List widget which manages two lists
 * (left/right).
 */
struct ui_transfer_list_base {
  struct ui_component *component;
  struct ui_transfer_list_item *left_list;
  struct ui_transfer_list_item *right_list;

  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data);
  void *cva_on_change_user_data;

  ui_error_t (*cva_on_touched)(void *user_data);
  void *cva_on_touched_user_data;

  int is_disabled;
};

/**
 * @brief Initializes a base transfer list component.
 *
 * @param list Pointer to the transfer list struct.
 * @param component The UI component to bind to.
 * @param out_cva Optional pointer to receive the CVA interface.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_transfer_list_base_init(struct ui_transfer_list_base *list,
                           struct ui_component *component,
                           struct ui_control_value_accessor *out_cva);

/**
 * @brief Adds an item to one of the lists.
 *
 * @param list Pointer to the transfer list struct.
 * @param to_right If 1, adds to the right list; otherwise to the left.
 * @param id The unique identifier for the item.
 * @param data User data associated with the item.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_transfer_list_base_add_item(struct ui_transfer_list_base *list,
                                          int to_right, int id, void *data);

/**
 * @brief Selects or unselects an item by ID.
 *
 * @param list Pointer to the transfer list struct.
 * @param id The ID of the item.
 * @param selected The selection state to apply.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_transfer_list_base_set_selected(struct ui_transfer_list_base *list, int id,
                                   int selected);

/**
 * @brief Moves selected items between lists.
 *
 * @param list Pointer to the transfer list struct.
 * @param to_right If 1, moves selected left items to right; otherwise right
 * to left.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_transfer_list_base_move_selected(struct ui_transfer_list_base *list,
                                    int to_right);

/**
 * @brief Moves all items between lists regardless of selection state.
 *
 * @param list Pointer to the transfer list struct.
 * @param to_right If 1, moves all left items to right; otherwise right to
 * left.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_transfer_list_base_move_all(struct ui_transfer_list_base *list,
                                          int to_right);

/**
 * @brief Cleans up resources allocated by the transfer list.
 *
 * @param list Pointer to the transfer list struct.
 */
ui_error_t ui_transfer_list_base_cleanup(struct ui_transfer_list_base *list);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TRANSFER_LIST_BASE_H */
