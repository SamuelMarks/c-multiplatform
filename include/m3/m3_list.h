#ifndef M3_LIST_H
#define M3_LIST_H

/**
 * @file m3_list.h
 * @brief Virtualized list and grid widgets for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_layout.h"
#include "m3_render.h"

/** @brief Vertical list orientation (scroll on Y axis). */
#define M3_LIST_ORIENTATION_VERTICAL 0
/** @brief Horizontal list orientation (scroll on X axis). */
#define M3_LIST_ORIENTATION_HORIZONTAL 1

/** @brief Invalid list index sentinel. */
#define M3_LIST_INVALID_INDEX ((m3_usize) ~(m3_usize)0)

/** @brief Default list item extent in pixels. */
#define M3_LIST_DEFAULT_ITEM_EXTENT 48.0f
/** @brief Default list item spacing in pixels. */
#define M3_LIST_DEFAULT_SPACING 0.0f
/** @brief Default list overscan in items. */
#define M3_LIST_DEFAULT_OVERSCAN 1u

/** @brief Vertical grid scrolling (rows on Y axis). */
#define M3_GRID_SCROLL_VERTICAL 0
/** @brief Horizontal grid scrolling (columns on X axis). */
#define M3_GRID_SCROLL_HORIZONTAL 1

/** @brief Default grid span (columns for vertical, rows for horizontal). */
#define M3_GRID_DEFAULT_SPAN 2u
/** @brief Default grid item width in pixels. */
#define M3_GRID_DEFAULT_ITEM_WIDTH 64.0f
/** @brief Default grid item height in pixels. */
#define M3_GRID_DEFAULT_ITEM_HEIGHT 64.0f
/** @brief Default grid spacing in pixels. */
#define M3_GRID_DEFAULT_SPACING 0.0f
/** @brief Default grid overscan in rows/columns. */
#define M3_GRID_DEFAULT_OVERSCAN 1u

/**
 * @brief Slot for a recycled list/grid item.
 */
typedef struct M3ListSlot {
  m3_usize index;    /**< Bound item index or M3_LIST_INVALID_INDEX. */
  M3RenderNode node; /**< Render node for the item widget. */
  void *user;        /**< User-defined slot state. */
} M3ListSlot;

struct M3ListView;

/**
 * @brief List/grid bind callback signature.
 *
 * @param ctx User callback context pointer.
 * @param slot Slot to populate (must set slot->node.widget).
 * @param index Item index to bind.
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3ListBindFn)(void *ctx, M3ListSlot *slot, m3_usize index);

/**
 * @brief List style descriptor.
 */
typedef struct M3ListStyle {
  m3_u32 orientation;       /**< List orientation (M3_LIST_ORIENTATION_*). */
  M3LayoutEdges padding;    /**< Padding around list contents. */
  M3Scalar spacing;         /**< Spacing between items in pixels (>= 0). */
  M3Scalar item_extent;     /**< Item extent along the scroll axis (>= 0). */
  M3Color background_color; /**< Background fill color. */
  m3_usize overscan; /**< Extra items to include before/after viewport. */
} M3ListStyle;

/**
 * @brief List widget instance.
 */
typedef struct M3ListView {
  M3Widget widget;         /**< Widget interface (points to this instance). */
  M3ListStyle style;       /**< Current list style. */
  M3Rect bounds;           /**< Layout bounds. */
  M3Scalar scroll_offset;  /**< Scroll offset along the main axis. */
  M3Scalar content_extent; /**< Content extent along the main axis. */
  m3_usize item_count;     /**< Number of items in the list. */
  m3_usize visible_count;  /**< Number of visible items after update. */
  m3_usize visible_first;  /**< First visible item index (or
                              M3_LIST_INVALID_INDEX). */
  m3_usize
      visible_last;  /**< Last visible item index (or M3_LIST_INVALID_INDEX). */
  M3ListSlot *slots; /**< Slot pool for recycling items. */
  M3RenderNode **visible_nodes; /**< Pointer list for visible nodes. */
  m3_usize slot_capacity;       /**< Number of slots allocated. */
  M3Allocator allocator;        /**< Allocator for slots. */
  M3ListBindFn bind;            /**< Bind callback for item recycling. */
  void *bind_ctx;               /**< Bind callback context pointer. */
} M3ListView;

