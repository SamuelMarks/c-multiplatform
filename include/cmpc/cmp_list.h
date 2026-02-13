#ifndef CMP_LIST_H
#define CMP_LIST_H

/**
 * @file cmp_list.h
 * @brief Virtualized list and grid widgets for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_layout.h"
#include "cmpc/cmp_render.h"

/** @brief Vertical list orientation (scroll on Y axis). */
#define CMP_LIST_ORIENTATION_VERTICAL 0
/** @brief Horizontal list orientation (scroll on X axis). */
#define CMP_LIST_ORIENTATION_HORIZONTAL 1

/** @brief Invalid list index sentinel. */
#define CMP_LIST_INVALID_INDEX ((cmp_usize) ~(cmp_usize)0)

/** @brief Default list item extent in pixels. */
#define CMP_LIST_DEFAULT_ITEM_EXTENT 48.0f
/** @brief Default list item spacing in pixels. */
#define CMP_LIST_DEFAULT_SPACING 0.0f
/** @brief Default list overscan in items. */
#define CMP_LIST_DEFAULT_OVERSCAN 1u

/** @brief Vertical grid scrolling (rows on Y axis). */
#define CMP_GRID_SCROLL_VERTICAL 0
/** @brief Horizontal grid scrolling (columns on X axis). */
#define CMP_GRID_SCROLL_HORIZONTAL 1

/** @brief Default grid span (columns for vertical, rows for horizontal). */
#define CMP_GRID_DEFAULT_SPAN 2u
/** @brief Default grid item width in pixels. */
#define CMP_GRID_DEFAULT_ITEM_WIDTH 64.0f
/** @brief Default grid item height in pixels. */
#define CMP_GRID_DEFAULT_ITEM_HEIGHT 64.0f
/** @brief Default grid spacing in pixels. */
#define CMP_GRID_DEFAULT_SPACING 0.0f
/** @brief Default grid overscan in rows/columns. */
#define CMP_GRID_DEFAULT_OVERSCAN 1u

/**
 * @brief Slot for a recycled list/grid item.
 */
typedef struct CMPListSlot {
  cmp_usize index;    /**< Bound item index or CMP_LIST_INVALID_INDEX. */
  CMPRenderNode node; /**< Render node for the item widget. */
  void *user;         /**< User-defined slot state. */
} CMPListSlot;

struct CMPListView;

/**
 * @brief List/grid bind callback signature.
 *
 * @param ctx User callback context pointer.
 * @param slot Slot to populate (must set slot->node.widget).
 * @param index Item index to bind.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPListBindFn)(void *ctx, CMPListSlot *slot,
                                     cmp_usize index);

/**
 * @brief List style descriptor.
 */
typedef struct CMPListStyle {
  cmp_u32 orientation;       /**< List orientation (CMP_LIST_ORIENTATION_*). */
  CMPLayoutEdges padding;    /**< Padding around list contents. */
  CMPScalar spacing;         /**< Spacing between items in pixels (>= 0). */
  CMPScalar item_extent;     /**< Item extent along the scroll axis (>= 0). */
  CMPColor background_color; /**< Background fill color. */
  cmp_usize overscan; /**< Extra items to include before/after viewport. */
} CMPListStyle;

/**
 * @brief List widget instance.
 */
typedef struct CMPListView {
  CMPWidget widget;         /**< Widget interface (points to this instance). */
  CMPListStyle style;       /**< Current list style. */
  CMPRect bounds;           /**< Layout bounds. */
  CMPScalar scroll_offset;  /**< Scroll offset along the main axis. */
  CMPScalar content_extent; /**< Content extent along the main axis. */
  cmp_usize item_count;     /**< Number of items in the list. */
  cmp_usize visible_count;  /**< Number of visible items after update. */
  cmp_usize visible_first;  /**< First visible item index (or
                              CMP_LIST_INVALID_INDEX). */
  cmp_usize
      visible_last; /**< Last visible item index (or CMP_LIST_INVALID_INDEX). */
  CMPListSlot *slots;            /**< Slot pool for recycling items. */
  CMPRenderNode **visible_nodes; /**< Pointer list for visible nodes. */
  cmp_usize slot_capacity;       /**< Number of slots allocated. */
  CMPAllocator allocator;        /**< Allocator for slots. */
  CMPListBindFn bind;            /**< Bind callback for item recycling. */
  void *bind_ctx;                /**< Bind callback context pointer. */
} CMPListView;

