#ifndef M3_TABS_H
#define M3_TABS_H

/**
 * @file m3_tabs.h
 * @brief Tab row and segmented button widgets for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_anim.h"
#include "m3_layout.h"
#include "m3_text.h"

/** @brief Fixed tab row layout (tabs share available width). */
#define M3_TAB_MODE_FIXED 1
/** @brief Scrollable tab row layout (tabs sized to content). */
#define M3_TAB_MODE_SCROLLABLE 2

/** @brief Invalid tab index sentinel. */
#define M3_TAB_INVALID_INDEX ((m3_usize) ~(m3_usize)0)

/** @brief Default tab minimum width in pixels. */
#define M3_TAB_DEFAULT_MIN_WIDTH 90.0f
/** @brief Default tab minimum height in pixels. */
#define M3_TAB_DEFAULT_MIN_HEIGHT 48.0f
/** @brief Default horizontal tab padding in pixels. */
#define M3_TAB_DEFAULT_PADDING_X 16.0f
/** @brief Default vertical tab padding in pixels. */
#define M3_TAB_DEFAULT_PADDING_Y 12.0f
/** @brief Default spacing between tabs in pixels. */
#define M3_TAB_DEFAULT_SPACING 0.0f
/** @brief Default indicator thickness in pixels. */
#define M3_TAB_DEFAULT_INDICATOR_THICKNESS 2.0f
/** @brief Default indicator corner radius in pixels. */
#define M3_TAB_DEFAULT_INDICATOR_CORNER 0.0f
/** @brief Default indicator animation duration in seconds. */
#define M3_TAB_DEFAULT_INDICATOR_DURATION 0.2f
/** @brief Default indicator easing mode. */
#define M3_TAB_DEFAULT_INDICATOR_EASING M3_ANIM_EASE_OUT

/**
 * @brief Tab item descriptor.
 */
typedef struct M3TabItem {
  const char
      *utf8_label; /**< UTF-8 label pointer (may be NULL when utf8_len is 0). */
  m3_usize utf8_len; /**< UTF-8 label length in bytes. */
} M3TabItem;

struct M3TabRow;

/**
 * @brief Tab selection callback signature.
 *
 * @param ctx User callback context pointer.
 * @param row Tab row instance.
 * @param index Newly selected index.
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3TabRowOnSelect)(void *ctx, struct M3TabRow *row,
                                       m3_usize index);

/**
 * @brief Tab row style descriptor.
 */
typedef struct M3TabRowStyle {
  m3_u32 mode;                  /**< Tab layout mode (M3_TAB_MODE_*). */
  M3LayoutEdges padding;        /**< Padding around the tab row. */
  M3Scalar spacing;             /**< Spacing between tabs in pixels (>= 0). */
  M3Scalar min_width;           /**< Minimum tab width in pixels (>= 0). */
  M3Scalar min_height;          /**< Minimum tab height in pixels (>= 0). */
  M3Scalar padding_x;           /**< Horizontal tab padding in pixels (>= 0). */
  M3Scalar padding_y;           /**< Vertical tab padding in pixels (>= 0). */
  M3Scalar indicator_thickness; /**< Indicator thickness in pixels (>= 0). */
  M3Scalar indicator_corner; /**< Indicator corner radius in pixels (>= 0). */
  M3Scalar indicator_anim_duration; /**< Indicator animation duration in
                                       seconds (>= 0). */
  m3_u32 indicator_anim_easing; /**< Indicator easing mode (M3_ANIM_EASE_*). */
  M3TextStyle text_style;       /**< Base text style. */
  M3Color selected_text_color;  /**< Selected label color. */
  M3Color indicator_color;      /**< Indicator color. */
  M3Color background_color;     /**< Background fill color. */
  M3Color disabled_text_color;  /**< Disabled label color. */
  M3Color disabled_indicator_color; /**< Disabled indicator color. */
} M3TabRowStyle;

/**
 * @brief Tab row widget instance.
 */