/**
 * @brief Grid style descriptor.
 */
typedef struct M3GridStyle {
  m3_u32 scroll_axis; /**< Scroll axis (M3_GRID_SCROLL_*). */
  m3_usize span; /**< Columns (vertical scroll) or rows (horizontal scroll). */
  M3LayoutEdges padding;    /**< Padding around grid contents. */
  M3Scalar spacing_x;       /**< Horizontal spacing between items (>= 0). */
  M3Scalar spacing_y;       /**< Vertical spacing between items (>= 0). */
  M3Scalar item_width;      /**< Item width in pixels (>= 0). */
  M3Scalar item_height;     /**< Item height in pixels (>= 0). */
  M3Color background_color; /**< Background fill color. */
  m3_usize overscan; /**< Extra rows/columns to include beyond the viewport. */
} M3GridStyle;

/**
 * @brief Grid widget instance.
 */
typedef struct M3GridView {
  M3Widget widget;         /**< Widget interface (points to this instance). */
  M3GridStyle style;       /**< Current grid style. */
  M3Rect bounds;           /**< Layout bounds. */
  M3Scalar scroll_offset;  /**< Scroll offset along the main axis. */
  M3Scalar content_extent; /**< Content extent along the main axis. */
  m3_usize item_count;     /**< Number of items in the grid. */
  m3_usize visible_count;  /**< Number of visible items after update. */
  m3_usize visible_first;  /**< First visible item index (or
                              M3_LIST_INVALID_INDEX). */
  m3_usize
      visible_last;  /**< Last visible item index (or M3_LIST_INVALID_INDEX). */
  M3ListSlot *slots; /**< Slot pool for recycling items. */
  M3RenderNode **visible_nodes; /**< Pointer list for visible nodes. */
  m3_usize slot_capacity;       /**< Number of slots allocated. */
  M3Allocator allocator;        /**< Allocator for slots. */
  M3ListBindFn bind;            /**< Bind callback for item recycling. */
  void *bind_ctx;               /**< Bind callback context pointer. */
} M3GridView;

