#ifndef UI_DRAG_DROP_H
#define UI_DRAG_DROP_H

/* clang-format off */
#include "ui_error.h"
#include "ui_event.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief States of a drag and drop operation.
 */
enum ui_drag_state {
  UI_DRAG_STATE_IDLE = 0,
  UI_DRAG_STATE_PENDING, /**< Pointer down, but drag threshold not yet met */
  UI_DRAG_STATE_DRAGGING
};

/**
 * @brief Layout orientation of a list.
 */
enum ui_drag_list_orientation {
  UI_DRAG_LIST_ORIENTATION_VERTICAL = 0,
  UI_DRAG_LIST_ORIENTATION_HORIZONTAL
};

/**
 * @brief Represents the geometric bounds of a draggable item.
 */
struct ui_drag_item {
  int item_id; /**< User-defined identifier for the item */
  int x;       /**< X coordinate of the item's top-left corner */
  int y;       /**< Y coordinate of the item's top-left corner */
  int width;   /**< Width of the item */
  int height;  /**< Height of the item */
};

/**
 * @brief Represents a list/container that can accept dropped items and
 * participate in sorting.
 */
struct ui_drag_list {
  int list_id; /**< User-defined identifier for the list */
  int x;       /**< X coordinate of the list's bounding box */
  int y;       /**< Y coordinate of the list's bounding box */
  int width;   /**< Width of the list's bounding box */
  int height;  /**< Height of the list's bounding box */

  enum ui_drag_list_orientation orientation; /**< Layout orientation */

  struct ui_drag_item *items; /**< Array of items currently in this list */
  int item_count;             /**< Number of items in the list */
};

/**
 * @brief Represents a visual placeholder for the item being dragged.
 */
struct ui_drag_placeholder {
  int active;  /**< 1 if placeholder should be shown, 0 otherwise */
  int list_id; /**< The ID of the list where the placeholder is currently
                  located */
  int index;   /**< The index within the list where the item would be dropped */
  int x;       /**< X coordinate for placeholder rendering */
  int y;       /**< Y coordinate for placeholder rendering */
  int width;   /**< Width of placeholder */
  int height;  /**< Height of placeholder */
};

/**
 * @brief Context for managing drag and drop state across multiple lists.
 */
struct ui_drag_drop_context;

/**
 * @brief Creates a new drag and drop context.
 * @param out_ctx Pointer to receive the context.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_drag_drop_create(struct ui_drag_drop_context **out_ctx);

/**
 * @brief Destroys a drag and drop context.
 * @param ctx The context to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_drag_drop_destroy(struct ui_drag_drop_context *ctx);

/**
 * @brief Configures the pointer distance threshold (in pixels) to differentiate
 * a click from a drag.
 * @param ctx The context.
 * @param distance The distance threshold (e.g., 5).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_drag_drop_set_drag_threshold(struct ui_drag_drop_context *ctx,
                                           int distance);

/**
 * @brief Clears all registered lists. Typically called at the beginning of an
 * update cycle.
 * @param ctx The context.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_drag_drop_clear_lists(struct ui_drag_drop_context *ctx);

/**
 * @brief Registers a list and its items with the drag context for the current
 * frame/update. This should be called with up-to-date layout bounds. The list
 * data is copied internally.
 * @param ctx The context.
 * @param list The list bounds and its items.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_drag_drop_add_list(struct ui_drag_drop_context *ctx,
                                 const struct ui_drag_list *list);

/**
 * @brief Processes an input event.
 * @param ctx The context.
 * @param event The input event to process (mouse, touch).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_drag_drop_process_event(struct ui_drag_drop_context *ctx,
                                      const struct ui_event *event);

/**
 * @brief Retrieves the current state of the drag operation.
 * @param ctx The context.
 * @param out_state Pointer to receive the state.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_drag_drop_get_state(const struct ui_drag_drop_context *ctx,
                                  enum ui_drag_state *out_state);

/**
 * @brief Retrieves information about the item currently being dragged.
 * @param ctx The context.
 * @param out_item_id Pointer to receive the dragged item's ID.
 * @param out_source_list_id Pointer to receive the ID of the list the item
 * originated from.
 * @param out_current_x Pointer to receive the current X coordinate of the
 * dragged item.
 * @param out_current_y Pointer to receive the current Y coordinate of the
 * dragged item.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_STATE if not dragging.
 */
ui_error_t ui_drag_drop_get_dragged_item(const struct ui_drag_drop_context *ctx,
                                         int *out_item_id,
                                         int *out_source_list_id,
                                         int *out_current_x,
                                         int *out_current_y);

/**
 * @brief Retrieves the calculated placeholder position for visual rendering.
 * @param ctx The context.
 * @param out_placeholder Pointer to receive the placeholder data.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_drag_drop_get_placeholder(const struct ui_drag_drop_context *ctx,
                             struct ui_drag_placeholder *out_placeholder);

/**
 * @brief Checks if a drop event just occurred in the last processed event.
 * If so, clears the internal drop event flag.
 * @param ctx The context.
 * @param out_dropped Flag set to 1 if a drop occurred, 0 otherwise.
 * @param out_item_id The item that was dropped.
 * @param out_from_list The originating list ID.
 * @param out_to_list The destination list ID.
 * @param out_to_index The destination index within the destination list.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_drag_drop_get_drop_event(struct ui_drag_drop_context *ctx,
                                       int *out_dropped, int *out_item_id,
                                       int *out_from_list, int *out_to_list,
                                       int *out_to_index);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_DRAG_DROP_H */
