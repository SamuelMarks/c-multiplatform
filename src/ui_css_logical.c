/* clang-format off */
#include "ui_css_logical.h"
/* clang-format on */

enum ui_css_physical_edge
ui_css_logical_to_physical_edge(enum ui_css_logical_edge logical_edge,
                                enum ui_css_writing_mode writing_mode,
                                enum ui_css_direction direction) {
  if (writing_mode == UI_CSS_WRITING_MODE_HORIZONTAL_TB) {
    if (logical_edge == UI_CSS_LOGICAL_EDGE_BLOCK_START)
      return UI_CSS_PHYSICAL_EDGE_TOP;
    if (logical_edge == UI_CSS_LOGICAL_EDGE_BLOCK_END)
      return UI_CSS_PHYSICAL_EDGE_BOTTOM;
    if (logical_edge == UI_CSS_LOGICAL_EDGE_INLINE_START) {
      return (direction == UI_CSS_DIRECTION_LTR) ? UI_CSS_PHYSICAL_EDGE_LEFT
                                                 : UI_CSS_PHYSICAL_EDGE_RIGHT;
    }
    if (logical_edge == UI_CSS_LOGICAL_EDGE_INLINE_END) {
      return (direction == UI_CSS_DIRECTION_LTR) ? UI_CSS_PHYSICAL_EDGE_RIGHT
                                                 : UI_CSS_PHYSICAL_EDGE_LEFT;
    }
  } else if (writing_mode == UI_CSS_WRITING_MODE_VERTICAL_RL) {
    if (logical_edge == UI_CSS_LOGICAL_EDGE_BLOCK_START)
      return UI_CSS_PHYSICAL_EDGE_RIGHT;
    if (logical_edge == UI_CSS_LOGICAL_EDGE_BLOCK_END)
      return UI_CSS_PHYSICAL_EDGE_LEFT;
    if (logical_edge == UI_CSS_LOGICAL_EDGE_INLINE_START) {
      return (direction == UI_CSS_DIRECTION_LTR) ? UI_CSS_PHYSICAL_EDGE_TOP
                                                 : UI_CSS_PHYSICAL_EDGE_BOTTOM;
    }
    if (logical_edge == UI_CSS_LOGICAL_EDGE_INLINE_END) {
      return (direction == UI_CSS_DIRECTION_LTR) ? UI_CSS_PHYSICAL_EDGE_BOTTOM
                                                 : UI_CSS_PHYSICAL_EDGE_TOP;
    }
  } else if (writing_mode == UI_CSS_WRITING_MODE_VERTICAL_LR) {
    if (logical_edge == UI_CSS_LOGICAL_EDGE_BLOCK_START)
      return UI_CSS_PHYSICAL_EDGE_LEFT;
    if (logical_edge == UI_CSS_LOGICAL_EDGE_BLOCK_END)
      return UI_CSS_PHYSICAL_EDGE_RIGHT;
    if (logical_edge == UI_CSS_LOGICAL_EDGE_INLINE_START) {
      return (direction == UI_CSS_DIRECTION_LTR) ? UI_CSS_PHYSICAL_EDGE_TOP
                                                 : UI_CSS_PHYSICAL_EDGE_BOTTOM;
    }
    if (logical_edge == UI_CSS_LOGICAL_EDGE_INLINE_END) {
      return (direction == UI_CSS_DIRECTION_LTR) ? UI_CSS_PHYSICAL_EDGE_BOTTOM
                                                 : UI_CSS_PHYSICAL_EDGE_TOP;
    }
  }

  /* Fallback */
  return UI_CSS_PHYSICAL_EDGE_TOP;
}

/** \brief ui_css_logical_to_physical_size */
enum ui_error ui_css_logical_to_physical_size(
    int is_inline, enum ui_css_writing_mode writing_mode, int *out_is_width) {
  if (!out_is_width)
    return UI_ERROR_INVALID_ARGUMENT;
  if (writing_mode == UI_CSS_WRITING_MODE_HORIZONTAL_TB) {
    *out_is_width = is_inline; /* inline = width, block = height */
  } else {
    *out_is_width = !is_inline; /* inline = height, block = width */
  }
  return UI_ERROR_NONE;
}
