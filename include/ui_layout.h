/**
 * @file ui_layout.h
 * @brief CSS Layout Engine structures and constants.
 *
 * This header declares structures and types representing the layout tree and
 * geometric box model (CSS Display Module, Flexible Box Layout, CSS Grid,
 * etc.).
 */

#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_dom_node.h"
#include "ui_cssom.h"
#include "ui_css_values.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Represents the box model edge indices.
 */
enum ui_box_edge {
  UI_BOX_EDGE_TOP = 0,    /**< Top edge index. */
  UI_BOX_EDGE_RIGHT = 1,  /**< Right edge index. */
  UI_BOX_EDGE_BOTTOM = 2, /**< Bottom edge index. */
  UI_BOX_EDGE_LEFT = 3    /**< Left edge index. */
};

/**
 * @brief Represents the outside display type of a layout node (CSS Display
 * Level 4).
 */
enum ui_layout_display_outside {
  UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK,  /**< Block level display. */
  UI_LAYOUT_DISPLAY_OUTSIDE_INLINE, /**< Inline level display. */
  UI_LAYOUT_DISPLAY_OUTSIDE_RUN_IN, /**< Run-in level display. */
  UI_LAYOUT_DISPLAY_OUTSIDE_NONE    /**< Display none (element is removed from
                                       tree). */
};

/**
 * @brief Represents the inside display type of a layout node (CSS Display Level
 * 4).
 */
enum ui_layout_display_inside {
  UI_LAYOUT_DISPLAY_INSIDE_FLOW,               /**< Flow layout. */
  UI_LAYOUT_DISPLAY_INSIDE_FLOW_ROOT,          /**< Flow-root layout. */
  UI_LAYOUT_DISPLAY_INSIDE_TABLE,              /**< Table layout. */
  UI_LAYOUT_DISPLAY_INSIDE_TABLE_ROW_GROUP,    /**< Table row group layout. */
  UI_LAYOUT_DISPLAY_INSIDE_TABLE_HEADER_GROUP, /**< Table header group layout.
                                                */
  UI_LAYOUT_DISPLAY_INSIDE_TABLE_FOOTER_GROUP, /**< Table footer group layout.
                                                */
  UI_LAYOUT_DISPLAY_INSIDE_TABLE_ROW,          /**< Table row layout. */
  UI_LAYOUT_DISPLAY_INSIDE_TABLE_CELL,         /**< Table cell layout. */
  UI_LAYOUT_DISPLAY_INSIDE_FLEX,               /**< Flexbox layout. */
  UI_LAYOUT_DISPLAY_INSIDE_GRID,               /**< Grid layout. */
  UI_LAYOUT_DISPLAY_INSIDE_RUBY,               /**< Ruby layout. */
  UI_LAYOUT_DISPLAY_INSIDE_RUBY_BASE,          /**< Ruby base layout. */
  UI_LAYOUT_DISPLAY_INSIDE_RUBY_TEXT           /**< Ruby text layout. */
};

/**
 * @brief Represents the break behavior (CSS Fragmentation Module Level 3).
 */
enum ui_layout_break {
  UI_LAYOUT_BREAK_AUTO,         /**< Automatic break behavior. */
  UI_LAYOUT_BREAK_AVOID,        /**< Avoid breaks. */
  UI_LAYOUT_BREAK_ALWAYS,       /**< Always break. */
  UI_LAYOUT_BREAK_ALL,          /**< Break all. */
  UI_LAYOUT_BREAK_AVOID_PAGE,   /**< Avoid page breaks. */
  UI_LAYOUT_BREAK_PAGE,         /**< Page break. */
  UI_LAYOUT_BREAK_LEFT,         /**< Left page break. */
  UI_LAYOUT_BREAK_RIGHT,        /**< Right page break. */
  UI_LAYOUT_BREAK_RECTO,        /**< Recto page break. */
  UI_LAYOUT_BREAK_VERSO,        /**< Verso page break. */
  UI_LAYOUT_BREAK_AVOID_COLUMN, /**< Avoid column breaks. */
  UI_LAYOUT_BREAK_COLUMN,       /**< Column break. */
  UI_LAYOUT_BREAK_AVOID_REGION, /**< Avoid region breaks. */
  UI_LAYOUT_BREAK_REGION        /**< Region break. */
};

/**
 * @brief Represents the overflow behavior of a layout node.
 */
enum ui_layout_overflow {
  UI_LAYOUT_OVERFLOW_VISIBLE, /**< Visible overflow. */
  UI_LAYOUT_OVERFLOW_HIDDEN,  /**< Hidden overflow. */
  UI_LAYOUT_OVERFLOW_SCROLL,  /**< Scroll overflow. */
  UI_LAYOUT_OVERFLOW_AUTO     /**< Auto overflow. */
};

/**
 * @brief Represents the box alignment values (CSS Box Alignment Module Level
 * 3).
 */
