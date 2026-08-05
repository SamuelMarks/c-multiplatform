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
  UI_CSS_SCROLL_SNAP_AXIS_NONE,
  UI_CSS_SCROLL_SNAP_AXIS_X,
  UI_CSS_SCROLL_SNAP_AXIS_Y,
  UI_CSS_SCROLL_SNAP_AXIS_BLOCK,
  UI_CSS_SCROLL_SNAP_AXIS_INLINE,
  UI_CSS_SCROLL_SNAP_AXIS_BOTH
};

/**
 * @brief Scroll snap strictness.
 */
enum ui_css_scroll_snap_strictness {
  UI_CSS_SCROLL_SNAP_STRICTNESS_NONE,
  UI_CSS_SCROLL_SNAP_STRICTNESS_MANDATORY,
  UI_CSS_SCROLL_SNAP_STRICTNESS_PROXIMITY
};

/**
 * @brief CSS scroll-snap-type property.
 */
struct ui_css_scroll_snap_type {
  enum ui_css_scroll_snap_axis axis;
  enum ui_css_scroll_snap_strictness strictness;
};

/**
 * @brief Scroll snap align keyword.
 */
enum ui_css_scroll_snap_align_keyword {
  UI_CSS_SCROLL_SNAP_ALIGN_NONE,
  UI_CSS_SCROLL_SNAP_ALIGN_START,
  UI_CSS_SCROLL_SNAP_ALIGN_END,
  UI_CSS_SCROLL_SNAP_ALIGN_CENTER
};

/**
 * @brief CSS scroll-snap-align property.
 */
struct ui_css_scroll_snap_align {
  enum ui_css_scroll_snap_align_keyword block;
  enum ui_css_scroll_snap_align_keyword inline_axis;
};

/**
 * @brief CSS scroll-snap-stop property.
 */
enum ui_css_scroll_snap_stop {
  UI_CSS_SCROLL_SNAP_STOP_NORMAL,
  UI_CSS_SCROLL_SNAP_STOP_ALWAYS
};

/**
 * @brief CSS scroll-padding property.
 */
struct ui_css_scroll_padding {
  struct ui_css_value top;
  struct ui_css_value right;
  struct ui_css_value bottom;
  struct ui_css_value left;
};

/**
 * @brief CSS scroll-margin property.
 */
struct ui_css_scroll_margin {
  struct ui_css_value top;
  struct ui_css_value right;
  struct ui_css_value bottom;
  struct ui_css_value left;
};

/**
 * @brief Aggregated scroll snap properties.
 */
struct ui_css_scroll_snap_properties {
  struct ui_css_scroll_snap_type type;
  struct ui_css_scroll_snap_align align;
  enum ui_css_scroll_snap_stop stop;
  struct ui_css_scroll_padding padding;
  struct ui_css_scroll_margin margin;
};

struct ui_css_computed_style; /* Forward declare */

/**
 * @brief Parses the CSS scroll snap properties from a computed style.
 *
 * @param style The computed style.
 * @param out_props Pointer to the struct to populate.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_css_scroll_snap_parse(const struct ui_css_computed_style *style,
                         struct ui_css_scroll_snap_properties *out_props);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_SCROLL_SNAP_H */
