/**
 * @file ui_transfer_list_base.h
 * @brief Transfer list component base declarations.
 *
 * @defgroup TransferListBase Transfer List Base
 * @brief Base implementation for dual-list transfer selection widgets.
 * @{
 */

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

/**
 * @brief Opaque structure representing a UI component.
 */
struct ui_component;

/**
 * @struct ui_transfer_list_item
 * @brief Node for a transfer list item.
 */
struct ui_transfer_list_item {
  /** @brief Unique identifier for the item. */
  int id;
  /** @brief Opaque data payload. */
  void *data;
  /** @brief Boolean indicating if the item is currently selected. */
  int selected;
  /** @brief Pointer to the next item in the list. */
  struct ui_transfer_list_item *next;
};

/**
 * @struct ui_transfer_list_payload
 * @brief Structure representing the CVA payload for transfer lists.
 */
struct ui_transfer_list_payload {
  /** @brief Linked list of items in the left list. */
  struct ui_transfer_list_item *left_list;
  /** @brief Linked list of items in the right list. */
  struct ui_transfer_list_item *right_list;
};

/**
 * @struct ui_transfer_list_base
 * @brief Represents a Transfer List widget which manages two lists
 * (left/right).
 */
struct ui_transfer_list_base {
  /** @brief The base component. */
  struct ui_component *component;
  /** @brief Linked list of items in the left list. */
  struct ui_transfer_list_item *left_list;
  /** @brief Linked list of items in the right list. */
  struct ui_transfer_list_item *right_list;

  /** @brief Callback for CVA on-change events. */
  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data);
  /** @brief Opaque user data for the on_change callback. */
  void *cva_on_change_user_data;

  /** @brief Callback for CVA on-touched events. */
  ui_error_t (*cva_on_touched)(void *user_data);
  /** @brief Opaque user data for the on_touched callback. */
  void *cva_on_touched_user_data;

  /** @brief Non-zero if the transfer list is disabled. */
  int is_disabled;
};

/**
 * @brief Initializes a base transfer list component.
 *
 * @param list Pointer to the transfer list struct.
 * @param component The UI component to bind to.
 * @param out_cva Optional pointer to receive the CVA interface.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
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
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_transfer_list_base_add_item(struct ui_transfer_list_base *list,
                                          int to_right, int id, void *data);

/**
 * @brief Selects or unselects an item by ID.
 *
 * @param list Pointer to the transfer list struct.
 * @param id The ID of the item.
 * @param selected The selection state to apply (non-zero for selected).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
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
 * @return UI_ERROR_NONE on success, or an appropriate error code.
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
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_transfer_list_base_move_all(struct ui_transfer_list_base *list,
                                          int to_right);

/**
 * @brief Cleans up resources allocated by the transfer list.
 *
 * @param list Pointer to the transfer list struct.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_transfer_list_base_cleanup(struct ui_transfer_list_base *list);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TRANSFER_LIST_BASE_H */

/** @} */