enum ui_layout_alignment {
  UI_LAYOUT_ALIGN_AUTO,          /**< Auto alignment. */
  UI_LAYOUT_ALIGN_NORMAL,        /**< Normal alignment. */
  UI_LAYOUT_ALIGN_START,         /**< Start alignment. */
  UI_LAYOUT_ALIGN_END,           /**< End alignment. */
  UI_LAYOUT_ALIGN_CENTER,        /**< Center alignment. */
  UI_LAYOUT_ALIGN_STRETCH,       /**< Stretch alignment. */
  UI_LAYOUT_ALIGN_SPACE_BETWEEN, /**< Space-between alignment. */
  UI_LAYOUT_ALIGN_SPACE_AROUND,  /**< Space-around alignment. */
  UI_LAYOUT_ALIGN_SPACE_EVENLY   /**< Space-evenly alignment. */
};

/**
 * @brief Represents margin-trim behaviors (CSS Box Model Module Level 4).
 */
enum ui_layout_margin_trim {
  UI_LAYOUT_MARGIN_TRIM_NONE = 0,         /**< No margin trimming. */
  UI_LAYOUT_MARGIN_TRIM_BLOCK = 1,        /**< Trim block margins. */
  UI_LAYOUT_MARGIN_TRIM_BLOCK_START = 2,  /**< Trim block-start margin. */
  UI_LAYOUT_MARGIN_TRIM_BLOCK_END = 3,    /**< Trim block-end margin. */
  UI_LAYOUT_MARGIN_TRIM_INLINE = 4,       /**< Trim inline margins. */
  UI_LAYOUT_MARGIN_TRIM_INLINE_START = 5, /**< Trim inline-start margin. */
  UI_LAYOUT_MARGIN_TRIM_INLINE_END = 6,   /**< Trim inline-end margin. */
  UI_LAYOUT_MARGIN_TRIM_ALL = 7           /**< Trim all margins. */
};

/**
 * @brief Represents the wrap-flow behavior (CSS Exclusions Module Level 1).
 */
enum ui_layout_wrap_flow {
  UI_LAYOUT_WRAP_FLOW_AUTO,    /**< Auto wrap flow. */
  UI_LAYOUT_WRAP_FLOW_BOTH,    /**< Wrap flow both. */
  UI_LAYOUT_WRAP_FLOW_START,   /**< Wrap flow start. */
  UI_LAYOUT_WRAP_FLOW_END,     /**< Wrap flow end. */
  UI_LAYOUT_WRAP_FLOW_MINIMUM, /**< Wrap flow minimum. */
  UI_LAYOUT_WRAP_FLOW_MAXIMUM, /**< Wrap flow maximum. */
  UI_LAYOUT_WRAP_FLOW_CLEAR    /**< Wrap flow clear. */
};

/**
 * @brief Represents the wrap-through behavior (CSS Exclusions Module Level 1).
 */
enum ui_layout_wrap_through {
  UI_LAYOUT_WRAP_THROUGH_WRAP, /**< Wrap through elements. */
  UI_LAYOUT_WRAP_THROUGH_NONE  /**< Do not wrap through elements. */
};

/**
 * @brief Represents white-space behavior (CSS Text Module Level 3).
 */
enum ui_layout_white_space {
  UI_LAYOUT_WHITE_SPACE_NORMAL,      /**< Normal white-space. */
  UI_LAYOUT_WHITE_SPACE_NOWRAP,      /**< No-wrap white-space. */
  UI_LAYOUT_WHITE_SPACE_PRE,         /**< Pre white-space. */
  UI_LAYOUT_WHITE_SPACE_PRE_WRAP,    /**< Pre-wrap white-space. */
  UI_LAYOUT_WHITE_SPACE_PRE_LINE,    /**< Pre-line white-space. */
  UI_LAYOUT_WHITE_SPACE_BREAK_SPACES /**< Break-spaces white-space. */
};

/**
 * @brief Represents text-align behavior (CSS Text Module Level 3).
 */
enum ui_layout_text_align {
  UI_LAYOUT_TEXT_ALIGN_START,        /**< Start alignment. */
  UI_LAYOUT_TEXT_ALIGN_END,          /**< End alignment. */
  UI_LAYOUT_TEXT_ALIGN_LEFT,         /**< Left alignment. */
  UI_LAYOUT_TEXT_ALIGN_RIGHT,        /**< Right alignment. */
  UI_LAYOUT_TEXT_ALIGN_CENTER,       /**< Center alignment. */
  UI_LAYOUT_TEXT_ALIGN_JUSTIFY,      /**< Justify alignment. */
  UI_LAYOUT_TEXT_ALIGN_MATCH_PARENT, /**< Match-parent alignment. */
  UI_LAYOUT_TEXT_ALIGN_JUSTIFY_ALL   /**< Justify-all alignment. */
};

