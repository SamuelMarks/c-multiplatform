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
  UI_BOX_EDGE_TOP = 0,
  UI_BOX_EDGE_RIGHT = 1,
  UI_BOX_EDGE_BOTTOM = 2,
  UI_BOX_EDGE_LEFT = 3
};

/**
 * @brief Represents the outside display type of a layout node (CSS Display
 * Level 4).
 */
enum ui_layout_display_outside {
  UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK,
  UI_LAYOUT_DISPLAY_OUTSIDE_INLINE,
  UI_LAYOUT_DISPLAY_OUTSIDE_RUN_IN,
  UI_LAYOUT_DISPLAY_OUTSIDE_NONE
};

/**
 * @brief Represents the inside display type of a layout node (CSS Display Level
 * 4).
 */
enum ui_layout_display_inside {
  UI_LAYOUT_DISPLAY_INSIDE_FLOW,
  UI_LAYOUT_DISPLAY_INSIDE_FLOW_ROOT,
  UI_LAYOUT_DISPLAY_INSIDE_TABLE,
  UI_LAYOUT_DISPLAY_INSIDE_TABLE_ROW_GROUP,
  UI_LAYOUT_DISPLAY_INSIDE_TABLE_HEADER_GROUP,
  UI_LAYOUT_DISPLAY_INSIDE_TABLE_FOOTER_GROUP,
  UI_LAYOUT_DISPLAY_INSIDE_TABLE_ROW,
  UI_LAYOUT_DISPLAY_INSIDE_TABLE_CELL,
  UI_LAYOUT_DISPLAY_INSIDE_FLEX,
  UI_LAYOUT_DISPLAY_INSIDE_GRID,
  UI_LAYOUT_DISPLAY_INSIDE_RUBY,
  UI_LAYOUT_DISPLAY_INSIDE_RUBY_BASE,
  UI_LAYOUT_DISPLAY_INSIDE_RUBY_TEXT
};

/**
 * @brief Represents the overflow behavior of a layout node.
 */

/**
 * @brief Represents the break behavior (CSS Fragmentation Module Level 3).
 */
enum ui_layout_break {
  UI_LAYOUT_BREAK_AUTO,
  UI_LAYOUT_BREAK_AVOID,
  UI_LAYOUT_BREAK_ALWAYS,
  UI_LAYOUT_BREAK_ALL,
  UI_LAYOUT_BREAK_AVOID_PAGE,
  UI_LAYOUT_BREAK_PAGE,
  UI_LAYOUT_BREAK_LEFT,
  UI_LAYOUT_BREAK_RIGHT,
  UI_LAYOUT_BREAK_RECTO,
  UI_LAYOUT_BREAK_VERSO,
  UI_LAYOUT_BREAK_AVOID_COLUMN,
  UI_LAYOUT_BREAK_COLUMN,
  UI_LAYOUT_BREAK_AVOID_REGION,
  UI_LAYOUT_BREAK_REGION
};

/**
 * @brief Represents the overflow behavior of a layout node.
 */
enum ui_layout_overflow {
  UI_LAYOUT_OVERFLOW_VISIBLE,
  UI_LAYOUT_OVERFLOW_HIDDEN,
  UI_LAYOUT_OVERFLOW_SCROLL,
  UI_LAYOUT_OVERFLOW_AUTO
};

/**
 * @brief Represents the box alignment values (CSS Box Alignment Module Level
 * 3).
 */
enum ui_layout_alignment {
  UI_LAYOUT_ALIGN_AUTO,
  UI_LAYOUT_ALIGN_NORMAL,
  UI_LAYOUT_ALIGN_START,
  UI_LAYOUT_ALIGN_END,
  UI_LAYOUT_ALIGN_CENTER,
  UI_LAYOUT_ALIGN_STRETCH,
  UI_LAYOUT_ALIGN_SPACE_BETWEEN,
  UI_LAYOUT_ALIGN_SPACE_AROUND,
  UI_LAYOUT_ALIGN_SPACE_EVENLY
};

