/**
 * @file ui_css_logical.h
 * @brief CSS Logical Properties resolution based on writing mode and direction.
 */

#ifndef UI_CSS_LOGICAL_H
#define UI_CSS_LOGICAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Represents the writing mode.
 */
enum ui_css_writing_mode {
  UI_CSS_WRITING_MODE_HORIZONTAL_TB, /**< Horizontal, top-to-bottom */
  UI_CSS_WRITING_MODE_VERTICAL_RL,   /**< Vertical, right-to-left */
  UI_CSS_WRITING_MODE_VERTICAL_LR    /**< Vertical, left-to-right */
};

/**
 * @brief Represents text direction.
 */
enum ui_css_direction {
  UI_CSS_DIRECTION_LTR, /**< Left-to-right */
  UI_CSS_DIRECTION_RTL  /**< Right-to-left */
};

/**
 * @brief Logical edges.
 */
enum ui_css_logical_edge {
  UI_CSS_LOGICAL_EDGE_BLOCK_START,  /**< Block start */
  UI_CSS_LOGICAL_EDGE_BLOCK_END,    /**< Block end */
  UI_CSS_LOGICAL_EDGE_INLINE_START, /**< Inline start */
  UI_CSS_LOGICAL_EDGE_INLINE_END    /**< Inline end */
};

/**
 * @brief Physical edges.
 */
enum ui_css_physical_edge {
  UI_CSS_PHYSICAL_EDGE_TOP = 0,    /**< Matches UI_BOX_EDGE_TOP */
  UI_CSS_PHYSICAL_EDGE_RIGHT = 1,  /**< Matches UI_BOX_EDGE_RIGHT */
  UI_CSS_PHYSICAL_EDGE_BOTTOM = 2, /**< Matches UI_BOX_EDGE_BOTTOM */
  UI_CSS_PHYSICAL_EDGE_LEFT = 3    /**< Matches UI_BOX_EDGE_LEFT */
};

/**
 * @brief Maps a logical edge to a physical edge based on writing-mode and
 * direction.
 *
 * @param logical_edge The logical edge to map.
 * @param writing_mode The current writing mode.
 * @param direction The current text direction.
 * @return The corresponding physical edge.
 */
enum ui_css_physical_edge
ui_css_logical_to_physical_edge(enum ui_css_logical_edge logical_edge,
                                enum ui_css_writing_mode writing_mode,
                                enum ui_css_direction direction);

/**
 * @brief Maps logical size (inline-size, block-size) to physical size (width,
 * height)
 *
 * @param is_inline 1 if looking for inline-size, 0 for block-size.
 * @param writing_mode The current writing mode.
 * @param out_is_width Pointer to receive 1 if the mapped size is physical
 * width, 0 if physical height.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_logical_to_physical_size(
    int is_inline, enum ui_css_writing_mode writing_mode, int *out_is_width);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_LOGICAL_H */