/**
 * @brief Represents word-break behavior (CSS Text Module Level 3).
 */
enum ui_layout_word_break {
  UI_LAYOUT_WORD_BREAK_NORMAL,    /**< Normal word break. */
  UI_LAYOUT_WORD_BREAK_KEEP_ALL,  /**< Keep all word break. */
  UI_LAYOUT_WORD_BREAK_BREAK_ALL, /**< Break all word break. */
  UI_LAYOUT_WORD_BREAK_BREAK_WORD /**< Break word. */
};

/**
 * @brief Represents hyphens behavior (CSS Text Module Level 3).
 */
enum ui_layout_hyphens {
  UI_LAYOUT_HYPHENS_NONE,   /**< No hyphens. */
  UI_LAYOUT_HYPHENS_MANUAL, /**< Manual hyphens. */
  UI_LAYOUT_HYPHENS_AUTO    /**< Auto hyphens. */
};

/**
 * @brief Represents writing mode (CSS Writing Modes Level 3/4).
 */
enum ui_layout_writing_mode {
  UI_LAYOUT_WRITING_MODE_HORIZONTAL_TB, /**< Horizontal top-to-bottom. */
  UI_LAYOUT_WRITING_MODE_VERTICAL_RL,   /**< Vertical right-to-left. */
  UI_LAYOUT_WRITING_MODE_VERTICAL_LR    /**< Vertical left-to-right. */
};

/**
 * @brief Represents direction (CSS Writing Modes Level 3/4).
 */
enum ui_layout_direction {
  UI_LAYOUT_DIRECTION_LTR, /**< Left to right. */
  UI_LAYOUT_DIRECTION_RTL  /**< Right to left. */
};

/**
 * @brief Represents unicode-bidi behavior.
 */
enum ui_layout_unicode_bidi {
  UI_LAYOUT_UNICODE_BIDI_NORMAL,           /**< Normal. */
  UI_LAYOUT_UNICODE_BIDI_EMBED,            /**< Embed. */
  UI_LAYOUT_UNICODE_BIDI_BIDI_OVERRIDE,    /**< Bidi override. */
  UI_LAYOUT_UNICODE_BIDI_ISOLATE,          /**< Isolate. */
  UI_LAYOUT_UNICODE_BIDI_ISOLATE_OVERRIDE, /**< Isolate override. */
  UI_LAYOUT_UNICODE_BIDI_PLAINTEXT         /**< Plaintext. */
};

/**
 * @brief Represents text orientation.
 */
enum ui_layout_text_orientation {
  UI_LAYOUT_TEXT_ORIENTATION_MIXED,   /**< Mixed orientation. */
  UI_LAYOUT_TEXT_ORIENTATION_UPRIGHT, /**< Upright orientation. */
  UI_LAYOUT_TEXT_ORIENTATION_SIDEWAYS /**< Sideways orientation. */
};

/**
 * @brief Represents line grid creation (CSS Line Grid).
 */
enum ui_layout_line_grid {
  UI_LAYOUT_LINE_GRID_MATCH_PARENT, /**< Match parent grid. */
  UI_LAYOUT_LINE_GRID_CREATE        /**< Create new grid. */
};

/**
 * @brief Represents line snap (CSS Line Grid).
 */
enum ui_layout_line_snap {
  UI_LAYOUT_LINE_SNAP_NONE,     /**< No snapping. */
  UI_LAYOUT_LINE_SNAP_BASELINE, /**< Snap to baseline. */
  UI_LAYOUT_LINE_SNAP_CONTAIN   /**< Snap to contain. */
};

/**
 * @brief Represents box snap (CSS Line Grid).
 */
enum ui_layout_box_snap {
  UI_LAYOUT_BOX_SNAP_NONE,        /**< No snap. */
  UI_LAYOUT_BOX_SNAP_BLOCK_START, /**< Snap block start. */
  UI_LAYOUT_BOX_SNAP_BLOCK_END,   /**< Snap block end. */
  UI_LAYOUT_BOX_SNAP_CENTER,      /**< Snap center. */
  UI_LAYOUT_BOX_SNAP_BASELINE     /**< Snap baseline. */
};

/**
 * @brief Represents block step insert (CSS Rhythmic Sizing).
 */
enum ui_layout_block_step_insert {
  UI_LAYOUT_BLOCK_STEP_INSERT_MARGIN, /**< Insert margin. */
  UI_LAYOUT_BLOCK_STEP_INSERT_PADDING /**< Insert padding. */
};

/**
 * @brief Represents block step align (CSS Rhythmic Sizing).
 */
enum ui_layout_block_step_align {
  UI_LAYOUT_BLOCK_STEP_ALIGN_AUTO,   /**< Auto alignment. */
  UI_LAYOUT_BLOCK_STEP_ALIGN_CENTER, /**< Center alignment. */
  UI_LAYOUT_BLOCK_STEP_ALIGN_START,  /**< Start alignment. */
  UI_LAYOUT_BLOCK_STEP_ALIGN_END     /**< End alignment. */
};