/**
 * @brief Grid style descriptor.
 */
typedef struct CMPGridStyle {
  cmp_u32 scroll_axis; /**< Scroll axis (CMP_GRID_SCROLL_*). */
  cmp_usize span; /**< Columns (vertical scroll) or rows (horizontal scroll). */
  CMPLayoutEdges padding;    /**< Padding around grid contents. */
  CMPScalar spacing_x;       /**< Horizontal spacing between items (>= 0). */
  CMPScalar spacing_y;       /**< Vertical spacing between items (>= 0). */
  CMPScalar item_width;      /**< Item width in pixels (>= 0). */
  CMPScalar item_height;     /**< Item height in pixels (>= 0). */
  CMPColor background_color; /**< Background fill color. */
  cmp_usize overscan; /**< Extra rows/columns to include beyond the viewport. */
} CMPGridStyle;

/**
 * @brief Grid widget instance.
 */
typedef struct CMPGridView {
  CMPWidget widget;         /**< Widget interface (points to this instance). */
  CMPGridStyle style;       /**< Current grid style. */
  CMPRect bounds;           /**< Layout bounds. */
  CMPScalar scroll_offset;  /**< Scroll offset along the main axis. */
  CMPScalar content_extent; /**< Content extent along the main axis. */
  cmp_usize item_count;     /**< Number of items in the grid. */
  cmp_usize visible_count;  /**< Number of visible items after update. */
  cmp_usize visible_first;  /**< First visible item index (or
                              CMP_LIST_INVALID_INDEX). */
  cmp_usize
      visible_last; /**< Last visible item index (or CMP_LIST_INVALID_INDEX). */
  CMPListSlot *slots;            /**< Slot pool for recycling items. */
  CMPRenderNode **visible_nodes; /**< Pointer list for visible nodes. */
  cmp_usize slot_capacity;       /**< Number of slots allocated. */
  CMPAllocator allocator;        /**< Allocator for slots. */
  CMPListBindFn bind;            /**< Bind callback for item recycling. */
  void *bind_ctx;                /**< Bind callback context pointer. */
} CMPGridView;