typedef struct M3TabRow {
  M3Widget widget; /**< Widget interface (points to this instance). */
  M3TextBackend text_backend; /**< Text backend instance. */
  M3TabRowStyle style;        /**< Current tab row style. */
  const M3TabItem *items;     /**< Tab items (not owned). */
  m3_usize item_count;        /**< Number of tab items. */
  m3_usize selected_index; /**< Selected tab index or M3_TAB_INVALID_INDEX. */
  m3_usize pressed_index;  /**< Pressed tab index or M3_TAB_INVALID_INDEX. */
  M3Rect bounds;           /**< Layout bounds. */
  M3Handle font;           /**< Font handle for labels. */
  M3AnimController indicator_pos_anim;   /**< Indicator position animation. */
  M3AnimController indicator_width_anim; /**< Indicator width animation. */
  M3Scalar indicator_pos;                /**< Indicator x offset in content. */
  M3Scalar indicator_width;              /**< Indicator width in pixels. */
  M3Scalar scroll_offset;     /**< Scroll offset for scrollable tabs. */
  M3Scalar content_width;     /**< Content width for scrollable tabs. */
  M3TabRowOnSelect on_select; /**< Selection callback (may be NULL). */
  void *on_select_ctx;        /**< Selection callback context pointer. */
  M3Bool owns_font;           /**< M3_TRUE when widget owns the font. */
} M3TabRow;

/**
 * @brief Initialize a tab row style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tab_row_style_init(M3TabRowStyle *style);

/**
 * @brief Initialize a tab row widget.
 * @param row Tab row instance.
 * @param backend Text backend instance.
 * @param style Tab row style descriptor.
 * @param items Tab item array (may be NULL when item_count is 0).
 * @param item_count Number of items.
 * @param selected_index Initial selection or M3_TAB_INVALID_INDEX.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tab_row_init(M3TabRow *row, const M3TextBackend *backend,
                                   const M3TabRowStyle *style,
                                   const M3TabItem *items, m3_usize item_count,
                                   m3_usize selected_index);

/**
 * @brief Replace the tab items.
 * @param row Tab row instance.
 * @param items Tab item array (may be NULL when item_count is 0).
 * @param item_count Number of items.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tab_row_set_items(M3TabRow *row, const M3TabItem *items,
                                        m3_usize item_count);

/**
 * @brief Update the tab row style.
 * @param row Tab row instance.
 * @param style New style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tab_row_set_style(M3TabRow *row,
                                        const M3TabRowStyle *style);

/**
 * @brief Update the selected tab.
 * @param row Tab row instance.
 * @param selected_index Selected index or M3_TAB_INVALID_INDEX.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tab_row_set_selected(M3TabRow *row,
                                           m3_usize selected_index);

/**
 * @brief Retrieve the selected tab index.
 * @param row Tab row instance.
 * @param out_selected Receives the selected index.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tab_row_get_selected(const M3TabRow *row,
                                           m3_usize *out_selected);

/**
 * @brief Update the scroll offset for scrollable tabs.
 * @param row Tab row instance.
 * @param offset Scroll offset in pixels (>= 0).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tab_row_set_scroll(M3TabRow *row, M3Scalar offset);

/**
 * @brief Retrieve the current scroll offset.
 * @param row Tab row instance.
 * @param out_offset Receives the scroll offset in pixels.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tab_row_get_scroll(const M3TabRow *row,
                                         M3Scalar *out_offset);

/**
 * @brief Assign a selection callback for the tab row.
 * @param row Tab row instance.
 * @param on_select Selection callback (may be NULL).
 * @param ctx Callback context pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tab_row_set_on_select(M3TabRow *row,
                                            M3TabRowOnSelect on_select,
                                            void *ctx);

/**
 * @brief Step the tab row indicator animations.
 * @param row Tab row instance.
 * @param dt Delta time in seconds (>= 0).
 * @param out_changed Receives M3_TRUE when state changed.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tab_row_step(M3TabRow *row, M3Scalar dt,
                                   M3Bool *out_changed);

/** @brief Single-select segmented buttons. */
#define M3_SEGMENTED_MODE_SINGLE 1
/** @brief Multi-select segmented buttons. */
#define M3_SEGMENTED_MODE_MULTI 2