/**
 * @brief Represents block step round (CSS Rhythmic Sizing).
 */
enum ui_layout_block_step_round {
  UI_LAYOUT_BLOCK_STEP_ROUND_UP,     /**< Round up. */
  UI_LAYOUT_BLOCK_STEP_ROUND_DOWN,   /**< Round down. */
  UI_LAYOUT_BLOCK_STEP_ROUND_NEAREST /**< Round to nearest. */
};

/**
 * @brief Represents text-decoration-line behavior (CSS Text Decoration Module
 * Level 3).
 */
enum ui_layout_text_decoration_line {
  UI_LAYOUT_TEXT_DECORATION_LINE_NONE = 0,      /**< No text decoration line. */
  UI_LAYOUT_TEXT_DECORATION_LINE_UNDERLINE = 1, /**< Underline. */
  UI_LAYOUT_TEXT_DECORATION_LINE_OVERLINE = 2,  /**< Overline. */
  UI_LAYOUT_TEXT_DECORATION_LINE_LINE_THROUGH = 4, /**< Line-through. */
  UI_LAYOUT_TEXT_DECORATION_LINE_BLINK = 8         /**< Blink (deprecated). */
};

/**
 * @brief Represents text-decoration-style behavior (CSS Text Decoration Module
 * Level 3).
 */
enum ui_layout_text_decoration_style {
  UI_LAYOUT_TEXT_DECORATION_STYLE_SOLID,  /**< Solid line. */
  UI_LAYOUT_TEXT_DECORATION_STYLE_DOUBLE, /**< Double line. */
  UI_LAYOUT_TEXT_DECORATION_STYLE_DOTTED, /**< Dotted line. */
  UI_LAYOUT_TEXT_DECORATION_STYLE_DASHED, /**< Dashed line. */
  UI_LAYOUT_TEXT_DECORATION_STYLE_WAVY    /**< Wavy line. */
};

/**
 * @brief Represents font-style behavior (CSS Fonts Module Level 3/4).
 */
enum ui_layout_font_style {
  UI_LAYOUT_FONT_STYLE_NORMAL, /**< Normal font style. */
  UI_LAYOUT_FONT_STYLE_ITALIC, /**< Italic font style. */
  UI_LAYOUT_FONT_STYLE_OBLIQUE /**< Oblique font style. */
};

/**
 * @brief Represents text-size-adjust types.
 */
enum ui_layout_text_size_adjust_type {
  UI_LAYOUT_TEXT_SIZE_ADJUST_AUTO,      /**< Auto size adjust. */
  UI_LAYOUT_TEXT_SIZE_ADJUST_NONE,      /**< No size adjust. */
  UI_LAYOUT_TEXT_SIZE_ADJUST_PERCENTAGE /**< Percentage size adjust. */
};

/**
 * @brief Structure for text-size-adjust.
 */
struct ui_layout_text_size_adjust {
  enum ui_layout_text_size_adjust_type type; /**< Type of adjustment. */
  float percentage;                          /**< Adjustment percentage. */
};

/**
 * @brief Represents background-size type (CSS Backgrounds and Borders Module).
 */
enum ui_layout_background_size_type {
  UI_LAYOUT_BACKGROUND_SIZE_AUTO,    /**< Auto background size. */
  UI_LAYOUT_BACKGROUND_SIZE_COVER,   /**< Cover background size. */
  UI_LAYOUT_BACKGROUND_SIZE_CONTAIN, /**< Contain background size. */
  UI_LAYOUT_BACKGROUND_SIZE_LENGTH   /**< Length background size. */
};

/**
 * @brief Structure for background-size.
 */
struct ui_layout_background_size {
  enum ui_layout_background_size_type type_x; /**< Type for X axis. */
  enum ui_layout_background_size_type type_y; /**< Type for Y axis. */
  float x;                                    /**< Value for X axis. */
  float y;                                    /**< Value for Y axis. */
};

/**
 * @brief Represents background-repeat type.
 */
enum ui_layout_background_repeat {
  UI_LAYOUT_BACKGROUND_REPEAT_REPEAT,   /**< Repeat background. */
  UI_LAYOUT_BACKGROUND_REPEAT_SPACE,    /**< Space background. */
  UI_LAYOUT_BACKGROUND_REPEAT_ROUND,    /**< Round background. */
  UI_LAYOUT_BACKGROUND_REPEAT_NO_REPEAT /**< No-repeat background. */
};

/**
 * @brief Represents color-scheme (CSS Color Adjustment Module Level 1).
 */