/**
 * @brief Represents margin-trim behaviors (CSS Box Model Module Level 4).
 */
enum ui_layout_margin_trim {
  UI_LAYOUT_MARGIN_TRIM_NONE = 0,
  UI_LAYOUT_MARGIN_TRIM_BLOCK = 1,
  UI_LAYOUT_MARGIN_TRIM_BLOCK_START = 2,
  UI_LAYOUT_MARGIN_TRIM_BLOCK_END = 3,
  UI_LAYOUT_MARGIN_TRIM_INLINE = 4,
  UI_LAYOUT_MARGIN_TRIM_INLINE_START = 5,
  UI_LAYOUT_MARGIN_TRIM_INLINE_END = 6,
  UI_LAYOUT_MARGIN_TRIM_ALL = 7
};

/**
 * @brief Represents the wrap-flow behavior (CSS Exclusions Module Level 1).
 */
enum ui_layout_wrap_flow {
  UI_LAYOUT_WRAP_FLOW_AUTO,
  UI_LAYOUT_WRAP_FLOW_BOTH,
  UI_LAYOUT_WRAP_FLOW_START,
  UI_LAYOUT_WRAP_FLOW_END,
  UI_LAYOUT_WRAP_FLOW_MINIMUM,
  UI_LAYOUT_WRAP_FLOW_MAXIMUM,
  UI_LAYOUT_WRAP_FLOW_CLEAR
};

/**
 * @brief Represents the wrap-through behavior (CSS Exclusions Module Level 1).
 */
enum ui_layout_wrap_through {
  UI_LAYOUT_WRAP_THROUGH_WRAP,
  UI_LAYOUT_WRAP_THROUGH_NONE
};

/**
 * @brief Represents white-space behavior (CSS Text Module Level 3).
 */
enum ui_layout_white_space {
  UI_LAYOUT_WHITE_SPACE_NORMAL,
  UI_LAYOUT_WHITE_SPACE_NOWRAP,
  UI_LAYOUT_WHITE_SPACE_PRE,
  UI_LAYOUT_WHITE_SPACE_PRE_WRAP,
  UI_LAYOUT_WHITE_SPACE_PRE_LINE,
  UI_LAYOUT_WHITE_SPACE_BREAK_SPACES
};

/**
 * @brief Represents text-align behavior (CSS Text Module Level 3).
 */
enum ui_layout_text_align {
  UI_LAYOUT_TEXT_ALIGN_START,
  UI_LAYOUT_TEXT_ALIGN_END,
  UI_LAYOUT_TEXT_ALIGN_LEFT,
  UI_LAYOUT_TEXT_ALIGN_RIGHT,
  UI_LAYOUT_TEXT_ALIGN_CENTER,
  UI_LAYOUT_TEXT_ALIGN_JUSTIFY,
  UI_LAYOUT_TEXT_ALIGN_MATCH_PARENT,
  UI_LAYOUT_TEXT_ALIGN_JUSTIFY_ALL
};

/**
 * @brief Represents word-break behavior (CSS Text Module Level 3).
 */
enum ui_layout_word_break {
  UI_LAYOUT_WORD_BREAK_NORMAL,
  UI_LAYOUT_WORD_BREAK_KEEP_ALL,
  UI_LAYOUT_WORD_BREAK_BREAK_ALL,
  UI_LAYOUT_WORD_BREAK_BREAK_WORD
};

/**
 * @brief Represents hyphens behavior (CSS Text Module Level 3).
 */
enum ui_layout_hyphens {
  UI_LAYOUT_HYPHENS_NONE,
  UI_LAYOUT_HYPHENS_MANUAL,
  UI_LAYOUT_HYPHENS_AUTO
};

/**
 * @brief Represents writing mode (CSS Writing Modes Level 3/4).
 */
enum ui_layout_writing_mode {
  UI_LAYOUT_WRITING_MODE_HORIZONTAL_TB,
  UI_LAYOUT_WRITING_MODE_VERTICAL_RL,
  UI_LAYOUT_WRITING_MODE_VERTICAL_LR
};

