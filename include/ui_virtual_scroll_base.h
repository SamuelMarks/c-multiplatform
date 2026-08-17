/**
 * @file ui_virtual_scroll_base.h
 * @brief Virtual scroll base component for efficient long lists.
 */
#ifndef UI_VIRTUAL_SCROLL_BASE_H
#define UI_VIRTUAL_SCROLL_BASE_H

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
 * @struct ui_virtual_scroll_base
 * @brief Opaque handle for a virtual scroll base component.
 */
struct ui_virtual_scroll_base;

struct ui_dom_node;
struct ui_scroll_base;

/**
 * @enum ui_virtual_scroll_strategy
 * @brief Strategy for virtual scrolling
 */
enum ui_virtual_scroll_strategy {
  /** @brief Items have a fixed, uniform size. */
  UI_VIRTUAL_SCROLL_FIXED_SIZE,
  /** @brief Items have variable sizes (requires calculating or estimating
     size). */
  UI_VIRTUAL_SCROLL_VARIABLE_SIZE
};

/**
 * @enum ui_virtual_scroll_orientation
 * @brief Orientation for virtual scrolling
 */
enum ui_virtual_scroll_orientation {
  /** @brief Vertical scrolling (y-axis). */
  UI_VIRTUAL_SCROLL_ORIENTATION_VERTICAL = 0,
  /** @brief Horizontal scrolling (x-axis). */
  UI_VIRTUAL_SCROLL_ORIENTATION_HORIZONTAL = 1
};

/**
 * @brief Callback to create a new DOM node for an item.
 *
 * @param index The index of the item.
 * @param out_node Pointer to receive the new node.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_virtual_scroll_create_node_fn)(
    size_t index, struct ui_dom_node **out_node, void *user_data);

/**
 * @brief Callback to update an existing DOM node for a new item.
 *
 * @param index The index of the item.
 * @param node The DOM node to update.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_virtual_scroll_update_node_fn)(size_t index,
                                                       struct ui_dom_node *node,
                                                       void *user_data);

/**
 * @struct ui_virtual_scroll_config
 * @brief Configuration for a virtual scroller.
 */
struct ui_virtual_scroll_config {
  /** @brief Sizing strategy for the items. */
  enum ui_virtual_scroll_strategy strategy;
  /** @brief The scrolling direction. */
  enum ui_virtual_scroll_orientation orientation;
  /** @brief Size used if strategy is FIXED_SIZE. */
  float fixed_item_size;
  /** @brief Function to get item size if strategy is VARIABLE_SIZE. */
  float (*get_item_size)(size_t index, void *user_data);

  /** @brief Callback to create a new item node. */
  ui_virtual_scroll_create_node_fn create_node;
  /** @brief Callback to update a recycled item node. */
  ui_virtual_scroll_update_node_fn update_node;
  /** @brief Opaque user data passed to callbacks. */
  void *user_data;
};

/**
 * @brief Creates a new virtual scroller base component.
 *
 * @param out_virtual_scroll Pointer to receive the allocated virtual scroll
 * base.
 * @param config Configuration for the virtual scroller.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_virtual_scroll_base_create(
    struct ui_virtual_scroll_base **out_virtual_scroll,
    const struct ui_virtual_scroll_config *config);

/**
 * @brief Destroys a virtual scroller component.
 *
 * @param vs The virtual scroll area to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_virtual_scroll_base_destroy(struct ui_virtual_scroll_base *vs);

/**
 * @brief Sets the number of items in the dataset.
 *
 * @param vs The virtual scroll area.
 * @param count The number of items.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_virtual_scroll_base_set_item_count(struct ui_virtual_scroll_base *vs,
                                      size_t count);

/**
 * @brief Sets the viewport size for visible range calculations.
 *
 * @param vs The virtual scroll area.
 * @param width The viewport width.
 * @param height The viewport height.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_virtual_scroll_base_set_viewport_size(struct ui_virtual_scroll_base *vs,
                                         float width, float height);

/**
 * @brief Calculates the total content height based on the current strategy and
 * item count.
 *
 * @param vs The virtual scroll area.
 * @param out_height Pointer to receive the total estimated or exact content
 * height.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_virtual_scroll_base_get_total_height(const struct ui_virtual_scroll_base *vs,
                                        float *out_height);

/**
 * @brief Calculates the currently visible range of items based on scroll
 * offset.
 *
 * @param vs The virtual scroll area.
 * @param scroll_y The vertical scroll position.
 * @param out_start_index Pointer to receive the start index.
 * @param out_end_index Pointer to receive the end index.
 * @param out_offset_y Pointer to receive the Y offset of the start index.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_virtual_scroll_base_get_visible_range(
    const struct ui_virtual_scroll_base *vs, float scroll_y,
    size_t *out_start_index, size_t *out_end_index, float *out_offset_y);

/**
 * @brief Updates the recycled DOM nodes based on the current scroll position.
 *
 * @param vs The virtual scroll area.
 * @param scroll_y The vertical scroll position.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_virtual_scroll_base_render(struct ui_virtual_scroll_base *vs,
                                         float scroll_y);

/**
 * @brief Mounts the virtual scroller to a container DOM node.
 *
 * @param vs The virtual scroll area.
 * @param container The container DOM node.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_virtual_scroll_base_mount(struct ui_virtual_scroll_base *vs,
                                        struct ui_dom_node *container);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_virtual_scroll_base_bind_data(struct ui_virtual_scroll_base *widget,
                                 struct ui_computed *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_VIRTUAL_SCROLL_BASE_H */
