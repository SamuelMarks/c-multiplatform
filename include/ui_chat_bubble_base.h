#ifndef UI_CHAT_BUBBLE_BASE_H
#define UI_CHAT_BUBBLE_BASE_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
#include "ui_signal.h"
#include "ui_component.h"
#include "ui_geometry.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle for the chat bubble base component.
 */
struct ui_chat_bubble_base;

/**
 * @brief Indicates the topological placement of the conversational tail on the
 * bubble.
 */
enum ui_chat_bubble_tail_placement {
  UI_CHAT_BUBBLE_TAIL_NONE =
      0, /**< Bubble without a tail (e.g., middle of a grouped series) */
  UI_CHAT_BUBBLE_TAIL_BOTTOM_LEFT = 1,  /**< Standard incoming message tail */
  UI_CHAT_BUBBLE_TAIL_BOTTOM_RIGHT = 2, /**< Standard outgoing message tail */
  UI_CHAT_BUBBLE_TAIL_TOP_LEFT = 3,     /**< Alternative layout */
  UI_CHAT_BUBBLE_TAIL_TOP_RIGHT = 4     /**< Alternative layout */
};

/**
 * @brief Indicates the position of this bubble within a contiguous group of
 * messages from the same sender. Used by renderers to determine corner radius
 * combinations (e.g., rounding all corners except the one pointing to the
 * tail).
 */
enum ui_chat_bubble_group_position {
  UI_CHAT_BUBBLE_GROUP_SINGLE = 0, /**< Isolated message, not grouped */
  UI_CHAT_BUBBLE_GROUP_FIRST = 1,  /**< First message in a group (top) */
  UI_CHAT_BUBBLE_GROUP_MIDDLE = 2, /**< Middle message in a group */
  UI_CHAT_BUBBLE_GROUP_LAST = 3    /**< Last message in a group (bottom) */
};

/**
 * @brief Configuration defining the topological layout of a single chat bubble.
 */
struct ui_chat_bubble_config {
  enum ui_chat_bubble_tail_placement tail_placement;
  enum ui_chat_bubble_group_position group_position;
};

/**
 * @brief Creates a chat bubble base component.
 *
 * @param arena The memory arena to use for allocation.
 * @param config The structural configuration of the bubble.
 * @param out_bubble Pointer to receive the created component handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_chat_bubble_base_create(struct ui_arena *arena,
                           const struct ui_chat_bubble_config *config,
                           struct ui_chat_bubble_base **out_bubble);

/**
 * @brief Destroys a chat bubble base component.
 *
 * @param bubble The component to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_chat_bubble_base_destroy(struct ui_chat_bubble_base *bubble);

/**
 * @brief Updates the structural configuration of the bubble.
 * Typically invoked by a layout manager when adjacent nodes change (e.g. sender
 * continues typing).
 *
 * @param bubble The component.
 * @param config The new structural configuration.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_chat_bubble_base_set_config(struct ui_chat_bubble_base *bubble,
                               const struct ui_chat_bubble_config *config);

/**
 * @brief Retrieves the signal emitted when the structural configuration
 * changes. The payload is an int representing a bitmask of the new states.
 *
 * @param bubble The component.
 * @param out_signal Pointer to receive the signal handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_chat_bubble_base_get_config_signal(struct ui_chat_bubble_base *bubble,
                                      ui_signal_t **out_signal);

/**
 * @brief A helper to calculate the optimal text wrapping bounds within the
 * bubble's irregular geometry.
 *
 * @param bubble The component.
 * @param raw_bounds The full rectangular bounds of the bubble.
 * @param out_text_bounds Pointer to receive the safe rectangular area for text
 * layout.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_chat_bubble_base_calculate_text_bounds(
    const struct ui_chat_bubble_base *bubble,
    const struct ui_dom_rect *raw_bounds, struct ui_dom_rect *out_text_bounds);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CHAT_BUBBLE_BASE_H */