/**
 * @brief Represents direction (CSS Writing Modes Level 3/4).
 */
enum ui_layout_direction { UI_LAYOUT_DIRECTION_LTR, UI_LAYOUT_DIRECTION_RTL };

/**
 * @brief Represents unicode-bidi behavior.
 */
enum ui_layout_unicode_bidi {
  UI_LAYOUT_UNICODE_BIDI_NORMAL,
  UI_LAYOUT_UNICODE_BIDI_EMBED,
  UI_LAYOUT_UNICODE_BIDI_BIDI_OVERRIDE,
  UI_LAYOUT_UNICODE_BIDI_ISOLATE,
  UI_LAYOUT_UNICODE_BIDI_ISOLATE_OVERRIDE,
  UI_LAYOUT_UNICODE_BIDI_PLAINTEXT
};

/**
 * @brief Represents text orientation.
 */
enum ui_layout_text_orientation {
  UI_LAYOUT_TEXT_ORIENTATION_MIXED,
  UI_LAYOUT_TEXT_ORIENTATION_UPRIGHT,
  UI_LAYOUT_TEXT_ORIENTATION_SIDEWAYS
};

/**
 * @brief Represents line grid creation (CSS Line Grid).
 */
enum ui_layout_line_grid {
  UI_LAYOUT_LINE_GRID_MATCH_PARENT,
  UI_LAYOUT_LINE_GRID_CREATE
};

/**
 * @brief Represents line snap (CSS Line Grid).
 */
enum ui_layout_line_snap {
  UI_LAYOUT_LINE_SNAP_NONE,
  UI_LAYOUT_LINE_SNAP_BASELINE,
  UI_LAYOUT_LINE_SNAP_CONTAIN
};

/**
 * @brief Represents box snap (CSS Line Grid).
 */
enum ui_layout_box_snap {
  UI_LAYOUT_BOX_SNAP_NONE,
  UI_LAYOUT_BOX_SNAP_BLOCK_START,
  UI_LAYOUT_BOX_SNAP_BLOCK_END,
  UI_LAYOUT_BOX_SNAP_CENTER,
  UI_LAYOUT_BOX_SNAP_BASELINE
};

/**
 * @brief Represents block step insert (CSS Rhythmic Sizing).
 */
enum ui_layout_block_step_insert {
  UI_LAYOUT_BLOCK_STEP_INSERT_MARGIN,
  UI_LAYOUT_BLOCK_STEP_INSERT_PADDING
};

/**
 * @brief Represents block step align (CSS Rhythmic Sizing).
 */
enum ui_layout_block_step_align {
  UI_LAYOUT_BLOCK_STEP_ALIGN_AUTO,
  UI_LAYOUT_BLOCK_STEP_ALIGN_CENTER,
  UI_LAYOUT_BLOCK_STEP_ALIGN_START,
  UI_LAYOUT_BLOCK_STEP_ALIGN_END
};

/**
 * @brief Represents block step round (CSS Rhythmic Sizing).
 */
enum ui_layout_block_step_round {
  UI_LAYOUT_BLOCK_STEP_ROUND_UP,
  UI_LAYOUT_BLOCK_STEP_ROUND_DOWN,
  UI_LAYOUT_BLOCK_STEP_ROUND_NEAREST
};

/**
 * @brief Represents text-decoration-line behavior (CSS Text Decoration Module
 * Level 3).
 */
enum ui_layout_text_decoration_line {
  UI_LAYOUT_TEXT_DECORATION_LINE_NONE = 0,
  UI_LAYOUT_TEXT_DECORATION_LINE_UNDERLINE = 1,
  UI_LAYOUT_TEXT_DECORATION_LINE_OVERLINE = 2,
  UI_LAYOUT_TEXT_DECORATION_LINE_LINE_THROUGH = 4,
  UI_LAYOUT_TEXT_DECORATION_LINE_BLINK = 8
};