/**
 * @brief Initialize a list style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_style_init(M3ListStyle *style);

/**
 * @brief Initialize a list view.
 * @param view List view instance.
 * @param style Style descriptor.
 * @param allocator Allocator for slots (NULL uses default allocator).
 * @param item_count Number of list items.
 * @param slot_capacity Slot pool capacity (0 leaves unallocated).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_view_init(M3ListView *view, const M3ListStyle *style,
                                     const M3Allocator *allocator,
                                     m3_usize item_count,
                                     m3_usize slot_capacity);

/**
 * @brief Assign the bind callback for list recycling.
 * @param view List view instance.
 * @param bind Bind callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_view_set_bind(M3ListView *view, M3ListBindFn bind,
                                         void *ctx);

/**
 * @brief Update the list item count.
 * @param view List view instance.
 * @param item_count New item count.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_view_set_item_count(M3ListView *view,
                                               m3_usize item_count);

/**
 * @brief Update the list style.
 * @param view List view instance.
 * @param style New style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_view_set_style(M3ListView *view,
                                          const M3ListStyle *style);

/**
 * @brief Update the scroll offset (clamped to the content bounds).
 * @param view List view instance.
 * @param offset Scroll offset in pixels (>= 0).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_view_set_scroll(M3ListView *view, M3Scalar offset);

/**
 * @brief Retrieve the current scroll offset.
 * @param view List view instance.
 * @param out_offset Receives the scroll offset.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_view_get_scroll(const M3ListView *view,
                                           M3Scalar *out_offset);

/**
 * @brief Retrieve the list content extent along the main axis.
 * @param view List view instance.
 * @param out_extent Receives the content extent.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_view_get_content_extent(const M3ListView *view,
                                                   M3Scalar *out_extent);

/**
 * @brief Compute the required slot count for the current viewport.
 * @param view List view instance.
 * @param out_required Receives the required slot count.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_view_get_required_slots(const M3ListView *view,
                                                   m3_usize *out_required);

/**
 * @brief Ensure the list slot pool can hold the requested capacity.
 * @param view List view instance.
 * @param capacity Slot capacity to reserve.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_view_reserve(M3ListView *view, m3_usize capacity);

/**
 * @brief Update the visible slot bindings and node bounds.
 * @param view List view instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_view_update(M3ListView *view);

/**
 * @brief Retrieve the visible render node list.
 * @param view List view instance.
 * @param out_nodes Receives the node pointer array (may be NULL when count is
 * 0).
 * @param out_count Receives the number of visible nodes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_view_get_visible(const M3ListView *view,
                                            M3RenderNode ***out_nodes,
                                            m3_usize *out_count);

/**
 * @brief Initialize a grid style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_grid_style_init(M3GridStyle *style);

/**
 * @brief Initialize a grid view.
 * @param view Grid view instance.
 * @param style Style descriptor.
 * @param allocator Allocator for slots (NULL uses default allocator).
 * @param item_count Number of grid items.
 * @param slot_capacity Slot pool capacity (0 leaves unallocated).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_grid_view_init(M3GridView *view, const M3GridStyle *style,
                                     const M3Allocator *allocator,
                                     m3_usize item_count,
                                     m3_usize slot_capacity);

/**
 * @brief Assign the bind callback for grid recycling.
 * @param view Grid view instance.
 * @param bind Bind callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_grid_view_set_bind(M3GridView *view, M3ListBindFn bind,
                                         void *ctx);

/**
 * @brief Update the grid item count.
 * @param view Grid view instance.
 * @param item_count New item count.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_grid_view_set_item_count(M3GridView *view,
                                               m3_usize item_count);

/**
 * @brief Update the grid style.
 * @param view Grid view instance.
 * @param style New style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_grid_view_set_style(M3GridView *view,
                                          const M3GridStyle *style);

/**
 * @brief Update the scroll offset (clamped to the content bounds).
 * @param view Grid view instance.
 * @param offset Scroll offset in pixels (>= 0).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_grid_view_set_scroll(M3GridView *view, M3Scalar offset);

/**
 * @brief Retrieve the current scroll offset.
 * @param view Grid view instance.
 * @param out_offset Receives the scroll offset.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_grid_view_get_scroll(const M3GridView *view,
                                           M3Scalar *out_offset);

/**
 * @brief Retrieve the grid content extent along the main axis.
 * @param view Grid view instance.
 * @param out_extent Receives the content extent.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_grid_view_get_content_extent(const M3GridView *view,
                                                   M3Scalar *out_extent);

/**
 * @brief Compute the required slot count for the current viewport.
 * @param view Grid view instance.
 * @param out_required Receives the required slot count.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_grid_view_get_required_slots(const M3GridView *view,
                                                   m3_usize *out_required);

/**
 * @brief Ensure the grid slot pool can hold the requested capacity.
 * @param view Grid view instance.
 * @param capacity Slot capacity to reserve.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_grid_view_reserve(M3GridView *view, m3_usize capacity);

/**
 * @brief Update the visible slot bindings and node bounds.
 * @param view Grid view instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_grid_view_update(M3GridView *view);

/**
 * @brief Retrieve the visible render node list.
 * @param view Grid view instance.
 * @param out_nodes Receives the node pointer array (may be NULL when count is
 * 0).
 * @param out_count Receives the number of visible nodes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_grid_view_get_visible(const M3GridView *view,
                                            M3RenderNode ***out_nodes,
                                            m3_usize *out_count);

#ifdef M3_TESTING
/**
 * @brief Set a list test fail point.
 * @param fail_point Fail point identifier.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_test_set_fail_point(m3_u32 fail_point);

/**
 * @brief Clear list test fail points.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_test_clear_fail_points(void);

/**
 * @brief Test hook for list multiply-overflow helper.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives product.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_test_mul_overflow(m3_usize a, m3_usize b,
                                             m3_usize *out_value);
/**
 * @brief Test hook for validating list colors.
 * @param color Color to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_test_validate_color(const M3Color *color);
/**
 * @brief Test hook for validating list layout edges.
 * @param edges Edges to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_test_validate_edges(const M3LayoutEdges *edges);
/**
 * @brief Test hook for validating rectangles.
 * @param rect Rectangle to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_test_validate_rect(const M3Rect *rect);
/**
 * @brief Test hook for validating measure specs.
 * @param spec Measure spec to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_test_validate_measure_spec(M3MeasureSpec spec);
/**
 * @brief Test hook for validating list styles.
 * @param style Style to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_test_validate_style(const M3ListStyle *style);
/**
 * @brief Test hook for validating grid styles.
 * @param style Style to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_test_validate_grid_style(const M3GridStyle *style);
/**
 * @brief Test hook for computing list content extents.
 * @param item_count Number of items.
 * @param item_extent Extent of each item.
 * @param spacing Item spacing.
 * @param padding_start Start padding.
 * @param padding_end End padding.
 * @param out_extent Receives computed extent.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_test_compute_content_extent(
    m3_usize item_count, M3Scalar item_extent, M3Scalar spacing,
    M3Scalar padding_start, M3Scalar padding_end, M3Scalar *out_extent);
/**
 * @brief Test hook for computing list visible ranges.
 * @param item_count Number of items.
 * @param item_extent Extent of each item.
 * @param spacing Item spacing.
 * @param padding_start Start padding.
 * @param scroll Scroll offset.
 * @param viewport Viewport size.
 * @param overscan Overscan count.
 * @param out_first Receives first visible index.
 * @param out_last Receives last visible index.
 * @param out_count Receives visible count.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_test_compute_visible_range(
    m3_usize item_count, M3Scalar item_extent, M3Scalar spacing,
    M3Scalar padding_start, M3Scalar scroll, M3Scalar viewport,
    m3_usize overscan, m3_usize *out_first, m3_usize *out_last,
    m3_usize *out_count);
/**
 * @brief Test hook for computing grid visible ranges.
 * @param view Grid view to inspect.
 * @param out_first Receives first visible index.
 * @param out_last Receives last visible index.
 * @param out_count Receives visible count.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_test_grid_compute_visible_range(
    const M3GridView *view, m3_usize *out_first, m3_usize *out_last,
    m3_usize *out_count);
/**
 * @brief Test hook for computing list item bounds.
 * @param view List view.
 * @param index Item index.
 * @param out_bounds Receives bounds.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_test_compute_item_bounds(const M3ListView *view,
                                                    m3_usize index,
                                                    M3Rect *out_bounds);
/**
 * @brief Test hook for computing grid item bounds.
 * @param view Grid view.
 * @param index Item index.
 * @param out_bounds Receives bounds.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_test_grid_compute_item_bounds(const M3GridView *view,
                                                         m3_usize index,
                                                         M3Rect *out_bounds);
/**
 * @brief Test hook for updating list metrics.
 * @param view List view instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_test_update_metrics(M3ListView *view);
/**
 * @brief Test hook for updating grid metrics.
 * @param view Grid view instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_test_grid_update_metrics(M3GridView *view);
/**
 * @brief Test hook for reserving list slot storage.
 * @param slots Slot array pointer.
 * @param visible_nodes Visible node array pointer.
 * @param slot_capacity Slot capacity pointer.
 * @param allocator Allocator to use.
 * @param capacity Requested capacity.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_list_test_reserve_slots(M3ListSlot **slots,
                                              M3RenderNode ***visible_nodes,
                                              m3_usize *slot_capacity,
                                              const M3Allocator *allocator,
                                              m3_usize capacity);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_LIST_H */