/**
 * @brief Initialize a list style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_style_init(CMPListStyle *style);

/**
 * @brief Initialize a list view.
 * @param view List view instance.
 * @param style Style descriptor.
 * @param allocator Allocator for slots (NULL uses default allocator).
 * @param item_count Number of list items.
 * @param slot_capacity Slot pool capacity (0 leaves unallocated).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_view_init(CMPListView *view,
                                        const CMPListStyle *style,
                                        const CMPAllocator *allocator,
                                        cmp_usize item_count,
                                        cmp_usize slot_capacity);

/**
 * @brief Assign the bind callback for list recycling.
 * @param view List view instance.
 * @param bind Bind callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_view_set_bind(CMPListView *view,
                                            CMPListBindFn bind, void *ctx);

/**
 * @brief Update the list item count.
 * @param view List view instance.
 * @param item_count New item count.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_view_set_item_count(CMPListView *view,
                                                  cmp_usize item_count);

/**
 * @brief Update the list style.
 * @param view List view instance.
 * @param style New style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_view_set_style(CMPListView *view,
                                             const CMPListStyle *style);

/**
 * @brief Update the scroll offset (clamped to the content bounds).
 * @param view List view instance.
 * @param offset Scroll offset in pixels (>= 0).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_view_set_scroll(CMPListView *view,
                                              CMPScalar offset);

/**
 * @brief Retrieve the current scroll offset.
 * @param view List view instance.
 * @param out_offset Receives the scroll offset.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_view_get_scroll(const CMPListView *view,
                                              CMPScalar *out_offset);

/**
 * @brief Retrieve the list content extent along the main axis.
 * @param view List view instance.
 * @param out_extent Receives the content extent.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_view_get_content_extent(const CMPListView *view,
                                                      CMPScalar *out_extent);

/**
 * @brief Compute the required slot count for the current viewport.
 * @param view List view instance.
 * @param out_required Receives the required slot count.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_view_get_required_slots(const CMPListView *view,
                                                      cmp_usize *out_required);

/**
 * @brief Ensure the list slot pool can hold the requested capacity.
 * @param view List view instance.
 * @param capacity Slot capacity to reserve.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_view_reserve(CMPListView *view,
                                           cmp_usize capacity);

/**
 * @brief Update the visible slot bindings and node bounds.
 * @param view List view instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_view_update(CMPListView *view);

/**
 * @brief Retrieve the visible render node list.
 * @param view List view instance.
 * @param out_nodes Receives the node pointer array (may be NULL when count is
 * 0).
 * @param out_count Receives the number of visible nodes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_view_get_visible(const CMPListView *view,
                                               CMPRenderNode ***out_nodes,
                                               cmp_usize *out_count);

/**
 * @brief Initialize a grid style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_grid_style_init(CMPGridStyle *style);

/**
 * @brief Initialize a grid view.
 * @param view Grid view instance.
 * @param style Style descriptor.
 * @param allocator Allocator for slots (NULL uses default allocator).
 * @param item_count Number of grid items.
 * @param slot_capacity Slot pool capacity (0 leaves unallocated).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_grid_view_init(CMPGridView *view,
                                        const CMPGridStyle *style,
                                        const CMPAllocator *allocator,
                                        cmp_usize item_count,
                                        cmp_usize slot_capacity);

/**
 * @brief Assign the bind callback for grid recycling.
 * @param view Grid view instance.
 * @param bind Bind callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_grid_view_set_bind(CMPGridView *view,
                                            CMPListBindFn bind, void *ctx);

/**
 * @brief Update the grid item count.
 * @param view Grid view instance.
 * @param item_count New item count.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_grid_view_set_item_count(CMPGridView *view,
                                                  cmp_usize item_count);

/**
 * @brief Update the grid style.
 * @param view Grid view instance.
 * @param style New style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_grid_view_set_style(CMPGridView *view,
                                             const CMPGridStyle *style);

/**
 * @brief Update the scroll offset (clamped to the content bounds).
 * @param view Grid view instance.
 * @param offset Scroll offset in pixels (>= 0).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_grid_view_set_scroll(CMPGridView *view,
                                              CMPScalar offset);

/**
 * @brief Retrieve the current scroll offset.
 * @param view Grid view instance.
 * @param out_offset Receives the scroll offset.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_grid_view_get_scroll(const CMPGridView *view,
                                              CMPScalar *out_offset);

/**
 * @brief Retrieve the grid content extent along the main axis.
 * @param view Grid view instance.
 * @param out_extent Receives the content extent.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_grid_view_get_content_extent(const CMPGridView *view,
                                                      CMPScalar *out_extent);

/**
 * @brief Compute the required slot count for the current viewport.
 * @param view Grid view instance.
 * @param out_required Receives the required slot count.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_grid_view_get_required_slots(const CMPGridView *view,
                                                      cmp_usize *out_required);

/**
 * @brief Ensure the grid slot pool can hold the requested capacity.
 * @param view Grid view instance.
 * @param capacity Slot capacity to reserve.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_grid_view_reserve(CMPGridView *view,
                                           cmp_usize capacity);

/**
 * @brief Update the visible slot bindings and node bounds.
 * @param view Grid view instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_grid_view_update(CMPGridView *view);

/**
 * @brief Retrieve the visible render node list.
 * @param view Grid view instance.
 * @param out_nodes Receives the node pointer array (may be NULL when count is
 * 0).
 * @param out_count Receives the number of visible nodes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_grid_view_get_visible(const CMPGridView *view,
                                               CMPRenderNode ***out_nodes,
                                               cmp_usize *out_count);

#ifdef CMP_TESTING
/**
 * @brief Set a list test fail point.
 * @param fail_point Fail point identifier.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_test_set_fail_point(cmp_u32 fail_point);

/**
 * @brief Clear list test fail points.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_test_clear_fail_points(void);

/**
 * @brief Test hook for list multiply-overflow helper.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives product.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_test_mul_overflow(cmp_usize a, cmp_usize b,
                                                cmp_usize *out_value);
/**
 * @brief Test hook for validating list colors.
 * @param color Color to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_test_validate_color(const CMPColor *color);
/**
 * @brief Test hook for validating list layout edges.
 * @param edges Edges to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_test_validate_edges(const CMPLayoutEdges *edges);
/**
 * @brief Test hook for validating rectangles.
 * @param rect Rectangle to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_test_validate_rect(const CMPRect *rect);
/**
 * @brief Test hook for validating measure specs.
 * @param spec Measure spec to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_test_validate_measure_spec(CMPMeasureSpec spec);
/**
 * @brief Test hook for validating list styles.
 * @param style Style to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_test_validate_style(const CMPListStyle *style);
/**
 * @brief Test hook for validating grid styles.
 * @param style Style to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_list_test_validate_grid_style(const CMPGridStyle *style);
/**
 * @brief Test hook for computing list content extents.
 * @param item_count Number of items.
 * @param item_extent Extent of each item.
 * @param spacing Item spacing.
 * @param padding_start Start padding.
 * @param padding_end End padding.
 * @param out_extent Receives computed extent.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_test_compute_content_extent(
    cmp_usize item_count, CMPScalar item_extent, CMPScalar spacing,
    CMPScalar padding_start, CMPScalar padding_end, CMPScalar *out_extent);
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
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_test_compute_visible_range(
    cmp_usize item_count, CMPScalar item_extent, CMPScalar spacing,
    CMPScalar padding_start, CMPScalar scroll, CMPScalar viewport,
    cmp_usize overscan, cmp_usize *out_first, cmp_usize *out_last,
    cmp_usize *out_count);
/**
 * @brief Test hook for computing grid visible ranges.
 * @param view Grid view to inspect.
 * @param out_first Receives first visible index.
 * @param out_last Receives last visible index.
 * @param out_count Receives visible count.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_test_grid_compute_visible_range(
    const CMPGridView *view, cmp_usize *out_first, cmp_usize *out_last,
    cmp_usize *out_count);
/**
 * @brief Test hook for computing list item bounds.
 * @param view List view.
 * @param index Item index.
 * @param out_bounds Receives bounds.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_test_compute_item_bounds(const CMPListView *view,
                                                       cmp_usize index,
                                                       CMPRect *out_bounds);
/**
 * @brief Test hook for computing grid item bounds.
 * @param view Grid view.
 * @param index Item index.
 * @param out_bounds Receives bounds.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_test_grid_compute_item_bounds(
    const CMPGridView *view, cmp_usize index, CMPRect *out_bounds);
/**
 * @brief Test hook for updating list metrics.
 * @param view List view instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_test_update_metrics(CMPListView *view);
/**
 * @brief Test hook for updating grid metrics.
 * @param view Grid view instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_test_grid_update_metrics(CMPGridView *view);
/**
 * @brief Test hook for reserving list slot storage.
 * @param slots Slot array pointer.
 * @param visible_nodes Visible node array pointer.
 * @param slot_capacity Slot capacity pointer.
 * @param allocator Allocator to use.
 * @param capacity Requested capacity.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_list_test_reserve_slots(CMPListSlot **slots,
                                                 CMPRenderNode ***visible_nodes,
                                                 cmp_usize *slot_capacity,
                                                 const CMPAllocator *allocator,
                                                 cmp_usize capacity);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_LIST_H */