/**
 * @brief Represents text-decoration-style behavior (CSS Text Decoration Module
 * Level 3).
 */
enum ui_layout_text_decoration_style {
  UI_LAYOUT_TEXT_DECORATION_STYLE_SOLID,
  UI_LAYOUT_TEXT_DECORATION_STYLE_DOUBLE,
  UI_LAYOUT_TEXT_DECORATION_STYLE_DOTTED,
  UI_LAYOUT_TEXT_DECORATION_STYLE_DASHED,
  UI_LAYOUT_TEXT_DECORATION_STYLE_WAVY
};

/**
 * @brief Represents font-style behavior (CSS Fonts Module Level 3/4).
 */
enum ui_layout_font_style {
  UI_LAYOUT_FONT_STYLE_NORMAL,
  UI_LAYOUT_FONT_STYLE_ITALIC,
  UI_LAYOUT_FONT_STYLE_OBLIQUE
};

/**
 * @brief Represents text-size-adjust types.
 */
enum ui_layout_text_size_adjust_type {
  UI_LAYOUT_TEXT_SIZE_ADJUST_AUTO,
  UI_LAYOUT_TEXT_SIZE_ADJUST_NONE,
  UI_LAYOUT_TEXT_SIZE_ADJUST_PERCENTAGE
};

/**
 * @brief Structure for text-size-adjust.
 */
struct ui_layout_text_size_adjust {
  enum ui_layout_text_size_adjust_type type;
  float percentage;
};

/**
 * @brief Represents background-size type (CSS Backgrounds and Borders Module).
 */
enum ui_layout_background_size_type {
  UI_LAYOUT_BACKGROUND_SIZE_AUTO,
  UI_LAYOUT_BACKGROUND_SIZE_COVER,
  UI_LAYOUT_BACKGROUND_SIZE_CONTAIN,
  UI_LAYOUT_BACKGROUND_SIZE_LENGTH
};

/**
 * @brief Structure for background-size.
 */
struct ui_layout_background_size {
  enum ui_layout_background_size_type type_x;
  enum ui_layout_background_size_type type_y;
  float x;
  float y;
};

/**
 * @brief Represents background-repeat type.
 */
enum ui_layout_background_repeat {
  UI_LAYOUT_BACKGROUND_REPEAT_REPEAT,
  UI_LAYOUT_BACKGROUND_REPEAT_SPACE,
  UI_LAYOUT_BACKGROUND_REPEAT_ROUND,
  UI_LAYOUT_BACKGROUND_REPEAT_NO_REPEAT
};

/**
 * @brief Represents color-scheme (CSS Color Adjustment Module Level 1).
 */
enum ui_layout_color_scheme {
  UI_LAYOUT_COLOR_SCHEME_NORMAL = 0,
  UI_LAYOUT_COLOR_SCHEME_LIGHT = 1,
  UI_LAYOUT_COLOR_SCHEME_DARK = 2,
  UI_LAYOUT_COLOR_SCHEME_LIGHT_DARK = 3
};

/**
 * @brief Represents box-decoration-break (CSS Borders and Box Decorations
 * Module Level 4).
 */
enum ui_layout_box_decoration_break {
  UI_LAYOUT_BOX_DECORATION_BREAK_SLICE,
  UI_LAYOUT_BOX_DECORATION_BREAK_CLONE
};

/**
 * @brief Represents a single box-shadow (CSS Borders and Box Decorations Module
 * Level 4).
 */
struct ui_layout_box_shadow {
  float offset_x;
  float offset_y;
  float blur_radius;
  float spread_radius;
  struct ui_css_color color;
  int is_inset;
};

/**
 * @brief Represents print-color-adjust (CSS Color Adjustment Module Level 1).
 */
enum ui_layout_print_color_adjust {
  UI_LAYOUT_PRINT_COLOR_ADJUST_ECONOMY,
  UI_LAYOUT_PRINT_COLOR_ADJUST_EXACT
};