/** @brief Invalid segmented index sentinel. */
#define M3_SEGMENTED_INVALID_INDEX ((m3_usize) ~(m3_usize)0)

/** @brief Default segmented minimum width in pixels. */
#define M3_SEGMENTED_DEFAULT_MIN_WIDTH 64.0f
/** @brief Default segmented minimum height in pixels. */
#define M3_SEGMENTED_DEFAULT_MIN_HEIGHT 32.0f
/** @brief Default segmented horizontal padding in pixels. */
#define M3_SEGMENTED_DEFAULT_PADDING_X 12.0f
/** @brief Default segmented vertical padding in pixels. */
#define M3_SEGMENTED_DEFAULT_PADDING_Y 8.0f
/** @brief Default segmented spacing in pixels. */
#define M3_SEGMENTED_DEFAULT_SPACING 0.0f
/** @brief Default segmented outline width in pixels. */
#define M3_SEGMENTED_DEFAULT_OUTLINE_WIDTH 1.0f
/** @brief Default segmented corner radius in pixels. */
#define M3_SEGMENTED_DEFAULT_CORNER_RADIUS 4.0f

/**
 * @brief Segmented button item descriptor.
 */
typedef struct M3SegmentedItem {
  const char
      *utf8_label; /**< UTF-8 label pointer (may be NULL when utf8_len is 0). */
  m3_usize utf8_len; /**< UTF-8 label length in bytes. */
} M3SegmentedItem;

struct M3SegmentedButtons;

/**
 * @brief Segmented selection callback signature.
 *
 * @param ctx User callback context pointer.
 * @param buttons Segmented button group.
 * @param index Index that changed.
 * @param selected M3_TRUE when selected.
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3SegmentedOnSelect)(void *ctx,
                                          struct M3SegmentedButtons *buttons,
                                          m3_usize index, M3Bool selected);

/**
 * @brief Segmented button style descriptor.
 */
typedef struct M3SegmentedStyle {
  M3LayoutEdges padding;    /**< Padding around the segmented group. */
  M3Scalar spacing;         /**< Spacing between segments in pixels (>= 0). */
  M3Scalar min_width;       /**< Minimum segment width in pixels (>= 0). */
  M3Scalar min_height;      /**< Minimum segment height in pixels (>= 0). */
  M3Scalar padding_x;       /**< Horizontal segment padding in pixels (>= 0). */
  M3Scalar padding_y;       /**< Vertical segment padding in pixels (>= 0). */
  M3Scalar outline_width;   /**< Outline width in pixels (>= 0). */
  M3Scalar corner_radius;   /**< Corner radius in pixels (>= 0). */
  M3TextStyle text_style;   /**< Base text style. */
  M3Color background_color; /**< Background color. */
  M3Color outline_color;    /**< Outline color. */
  M3Color selected_background_color; /**< Background color when selected. */
  M3Color selected_text_color;       /**< Text color when selected. */
  M3Color disabled_background_color; /**< Background color when disabled. */
  M3Color disabled_outline_color;    /**< Outline color when disabled. */
  M3Color disabled_text_color;       /**< Text color when disabled. */
  M3Color disabled_selected_background_color; /**< Selected background when
                                                 disabled. */
  M3Color disabled_selected_text_color;       /**< Selected text color when
                                                 disabled. */
} M3SegmentedStyle;

/**
 * @brief Segmented buttons widget instance.
 */