enum ui_layout_color_scheme {
  UI_LAYOUT_COLOR_SCHEME_NORMAL = 0,    /**< Normal color scheme. */
  UI_LAYOUT_COLOR_SCHEME_LIGHT = 1,     /**< Light color scheme. */
  UI_LAYOUT_COLOR_SCHEME_DARK = 2,      /**< Dark color scheme. */
  UI_LAYOUT_COLOR_SCHEME_LIGHT_DARK = 3 /**< Light and dark color scheme. */
};

/**
 * @brief Represents box-decoration-break (CSS Borders and Box Decorations
 * Module Level 4).
 */
enum ui_layout_box_decoration_break {
  UI_LAYOUT_BOX_DECORATION_BREAK_SLICE, /**< Slice decoration break. */
  UI_LAYOUT_BOX_DECORATION_BREAK_CLONE  /**< Clone decoration break. */
};

/**
 * @brief Represents a single box-shadow (CSS Borders and Box Decorations Module
 * Level 4).
 */
struct ui_layout_box_shadow {
  float offset_x;            /**< Horizontal shadow offset. */
  float offset_y;            /**< Vertical shadow offset. */
  float blur_radius;         /**< Shadow blur radius. */
  float spread_radius;       /**< Shadow spread radius. */
  struct ui_css_color color; /**< Shadow color. */
  int is_inset;              /**< 1 if inset shadow, 0 otherwise. */
};

/**
 * @brief Represents print-color-adjust (CSS Color Adjustment Module Level 1).
 */
enum ui_layout_print_color_adjust {
  UI_LAYOUT_PRINT_COLOR_ADJUST_ECONOMY, /**< Economy mode for printing. */
  UI_LAYOUT_PRINT_COLOR_ADJUST_EXACT    /**< Exact mode for printing. */
};

/**
 * @brief Represents forced-color-adjust (CSS Color Adjustment Module Level 1).
 */
enum ui_layout_forced_color_adjust {
  UI_LAYOUT_FORCED_COLOR_ADJUST_AUTO, /**< Auto forced color adjust. */
  UI_LAYOUT_FORCED_COLOR_ADJUST_NONE, /**< None forced color adjust. */
  UI_LAYOUT_FORCED_COLOR_ADJUST_PRESERVE_PARENT_COLOR /**< Preserve parent color
                                                         forced color adjust. */
};

/**
 * @brief Represents font-stretch behavior (CSS Fonts Module Level 3/4).
 */
enum ui_layout_font_stretch {
  UI_LAYOUT_FONT_STRETCH_NORMAL,          /**< Normal font stretch. */
  UI_LAYOUT_FONT_STRETCH_ULTRA_CONDENSED, /**< Ultra condensed font stretch. */
  UI_LAYOUT_FONT_STRETCH_EXTRA_CONDENSED, /**< Extra condensed font stretch. */
  UI_LAYOUT_FONT_STRETCH_CONDENSED,       /**< Condensed font stretch. */
  UI_LAYOUT_FONT_STRETCH_SEMI_CONDENSED,  /**< Semi condensed font stretch. */
  UI_LAYOUT_FONT_STRETCH_SEMI_EXPANDED,   /**< Semi expanded font stretch. */
  UI_LAYOUT_FONT_STRETCH_EXPANDED,        /**< Expanded font stretch. */
  UI_LAYOUT_FONT_STRETCH_EXTRA_EXPANDED,  /**< Extra expanded font stretch. */
  UI_LAYOUT_FONT_STRETCH_ULTRA_EXPANDED   /**< Ultra expanded font stretch. */
};

/**
 * @brief Represents the flex direction (CSS Flexible Box Layout Module Level
 * 1).
 */
enum ui_layout_flex_direction {
  UI_LAYOUT_FLEX_DIRECTION_ROW,           /**< Flex row direction. */
  UI_LAYOUT_FLEX_DIRECTION_ROW_REVERSE,   /**< Flex row reverse direction. */
  UI_LAYOUT_FLEX_DIRECTION_COLUMN,        /**< Flex column direction. */
  UI_LAYOUT_FLEX_DIRECTION_COLUMN_REVERSE /**< Flex column reverse direction. */
};

/**
 * @brief Represents the flex wrap behavior (CSS Flexible Box Layout Module
 * Level 1).
 */
enum ui_layout_flex_wrap {
  UI_LAYOUT_FLEX_WRAP_NOWRAP,      /**< Flex no-wrap. */
  UI_LAYOUT_FLEX_WRAP_WRAP,        /**< Flex wrap. */
  UI_LAYOUT_FLEX_WRAP_WRAP_REVERSE /**< Flex wrap reverse. */
};

/**
 * @brief Represents the size determination type for a dimension.
 */