/**
 * @brief Represents forced-color-adjust (CSS Color Adjustment Module Level 1).
 */
enum ui_layout_forced_color_adjust {
  UI_LAYOUT_FORCED_COLOR_ADJUST_AUTO,
  UI_LAYOUT_FORCED_COLOR_ADJUST_NONE,
  UI_LAYOUT_FORCED_COLOR_ADJUST_PRESERVE_PARENT_COLOR
};

/**
 * @brief Represents font-stretch behavior (CSS Fonts Module Level 3/4).
 */
enum ui_layout_font_stretch {
  UI_LAYOUT_FONT_STRETCH_NORMAL,
  UI_LAYOUT_FONT_STRETCH_ULTRA_CONDENSED,
  UI_LAYOUT_FONT_STRETCH_EXTRA_CONDENSED,
  UI_LAYOUT_FONT_STRETCH_CONDENSED,
  UI_LAYOUT_FONT_STRETCH_SEMI_CONDENSED,
  UI_LAYOUT_FONT_STRETCH_SEMI_EXPANDED,
  UI_LAYOUT_FONT_STRETCH_EXPANDED,
  UI_LAYOUT_FONT_STRETCH_EXTRA_EXPANDED,
  UI_LAYOUT_FONT_STRETCH_ULTRA_EXPANDED
};

/**
 * @brief Represents the flex direction (CSS Flexible Box Layout Module Level
 * 1).
 */
enum ui_layout_flex_direction {
  UI_LAYOUT_FLEX_DIRECTION_ROW,
  UI_LAYOUT_FLEX_DIRECTION_ROW_REVERSE,
  UI_LAYOUT_FLEX_DIRECTION_COLUMN,
  UI_LAYOUT_FLEX_DIRECTION_COLUMN_REVERSE
};

/**
 * @brief Represents the flex wrap behavior (CSS Flexible Box Layout Module
 * Level 1).
 */
enum ui_layout_flex_wrap {
  UI_LAYOUT_FLEX_WRAP_NOWRAP,
  UI_LAYOUT_FLEX_WRAP_WRAP,
  UI_LAYOUT_FLEX_WRAP_WRAP_REVERSE
};

/**
 * @brief Represents the size determination type for a dimension.
 */
enum ui_layout_size_type {
  UI_LAYOUT_SIZE_PIXELS,
  UI_LAYOUT_SIZE_PERCENTAGE,
  UI_LAYOUT_SIZE_AUTO,
  UI_LAYOUT_SIZE_MIN_CONTENT,
  UI_LAYOUT_SIZE_MAX_CONTENT,
  UI_LAYOUT_SIZE_FIT_CONTENT,
  UI_LAYOUT_SIZE_FR,
  UI_LAYOUT_SIZE_SUBGRID,
  UI_LAYOUT_SIZE_MASONRY,
  UI_LAYOUT_SIZE_ANCHOR
};

#define UI_LAYOUT_MAX_GRID_TRACKS 16

/**
 * @brief Represents a parsed CSS grid track.
 */
struct ui_layout_grid_track {
  enum ui_layout_size_type type;
  float value;
};

/**
 * @brief Represents the position type (CSS Positioned Layout Module Level 3).
 */
enum ui_layout_position {
  UI_LAYOUT_POSITION_STATIC,
  UI_LAYOUT_POSITION_RELATIVE,
  UI_LAYOUT_POSITION_ABSOLUTE,
  UI_LAYOUT_POSITION_FIXED,
  UI_LAYOUT_POSITION_STICKY
};

/**
 * @brief Represents the side of an anchor to tether to.
 */
enum ui_layout_anchor_side {
  UI_LAYOUT_ANCHOR_SIDE_NONE,
  UI_LAYOUT_ANCHOR_SIDE_TOP,
  UI_LAYOUT_ANCHOR_SIDE_RIGHT,
  UI_LAYOUT_ANCHOR_SIDE_BOTTOM,
  UI_LAYOUT_ANCHOR_SIDE_LEFT,
  UI_LAYOUT_ANCHOR_SIDE_CENTER
};

