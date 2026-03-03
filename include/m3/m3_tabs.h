#ifndef M3_TABS_H
#define M3_TABS_H

/**
 * @file m3_tabs.h
 * @brief Tab row and segmented button widgets for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_anim.h"
#include "cmpc/cmp_layout.h"
#include "cmpc/cmp_text.h"

/** @brief Fixed tab row layout (tabs share available width). */
#define M3_TAB_MODE_FIXED 1
/** @brief Scrollable tab row layout (tabs sized to content). */
#define M3_TAB_MODE_SCROLLABLE 2

/** @brief Invalid tab index sentinel. */
#define M3_TAB_INVALID_INDEX ((cmp_usize) ~(cmp_usize)0)

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
#define M3_TAB_DEFAULT_INDICATOR_EASING CMP_ANIM_EASE_OUT

/**
 * @brief Tab item descriptor.
 */
typedef struct M3TabItem {
  const char
      *utf8_label; /**< UTF-8 label pointer (may be NULL when utf8_len is 0). */
  cmp_usize utf8_len; /**< UTF-8 label length in bytes. */
} M3TabItem;

struct M3TabRow;

/**
 * @brief Tab selection callback signature.
 *
 * @param ctx User callback context pointer.
 * @param row Tab row instance.
 * @param index Newly selected index.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPTabRowOnSelect)(void *ctx, struct M3TabRow *row,
                                         cmp_usize index);

/**
 * @brief Tab row style descriptor.
 */
typedef struct M3TabRowStyle {
  cmp_u32 mode;           /**< Tab layout mode (CMP_TAB_MODE_*). */
  CMPLayoutEdges padding; /**< Padding around the tab row. */
  CMPScalar spacing;      /**< Spacing between tabs in pixels (>= 0). */
  CMPScalar min_width;    /**< Minimum tab width in pixels (>= 0). */
  CMPScalar min_height;   /**< Minimum tab height in pixels (>= 0). */
  CMPScalar padding_x;    /**< Horizontal tab padding in pixels (>= 0). */
  CMPScalar padding_y;    /**< Vertical tab padding in pixels (>= 0). */
  CMPScalar indicator_thickness; /**< Indicator thickness in pixels (>= 0). */
  CMPScalar indicator_corner; /**< Indicator corner radius in pixels (>= 0). */
  CMPScalar indicator_anim_duration; /**< Indicator animation duration in
                                       seconds (>= 0). */
  cmp_u32
      indicator_anim_easing;    /**< Indicator easing mode (CMP_ANIM_EASE_*). */
  CMPTextStyle text_style;      /**< Base text style. */
  CMPColor selected_text_color; /**< Selected label color. */
  CMPColor indicator_color;     /**< Indicator color. */
  CMPColor background_color;    /**< Background fill color. */
  CMPColor disabled_text_color; /**< Disabled label color. */
  CMPColor disabled_indicator_color; /**< Disabled indicator color. */
  CMPBool is_rtl; /**< CMP_TRUE if layout should be right-to-left. */
} M3TabRowStyle;

/**
 * @brief Tab row widget instance.
 */
typedef struct M3TabRow {
  CMPWidget widget; /**< Widget interface (points to this instance). */
  CMPTextBackend text_backend; /**< Text backend instance. */
  M3TabRowStyle style;         /**< Current tab row style. */
  const M3TabItem *items;      /**< Tab items (not owned). */
  cmp_usize item_count;        /**< Number of tab items. */
  cmp_usize selected_index; /**< Selected tab index or M3_TAB_INVALID_INDEX. */
  cmp_usize pressed_index;  /**< Pressed tab index or M3_TAB_INVALID_INDEX. */
  CMPRect bounds;           /**< Layout bounds. */
  CMPHandle font;           /**< Font handle for labels. */
  CMPAnimController indicator_pos_anim;   /**< Indicator position animation. */
  CMPAnimController indicator_width_anim; /**< Indicator width animation. */
  CMPScalar indicator_pos;                /**< Indicator x offset in content. */
  CMPScalar indicator_width;              /**< Indicator width in pixels. */
  CMPScalar scroll_offset;     /**< Scroll offset for scrollable tabs. */
  CMPScalar content_width;     /**< Content width for scrollable tabs. */
  CMPTabRowOnSelect on_select; /**< Selection callback (may be NULL). */
  void *on_select_ctx;         /**< Selection callback context pointer. */
  CMPBool owns_font;           /**< CMP_TRUE when widget owns the font. */
} M3TabRow;