enum ui_layout_size_type {
  UI_LAYOUT_SIZE_PIXELS,      /**< Size in pixels. */
  UI_LAYOUT_SIZE_PERCENTAGE,  /**< Size in percentage. */
  UI_LAYOUT_SIZE_AUTO,        /**< Auto size. */
  UI_LAYOUT_SIZE_MIN_CONTENT, /**< Min-content size. */
  UI_LAYOUT_SIZE_MAX_CONTENT, /**< Max-content size. */
  UI_LAYOUT_SIZE_FIT_CONTENT, /**< Fit-content size. */
  UI_LAYOUT_SIZE_FR,          /**< Fraction size (grid). */
  UI_LAYOUT_SIZE_SUBGRID,     /**< Subgrid size. */
  UI_LAYOUT_SIZE_MASONRY,     /**< Masonry size. */
  UI_LAYOUT_SIZE_ANCHOR       /**< Anchor size. */
};

#define UI_LAYOUT_MAX_GRID_TRACKS                                              \
  16 /**< Maximum number of grid tracks allowed. */

/**
 * @brief Represents a parsed CSS grid track.
 */
struct ui_layout_grid_track {
  enum ui_layout_size_type type; /**< Track size type. */
  float value;                   /**< Track size value. */
};

/**
 * @brief Represents the position type (CSS Positioned Layout Module Level 3).
 */
enum ui_layout_position {
  UI_LAYOUT_POSITION_STATIC,   /**< Static position. */
  UI_LAYOUT_POSITION_RELATIVE, /**< Relative position. */
  UI_LAYOUT_POSITION_ABSOLUTE, /**< Absolute position. */
  UI_LAYOUT_POSITION_FIXED,    /**< Fixed position. */
  UI_LAYOUT_POSITION_STICKY    /**< Sticky position. */
};

/**
 * @brief Represents the side of an anchor to tether to.
 */
enum ui_layout_anchor_side {
  UI_LAYOUT_ANCHOR_SIDE_NONE,   /**< No anchor side. */
  UI_LAYOUT_ANCHOR_SIDE_TOP,    /**< Top anchor side. */
  UI_LAYOUT_ANCHOR_SIDE_RIGHT,  /**< Right anchor side. */
  UI_LAYOUT_ANCHOR_SIDE_BOTTOM, /**< Bottom anchor side. */
  UI_LAYOUT_ANCHOR_SIDE_LEFT,   /**< Left anchor side. */
  UI_LAYOUT_ANCHOR_SIDE_CENTER  /**< Center anchor side. */
};

/**
 * @brief Represents a reference to an anchor side.
 */
struct ui_layout_anchor_ref {
  char name[64];                   /**< Name of the referenced anchor. */
  enum ui_layout_anchor_side side; /**< Side of the referenced anchor. */
};

/**
 * @brief Represents a box in the layout tree.
 */
struct ui_layout_node {
  float opacity;           /**< Opacity of the layout node. */
  char transform[64];      /**< CSS transform property. */
  int is_stacking_context; /**< 1 if node forms a stacking context, 0 otherwise.
                            */
  const struct ui_dom_node
      *dom_node; /**< Original DOM node. NULL for anonymous boxes. */
  struct ui_css_computed_style
      *computed_style; /**< Resolved style for this node. */

  int is_anonymous; /**< 1 if this is an anonymous block/inline box, 0
                       otherwise. */
  enum ui_layout_display_outside
      display_outside; /**< The outside display type. */
  enum ui_layout_display_inside display_inside; /**< The inside display type. */

  enum ui_layout_overflow overflow_x; /**< Overflow behavior on X axis. */
  enum ui_layout_overflow overflow_y; /**< Overflow behavior on Y axis. */

  enum ui_layout_break break_before; /**< Page break before. */
  enum ui_layout_break break_after;  /**< Page break after. */
  enum ui_layout_break break_inside; /**< Page break inside. */
  int orphans;                       /**< Orphans rule. */
  int widows;                        /**< Widows rule. */

  enum ui_layout_alignment justify_content; /**< Justify content rule. */
  enum ui_layout_alignment align_items;     /**< Align items rule. */
  enum ui_layout_alignment align_self;      /**< Align self rule. */
  enum ui_layout_alignment align_content;   /**< Align content rule. */

  enum ui_layout_flex_direction flex_direction; /**< Flex direction. */
  enum ui_layout_flex_wrap flex_wrap;           /**< Flex wrap. */
  float flex_grow;                              /**< Flex grow value. */
  float flex_shrink;                            /**< Flex shrink value. */
  float flex_basis;                             /**< Flex basis value. */
  enum ui_layout_size_type flex_basis_type;     /**< Flex basis type. */

  struct ui_layout_grid_track
      grid_template_columns[UI_LAYOUT_MAX_GRID_TRACKS]; /**< Array of grid
                                                           column tracks. */
  int grid_template_columns_count; /**< Number of grid column tracks. */
  struct ui_layout_grid_track
      grid_template_rows[UI_LAYOUT_MAX_GRID_TRACKS]; /**< Array of grid row
                                                        tracks. */
  int grid_template_rows_count; /**< Number of grid row tracks. */
  int grid_column_start;        /**< Grid column start index. */
  int grid_column_span;         /**< Grid column span. */
  int grid_row_start;           /**< Grid row start index. */
  int grid_row_span;            /**< Grid row span. */

