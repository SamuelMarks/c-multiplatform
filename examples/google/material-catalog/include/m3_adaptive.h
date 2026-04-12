/* clang-format off */
#ifndef M3_ADAPTIVE_H
#define M3_ADAPTIVE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cmp.h"

/**
 * @brief Window size classes defined by Material Design 3.
 */
typedef enum m3_window_size_class {
    M3_WINDOW_SIZE_CLASS_COMPACT,
    M3_WINDOW_SIZE_CLASS_MEDIUM,
    M3_WINDOW_SIZE_CLASS_EXPANDED
} m3_window_size_class_t;

/**
 * @brief Layout types for canonical layouts.
 */
typedef enum m3_canonical_layout {
    M3_CANONICAL_LAYOUT_LIST_DETAIL,
    M3_CANONICAL_LAYOUT_SUPPORTING_PANE,
    M3_CANONICAL_LAYOUT_FEED
} m3_canonical_layout_t;

/**
 * @brief Navigation scaffolding types.
 */
typedef enum m3_nav_scaffolding {
    M3_NAV_SCAFFOLDING_BOTTOM_BAR,
    M3_NAV_SCAFFOLDING_NAVIGATION_RAIL,
    M3_NAV_SCAFFOLDING_PERMANENT_DRAWER
} m3_nav_scaffolding_t;

/**
 * @brief List-Detail view layout configurations.
 */
typedef enum m3_list_detail_layout {
    M3_LIST_DETAIL_STACKED,
    M3_LIST_DETAIL_SIDE_BY_SIDE
} m3_list_detail_layout_t;

/**
 * @brief Supporting Pane layout configurations.
 */
typedef enum m3_supporting_pane_layout {
    M3_SUPPORTING_PANE_HIDDEN_BOTTOM_SHEET,
    M3_SUPPORTING_PANE_VISIBLE_SIDE_PANEL
} m3_supporting_pane_layout_t;

/**
 * @brief Feed layout configurations.
 */
typedef enum m3_feed_layout {
    M3_FEED_SINGLE_COLUMN,
    M3_FEED_RESPONSIVE_GRID
} m3_feed_layout_t;

/**
 * @brief Determines the window size class based on the given width in dp.
 * 
 * @param width_dp The width of the window in density-independent pixels.
 * @param out_class Pointer to receive the window size class.
 * @return int 0 on success, non-zero on error.
 */
int m3_adaptive_get_window_size_class(float width_dp, m3_window_size_class_t* out_class);

/**
 * @brief Determines the recommended navigation scaffolding based on the window size class.
 * 
 * @param size_class The window size class.
 * @param out_nav Pointer to receive the navigation scaffolding type.
 * @return int 0 on success, non-zero on error.
 */
int m3_adaptive_get_nav_scaffolding(m3_window_size_class_t size_class, m3_nav_scaffolding_t* out_nav);

/**
 * @brief Determines the List-Detail layout based on window size class.
 * 
 * @param size_class The window size class.
 * @param out_layout Pointer to receive the layout type.
 * @return int 0 on success, non-zero on error.
 */
int m3_adaptive_get_list_detail_layout(m3_window_size_class_t size_class, m3_list_detail_layout_t* out_layout);

/**
 * @brief Determines the Supporting Pane layout based on window size class.
 * 
 * @param size_class The window size class.
 * @param out_layout Pointer to receive the layout type.
 * @return int 0 on success, non-zero on error.
 */
int m3_adaptive_get_supporting_pane_layout(m3_window_size_class_t size_class, m3_supporting_pane_layout_t* out_layout);

/**
 * @brief Determines the Feed layout based on window size class.
 * 
 * @param size_class The window size class.
 * @param out_layout Pointer to receive the layout type.
 * @return int 0 on success, non-zero on error.
 */
int m3_adaptive_get_feed_layout(m3_window_size_class_t size_class, m3_feed_layout_t* out_layout);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* M3_ADAPTIVE_H */
/* clang-format on */