/**
 * @brief Initialize a tab row style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tab_row_style_init(M3TabRowStyle *style);

/**
 * @brief Initialize a tab row widget.
 * @param row Tab row instance.
 * @param backend Text backend instance.
 * @param style Tab row style descriptor.
 * @param items Tab item array (may be NULL when item_count is 0).
 * @param item_count Number of items.
 * @param selected_index Initial selection or M3_TAB_INVALID_INDEX.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tab_row_init(
    M3TabRow *row, const CMPTextBackend *backend, const M3TabRowStyle *style,
    const M3TabItem *items, cmp_usize item_count, cmp_usize selected_index);

/**
 * @brief Replace the tab items.
 * @param row Tab row instance.
 * @param items Tab item array (may be NULL when item_count is 0).
 * @param item_count Number of items.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tab_row_set_items(M3TabRow *row, const M3TabItem *items,
                                          cmp_usize item_count);

/**
 * @brief Update the tab row style.
 * @param row Tab row instance.
 * @param style New style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tab_row_set_style(M3TabRow *row,
                                          const M3TabRowStyle *style);

/**
 * @brief Update the selected tab.
 * @param row Tab row instance.
 * @param selected_index Selected index or M3_TAB_INVALID_INDEX.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tab_row_set_selected(M3TabRow *row,
                                             cmp_usize selected_index);

/**
 * @brief Retrieve the selected tab index.
 * @param row Tab row instance.
 * @param out_selected Receives the selected index.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tab_row_get_selected(const M3TabRow *row,
                                             cmp_usize *out_selected);

/**
 * @brief Update the scroll offset for scrollable tabs.
 * @param row Tab row instance.
 * @param offset Scroll offset in pixels (>= 0).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tab_row_set_scroll(M3TabRow *row, CMPScalar offset);

/**
 * @brief Retrieve the current scroll offset.
 * @param row Tab row instance.
 * @param out_offset Receives the scroll offset in pixels.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tab_row_get_scroll(const M3TabRow *row,
                                           CMPScalar *out_offset);

/**
 * @brief Assign a selection callback for the tab row.
 * @param row Tab row instance.
 * @param on_select Selection callback (may be NULL).
 * @param ctx Callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tab_row_set_on_select(M3TabRow *row,
                                              CMPTabRowOnSelect on_select,
                                              void *ctx);

/**
 * @brief Step the tab row indicator animations.
 * @param row Tab row instance.
 * @param dt Delta time in seconds (>= 0).
 * @param out_changed Receives CMP_TRUE when state changed.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tab_row_step(M3TabRow *row, CMPScalar dt,
                                     CMPBool *out_changed);

/** @brief Single-select segmented buttons. */
#define M3_SEGMENTED_MODE_SINGLE 1
/** @brief Multi-select segmented buttons. */
#define M3_SEGMENTED_MODE_MULTI 2

/** @brief Invalid segmented index sentinel. */
#define M3_SEGMENTED_INVALID_INDEX ((cmp_usize) ~(cmp_usize)0)

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
  cmp_usize utf8_len; /**< UTF-8 label length in bytes. */
} M3SegmentedItem;

struct M3SegmentedButtons;

/**
 * @brief Segmented selection callback signature.
 *
 * @param ctx User callback context pointer.
 * @param buttons Segmented button group.
 * @param index Index that changed.
 * @param selected CMP_TRUE when selected.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPSegmentedOnSelect)(void *ctx,
                                            struct M3SegmentedButtons *buttons,
                                            cmp_usize index, CMPBool selected);

/**
 * @brief Segmented button style descriptor.
 */
typedef struct M3SegmentedStyle {
  CMPLayoutEdges padding;  /**< Padding around the segmented group. */
  CMPScalar spacing;       /**< Spacing between segments in pixels (>= 0). */
  CMPScalar min_width;     /**< Minimum segment width in pixels (>= 0). */
  CMPScalar min_height;    /**< Minimum segment height in pixels (>= 0). */
  CMPScalar padding_x;     /**< Horizontal segment padding in pixels (>= 0). */
  CMPScalar padding_y;     /**< Vertical segment padding in pixels (>= 0). */
  CMPScalar outline_width; /**< Outline width in pixels (>= 0). */
  CMPScalar corner_radius; /**< Corner radius in pixels (>= 0). */
  CMPTextStyle text_style; /**< Base text style. */
  CMPColor background_color;          /**< Background color. */
  CMPColor outline_color;             /**< Outline color. */
  CMPColor selected_background_color; /**< Background color when selected. */
  CMPColor selected_text_color;       /**< Text color when selected. */
  CMPColor disabled_background_color; /**< Background color when disabled. */
  CMPColor disabled_outline_color;    /**< Outline color when disabled. */
  CMPColor disabled_text_color;       /**< Text color when disabled. */
  CMPColor disabled_selected_background_color; /**< Selected background when
                                                 disabled. */
  CMPColor disabled_selected_text_color;       /**< Selected text color when
                                                 disabled. */
  CMPBool is_rtl; /**< CMP_TRUE if layout should be right-to-left. */
} M3SegmentedStyle;