  float subgrid_track_widths[UI_LAYOUT_MAX_GRID_TRACKS];  /**< Precomputed
                                                             subgrid widths. */
  float subgrid_track_heights[UI_LAYOUT_MAX_GRID_TRACKS]; /**< Precomputed
                                                             subgrid heights. */
  int subgrid_columns_count; /**< Subgrid columns count. */
  int subgrid_rows_count;    /**< Subgrid rows count. */

  enum ui_layout_size_type width_type;  /**< Type of the width. */
  enum ui_layout_size_type height_type; /**< Type of the height. */
  float raw_width;                      /**< Raw width value. */
  float raw_height;                     /**< Raw height value. */

  enum ui_layout_margin_trim margin_trim; /**< Margin trim setting. */

  int column_count;   /**< Column count (0 means auto). */
  float column_width; /**< Column width (0.0f means auto). */

  float row_gap;    /**< Gap between rows. */
  float column_gap; /**< Gap between columns. */

  enum ui_layout_position position;          /**< Layout position mode. */
  char anchor_name[64];                      /**< Anchor name. */
  char position_anchor[64];                  /**< Position anchor target. */
  struct ui_layout_anchor_ref top_anchor;    /**< Top anchor reference. */
  struct ui_layout_anchor_ref right_anchor;  /**< Right anchor reference. */
  struct ui_layout_anchor_ref bottom_anchor; /**< Bottom anchor reference. */
  struct ui_layout_anchor_ref left_anchor;   /**< Left anchor reference. */
  float top;                                 /**< Position top. */
  float right;                               /**< Position right. */
  float bottom;                              /**< Position bottom. */
  float left;                                /**< Position left. */
  enum ui_layout_size_type top_type;         /**< Type of top coordinate. */
  enum ui_layout_size_type right_type;       /**< Type of right coordinate. */
  enum ui_layout_size_type bottom_type;      /**< Type of bottom coordinate. */
  enum ui_layout_size_type left_type;        /**< Type of left coordinate. */
  int z_index;                               /**< Z-index. */

  enum ui_layout_wrap_flow wrap_flow;       /**< Wrap flow setting. */
  enum ui_layout_wrap_through wrap_through; /**< Wrap through setting. */

  char flow_into[64]; /**< Flow-into region. */
  char flow_from[64]; /**< Flow-from region. */

  enum ui_layout_white_space white_space; /**< White space behavior. */
  enum ui_layout_text_align text_align;   /**< Text alignment. */
  enum ui_layout_word_break word_break;   /**< Word break behavior. */
  enum ui_layout_hyphens hyphens;         /**< Hyphens setting. */

  enum ui_layout_writing_mode writing_mode; /**< Writing mode. */
  enum ui_layout_direction direction;       /**< Text direction. */
  enum ui_layout_unicode_bidi unicode_bidi; /**< Unicode bidi setting. */
  enum ui_layout_text_orientation text_orientation; /**< Text orientation. */

  enum ui_layout_line_grid line_grid; /**< Line grid. */
  enum ui_layout_line_snap line_snap; /**< Line snap. */
  enum ui_layout_box_snap box_snap;   /**< Box snap. */
  float block_step_size;              /**< Block step size (0.0f means none). */
  enum ui_layout_block_step_insert
      block_step_insert; /**< Block step insert rule. */
  enum ui_layout_block_step_align
      block_step_align; /**< Block step align rule. */
  enum ui_layout_block_step_round
      block_step_round; /**< Block step round rule. */

  int text_decoration_line; /**< Text decoration line flags. */
  enum ui_layout_text_decoration_style
      text_decoration_style;                 /**< Text decoration style. */
  struct ui_css_color text_decoration_color; /**< Text decoration color. */
  char text_shadow[128];                     /**< Text shadow. */
  char text_emphasis_style[64];              /**< Text emphasis style. */
  struct ui_css_color text_emphasis_color;   /**< Text emphasis color. */
  char text_emphasis_position[64];           /**< Text emphasis position. */

  char font_family[128];                    /**< Font family string. */
  int font_weight;                          /**< Font weight integer. */
  enum ui_layout_font_style font_style;     /**< Font style. */
  float font_size;                          /**< Font size. */
  enum ui_layout_font_stretch font_stretch; /**< Font stretch. */
  struct ui_layout_text_size_adjust
      text_size_adjust;             /**< Text size adjustment. */
  char font_variant[64];            /**< Font variant string. */
  char font_feature_settings[64];   /**< Font feature settings string. */
  char font_variation_settings[64]; /**< Font variation settings string. */