/**
 * @brief Represents a reference to an anchor side.
 */
struct ui_layout_anchor_ref {
  char name[64];
  enum ui_layout_anchor_side side;
};

/**
 * @brief Represents a box in the layout tree.
 */
struct ui_layout_node {
  float opacity;
  char transform[64];
  int is_stacking_context;
  const struct ui_dom_node
      *dom_node; /**< Original DOM node. NULL for anonymous boxes. */
  struct ui_css_computed_style
      *computed_style; /**< Resolved style for this node. */

  int is_anonymous; /**< 1 if this is an anonymous block/inline box, 0 otherwise
                     */
  enum ui_layout_display_outside display_outside;
  enum ui_layout_display_inside display_inside;

  enum ui_layout_overflow overflow_x;
  enum ui_layout_overflow overflow_y;

  enum ui_layout_break break_before;
  enum ui_layout_break break_after;
  enum ui_layout_break break_inside;
  int orphans;
  int widows;

  enum ui_layout_alignment justify_content;
  enum ui_layout_alignment align_items;
  enum ui_layout_alignment align_self;
  enum ui_layout_alignment align_content;

  enum ui_layout_flex_direction flex_direction;
  enum ui_layout_flex_wrap flex_wrap;
  float flex_grow;
  float flex_shrink;
  float flex_basis;
  enum ui_layout_size_type flex_basis_type;

  struct ui_layout_grid_track grid_template_columns[UI_LAYOUT_MAX_GRID_TRACKS];
  int grid_template_columns_count;
  struct ui_layout_grid_track grid_template_rows[UI_LAYOUT_MAX_GRID_TRACKS];
  int grid_template_rows_count;
  int grid_column_start;
  int grid_column_span;
  int grid_row_start;
  int grid_row_span;

  float subgrid_track_widths[UI_LAYOUT_MAX_GRID_TRACKS];
  float subgrid_track_heights[UI_LAYOUT_MAX_GRID_TRACKS];
  int subgrid_columns_count;
  int subgrid_rows_count;

  enum ui_layout_size_type width_type;
  enum ui_layout_size_type height_type;
  float raw_width;
  float raw_height;

  enum ui_layout_margin_trim margin_trim;

  int column_count;   /* 0 means auto */
  float column_width; /* 0.0f means auto */

  float row_gap;
  float column_gap;

  enum ui_layout_position position;
  char anchor_name[64];
  char position_anchor[64];
  struct ui_layout_anchor_ref top_anchor;
  struct ui_layout_anchor_ref right_anchor;
  struct ui_layout_anchor_ref bottom_anchor;
  struct ui_layout_anchor_ref left_anchor;
  float top;
  float right;
  float bottom;
  float left;
  enum ui_layout_size_type top_type;
  enum ui_layout_size_type right_type;
  enum ui_layout_size_type bottom_type;
  enum ui_layout_size_type left_type;
  int z_index;

  enum ui_layout_wrap_flow wrap_flow;
  enum ui_layout_wrap_through wrap_through;

  char flow_into[64];
  char flow_from[64];

  enum ui_layout_white_space white_space;
  enum ui_layout_text_align text_align;
  enum ui_layout_word_break word_break;
  enum ui_layout_hyphens hyphens;

  enum ui_layout_writing_mode writing_mode;
  enum ui_layout_direction direction;
  enum ui_layout_unicode_bidi unicode_bidi;
  enum ui_layout_text_orientation text_orientation;

  enum ui_layout_line_grid line_grid;
  enum ui_layout_line_snap line_snap;
  enum ui_layout_box_snap box_snap;
  float block_step_size; /* 0.0f means none */
  enum ui_layout_block_step_insert block_step_insert;
  enum ui_layout_block_step_align block_step_align;
  enum ui_layout_block_step_round block_step_round;