/**
 * @brief Segmented buttons widget instance.
 */
typedef struct M3SegmentedButtons {
  CMPWidget widget; /**< Widget interface (points to this instance). */
  CMPTextBackend text_backend;  /**< Text backend instance. */
  M3SegmentedStyle style;       /**< Current segmented style. */
  const M3SegmentedItem *items; /**< Segmented items (not owned). */
  cmp_usize item_count;         /**< Number of segmented items. */
  cmp_u32 mode;                 /**< Selection mode (CMP_SEGMENTED_MODE_*). */
  cmp_usize
      selected_index; /**< Selected index or M3_SEGMENTED_INVALID_INDEX. */
  CMPBool *selected_states; /**< Selected state array for multi-select. */
  cmp_usize pressed_index;  /**< Pressed index or M3_SEGMENTED_INVALID_INDEX. */
  CMPRect bounds;           /**< Layout bounds. */
  CMPHandle font;           /**< Font handle for labels. */
  CMPSegmentedOnSelect on_select; /**< Selection callback (may be NULL). */
  void *on_select_ctx;            /**< Selection callback context pointer. */
  CMPBool owns_font;              /**< CMP_TRUE when widget owns the font. */
} M3SegmentedButtons;

/**
 * @brief Initialize a segmented style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_segmented_style_init(M3SegmentedStyle *style);

/**
 * @brief Initialize a segmented button group.
 * @param buttons Segmented button group.
 * @param backend Text backend instance.
 * @param style Segmented style descriptor.
 * @param items Segmented items (may be NULL when item_count is 0).
 * @param item_count Number of items.
 * @param mode Selection mode (CMP_SEGMENTED_MODE_*).
 * @param selected_index Selected index for single-select mode or
 *        M3_SEGMENTED_INVALID_INDEX.
 * @param selected_states Selected state array for multi-select mode (may be
 *        NULL when item_count is 0).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_segmented_buttons_init(
    M3SegmentedButtons *buttons, const CMPTextBackend *backend,
    const M3SegmentedStyle *style, const M3SegmentedItem *items,
    cmp_usize item_count, cmp_u32 mode, cmp_usize selected_index,
    CMPBool *selected_states);

/**
 * @brief Replace the segmented items.
 * @param buttons Segmented button group.
 * @param items Segmented items (may be NULL when item_count is 0).
 * @param item_count Number of items.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_segmented_buttons_set_items(
    M3SegmentedButtons *buttons, const M3SegmentedItem *items,
    cmp_usize item_count);

/**
 * @brief Update the segmented style.
 * @param buttons Segmented button group.
 * @param style New style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_segmented_buttons_set_style(
    M3SegmentedButtons *buttons, const M3SegmentedStyle *style);

/**
 * @brief Update the selected index for single-select mode.
 * @param buttons Segmented button group.
 * @param selected_index Selected index or M3_SEGMENTED_INVALID_INDEX.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_segmented_buttons_set_selected_index(
    M3SegmentedButtons *buttons, cmp_usize selected_index);

/**
 * @brief Retrieve the selected index for single-select mode.
 * @param buttons Segmented button group.
 * @param out_selected Receives the selected index.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_segmented_buttons_get_selected_index(
    const M3SegmentedButtons *buttons, cmp_usize *out_selected);

/**
 * @brief Update the selected state for multi-select mode.
 * @param buttons Segmented button group.
 * @param index Segment index.
 * @param selected Selected state.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_segmented_buttons_set_selected_state(
    M3SegmentedButtons *buttons, cmp_usize index, CMPBool selected);

/**
 * @brief Retrieve the selected state for multi-select mode.
 * @param buttons Segmented button group.
 * @param index Segment index.
 * @param out_selected Receives the selected state.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_segmented_buttons_get_selected_state(
    const M3SegmentedButtons *buttons, cmp_usize index, CMPBool *out_selected);

/**
 * @brief Assign a selection callback for segmented buttons.
 * @param buttons Segmented button group.
 * @param on_select Selection callback (may be NULL).
 * @param ctx Callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_segmented_buttons_set_on_select(
    M3SegmentedButtons *buttons, CMPSegmentedOnSelect on_select, void *ctx);

CMP_API int CMP_CALL m3_tab_row_test_set_fail_point(cmp_u32 point);

CMP_API int CMP_CALL m3_tab_row_test_set_color_fail_after(cmp_u32 call_count);

CMP_API int CMP_CALL m3_tab_row_test_clear_fail_points(void);

CMP_API int CMP_CALL m3_segmented_test_set_fail_point(cmp_u32 point);

CMP_API int CMP_CALL m3_segmented_test_set_color_fail_after(cmp_u32 call_count);

CMP_API int CMP_CALL m3_segmented_test_clear_fail_points(void);

CMP_API int CMP_CALL m3_tab_row_test_validate_color(const CMPColor *color);

CMP_API int CMP_CALL m3_tab_row_test_color_set(CMPColor *color, CMPScalar r,
                                               CMPScalar g, CMPScalar b,
                                               CMPScalar a);

CMP_API int CMP_CALL m3_tab_row_test_color_with_alpha(const CMPColor *base,
                                                      CMPScalar alpha,
                                                      CMPColor *out_color);

CMP_API int CMP_CALL
m3_tab_row_test_validate_edges(const CMPLayoutEdges *edges);

CMP_API int CMP_CALL m3_tab_row_test_validate_text_style(
    const CMPTextStyle *style, CMPBool require_family);

CMP_API int CMP_CALL m3_tab_row_test_validate_style(const M3TabRowStyle *style,
                                                    CMPBool require_family);

CMP_API int CMP_CALL m3_tab_row_test_validate_items(const M3TabItem *items,
                                                    cmp_usize count);

CMP_API int CMP_CALL m3_tab_row_test_validate_measure_spec(CMPMeasureSpec spec);

CMP_API int CMP_CALL m3_tab_row_test_validate_rect(const CMPRect *rect);

CMP_API int CMP_CALL
m3_tab_row_test_validate_backend(const CMPTextBackend *backend);

CMP_API int CMP_CALL m3_tab_row_test_measure_content(const M3TabRow *row,
                                                     cmp_u32 mode,
                                                     CMPScalar *out_width,
                                                     CMPScalar *out_height);

CMP_API int CMP_CALL m3_tab_row_test_color_should_fail_null(void);

CMP_API int CMP_CALL m3_tab_row_test_fail_point_match_null(void);

CMP_API int CMP_CALL m3_tab_row_test_force_color_error(CMPBool enable);

CMP_API int CMP_CALL
m3_tab_row_test_set_fail_point_error_after(cmp_u32 call_count);

CMP_API int CMP_CALL m3_tab_row_test_set_value_fail_after(cmp_u32 call_count);

CMP_API int CMP_CALL m3_tab_row_test_set_start_fail_after(cmp_u32 call_count);

CMP_API int CMP_CALL m3_tab_row_test_measure_max_text(const M3TabRow *row,
                                                      CMPScalar *out_width,
                                                      CMPScalar *out_height,
                                                      CMPScalar *out_baseline);

CMP_API int CMP_CALL m3_tab_row_test_item_width(const M3TabRow *row,
                                                const M3TabItem *item,
                                                CMPScalar *out_width);

CMP_API int CMP_CALL m3_tab_row_test_compute_layout(
    const M3TabRow *row, CMPScalar *out_content_width, CMPScalar *out_tab_width,
    CMPScalar *out_tab_height, cmp_u32 *out_mode);

CMP_API int CMP_CALL
m3_tab_row_test_compute_layout_null_out(const M3TabRow *row);

CMP_API int CMP_CALL m3_tab_row_test_clamp_scroll(M3TabRow *row, cmp_u32 mode,
                                                  CMPScalar content_width,
                                                  CMPScalar available_width);

CMP_API int CMP_CALL m3_tab_row_test_clamp_scroll_null_layout(M3TabRow *row);

CMP_API int CMP_CALL m3_tab_row_test_indicator_target_null_layout(
    const M3TabRow *row, CMPScalar *out_pos, CMPScalar *out_width);

CMP_API int CMP_CALL
m3_tab_row_test_sync_indicator_null_layout(M3TabRow *row, CMPBool animate);

CMP_API int CMP_CALL m3_tab_row_test_item_rect_null_layout(const M3TabRow *row,
                                                           cmp_usize index,
                                                           CMPRect *out_rect);

CMP_API int CMP_CALL m3_tab_row_test_hit_test_null_layout(const M3TabRow *row,
                                                          cmp_i32 x, cmp_i32 y,
                                                          cmp_usize *out_index);

CMP_API int CMP_CALL m3_segmented_test_validate_color(const CMPColor *color);

CMP_API int CMP_CALL m3_segmented_test_color_set(CMPColor *color, CMPScalar r,
                                                 CMPScalar g, CMPScalar b,
                                                 CMPScalar a);

CMP_API int CMP_CALL m3_segmented_test_color_with_alpha(const CMPColor *base,
                                                        CMPScalar alpha,
                                                        CMPColor *out_color);

CMP_API int CMP_CALL
m3_segmented_test_validate_edges(const CMPLayoutEdges *edges);

CMP_API int CMP_CALL m3_segmented_test_validate_text_style(
    const CMPTextStyle *style, CMPBool require_family);

CMP_API int CMP_CALL m3_segmented_test_validate_style(
    const M3SegmentedStyle *style, CMPBool require_family);

CMP_API int CMP_CALL
m3_segmented_test_validate_items(const M3SegmentedItem *items, cmp_usize count);

CMP_API int CMP_CALL
m3_segmented_test_validate_measure_spec(CMPMeasureSpec spec);

CMP_API int CMP_CALL m3_segmented_test_validate_rect(const CMPRect *rect);

CMP_API int CMP_CALL
m3_segmented_test_validate_backend(const CMPTextBackend *backend);

CMP_API int CMP_CALL m3_segmented_test_validate_mode(cmp_u32 mode);

CMP_API int CMP_CALL m3_segmented_test_validate_selected_states(
    const CMPBool *states, cmp_usize count);

CMP_API int CMP_CALL
m3_segmented_test_measure_content(const M3SegmentedButtons *buttons,
                                  CMPScalar *out_width, CMPScalar *out_height);

CMP_API int CMP_CALL m3_segmented_test_color_should_fail_null(void);

CMP_API int CMP_CALL m3_segmented_test_fail_point_match_null(void);

CMP_API int CMP_CALL m3_segmented_test_force_color_error(CMPBool enable);

CMP_API int CMP_CALL
m3_segmented_test_set_fail_point_error_after(cmp_u32 call_count);

CMP_API int CMP_CALL m3_segmented_test_measure_max_text(
    const M3SegmentedButtons *buttons, CMPScalar *out_width,
    CMPScalar *out_height, CMPScalar *out_baseline);

CMP_API int CMP_CALL m3_segmented_test_compute_layout(
    const M3SegmentedButtons *buttons, CMPScalar *out_content_width,
    CMPScalar *out_segment_width, CMPScalar *out_segment_height,
    CMPScalar *out_spacing);

CMP_API int CMP_CALL
m3_segmented_test_compute_layout_null_out(const M3SegmentedButtons *buttons);

CMP_API int CMP_CALL m3_segmented_test_hit_test(
    const M3SegmentedButtons *buttons, CMPScalar start_x, CMPScalar start_y,
    CMPScalar segment_width, CMPScalar segment_height, CMPScalar spacing,
    CMPScalar content_width, cmp_i32 x, cmp_i32 y, cmp_usize *out_index);

CMP_API int CMP_CALL m3_segmented_test_hit_test_null_layout(
    const M3SegmentedButtons *buttons, cmp_i32 x, cmp_i32 y,
    cmp_usize *out_index);

CMP_API int CMP_CALL m3_segmented_test_is_selected(
    const M3SegmentedButtons *buttons, cmp_usize index, CMPBool *out_selected);

CMP_API int CMP_CALL m3_tab_row_test_indicator_target(
    const M3TabRow *row, cmp_u32 mode, CMPScalar tab_width, CMPScalar spacing,
    CMPScalar content_width, CMPScalar start_x, CMPScalar start_y,
    CMPScalar tab_height, CMPScalar *out_pos, CMPScalar *out_width);

CMP_API int CMP_CALL m3_tab_row_test_sync_indicator(
    M3TabRow *row, cmp_u32 mode, CMPScalar tab_width, CMPScalar spacing,
    CMPScalar content_width, CMPScalar start_x, CMPScalar start_y,
    CMPScalar tab_height, CMPBool animate);

CMP_API int CMP_CALL m3_tab_row_test_item_rect(
    const M3TabRow *row, cmp_u32 mode, CMPScalar start_x, CMPScalar start_y,
    CMPScalar tab_width, CMPScalar tab_height, CMPScalar spacing,
    CMPScalar content_width, CMPScalar available_width, cmp_usize index,
    CMPRect *out_rect);

CMP_API int CMP_CALL m3_tab_row_test_hit_test(
    const M3TabRow *row, cmp_u32 mode, CMPScalar start_x, CMPScalar start_y,
    CMPScalar tab_width, CMPScalar tab_height, CMPScalar spacing,
    CMPScalar content_width, CMPScalar available_width, cmp_i32 x, cmp_i32 y,
    cmp_usize *out_index);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_TABS_H */
