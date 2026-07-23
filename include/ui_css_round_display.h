#ifndef UI_CSS_ROUND_DISPLAY_H
#define UI_CSS_ROUND_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_css_values.h"
/* clang-format on */

/**
 * @brief Represents the shape-inside property.
 */
struct ui_css_shape_inside {
  enum ui_css_geometry_box box;
  struct ui_css_basic_shape shape;
  int is_display; /* 1 if 'display' keyword was used (meaning same shape as
                     display bounds) */
  int is_outside_shape; /* 1 if 'outside-shape' was used */
};

/**
 * @brief Represents the border-boundary property.
 */
enum ui_css_border_boundary {
  UI_CSS_BORDER_BOUNDARY_NONE,
  UI_CSS_BORDER_BOUNDARY_PARENT,
  UI_CSS_BORDER_BOUNDARY_DISPLAY
};

/**
 * @brief Parses CSS shape-inside.
 *
 * @param str The string to parse.
 * @param out_shape Pointer to receive the parsed shape.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_parse_shape_inside(const char *str,
                                        struct ui_css_shape_inside *out_shape);

/**
 * @brief Parses CSS border-boundary.
 *
 * @param str The string to parse.
 * @param out_boundary Pointer to receive the parsed boundary value.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_css_parse_border_boundary(const char *str,
                             enum ui_css_border_boundary *out_boundary);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_ROUND_DISPLAY_H */
