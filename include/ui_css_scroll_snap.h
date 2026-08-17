/**
 * @file ui_css_scroll_snap.h
 * @brief CSS Scroll Snap properties and parsing.
 *
 * This header defines structures, enumerations, and functions for handling
 * CSS scroll snapping properties.
 */

#ifndef UI_CSS_SCROLL_SNAP_H
#define UI_CSS_SCROLL_SNAP_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_css_values.h"
/* clang-format on */

/**
 * @brief Scroll snap axis.
 */
enum ui_css_scroll_snap_axis {
  UI_CSS_SCROLL_SNAP_AXIS_NONE,   /**< No snap axis. */
  UI_CSS_SCROLL_SNAP_AXIS_X,      /**< Snap on the x-axis. */
  UI_CSS_SCROLL_SNAP_AXIS_Y,      /**< Snap on the y-axis. */
  UI_CSS_SCROLL_SNAP_AXIS_BLOCK,  /**< Snap on the block axis. */
  UI_CSS_SCROLL_SNAP_AXIS_INLINE, /**< Snap on the inline axis. */
  UI_CSS_SCROLL_SNAP_AXIS_BOTH    /**< Snap on both axes. */
};

/**
 * @brief Scroll snap strictness.
 */
enum ui_css_scroll_snap_strictness {
  UI_CSS_SCROLL_SNAP_STRICTNESS_NONE,      /**< No strictness. */
  UI_CSS_SCROLL_SNAP_STRICTNESS_MANDATORY, /**< Mandatory snapping. */
  UI_CSS_SCROLL_SNAP_STRICTNESS_PROXIMITY  /**< Proximity snapping. */
};

/**
 * @brief CSS scroll-snap-type property.
 */
struct ui_css_scroll_snap_type {
  enum ui_css_scroll_snap_axis axis; /**< The snap axis. */
  enum ui_css_scroll_snap_strictness
      strictness; /**< The strictness of snapping. */
};

/**
 * @brief Scroll snap align keyword.
 */
enum ui_css_scroll_snap_align_keyword {
  UI_CSS_SCROLL_SNAP_ALIGN_NONE,  /**< No alignment. */
  UI_CSS_SCROLL_SNAP_ALIGN_START, /**< Align to the start. */
  UI_CSS_SCROLL_SNAP_ALIGN_END,   /**< Align to the end. */
  UI_CSS_SCROLL_SNAP_ALIGN_CENTER /**< Align to the center. */
};

/**
 * @brief CSS scroll-snap-align property.
 */
struct ui_css_scroll_snap_align {
  enum ui_css_scroll_snap_align_keyword
      block; /**< Alignment on the block axis. */
  enum ui_css_scroll_snap_align_keyword
      inline_axis; /**< Alignment on the inline axis. */
};

/**
 * @brief CSS scroll-snap-stop property.
 */
enum ui_css_scroll_snap_stop {
  UI_CSS_SCROLL_SNAP_STOP_NORMAL, /**< Normal scroll stop behavior. */
  UI_CSS_SCROLL_SNAP_STOP_ALWAYS  /**< Always stop on this snap point. */
};

/**
 * @brief CSS scroll-padding property.
 */
struct ui_css_scroll_padding {
  struct ui_css_value top;    /**< Top scroll padding. */
  struct ui_css_value right;  /**< Right scroll padding. */
  struct ui_css_value bottom; /**< Bottom scroll padding. */
  struct ui_css_value left;   /**< Left scroll padding. */
};

/**
 * @brief CSS scroll-margin property.
 */
struct ui_css_scroll_margin {
  struct ui_css_value top;    /**< Top scroll margin. */
  struct ui_css_value right;  /**< Right scroll margin. */
  struct ui_css_value bottom; /**< Bottom scroll margin. */
  struct ui_css_value left;   /**< Left scroll margin. */
};

/**
 * @brief Aggregated scroll snap properties.
 */
struct ui_css_scroll_snap_properties {
  struct ui_css_scroll_snap_type type;   /**< The scroll snap type. */
  struct ui_css_scroll_snap_align align; /**< The scroll snap align. */
  enum ui_css_scroll_snap_stop stop;     /**< The scroll snap stop behavior. */
  struct ui_css_scroll_padding padding;  /**< The scroll padding. */
  struct ui_css_scroll_margin margin;    /**< The scroll margin. */
};

/**
 * @brief Forward declaration of the computed style structure.
 */
struct ui_css_computed_style;

/**
 * @brief Parses the CSS scroll snap properties from a computed style.
 *
 * @param style Pointer to the computed style.
 * @param out_props Pointer to the structure to populate with parsed properties.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code on failure.
 */
ui_error_t
ui_css_scroll_snap_parse(const struct ui_css_computed_style *style,
                         struct ui_css_scroll_snap_properties *out_props);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_SCROLL_SNAP_H */