  int text_decoration_line;
  enum ui_layout_text_decoration_style text_decoration_style;
  struct ui_css_color text_decoration_color;
  char text_shadow[128];
  char text_emphasis_style[64];
  struct ui_css_color text_emphasis_color;
  char text_emphasis_position[64];

  char font_family[128];
  int font_weight;
  enum ui_layout_font_style font_style;
  float font_size;
  enum ui_layout_font_stretch font_stretch;
  struct ui_layout_text_size_adjust text_size_adjust;
  char font_variant[64];
  char font_feature_settings[64];
  char font_variation_settings[64];

  struct ui_css_color text_color;
  struct ui_css_color background_color;
  struct ui_css_image background_image[4]; /* Support up to 4 multiple
                                              backgrounds for Level 3/4 */
  struct ui_layout_background_size background_size[4];
  enum ui_layout_background_repeat background_repeat_x[4];
  enum ui_layout_background_repeat background_repeat_y[4];
  int background_image_count;
  struct ui_css_color border_color[4];
  float border_radius[4][2]; /* top-left, top-right, bottom-right, bottom-left
                                (x, y) */

  enum ui_layout_box_decoration_break box_decoration_break;
  struct ui_layout_box_shadow box_shadow[4]; /* Support up to 4 shadows */
  int box_shadow_count;
  struct ui_css_image border_image_source;

  enum ui_layout_color_scheme color_scheme;
  enum ui_layout_print_color_adjust print_color_adjust;
  enum ui_layout_forced_color_adjust forced_color_adjust;

  /* Geometry (to be computed by layout algorithms later) */
  float x;
  float y;
  float width;
  float height;
  float abs_x;
  float abs_y;

  /* CSS Box Model Metrics (Level 3 & 4) */
  float margin[4];
  float border[4];
  float padding[4];
  float content_width;
  float content_height;

  float min_width;
  float max_width;
  float min_height;
  float max_height;

  float aspect_ratio; /* 0.0 means auto (no aspect ratio) */

  float scrollbar_width;
  float scrollbar_height;

  /* Tree structure */
  struct ui_layout_node *parent;
  struct ui_layout_node *first_child;
  struct ui_layout_node *last_child;
  struct ui_layout_node *next_sibling;
  struct ui_layout_node *previous_sibling;
};

/**
 * @brief Generates a layout tree from a DOM tree and a stylesheet.
 *
 * This function resolves styles, skips nodes with "display: none",
 * and allocates a separate tree of layout nodes.
 *
 * @param dom_root The root of the DOM tree.
 * @param stylesheet The stylesheet to apply.
 * @param out_layout_root Pointer to receive the generated layout tree.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_layout_tree_generate(const struct ui_dom_node *dom_root,
                                   const struct ui_css_stylesheet *stylesheet,
                                   struct ui_layout_node **out_layout_root);

/**
 * @brief Viewport Root Solver: Forces the root html/body elements to strictly
 * match OS window dimensions.
 *
 * @param root The layout tree root.
 * @param window_width The OS window width.
 * @param window_height The OS window height.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_layout_solve_viewport(struct ui_layout_node *root,
                                    float window_width, float window_height);

/**
 * @brief Computes the final geometric layout (x, y, width, height) recursively.
 *
 * @param node The layout tree root.
 * @param available_width The viewport or containing block width.
 * @param available_height The viewport or containing block height.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_layout_compute(struct ui_layout_node *node, float available_width,
                             float available_height);

/**
 * @brief Containment Sanity Checks: Traverses the tree and asserts that
 * children do not illegally breach parent bounds.
 *
 * @param node The layout tree root to verify.
 * @return UI_ERROR_NONE if valid, UI_ERROR_LAYOUT_VIOLATION if containment is
 * breached.
 */
ui_error_t ui_layout_sanity_check(const struct ui_layout_node *node);

/**
 * @brief Recursively destroys a layout tree.
 *
 * @param node The layout tree root to destroy.
 */
ui_error_t ui_layout_tree_destroy(struct ui_layout_node *node);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_LAYOUT_H */