  struct ui_css_color text_color;          /**< The text color. */
  struct ui_css_color background_color;    /**< The background color. */
  struct ui_css_image background_image[4]; /**< Multiple backgrounds. */
  struct ui_layout_background_size background_size[4]; /**< Background sizes. */
  enum ui_layout_background_repeat
      background_repeat_x[4]; /**< Background repeat on X axis. */
  enum ui_layout_background_repeat
      background_repeat_y[4];          /**< Background repeat on Y axis. */
  int background_image_count;          /**< Background image count. */
  struct ui_css_color border_color[4]; /**< Border colors for TRBL edges. */
  float border_radius[4][2];           /**< Border radii (top-left, top-right,
                                          bottom-right, bottom-left). */

  enum ui_layout_box_decoration_break
      box_decoration_break; /**< Box decoration break behavior. */
  struct ui_layout_box_shadow
      box_shadow[4];    /**< Box shadows (up to 4 supported). */
  int box_shadow_count; /**< Count of active box shadows. */
  struct ui_css_image border_image_source; /**< Border image source. */

  enum ui_layout_color_scheme color_scheme; /**< Color scheme. */
  enum ui_layout_print_color_adjust
      print_color_adjust; /**< Print color adjust behavior. */
  enum ui_layout_forced_color_adjust
      forced_color_adjust; /**< Forced color adjust behavior. */

  /* Geometry (to be computed by layout algorithms later) */
  float x;      /**< Computed relative X position. */
  float y;      /**< Computed relative Y position. */
  float width;  /**< Computed width. */
  float height; /**< Computed height. */
  float abs_x;  /**< Computed absolute X position in document. */
  float abs_y;  /**< Computed absolute Y position in document. */

  /* CSS Box Model Metrics (Level 3 & 4) */
  float margin[4];      /**< Computed margins (TRBL). */
  float border[4];      /**< Computed borders (TRBL). */
  float padding[4];     /**< Computed paddings (TRBL). */
  float content_width;  /**< Computed content inner width. */
  float content_height; /**< Computed content inner height. */

  float min_width;  /**< Computed minimum width. */
  float max_width;  /**< Computed maximum width. */
  float min_height; /**< Computed minimum height. */
  float max_height; /**< Computed maximum height. */

  float aspect_ratio; /**< Aspect ratio (0.0 means auto). */

  float scrollbar_width;  /**< Width occupied by a scrollbar, if applicable. */
  float scrollbar_height; /**< Height occupied by a scrollbar, if applicable. */

  /* Tree structure */
  struct ui_layout_node *parent; /**< Pointer to the parent layout node. */
  struct ui_layout_node
      *first_child; /**< Pointer to the first child layout node. */
  struct ui_layout_node
      *last_child; /**< Pointer to the last child layout node. */
  struct ui_layout_node
      *next_sibling; /**< Pointer to the next sibling layout node. */
  struct ui_layout_node
      *previous_sibling; /**< Pointer to the previous sibling layout node. */
};

/**
 * @brief Generates a layout tree from a DOM tree and a stylesheet.
 *
 * This function resolves styles, skips nodes with "display: none",
 * and allocates a separate tree of layout nodes.
 *
 * @param dom_root Pointer to the root of the DOM tree.
 * @param stylesheet Pointer to the stylesheet to apply.
 * @param out_layout_root Pointer to receive the generated layout tree.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_layout_tree_generate(const struct ui_dom_node *dom_root,
                                   const struct ui_css_stylesheet *stylesheet,
                                   struct ui_layout_node **out_layout_root);

/**
 * @brief Viewport Root Solver: Forces the root html/body elements to strictly
 * match OS window dimensions.
 *
 * @param root Pointer to the layout tree root.
 * @param window_width The OS window width.
 * @param window_height The OS window height.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_layout_solve_viewport(struct ui_layout_node *root,
                                    float window_width, float window_height);

/**
 * @brief Computes the final geometric layout (x, y, width, height) recursively.
 *
 * @param node Pointer to the layout tree root.
 * @param available_width The viewport or containing block width.
 * @param available_height The viewport or containing block height.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_layout_compute(struct ui_layout_node *node, float available_width,
                             float available_height);

/**
 * @brief Containment Sanity Checks: Traverses the tree and asserts that
 * children do not illegally breach parent bounds.
 *
 * @param node Pointer to the layout tree root to verify.
 * @return `UI_ERROR_NONE` if valid, `UI_ERROR_LAYOUT_VIOLATION` if containment
 * is breached, or an appropriate error code.
 */
ui_error_t ui_layout_sanity_check(const struct ui_layout_node *node);

/**
 * @brief Recursively destroys a layout tree.
 *
 * @param node Pointer to the layout tree root to destroy.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_layout_tree_destroy(struct ui_layout_node *node);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_LAYOUT_H */