typedef struct M3SegmentedButtons {
  M3Widget widget; /**< Widget interface (points to this instance). */
  M3TextBackend text_backend;   /**< Text backend instance. */
  M3SegmentedStyle style;       /**< Current segmented style. */
  const M3SegmentedItem *items; /**< Segmented items (not owned). */
  m3_usize item_count;          /**< Number of segmented items. */
  m3_u32 mode;                  /**< Selection mode (M3_SEGMENTED_MODE_*). */
  m3_usize selected_index; /**< Selected index or M3_SEGMENTED_INVALID_INDEX. */
  M3Bool *selected_states; /**< Selected state array for multi-select. */
  m3_usize pressed_index;  /**< Pressed index or M3_SEGMENTED_INVALID_INDEX. */
  M3Rect bounds;           /**< Layout bounds. */
  M3Handle font;           /**< Font handle for labels. */
  M3SegmentedOnSelect on_select; /**< Selection callback (may be NULL). */
  void *on_select_ctx;           /**< Selection callback context pointer. */
  M3Bool owns_font;              /**< M3_TRUE when widget owns the font. */
} M3SegmentedButtons;

/**
 * @brief Initialize a segmented style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_segmented_style_init(M3SegmentedStyle *style);

/**
 * @brief Initialize a segmented button group.
 * @param buttons Segmented button group.
 * @param backend Text backend instance.
 * @param style Segmented style descriptor.
 * @param items Segmented items (may be NULL when item_count is 0).
 * @param item_count Number of items.
 * @param mode Selection mode (M3_SEGMENTED_MODE_*).
 * @param selected_index Selected index for single-select mode or
 *        M3_SEGMENTED_INVALID_INDEX.
 * @param selected_states Selected state array for multi-select mode (may be
 *        NULL when item_count is 0).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_segmented_buttons_init(M3SegmentedButtons *buttons,
                                             const M3TextBackend *backend,
                                             const M3SegmentedStyle *style,
                                             const M3SegmentedItem *items,
                                             m3_usize item_count, m3_u32 mode,
                                             m3_usize selected_index,
                                             M3Bool *selected_states);

/**
 * @brief Replace the segmented items.
 * @param buttons Segmented button group.
 * @param items Segmented items (may be NULL when item_count is 0).
 * @param item_count Number of items.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_segmented_buttons_set_items(M3SegmentedButtons *buttons,
                                                  const M3SegmentedItem *items,
                                                  m3_usize item_count);

/**
 * @brief Update the segmented style.
 * @param buttons Segmented button group.
 * @param style New style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_segmented_buttons_set_style(
    M3SegmentedButtons *buttons, const M3SegmentedStyle *style);

/**
 * @brief Update the selected index for single-select mode.
 * @param buttons Segmented button group.
 * @param selected_index Selected index or M3_SEGMENTED_INVALID_INDEX.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_segmented_buttons_set_selected_index(
    M3SegmentedButtons *buttons, m3_usize selected_index);

/**
 * @brief Retrieve the selected index for single-select mode.
 * @param buttons Segmented button group.
 * @param out_selected Receives the selected index.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_segmented_buttons_get_selected_index(
    const M3SegmentedButtons *buttons, m3_usize *out_selected);

/**
 * @brief Update the selected state for multi-select mode.
 * @param buttons Segmented button group.
 * @param index Segment index.
 * @param selected Selected state.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_segmented_buttons_set_selected_state(
    M3SegmentedButtons *buttons, m3_usize index, M3Bool selected);

/**
 * @brief Retrieve the selected state for multi-select mode.
 * @param buttons Segmented button group.
 * @param index Segment index.
 * @param out_selected Receives the selected state.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_segmented_buttons_get_selected_state(
    const M3SegmentedButtons *buttons, m3_usize index, M3Bool *out_selected);

/**
 * @brief Assign a selection callback for segmented buttons.
 * @param buttons Segmented button group.
 * @param on_select Selection callback (may be NULL).
 * @param ctx Callback context pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_segmented_buttons_set_on_select(
    M3SegmentedButtons *buttons, M3SegmentedOnSelect on_select, void *ctx);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_TABS_H */
